# cpp-lab

**C++ の並行処理（スレッド）を、ブラウザだけで学ぶ演習集です。**
Google Colab 上で C++ をコンパイル・実行します。環境構築は要りません。

---

## 演習

各行の左のバッジを押すと Colab が開きます。**上のセルから順に ▶ を押していく**だけです。

| | 演習 | 解答編 | 学ぶこと |
|---|---|---|---|
| **1** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads.ipynb) **スレッドとパイプライン** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads_answer.ipynb) | プロセスとスレッドの違い、`thread` / `join`、レイテンシとスループット、パイプライン |
| **2** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex02_race.ipynb) **データ競合** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex02_race_answer.ipynb) | 共有データが壊れる様子、`atomic` で足りる場合と足りない場合 |
| **3** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex03_mutex.ipynb) **`mutex` で守る** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex03_mutex_answer.ipynb) | `mutex` / `lock_guard` / RAII、ロック区間の広さと速度 |
| **4** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex04_queue_wait.ipynb) **待ち合わせ** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex04_queue_wait_answer.ipynb) | ビジーウェイトとポーリングの無駄、`condition_variable`、述語がなぜ要るか |
| **5** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex05_bounded_queue.ipynb) **スレッドセーフなキュー** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex05_bounded_queue_answer.ipynb) | 鍵1本＋条件変数2本、容量つきキューを最初から最後まで組み立てる |
| **6** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex06_backpressure.ipynb) **容量とバックプレッシャ** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex06_backpressure_answer.ipynb) | 容量で変わるもの・変わらないもの、遅れとメモリ、キュー長でボトルネックを当てる |
| **7** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex07_pipeline.ipynb) **パイプラインを組む** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex07_pipeline_answer.ipynb) | 2つの上限（一番遅い段とコア数）、ボトルネックの移動、どう速くしていくか |
| **8** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex08_ordering.ipynb) **並列化すると順序が崩れる** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex08_ordering_answer.ipynb) | 並列化の副作用、番号を持たせる、出口で並べ直す |
| **9** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex09_shutdown.ipynb) **安全な終了** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex09_shutdown_answer.ipynb) | 終了を下流へ伝える、キューを閉じる／番兵、`join()` が返る形にする |
| **10** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex10_measure.ipynb) **壊さないように測る** | [解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex10_measure_answer.ipynb) | 計測の値段、計測外、直列で測って組んで確かめる |

> 💡 **新しいタブで開きたいときは、バッジを `Ctrl`（Mac は `⌘`）を押しながらクリック**してください。
> マウスの中ボタン（ホイール）クリックでも同じです。

---

## 進め方

1. バッジを押して Colab を開く
2. **上のセルから順に ▶ を押す**
3. 「**予測クイズ**」が出てきたら、**実行する前に必ず自分で予測**する ―― 予測が外れることに意味があります
4. 一通り終えたら、**解答編**で答え合わせをする

各演習は 20〜30分程度です。全10回で、およそ4〜5時間を見込んでください。

> ⏱ 演習3・4・5・8・9 には、**デッドロックや終了漏れをわざと見せるセル**があります。
> `timeout 5 ./xxx` で時間を区切ってあるので Colab が固まることはありません。
> **終了コード 124 が「止まった」印**です。

---

## 前提

先に配布した **C/C++ 事前学習用 問題集**（問1〜問10）を終えていることを前提にしています。

| | 問題集 | この演習 |
|---|---|---|
| 扱うもの | C++ の**文法** | 並行処理の**考え方と挙動** |
| やること | 読んで答える | **動かして観る** |
| 答え | 一意に決まる | **実行するたびに変わることがある** |

並行処理は、文法を知っていても挙動が予測できないところに難しさがあります。
そのため「動かして、予測を外して、理由を考える」形式にしてあります。

---

## 実行結果が毎回変わることについて

**同じプログラムを実行しても結果が変わる**ことがあります。故障ではなく、並行処理の本質です。

- スレッドがどの順で動くかは OS が決めるため、実行ごとに変わります
- マシンのコア数（`std::thread::hardware_concurrency()` で確認できます）によっても変わります
- 演習2 のプログラムは**異常終了することがあります**。それも観察結果の1つです

**「動いたから正しい」が通用しない**こと自体を体験するのが狙いです。

---

## この先（ハッカソン本番）

本番では、KV260 上で動く次の2つのプログラムを読み、改造します。

- `yolov3_video_series_prof.cpp` ― 1フレームずつ直列に処理し、各段の時間を測るプロファイリング版
- `yolov3_video_study.cpp` ― 4スレッド + キュー2本でパイプライン化した版

この演習を終えてから読むと、「なぜこの構成なのか」を自分で説明できるようになっているはずです。

---

## リポジトリの構成

```
ex01_threads.ipynb                  演習1  スレッドとパイプライン
ex01_threads_answer.ipynb           演習1  解答編
ex02_race.ipynb                     演習2  データ競合
ex02_race_answer.ipynb              演習2  解答編
ex03_mutex.ipynb                    演習3  mutex で守る
ex03_mutex_answer.ipynb             演習3  解答編
ex04_queue_wait.ipynb               演習4  待ち合わせ
ex04_queue_wait_answer.ipynb        演習4  解答編
ex05_bounded_queue.ipynb            演習5  スレッドセーフなキュー
ex05_bounded_queue_answer.ipynb     演習5  解答編
ex06_backpressure.ipynb             演習6  容量とバックプレッシャ
ex06_backpressure_answer.ipynb      演習6  解答編
ex07_pipeline.ipynb                 演習7  パイプラインを組む
ex07_pipeline_answer.ipynb          演習7  解答編
ex08_ordering.ipynb                 演習8  並列化すると順序が崩れる
ex08_ordering_answer.ipynb          演習8  解答編
ex09_shutdown.ipynb                 演習9  安全な終了
ex09_shutdown_answer.ipynb          演習9  解答編
ex10_measure.ipynb                  演習10 壊さないように測る
ex10_measure_answer.ipynb           演習10 解答編
src/                                本番で使う C++ ソース
```

`.ipynb` が原本です。修正は Colab または Jupyter で直接行ってください。

---

## リンク一覧

Colab Notebooks for executing C++ code.

 [実習1: threads](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads.ipynb)  
 [実習1: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads_answer.ipynb)  
 [実習2: race](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex02_race.ipynb)  
 [実習2: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex02_race_answer.ipynb)  
 [実習3: mutex](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex03_mutex.ipynb)  
 [実習3: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex03_mutex_answer.ipynb)  
 [実習4: queue_wait](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex04_queue_wait.ipynb)  
 [実習4: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex04_queue_wait_answer.ipynb)  
 [実習5: bounded_queue](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex05_bounded_queue.ipynb)  
 [実習5: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex05_bounded_queue_answer.ipynb)  
 [実習6: backpressure](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex06_backpressure.ipynb)  
 [実習6: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex06_backpressure_answer.ipynb)  
 [実習7: pipeline](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex07_pipeline.ipynb)  
 [実習7: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex07_pipeline_answer.ipynb)  
 [実習8: ordering](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex08_ordering.ipynb)  
 [実習8: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex08_ordering_answer.ipynb)  
 [実習9: shutdown](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex09_shutdown.ipynb)  
 [実習9: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex09_shutdown_answer.ipynb)  
 [実習10: measure](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex10_measure.ipynb)  
 [実習10: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex10_measure_answer.ipynb)
