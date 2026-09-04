# cpp-lab

**C++ の並行処理（スレッド）を、ブラウザだけで学ぶ演習集です。**
Google Colab 上で C++ をコンパイル・実行します。環境構築は要りません。

---

## 演習（本編）

バッジを押すと Colab が開きます。**上のセルから順に ▶ を押していく**だけです。

| | 演習 | 学ぶこと |
|---|---|---|
| **1** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads.ipynb) **スレッドとパイプライン** | `thread` / `join`、待つ仕事と計算する仕事、レイテンシとスループット、ボトルネック |
| **2** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex02_queue.ipynb) **スレッドセーフなキューを使う** | 守らない共有は壊れる、`push` / `pop` / `size`、容量とバックプレッシャ |
| **3** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex03_pipeline.ipynb) **パイプラインを組んで、速くする** | 容量の決め方、バックプレッシャ、2つの上限（一番遅い段とマシンの計算力） |
| **4** | [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex04_measure.ipynb) **壊さないように測る** | 測る値段、計測外、5つの作法、直列で測ってから組んで確かめる |

> 💡 新しいタブで開きたいときは、バッジを `Ctrl`（Mac は `⌘`）を押しながらクリックしてください。

### 進め方

1. バッジを押して Colab を開く
2. **上のセルから順に ▶ を押す**
3. 「**予測クイズ**」が出てきたら、**実行する前に必ず自分で予測**する ―― 予測が外れることに意味があります

> ⏱ 一部のセルには、**デッドロックや終了漏れをわざと見せるセル**があります。
> `timeout 5 ./xxx` で時間を区切ってあるので Colab が固まることはありません。
> **終了コード 124 が「止まった」印**です。

---

## 発展課題（補足）

**本編を終えてから、興味のあるものだけ選んでください。全部やる必要はありません。**
各問は「問題 → 実験 → 解答」の順に並んでいます。**問題を読んだら、まず自分で予測**してください。

### 発展課題1 ―― スレッドとパイプライン

| | 問題 | |
|---|---|---|
| 1-1 | 各段が **Read=13ms / Infer=67ms / Show=33ms** のとき、3フレームを直列とパイプラインで処理すると何 ms か。ボトルネックはどの段で、上限は何 FPS か。**Infer だけを 2ms に**したら、ボトルネックはどこへ移り、上限 FPS は何倍になるか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv01_threads.ipynb#scrollTo=adv01_threads_02) |
| 1-2 | `ex01a.cpp` から `t1.join(); t2.join();` を消すと何が起きるか。なぜそうなるのか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv01_threads.ipynb#scrollTo=adv01_threads_06) |
| 1-3 | `hardware_concurrency()` が **2** のマシンで、**計算しかしない仕事**（1本 300ms 分）のスレッドを **3本** 立てた。3本目は動かずに待たされるのか。全部で何 ms か。3本は同時に終わるのか、順に終わるのか。**待つだけの仕事**なら答えは変わるか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv01_threads.ipynb#scrollTo=adv01_threads_10) |

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv01_threads.ipynb) `adv01_threads.ipynb`

### 発展課題2 ―― スレッドセーフなキュー

| | 問題 | |
|---|---|---|
| 2-1 | キューに3個までしか入れたくないので `if (q.size() < 3) q.push(x);` と書いた。`size()` も `push()` も鍵で守られている。**期待どおり動くか** | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv02_queue.ipynb#scrollTo=adv02_queue_04) |
| 2-2 | `ConcurrentQueue` が持つ条件変数2本（`can_pop_` / `can_push_`）を**1本にまとめたら**どうなるか。`notify_one()` と `notify_all()` で答えは変わるか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv02_queue.ipynb#scrollTo=adv02_queue_08) |
| 2-3 | `pop()` は空なら待つ。では**待たない `try_pop()`** が要るのはどんな場面か。逆に**待つ形**が要るのはどんな場面か | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv02_queue.ipynb#scrollTo=adv02_queue_10) |

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv02_queue.ipynb) `adv02_queue.ipynb`

### 発展課題3 ―― パイプラインを組む

| | 問題 | |
|---|---|---|
| 3-1 | Read 10ms / Infer 60ms / Show 30ms の仕事を **3段パイプライン**（構成②）にしたら、直列（構成①）の何倍になるか。3倍になるか。ならないなら、決めているのは何か | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv03_pipeline.ipynb#scrollTo=adv03_pipeline_05) |
| 3-2 | 一番遅い段（Infer）を **2人**に増やす（構成③）。上限A は 16.7 → 33.3 と倍になるのに、実測FPS も倍になるか。ならないなら**上限A と 上限B のどちらで止まっているか**を見分ける | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv03_pipeline.ipynb#scrollTo=adv03_pipeline_09) |
| 3-3 | 3段パイプラインのキュー2本の**長さを見るだけ**で、どの段がボトルネックか言い当てられるか。Infer が遅いとき／Show が遅いとき／Read が遅いとき、それぞれ2本はどうなるか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv03_pipeline.ipynb#scrollTo=adv03_pipeline_13) |
| 付録 | **速度倍率の測り方** ―― 同じ計算を1本／2本／3本／4本でやって時間を比べる。自分のマシンの倍率がそのまま出る（演習3-2 の 上限B の分母に使う） | [付録へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv03_pipeline.ipynb#scrollTo=adv03_pipeline_14) |

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv03_pipeline.ipynb) `adv03_pipeline.ipynb`

### 発展課題4 ―― 壊さないように測る

| | 問題 | |
|---|---|---|
| 4-1 | 演習4-3 の3段パイプライン（30 FPS、犯人は Show）で、**Show を 30ms → 10ms** に速くしたら FPS はいくつになるか。3倍の 90 FPS になるか。犯人はどの段へ移り、`取り出し待ち` / `正味` / `入れ待ち` の3列はどう変わるか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv04_measure.ipynb#scrollTo=adv04_measure_05) |
| 4-2 | シリアルコンソール（**115200 baud**）で1行 36 バイトのデバッグ表示を**1フレームに4行**出すと、1フレームあたり何 ms 増えるか。1フレーム 82ms の処理と 13ms の処理で、それぞれ何%か。表示を**計測区間の外**に置いた場合、各段の測定値は正しいままか。ではどうやって気づくか | [解答へ](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv04_measure.ipynb#scrollTo=adv04_measure_07) |

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/adv04_measure.ipynb) `adv04_measure.ipynb`

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

### 実行結果が毎回変わることについて

**同じプログラムを実行しても結果が変わる**ことがあります。故障ではなく、並行処理の本質です。

- スレッドがどの順で動くかは OS が決めるため、実行ごとに変わります
- マシンのコア数（`std::thread::hardware_concurrency()`）によっても変わります

**「動いたから正しい」が通用しない**こと自体を体験するのが狙いです。

---

## リポジトリの構成

```
ex01_threads.ipynb              演習1  スレッドとパイプライン
ex02_queue.ipynb                演習2  スレッドセーフなキューを使う
ex03_pipeline.ipynb             演習3  パイプラインを組んで、速くする
ex04_measure.ipynb              演習4  壊さないように測る

adv01_threads.ipynb             発展課題1（問題＋解答）
adv02_queue.ipynb               発展課題2（問題＋解答）
adv03_pipeline.ipynb            発展課題3（問題＋解答）
adv04_measure.ipynb             発展課題4（問題＋解答）

src/                            参考用の C++ ソース
img/                            ノートブックに埋め込んだ図（原寸で見たいとき用）
```

`cq.h`（`ConcurrentQueue`）を書き出すセルは、**各ノートブックの先頭に1回だけ**置いてあります。
**最初にそのセルを実行**すれば、以降のプログラムは `#include "cq.h"` で使えます。

`.ipynb` は生成物です。**Colab や Jupyter で直接直しても、次の配布で上書きされます。**
誤りや改善点は配布元までご連絡ください。
