import re

def convert_line(match):
    d_val = match.group(1)
    # << で区切られた数値を取り出す
    content = match.group(2)
    # 数値を抽出
    numbers = [x.strip() for x in content.split('<<') if x.strip()]
    
    lines = [f'    if (d == {d_val})', '    {']
    for i, num in enumerate(numbers):
        # 末尾のセミコロンを除去
        num = num.rstrip(';')
        lines.append(f'        eps[{i}] = {num};')
    lines.append('    }')
    return '\n'.join(lines)

with open('src/pruning.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# パターン: if (d == 数字) { eps << ... ; }
# 改行を含んでもマッチするように re.DOTALL を使用
pattern = re.compile(r'if\s*\(\s*d\s*==\s*(\d+)\s*\)\s*\{\s*eps\s(<<[^;]+;)\s*\}', re.DOTALL)

new_content = pattern.sub(convert_line, content)

with open('pruning_fixed.cpp', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("変換が完了しました: pruning_fixed.cpp")
