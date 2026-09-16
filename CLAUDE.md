# cpp-lab ―― Claude Code への指示

このリポジトリは、学生が Google Colab 上で C++ の並行処理（スレッド・キュー・パイプライン）を学ぶ演習教材です。
GitHub: https://github.com/takgto/cpp-lab （public。学生はここから Colab を開く）

## 最重要ルール：`.ipynb` は生成物。直接編集しない

- リポジトリ直下の `*.ipynb` は **すべて `gen/` のスクリプトから生成** している
- 教材を直すときは **`gen/` の Python か `src/` の C++ を直し、スクリプトを実行して再生成** する
- `.ipynb` を直接編集してはいけない（次の再生成で消える）
- Colab / Jupyter で `.ipynb` に実行結果（outputs）が入った状態を commit しない。再生成すれば outputs は空になる

```bash
# リポジトリのどこからでも実行できる。出力先はリポジトリ直下
python3 gen/nb01.py        # → ex01_threads.ipynb
python3 gen/nb02.py        # → ex02_queue.ipynb
python3 gen/nb03.py        # → ex03_pipeline.ipynb
python3 gen/nb04.py        # → ex04_measure.ipynb
python3 gen/nbadv12.py     # → adv01_threads.ipynb, adv02_queue.ipynb
python3 gen/nbadv03.py     # → adv03_pipeline.ipynb（本文は gen/adv03_text.md）
python3 gen/nbadv04.py     # → adv04_measure.ipynb
python3 gen/nbap.py        # → 付録（未公開。生成しても commit しない）
```

## 変更したときの確認手順

1. 変更したスクリプトを実行して `.ipynb` を再生成する
2. `git diff --stat` で、**意図したノートブックだけ** が変わっていることを確認する
3. C++ を変えたときは、`src/` の該当ファイルを `g++ -std=c++17 -pthread -O2 x.cpp -o x && ./x` でコンパイル・実行して動くことを確認する（Colab と同じオプション）
4. `%%writefile` セルの中身と `src/` のファイルは常に同一であること（`build.py` の `src()` が `src/` から読むので、自動的に同一になる）

## ディレクトリ構成

```
cpp-lab/
├── README.md              学生向け。演習・発展課題の一覧と Colab バッジ
├── CLAUDE.md              このファイル
├── ex01〜ex04_*.ipynb     演習1〜4（生成物）
├── adv01〜adv04_*.ipynb   発展課題1〜4（生成物）
├── src/                   ノートブックが %%writefile で書き出す C++ ソース
│   ├── cq.h               ConcurrentQueue（演習2以降で使う）
│   ├── ex*.cpp            演習
│   ├── adv*.cpp           発展課題
│   └── ap*.cpp            付録
├── img/                   ノートブックに base64 で埋め込む図（queue_*.png, pipe_*.png）
├── kv260/                 ハッカソン当日に配布する KV260 用サンプルの閲覧用コピー
│                          原本は private の takgto/yolov3_edge/cpp。ここは学生が読むためだけに置いてある
└── gen/                   ノートブック生成スクリプト（配布元用。詳細は gen/README.md）
    ├── build.py           共通部品。md()/code()/src()/img()/write()
    ├── nb01.py〜nb04.py   演習1〜4
    ├── nbadv12.py, nbadv03.py, nbadv04.py   発展課題
    ├── adv03_text.md      発展課題3の本文（nbadv03.py が読む。===== CELL nn ===== と {{IMG:...}} の行は消さない）
    ├── nbap.py            付録
    └── figs/gen.py, gen3.py   img/ の図の元になる Excel を figs/out/ に作る（openpyxl）
```

## 教材の約束ごと（変更するときに守る）

- 学生がこの演習に使える時間は **最大3時間**。本編は分量を増やさず、細かい話は発展課題か付録へ
- 各演習は「予測クイズ → 実行 → 結果 → 覚えるべきこと」の流れ。**実行前に予測させる** 構成を崩さない
- 時間計測は **`steady_clock`**（`system_clock` は使わない）
- 用語は「**実計算時間**」で統一。計測表の列順は「段 / 取り出し待ち / 実計算時間 / 入れ待ち」で、**「段」が先頭列**
- Colab 標準ランタイムは `hardware_concurrency()` = 2 だが実測の計算速度倍率は **約1.1倍で頭打ち**（1.5倍と書かない）
- KV260 側のキュー容量は **16**。教材の数値例を変えるときはここと矛盾させない
- わざとデッドロックや終了漏れを見せるセルは `timeout 5 ./xxx` で囲む（終了コード 124 が「止まった」印）
- ノートブック内リンクは `#scrollTo=<ノートブック名>_<セル番号2桁>` 形式。**セルを増減すると番号がずれる** ので、README の「解答」リンクも合わせて直す
- 前提の問題集は「C/C++ 事前学習用問題集」（問1〜問8、別リポジトリ予定）。演習1が参照している `std::ref` の説明は **問題集の問4** にある

## Git の扱い

- commit は自由に行ってよい。commit メッセージは日本語でよい
- **push は行う前に一度確認を取る**（学生が見ている public リポジトリのため）
- `gen/figs/out/` と `__pycache__/` は `.gitignore` 済み。生成した Excel やバイナリは commit しない
- 付録（`nbap.py` の出力）は未公開。生成しても commit しない

## 初回セットアップ（完了したらこの節は削除してよい）

生成スクリプトはまだこのリポジトリに入っていない。一式は次の zip にある。

- 環境：Windows 上の WSL2。このリポジトリは `/home/t_goto/colab/cpp-lab`
- zip は Windows 側の Downloads にある。WSL2 からは **`/mnt/c/Users/kurod/Downloads/cpp-lab-gen.zip`**（見つからなければ聞くこと）

1. `git pull` して最新にする
2. リポジトリ直下で `unzip -o /mnt/c/Users/kurod/Downloads/cpp-lab-gen.zip` を実行する。`gen/` が新規にでき、`src/` に `apA1.cpp`〜`apB3.cpp` が増え、`.gitignore`・`ex01_threads.ipynb`・`CLAUDE.md` が上書きされる（`unzip` が無ければ `sudo apt install unzip`）
3. 展開前後で `ex01_threads.ipynb` に差分がないことを `git diff ex01_threads.ipynb` で確認する（同一のはず）
4. `python3 gen/nb01.py` 〜 `nbadv04.py` をすべて実行し、`git status` で **`.ipynb` に差分が出ない** ことを確認する（生成スクリプトと現状の `.ipynb` が一致している証拠）
5. `git add gen src .gitignore CLAUDE.md` → commit
6. README の「リポジトリの構成」に `gen/` の一行（「ノートブック生成スクリプト（配布元用）」）を足して commit
7. push は確認を取ってから
