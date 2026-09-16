#!/usr/bin/env python3
"""ノートブック生成スクリプト。md(...) / code(...) / src(...) でセルを並べる。"""
import json, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))   # リポジトリ直下
CPP = os.path.join(ROOT, 'src')       # C++ ソース（%%writefile セルの中身）
OUT = ROOT                            # .ipynb の出力先＝リポジトリ直下


def _lines(text):
    """nbformat の source 形式：各行の末尾に \n（最終行だけは付けない）"""
    ls = text.strip('\n').split('\n')
    return [l + '\n' for l in ls[:-1]] + [ls[-1]]


def md(text):
    return {"cell_type": "markdown", "metadata": {}, "source": _lines(text)}


def code(text):
    return {"cell_type": "code", "execution_count": None, "metadata": {},
            "outputs": [], "source": _lines(text)}


def src(name, dest=None):
    """cpp/<name> の中身を %%writefile セルにする"""
    body = open(os.path.join(CPP, name)).read().rstrip('\n')
    return code("%%writefile " + (dest or name) + "\n" + body)


def img(name, alt="figure"):
    """img/<name> を base64 で埋め込んだ markdown セルにする（Colab でそのまま表示される）"""
    import base64
    p = os.path.join(ROOT, 'img', name)
    b64 = base64.b64encode(open(p, 'rb').read()).decode()
    return {"cell_type": "markdown", "metadata": {},
            "source": [f"![{alt}](data:image/png;base64,{b64})"]}


def imgtag(name, alt="figure"):
    """img/<name> を base64 で埋め込んだ markdown 断片を返す（md(...) の中に差し込む用）"""
    import base64
    p = os.path.join(ROOT, 'img', name)
    b64 = base64.b64encode(open(p, 'rb').read()).decode()
    return f"![{alt}](data:image/png;base64,{b64})"


def srcq(name, dest=None):
    """cpp/<name> の #include "cq.h" を、cq.h の中身そのものに置き換えて %%writefile セルにする。
    （セル単体でコンパイルできるようにするため）"""
    body = open(os.path.join(CPP, name)).read().rstrip('\n')
    cq = open(os.path.join(CPP, 'cq.h')).read()
    cq = cq.split('#pragma once', 1)[1].strip('\n')      # #pragma once より後ろだけ
    block = ("// ===== 演習2で使ったキュー（cq.h と同じもの。読み飛ばしてよい）=====\n"
             + cq +
             "\n// ==================================================================")
    body = body.replace('#include "cq.h"', block)
    return code("%%writefile " + (dest or name) + "\n" + body)


def write(fname, cells):
    nb = {
        "nbformat": 4, "nbformat_minor": 0,
        "metadata": {
            "colab": {"provenance": [], "toc_visible": True},
            "kernelspec": {"name": "python3", "display_name": "Python 3"},
            "language_info": {"name": "python"},
        },
        "cells": cells,
    }
    # セルごとに id を振る（Colab のアンカー用）
    for i, c in enumerate(nb['cells']):
        c['metadata']['id'] = f"{os.path.splitext(fname)[0]}_{i:02d}"
    path = os.path.join(OUT, fname)
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(nb, f, ensure_ascii=False, indent=1)
        f.write('\n')
    n_md = sum(1 for c in cells if c['cell_type'] == 'markdown')
    n_cd = len(cells) - n_md
    chars = sum(len(''.join(c['source'])) for c in cells)
    print(f"{fname:34s} cells={len(cells):3d} (md {n_md} / code {n_cd})  {chars} chars")
