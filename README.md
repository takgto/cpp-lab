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

> 💡 **新しいタブで開きたいときは、バッジを `Ctrl`（Mac は `⌘`）を押しながらクリック**してください。
> マウスの中ボタン（ホイール）クリックでも同じです。

---

## 進め方

1. バッジを押して Colab を開く
2. **上のセルから順に ▶ を押す**
3. 「**予測クイズ**」が出てきたら、**実行する前に必ず自分で予測**する ―― 予測が外れることに意味があります
4. 一通り終えたら、**解答編**で答え合わせをする

各演習は 20〜30分程度です。

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
ex01_threads.ipynb          演習1  スレッドとパイプライン
ex01_threads_answer.ipynb   演習1  解答編
ex02_race.ipynb             演習2  データ競合
ex02_race_answer.ipynb      演習2  解答編
ex03_mutex.ipynb            演習3  mutex で守る
ex03_mutex_answer.ipynb     演習3  解答編
src/                        本番で使う C++ ソース
```

`.ipynb` が原本です。修正は Colab または Jupyter で直接行ってください。
=======
Colab Notebooks for executing C++ code.

 [実習1: threads](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads.ipynb)  
 [実習1: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_threads_answer.ipynb)  
 [実習2: race](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_race.ipynb)  
 [実習2: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_race_answer.ipynb)  
 [実習3: mutex](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_mutex.ipynb)  
 [実習3: 発展課題解答](https://colab.research.google.com/github/takgto/cpp-lab/blob/main/ex01_mutex_answer.ipynb)  
