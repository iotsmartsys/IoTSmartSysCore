#!/usr/bin/env python3
import sys
from pathlib import Path


def strip_comments(text: str) -> str:
    out = []
    i = 0
    n = len(text)
    NORMAL, SLASH, LINE, BLOCK, STR, CHAR = range(6)
    state = NORMAL
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ''

        if state == NORMAL:
            if c == '/':
                if nxt == '/':
                    state = LINE
                    i += 2
                    continue
                elif nxt == '*':
                    state = BLOCK
                    i += 2
                    continue
                else:
                    out.append(c)
            elif c == '"':
                out.append(c)
                state = STR
            elif c == '\'':
                out.append(c)
                state = CHAR
            else:
                out.append(c)
        elif state == LINE:
            if c == '\n':
                out.append(c)
                state = NORMAL
        elif state == BLOCK:
            if c == '*' and nxt == '/':
                i += 2
                state = NORMAL
                continue
        elif state == STR:
            out.append(c)
            if c == '\\':
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                    continue
            elif c == '"':
                state = NORMAL
        elif state == CHAR:
            out.append(c)
            if c == '\\':
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                    continue
            elif c == '\'':
                state = NORMAL
        i += 1
    return ''.join(out)


def process_file(path: Path):
    original = path.read_text(encoding='utf-8', errors='ignore')
    stripped = strip_comments(original)
    if stripped != original:
        path.write_text(stripped, encoding='utf-8')
        return True
    return False


def main():
    exts = {'.c', '.h', '.hpp', '.cpp', '.cc', '.ino'}
    roots = [Path('src'), Path('examples')]
    changed = 0
    for root in roots:
        if not root.exists():
            continue
        for p in root.rglob('*'):
            if p.suffix in exts and p.is_file():
                try:
                    if process_file(p):
                        changed += 1
                        print(f"stripped: {p}")
                except Exception as e:
                    print(f"error processing {p}: {e}", file=sys.stderr)
    print(f"files changed: {changed}")


if __name__ == '__main__':
    main()

