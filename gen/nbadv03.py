# 発展課題3 ―― 本文は adv03_text.md（先生の原稿）から読み込む
from build import md, code, src, write, imgtag
import re, os

HERE = os.path.dirname(os.path.abspath(__file__))
ALT = {'pipe_31.png': '構成①と②のタイムチャート',
       'pipe_32.png': '人数を変えたときのタイムチャート',
       'pipe_33.png': 'キューの長さのタイムチャート'}

_p = re.split(r'^===== CELL (\d+).*?=====\s*$', open(os.path.join(HERE, 'adv03_text.md')).read(), flags=re.M)
TEXT = {int(_p[i]): _p[i + 1].strip('\n') for i in range(1, len(_p), 2)}


def mdcell(i):
    """{{IMG:xxx.png}} を base64 の画像に置き換えて markdown セルにする"""
    s = TEXT[i]
    return md(re.sub(r'\{\{IMG:([^}]+)\}\}',
                     lambda m: imgtag(m.group(1), ALT.get(m.group(1), '図')), s))


write('adv03_pipeline.ipynb', [
    mdcell(0),
    src('cq.h'),
    mdcell(2),
    src('adv03a.cpp'),
    code("!g++ -std=c++17 -pthread -O2 adv03a.cpp -o adv03a && ./adv03a"),
    mdcell(5),
    mdcell(6),
    src('adv03b.cpp'),
    code("!g++ -std=c++17 -pthread -O2 adv03b.cpp -o adv03b && ./adv03b"),
    mdcell(9),
    mdcell(10),
    src('adv03c.cpp'),
    code("!g++ -std=c++17 -pthread adv03c.cpp -o adv03c && ./adv03c"),
    mdcell(13),
    mdcell(14),
    src('adv03d.cpp'),
    code('!g++ -std=c++17 -pthread -O2 adv03d.cpp -o adv03d && ./adv03d\n'
         '!lscpu | grep -E "^CPU\\(s\\)|Thread\\(s\\) per core|Core\\(s\\) per socket"'),
    mdcell(17),
])
