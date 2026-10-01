"""Check screenshot dimensions and create a local C8 review index."""
import argparse
import html
import json
from pathlib import Path
from PIL import Image

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    parser.add_argument('phases', nargs=3, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    result = []
    body = ['<!doctype html><meta charset="utf-8"><title>C8 menü incelemesi</title>',
            '<style>body{font:16px system-ui;margin:32px;background:#f7f5ef}a{color:#176957}li{margin:8px}</style>',
            '<h1>C8 menü incelemesi</h1><p>Başlangıç, salgın öncesi ve sonrası. Gerçek atak22; iki tema, iki boyut.</p>']
    for phase in args.phases:
        files = sorted(phase.glob('*.png'))
        errors = []
        for path in files:
            expected = (1280, 720) if '1280x720' in path.name else (1920, 1080)
            with Image.open(path) as image:
                if image.size != expected:
                    errors.append({'file': path.name, 'actual': image.size, 'expected': expected})
        assert len(files) == 104, (phase, len(files))
        assert not errors, errors
        result.append({'phase': phase.name, 'images': len(files), 'dimension_errors': errors})
        # Paths are siblings beneath Saved/Screenshots/Menu.
        body.append(f'<h2>{html.escape(phase.name)}</h2><a href="../{html.escape(phase.name)}/index.html">Dönem galerisi</a><ul>')
        for path in files:
            body.append(f'<li><a href="../{html.escape(phase.name)}/{html.escape(path.name)}">{html.escape(path.name)}</a></li>')
        body.append('</ul>')
    (args.output / 'index.html').write_text('\n'.join(body), encoding='utf-8')
    (args.output / 'boyut_kontrolu.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(result, ensure_ascii=False))

if __name__ == '__main__':
    main()
