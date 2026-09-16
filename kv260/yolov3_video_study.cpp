// YOLOv3 を3段パイプラインで回す（Read 1本 → Infer NINFER 本 → Show 1本）
// 計測は演習4-3 の方法：段ごとに 取り出し待ち / 実計算時間 / 入れ待ち を
//   スレッドごとの記録場所に足し込み、表示は全部終わってから1回だけ
//
//   PROFILE 1 : 計測あり（表を出す）
//   PROFILE 0 : 計測なし（FPS の1行だけ。計測が結果を変えていないかの確認用）
//   コンパイル時に -DPROFILE=0 / -DPROFILE=1 でも切り替えられる
#ifndef PROFILE
#define PROFILE 1
#endif

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <memory>
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>

#include <xir/graph/graph.hpp>
#include "vitis/ai/collection_helper.hpp"
#include "common.h"
#include "utils.h"                             // ConcurrentQueue は common.h に入っている

using namespace std;
using namespace cv;
using namespace std::chrono;
using TP = steady_clock::time_point;

bool Lbox_on = false;                          // true: letterbox / false: resize
GraphInfo shapes;

const int    NINFER     = 2;                   // Infer の人数
const size_t CAP        = 16;                 // キューの容量（2本とも）
const int    WARMUP     = 3;                   // 各スレッドが最初に捨てる件数
const int    SHOW_EVERY = 2;                   // 表示は SHOW_EVERY 枚に1枚（元コードの index % 2 と同じ）

typedef pair<int, Mat> imagePair;              // .first = フレーム番号。-1 は「もう来ない」の合図
std::atomic<bool> stop_req{false};             // 画面で ESC / q → 読み込みをやめる

// =====================================================================
//  段ごとの記録。各スレッドが自分専用に1つ持つので、鍵は要らない（作法②）
//  1件ぶんの値は add() でまとめて足す。途中で抜けた件は足されない
// =====================================================================
struct Stat {
    long long pop_us = 0, pre_us = 0, dpu_us = 0, post_us = 0, push_us = 0;
    long long work_us = 0, work_max_us = 0;    // 実計算時間（= pre + dpu + post）とその最大
    int seen = 0, n = 0;                       // 通った件数 / 数えた件数（WARMUP を除く）
    int shown = 0;                             // 表示した件数（Show だけ使う）
    TP t0, tend;                               // 数えた区間の最初と最後

    bool add(long long pop, long long pre, long long dpu, long long post, long long push) {   // 数えたら true
        TP now = steady_clock::now();
        if (++seen <= WARMUP) { if (seen == WARMUP) t0 = now; return false; }   // 立ち上がりは捨てる（作法④）
        n++; tend = now;                                                    // 件数と時間は PROFILE 0 でも数える（FPS 用）
#if PROFILE
        long long work = pre + dpu + post;
        pop_us += pop; pre_us += pre; dpu_us += dpu; post_us += post; push_us += push;
        work_us += work;
        if (work > work_max_us) work_max_us = work;
#else
        (void)pop; (void)pre; (void)dpu; (void)post; (void)push;
#endif
        return true;
    }
    double sec() const { return n ? duration_cast<microseconds>(tend - t0).count() / 1e6 : 0; }
};

#if PROFILE
static inline long long lap(TP& t) {           // t から今までの us を返し、t を進める
    TP n = steady_clock::now();
    long long us = duration_cast<microseconds>(n - t).count();
    t = n;
    return us;
}
#else
static inline long long lap(TP&) { return 0; } // 計測なし：時計も読まない
#endif

// =====================================================================
//  Read：動画を1枚ずつ読んで q1 に入れる
// =====================================================================
void readFrame(const char* fileName, ConcurrentQueue<imagePair>& out, Stat& s) {
    VideoCapture video;
    if (!video.open(fileName)) {
        cout << "Fail to open specified video file:" << fileName << endl;
        stop_req = true;
    }
    int idx = 0;
    while (!stop_req) {
        TP t = steady_clock::now();
        Mat img;
        if (!video.read(img)) break;           // 動画の終わり
        long long w = lap(t);
        out.push(make_pair(idx++, img));
        long long p = lap(t);                  // 満杯なら、ここで待たされる（入れ待ち）
        s.add(0, w, 0, 0, p);
    }
    video.release();
    for (int k = 0; k < NINFER; k++) out.push(make_pair(-1, Mat()));   // Infer の人数ぶん「もう来ない」を流す
}

// =====================================================================
//  Infer：q1 から取り、pre → dpu → post をして q2 に入れる（NINFER 本が同じ2本のキューを共有）
// =====================================================================
void post_process(Mat& img, const vector<int8_t*>& out, const GraphInfo& shapes,
                  const float& scale, const int& sHeight, const int& sWidth) {
    vector<vector<float>> boxes;
    for (size_t i = 0; i < out.size(); i++) {
        int channel = shapes.outTensorList[i].channel;
        int width   = shapes.outTensorList[i].width;
        int height  = shapes.outTensorList[i].height;
        int sizeOut = shapes.outTensorList[i].size;
        boxes.reserve(sizeOut);
        detect(boxes, out[i], channel, height, width, i, sHeight, sWidth, scale);
    }
    if (Lbox_on) {
        correct_region_boxes(boxes, boxes.size(), img.cols, img.rows, sWidth, sHeight);
    }
    vector<vector<float>> res = applyNMS(boxes, classificationCnt, NMS_THRESHOLD);

    float h = img.rows;
    float w = img.cols;
    for (size_t i = 0; i < res.size(); ++i) {
        float xmin = (res[i][0] - res[i][2] / 2.0) * w + 1.0;
        float ymin = (res[i][1] - res[i][3] / 2.0) * h + 1.0;
        float xmax = (res[i][0] + res[i][2] / 2.0) * w + 1.0;
        float ymax = (res[i][1] + res[i][3] / 2.0) * h + 1.0;
        if (res[i][res[i][4] + 6] > CONF) {
            int type = res[i][4];
            Scalar color = (type == 0) ? Scalar(0, 0, 255)
                         : (type == 1) ? Scalar(255, 0, 0)
                                       : Scalar(0, 255, 255);
            rectangle(img, Point(xmin, ymin), Point(xmax, ymax), color, 1, 1, 0);
        }
    }
}

void setInputImageForYOLO(const Mat& frame, int8_t* data, float input_scale) {
    int width  = shapes.inTensorList[0].width;
    int height = shapes.inTensorList[0].height;
    int size   = shapes.inTensorList[0].size;
    image img_new  = load_image_cv(frame);
    image img_yolo = letterbox_image(img_new, width, height);

    vector<float> bb(size);
    for (int b = 0; b < height; ++b)
        for (int c = 0; c < width; ++c)
            for (int a = 0; a < 3; ++a)
                bb[b * width * 3 + c * 3 + a] = img_yolo.data[a * height * width + b * width + c];

    float scale = pow(2, 7);
    for (int i = 0; i < size; ++i) {
        data[i] = (int8_t)(bb.data()[i] * input_scale);
        if (data[i] < 0) data[i] = (int8_t)((float)(127 / scale) * input_scale);
    }
    free_image(img_new);
    free_image(img_yolo);
}

void setInputPointer(const Mat& frame, int8_t* data, const int& scale) {
    int width  = shapes.inTensorList[0].width;
    int height = shapes.inTensorList[0].height;
    int size   = shapes.inTensorList[0].size;

    Mat img = frame.clone();
    cvtColor(img, img, cv::COLOR_BGR2RGB);
    Mat image2 = cv::Mat(height, width, CV_8SC3);
    cv::resize(img, image2, Size(width, height), 0, 0, cv::INTER_LINEAR);

    unsigned char* imdata = image2.data;
    for (int i = 0; i < size; ++i) {
        float dataf = static_cast<float>(imdata[i]);
        data[i] = static_cast<int>(dataf * static_cast<float>(scale) / 256.0);
        if (data[i] < 0) data[i] = 127;
    }
}

void runYOLO(vart::Runner* runner, ConcurrentQueue<imagePair>& in, ConcurrentQueue<imagePair>& out, Stat& s) {
    auto inputTensors  = cloneTensorBuffer(runner->get_input_tensors());
    auto outputTensors = cloneTensorBuffer(runner->get_output_tensors());

    int inHeight  = shapes.inTensorList[0].height;
    int inWidth   = shapes.inTensorList[0].width;
    int inChannel = 3;
    int batchSize = 1;
    int inSize = inHeight * inWidth * inChannel;
    int8_t* imageInputs = new int8_t[inSize * batchSize];         // バッファは1回だけ確保して使い回す

    vector<int> output_mapping = shapes.output_mapping;
    auto conf_output_scale = get_output_scale(runner->get_output_tensors()[output_mapping[1]]);
    const int size0 = shapes.outTensorList[0].size;
    const int size1 = shapes.outTensorList[1].size;
    const int size2 = shapes.outTensorList[2].size;
    int8_t* result0 = new int8_t[size0 * batchSize];
    int8_t* result1 = new int8_t[size1 * batchSize];
    int8_t* result2 = new int8_t[size2 * batchSize];
    auto input_scale = get_input_scale(runner->get_input_tensors()[0]);

    std::vector<std::unique_ptr<vart::TensorBuffer>> inputs, outputs;
    std::vector<vart::TensorBuffer*> inputsPtr, outputsPtr;

    while (true) {
        TP t = steady_clock::now();
        imagePair f = in.pop();
        long long pop = lap(t);                                    // 取り出し待ち
        if (f.first < 0) { out.push(f); break; }                   // 合図は下流へ渡して終わる

        // ---- pre ----
        if (Lbox_on) setInputImageForYOLO(f.second, imageInputs, input_scale);
        else         setInputPointer(f.second, imageInputs, input_scale);
        inputs.push_back(std::make_unique<CpuFlatTensorBuffer>(imageInputs, inputTensors[0].get()));
        outputs.push_back(std::make_unique<CpuFlatTensorBuffer>(result0, outputTensors[output_mapping[0]].get()));
        outputs.push_back(std::make_unique<CpuFlatTensorBuffer>(result1, outputTensors[output_mapping[1]].get()));
        outputs.push_back(std::make_unique<CpuFlatTensorBuffer>(result2, outputTensors[output_mapping[2]].get()));
        inputsPtr.push_back(inputs[0].get());
        outputsPtr.push_back(outputs[0].get());
        outputsPtr.push_back(outputs[1].get());
        outputsPtr.push_back(outputs[2].get());
        long long pre = lap(t);

        // ---- dpu ----
        auto job_id = runner->execute_async(inputsPtr, outputsPtr);
        runner->wait(job_id.first, -1);
        long long dpu = lap(t);

        // ---- post ----
        vector<int8_t*> results = {result0, result1, result2};
        post_process(f.second, results, shapes, conf_output_scale, inHeight, inWidth);
        inputs.clear(); outputs.clear(); inputsPtr.clear(); outputsPtr.clear();
        long long post = lap(t);

        out.push(f);
        long long push = lap(t);                                   // 入れ待ち
        s.add(pop, pre, dpu, post, push);
    }
    delete[] imageInputs;
    delete[] result0;
    delete[] result1;
    delete[] result2;
}

// =====================================================================
//  Show：q2 から取って表示する
// =====================================================================
void displayFrame(ConcurrentQueue<imagePair>& in, Stat& s, int& inversions) {
    int done = 0, last = -1;
    TP start = steady_clock::now();
    while (true) {
        TP t = steady_clock::now();
        imagePair f = in.pop();
        long long pop = lap(t);                                    // 取り出し待ち
        if (f.first < 0) { if (++done == NINFER) break; continue; } // Infer 全員の合図が来たら終わり
        if (stop_req) continue;                                    // ESC の後は流すだけ（数えない）
        if (f.first < last) inversions++;                          // 前より小さい番号 ＝ 追い越された
        last = f.first;

        bool shown = (f.first % SHOW_EVERY == 0);
        if (shown) {
            double sec = duration_cast<microseconds>(steady_clock::now() - start).count() / 1e6;
            stringstream buffer;
            buffer << fixed << setprecision(1) << (f.first / sec) << " FPS";
            putText(f.second, buffer.str(), cv::Point(10, 15), 1, 1, cv::Scalar{0, 0, 240}, 1);
            imshow("YOLOv3 Detection@Xilinx DPU", f.second);
            int key = waitKey(1);
            if (key == 27 || key == 'q') stop_req = true;          // Read を止める → 合図が流れてきて全員が終わる
        }
        long long w = lap(t);
        if (s.add(pop, w, 0, 0, 0) && shown) s.shown++;          // 表示した枚数も数える
    }
}

// =====================================================================
//  表は全部終わってから1回だけ
// =====================================================================
void report(const Stat& rd, const vector<Stat>& inf, const Stat& sh, int inversions,
            const ConcurrentQueue<imagePair>& q1, const ConcurrentQueue<imagePair>& q2) {
    auto ms = [](long long us, int n) { return n ? us / 1000.0 / n : 0.0; };
    cout << fixed << setprecision(1)
         << "\n===== 3段パイプライン（Read 1人 / Infer " << NINFER << "人 / Show 1人、容量 " << CAP
         << "、表示は " << SHOW_EVERY << "枚に1枚、PROFILE=" << PROFILE << "） =====\n"
         << "Show の出口で " << sh.n << " フレーム / " << sh.sec() << " 秒 = "
         << (sh.sec() > 0 ? sh.n / sh.sec() : 0) << " FPS   追い越し " << inversions << " 回\n";
#if !PROFILE
    (void)rd; (void)inf; (void)q1; (void)q2; (void)ms;
    return;                                                    // 計測なしのときは表を出さない
#endif
    cout << "\n  段        取り出し待ち 実計算時間   入れ待ち    最大    件数\n";
    auto line = [&](const char* name, const Stat& s, bool has_pop, bool has_push) {
        cout << "  " << left << setw(8) << name << right;
        if (has_pop) cout << setw(11) << ms(s.pop_us, s.n) << "ms"; else cout << setw(13) << "-";
        cout << setw(9) << ms(s.work_us, s.n) << "ms";
        if (has_push) cout << setw(11) << ms(s.push_us, s.n) << "ms"; else cout << setw(13) << "-";
        cout << setw(8) << s.work_max_us / 1000.0 << "ms" << setw(7) << s.n;
    };
    line("Read", rd, false, true); cout << "\n";
    for (int k = 0; k < NINFER; k++) {
        line(("Infer#" + to_string(k)).c_str(), inf[k], true, true);
        cout << "   （実計算時間の内訳: pre " << ms(inf[k].pre_us, inf[k].n)
             << " / dpu " << ms(inf[k].dpu_us, inf[k].n)
             << " / post " << ms(inf[k].post_us, inf[k].n) << "）\n";
    }
    line("Show", sh, true, false);
    cout << "   （imshow 1回あたり " << (sh.shown ? sh.work_us / 1000.0 / sh.shown : 0.0)
         << "ms、" << sh.shown << " 枚表示。実計算時間は表示しなかった枚も含めた平均）\n";
    cout << "\nキューに並んだ最大数:  q1(Read→Infer) " << q1.peak() << " / " << CAP
         << "    q2(Infer→Show) " << q2.peak() << " / " << CAP << "\n";
}

int main(const int argc, const char** argv) {
    cout << "hardware_concurrency = " << std::thread::hardware_concurrency() << endl;
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " [model_file] [video_file]" << endl;
        return -1;
    }
    auto xmodel_file = std::string(argv[1]);

    auto graph = xir::Graph::deserialize(xmodel_file);
    auto subgraph = get_dpu_subgraph(graph.get());
    CHECK_EQ(subgraph.size(), 1u) << "yolov3 should have one and only one dpu subgraph." << endl;
    cout << "create running for subgraph: " << subgraph[0]->get_name() << endl;

    vector<std::unique_ptr<vart::Runner>> runners;              // Infer の人数ぶん
    for (int k = 0; k < NINFER; k++) runners.push_back(vart::Runner::create_runner(subgraph[0], "run"));

    auto inputTensors  = runners[0]->get_input_tensors();
    auto outputTensors = runners[0]->get_output_tensors();
    int inputCnt  = inputTensors.size();
    int outputCnt = outputTensors.size();
    TensorShape inshapes[inputCnt];
    TensorShape outshapes[outputCnt];
    shapes.inTensorList  = inshapes;
    shapes.outTensorList = outshapes;
    getTensorShape(runners[0].get(), &shapes, inputCnt, outputCnt);

    // ---- 2本のキューと、スレッドごとの記録場所 ----
    ConcurrentQueue<imagePair> q1(CAP), q2(CAP);               // q1: Read→Infer、q2: Infer→Show
    Stat st_read, st_show;
    vector<Stat> st_infer(NINFER);
    int inversions = 0;

    // ---- スレッドを立てる ----
    vector<thread> th;
    th.emplace_back(readFrame, argv[2], ref(q1), ref(st_read));
    for (int k = 0; k < NINFER; k++)
        th.emplace_back(runYOLO, runners[k].get(), ref(q1), ref(q2), ref(st_infer[k]));
    th.emplace_back(displayFrame, ref(q2), ref(st_show), ref(inversions));

    for (auto& t : th) t.join();                                // 全員が終わるまで待つ

    report(st_read, st_infer, st_show, inversions, q1, q2);
    return 0;
}
