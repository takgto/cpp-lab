# gen/ ―― ノートブック生成スクリプト（配布元用）

リポジトリ直下の `.ipynb` はすべてここのスクリプトから生成しています。
**`.ipynb` を直接編集せず、ここを直して再生成してください。**

```bash
# リポジトリのどこからでも実行できる（出力先はリポジトリ直下）
python3 gen/nb01.py        # ex01_threads.ipynb
python3 gen/nb02.py        # ex02_queue.ipynb
python3 gen/nb03.py        # ex03_pipeline.ipynb
python3 gen/nb04.py        # ex04_measure.ipynb
python3 gen/nbadv12.py     # adv01_threads.ipynb, adv02_queue.ipynb
python3 gen/nbadv03.py     # adv03_pipeline.ipynb（本文は adv03_text.md から読む）
python3 gen/nbadv04.py     # adv04_measure.ipynb
python3 gen/nbap.py        # 付録（未公開）
```

| ファイル | 役割 |
|---|---|
| `build.py` | 共通部品。`md()` / `code()` / `src()` / `img()` / `write()`。`src('x.cpp')` は `../src/x.cpp` を `%%writefile` セルにする |
| `nb01.py` 〜 `nb04.py` | 演習1〜4 |
| `nbadv12.py`, `nbadv03.py`, `nbadv04.py` | 発展課題1〜4 |
| `adv03_text.md` | 発展課題3の Markdown 本文（`nbadv03.py` が読み込む。`===== CELL nn =====` と `{{IMG:...}}` の行は消さない） |
| `nbap.py` | 付録 |
| `figs/gen.py`, `figs/gen3.py` | `img/queue_*.png`, `img/pipe_*.png` の元になる Excel タイムチャートを `figs/out/` に作る（openpyxl が必要）。PDF → PNG 化は手作業 |

`.cpp` の本体は `../src/` にあります（`ex*.cpp`, `adv*.cpp`, `cq.h` = 演習・発展課題、`ap*.cpp` = 付録）。
