/*
 * Copyright 2019 Xilinx Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <glog/logging.h>

#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>

/* header file for Vitis AI unified API */
#include <vart/mm/host_flat_tensor_buffer.hpp>
#include <vart/runner.hpp>
#include <xir/graph/graph.hpp>
#include <xir/tensor/tensor.hpp>
#include <xir/util/data_type.hpp>

struct TensorShape {
  unsigned int height;
  unsigned int width;
  unsigned int channel;
  unsigned int size;
};

struct GraphInfo {
  struct TensorShape* inTensorList;
  struct TensorShape* outTensorList;
  std::vector<int> output_mapping;
};

int getTensorShape(vart::Runner* runner, GraphInfo* shapes, int cntin,
                   const std::vector<std::string> output_names);
int getTensorShape(vart::Runner* runner, GraphInfo* shapes, int cntin,
                   int cnout);
// fix_point to scale for input tensor
inline float get_input_scale(const xir::Tensor* tensor) {
  int fixpos = tensor->template get_attr<int>("fix_point");
  return std::exp2f(1.0f * (float)fixpos);
}
// fix_point to scale for output tensor
inline float get_output_scale(const xir::Tensor* tensor) {
  int fixpos = tensor->template get_attr<int>("fix_point");
  return std::exp2f(-1.0f * (float)fixpos);
}

inline std::vector<std::unique_ptr<xir::Tensor>> cloneTensorBuffer(
    const std::vector<const xir::Tensor*>& tensors) {
  auto ret = std::vector<std::unique_ptr<xir::Tensor>>{};
  auto type = xir::DataType::XINT;
  ret.reserve(tensors.size());
  for (const auto& tensor : tensors) {
    ret.push_back(std::unique_ptr<xir::Tensor>(xir::Tensor::create(
        tensor->get_name(), tensor->get_shape(), xir::DataType{type, 8u})));
  }
  return ret;
}

inline std::vector<const xir::Subgraph*> get_dpu_subgraph(
    const xir::Graph* graph) {
  auto root = graph->get_root_subgraph();
  auto children = root->children_topological_sort();
  auto ret = std::vector<const xir::Subgraph*>();
  for (auto c : children) {
    CHECK(c->has_attr("device"));
    auto device = c->get_attr<std::string>("device");
    if (device == "DPU") {
      ret.emplace_back(c);
    }
  }
  return ret;
}

class CpuFlatTensorBuffer : public vart::TensorBuffer {
 public:
  explicit CpuFlatTensorBuffer(void* data, const xir::Tensor* tensor)
      : TensorBuffer{tensor}, data_{data} {}
  virtual ~CpuFlatTensorBuffer() = default;

 public:
  virtual std::pair<uint64_t, size_t> data(
      const std::vector<int> idx = {}) override {
    uint32_t size = std::ceil(tensor_->get_data_type().bit_width / 8.f);
    if (idx.size() == 0) {
      return {reinterpret_cast<uint64_t>(data_),
              tensor_->get_element_num() * size};
    }
    auto dims = tensor_->get_shape();
    auto offset = 0;
    for (auto k = 0; k < tensor_->get_shape().size(); k++) {
      auto stride = 1;
      for (auto m = k + 1; m < tensor_->get_shape().size(); m++) {
        stride *= dims[m];
      }
      offset += idx[k] * stride;
    }
    auto elem_num = tensor_->get_element_num();
    return {reinterpret_cast<uint64_t>(data_) + offset * size,
            (elem_num - offset) * size};
  }

 private:
  void* data_;
};

// ============================================================
//  ConcurrentQueue ―― スレッドセーフなキュー（演習の cq.h と同じもの）
//  中身は読まなくてよい。使うのは push / pop / size / peak の4つだけ。
// ============================================================
template <typename T>
class ConcurrentQueue {
public:
    explicit ConcurrentQueue(std::size_t capacity) : capacity_(capacity) {}

    // 入れる。満杯なら空くまで待つ
    void push(const T& v) {
        std::unique_lock<std::mutex> lk(mtx_);
        can_push_.wait(lk, [this] { return q_.size() < capacity_; });
        q_.push(v);
        if (q_.size() > peak_) peak_ = q_.size();
        lk.unlock();
        can_pop_.notify_one();
    }

    // 取り出す。空なら来るまで待つ（失敗しない）
    T pop() {
        std::unique_lock<std::mutex> lk(mtx_);
        can_pop_.wait(lk, [this] { return !q_.empty(); });
        T v = q_.front(); q_.pop();
        lk.unlock();
        can_push_.notify_one();
        return v;
    }

    std::size_t size() const { std::lock_guard<std::mutex> g(mtx_); return q_.size(); }
    std::size_t peak() const { std::lock_guard<std::mutex> g(mtx_); return peak_; }   // 観測用：並んだ最大数

private:
    std::queue<T> q_;
    std::size_t capacity_;
    std::size_t peak_ = 0;
    mutable std::mutex mtx_;
    std::condition_variable can_pop_;    // 「取り出せるようになった」
    std::condition_variable can_push_;   // 「入れられるようになった」
};

#endif
