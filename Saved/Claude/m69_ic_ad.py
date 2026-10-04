# M69c (Claude Cowork, 04.10.2026): ic kod adi MirasMarket -> MarketSim.
# Mustafa: "oyunun adi Miras olmasin"; satis adi henuz kesin degil, bu yuzden ic ad satis adindan bagimsiz.
# Proje kokunden calisir: python Saved\Claude\m69_ic_ad.py
# 1) klasor ve dosya adlari git mv ile tasinir, 2) metin dosyalarinda belirli adlar degisir,
# 3) eski adla kalan her satir listelenir (icerik varlik yollari bilerek kalir: /Game/Materials/Miras, M_Miras*).
import os
import re
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(ROOT)

MOVES = [
    ('Source/MirasMarket/MirasMarket.Build.cs', 'Source/MirasMarket/MarketSim.Build.cs'),
    ('Source/MirasMarket/MirasMarket.cpp', 'Source/MirasMarket/MarketSim.cpp'),
    ('Source/MirasMarket/MirasAutoPlayCommandlet.cpp', 'Source/MirasMarket/MarketSimAutoPlayCommandlet.cpp'),
    ('Source/MirasMarket/MirasAutoPlayCommandlet.h', 'Source/MirasMarket/MarketSimAutoPlayCommandlet.h'),
    ('Source/MirasMarket', 'Source/MarketSim'),
    ('Source/MirasMarketStudio/MirasMarketStudio.Build.cs', 'Source/MirasMarketStudio/MarketSimStudio.Build.cs'),
    ('Source/MirasMarketStudio/Private/MirasMarketStudioModule.cpp', 'Source/MirasMarketStudio/Private/MarketSimStudioModule.cpp'),
    ('Source/MirasMarketStudio', 'Source/MarketSimStudio'),
    ('Source/MirasMarket.Target.cs', 'Source/MarketSim.Target.cs'),
    ('Source/MirasMarketEditor.Target.cs', 'Source/MarketSimEditor.Target.cs'),
    ('MirasMarket.uproject', 'MarketSim.uproject'),
]

# Sira onemli: uzun adlar once.
TOKENS = [
    ('MIRASMARKETSTUDIO_API', 'MARKETSIMSTUDIO_API'),
    ('MIRASMARKET_API', 'MARKETSIM_API'),
    ('MirasMarketStudio', 'MarketSimStudio'),
    ('MirasMarketEditor', 'MarketSimEditor'),
    ('MirasAutoPlayCommandlet', 'MarketSimAutoPlayCommandlet'),
    ('MirasMarket', 'MarketSim'),
    # komut satiri bayraklari ve aktor etiketleri (oyuncu gormez, ama ic ad tutarli kalsin)
    ('MirasSmoke', 'SimSmoke'),
    ('MirasCapture', 'SimCapture'),
    ('MirasStorePreview', 'SimStorePreview'),
    ('MirasStoreTour', 'SimStoreTour'),
    ('MirasStoreBenchmark', 'SimStoreBenchmark'),
    ('MirasStoreKit', 'SimStoreKit'),
    ('MirasStoreRoof', 'SimStoreRoof'),
    ('MirasAutoPlay', 'SimAutoPlay'),
    ('MirasNoInternet', 'SimNoInternet'),
    ('MirasNoGrowth', 'SimNoGrowth'),
    ('MirasMenu', 'SimMenu'),
    ('MirasProduct:', 'SimProduct:'),
]
TEXT_EXT = {'.cpp', '.h', '.cs', '.ini', '.uproject', '.ps1', '.cmd', '.bat', '.py', '.json', '.md', '.txt'}
# Gecmis kayitlari degistirme: surec gunlukleri, kurgu gecmisi, uretim ve varlik klasorleri.
SKIP_PREFIX = ('Docs/Surec/', 'Docs/Kurgu/', 'Docs/Planlama/', 'Docs/Devam_', 'Content/', 'AssetInbox/', 'Uretim/', 'Binaries/',
               'Intermediate/', 'DerivedDataCache/', 'Tools/Arsiv/')
# Calisan toplu dosyalarin ustune yazilmaz (cmd yorumlayicisi bozulur).
SKIP_FILES = {'CLAUDE_KOS.cmd', 'Saved/Claude/is.cmd', 'Saved/Claude/adim.cmd', 'Saved/Claude/m69_ic_ad.py'}
KEEP = re.compile(r'/Game/Materials/Miras|M_Miras(Surface|Acrylic)')


def git(*args):
    return subprocess.run(['git'] + list(args), check=True, capture_output=True, text=True).stdout


def main():
    if not os.path.exists('MirasMarket.uproject'):
        print('ICAD_ZATEN_YAPILMIS (MirasMarket.uproject yok)')
        return 0
    for old, new in MOVES:
        if os.path.exists(old):
            git('mv', old, new)
            print('tasindi', old, '->', new)
    files = [f for f in git('ls-files').splitlines()]
    files += ['Saved/Claude/ozet.ps1', 'Saved/Claude/bot.ps1']
    changed = 0
    for f in sorted(set(files)):
        if f in SKIP_FILES or f.startswith(SKIP_PREFIX) or os.path.splitext(f)[1].lower() not in TEXT_EXT or not os.path.isfile(f):
            continue
        with open(f, 'rb') as h:
            raw = h.read()
        try:
            text = raw.decode('utf-8')
        except UnicodeDecodeError:
            continue
        new = text
        for a, b in TOKENS:
            new = new.replace(a, b)
        if new != text:
            with open(f, 'wb') as h:
                h.write(new.encode('utf-8'))
            changed += 1
    print('degisen dosya', changed)
    # Unreal: eski adla kayitli siniflar icin yonlendirme (icerikteki baglantilar kopmasin).
    engine = 'Config/DefaultEngine.ini'
    with open(engine, 'rb') as h:
        ini = h.read().decode('utf-8')
    if '[CoreRedirects]' not in ini:
        nl = '\r\n' if '\r\n' in ini else '\n'
        ini = ini.rstrip() + nl + nl + '[CoreRedirects]' + nl \
            + '+PackageRedirects=(OldName="/Script/MirasMarket",NewName="/Script/MarketSim")' + nl \
            + '+PackageRedirects=(OldName="/Script/MirasMarketStudio",NewName="/Script/MarketSimStudio")' + nl
        with open(engine, 'wb') as h:
            h.write(ini.encode('utf-8'))
        print('CoreRedirects eklendi')
    # Kalanlar.
    left = 0
    for f in git('ls-files').splitlines():
        if f.startswith(SKIP_PREFIX) or os.path.splitext(f)[1].lower() not in TEXT_EXT or not os.path.isfile(f):
            continue
        try:
            with open(f, encoding='utf-8') as h:
                for n, line in enumerate(h, 1):
                    if 'Miras' in line and not KEEP.search(line) and 'CoreRedirects' not in line and 'OldName="/Script/Miras' not in line:
                        left += 1
                        if left <= 60:
                            print('KALAN %s:%d: %s' % (f, n, line.strip()[:160]))
        except UnicodeDecodeError:
            pass
    print('ICAD_KALAN_SATIR', left)
    print('ICAD_TAMAM')
    return 0


if __name__ == '__main__':
    sys.exit(main())
