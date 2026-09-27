"""Replace non-ASCII characters in C++ sources with \\uXXXX escapes.

MSVC may read UTF-8 sources without BOM with the system code page, which corrupts Turkish
text. Write Turkish freely inside TEXT("...") literals, then run:

    python Tools/escape_unicode.py            # rewrite Source/**/*.h|*.cpp in place
    python Tools/escape_unicode.py --check    # exit 1 if any file still has non-ASCII
"""
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent / "Source"


def escape(text: str) -> str:
    out = []
    for ch in text:
        code = ord(ch)
        if code < 128:
            out.append(ch)
        elif code <= 0xFFFF:
            out.append("\\u%04x" % code)
        else:
            out.append("\\U%08x" % code)
    return "".join(out)


def main() -> int:
    check = "--check" in sys.argv
    dirty = []
    for path in sorted(ROOT.rglob("*")):
        if path.suffix not in (".h", ".cpp"):
            continue
        text = path.read_text(encoding="utf-8")
        fixed = escape(text)
        if fixed != text:
            dirty.append(path)
            if not check:
                path.write_text(fixed, encoding="utf-8", newline="\n")
    for path in dirty:
        print(("NON-ASCII " if check else "escaped   ") + str(path.relative_to(ROOT.parent)))
    return 1 if (check and dirty) else 0


if __name__ == "__main__":
    sys.exit(main())
