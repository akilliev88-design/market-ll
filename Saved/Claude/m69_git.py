# Claude (Cowork) 04.10.2026: yerel akis-cc2 ile GitHub'daki akis-cc2 ayrisinca guvenli birlestirme.
# Kullanim: python Saved\Claude\m69_git.py <uzak_ad>
# Yereldeki commit edilmemis degisikliklere dokunmaz (stash yok). Uzaktaki yeni commit'ler bu degisikliklerle ayni
# dosyaya ya da Saved/Claude betiklerine dokunuyorsa birlestirmez, durur ve listeyi yazar.
import subprocess
import sys


def git(*a, check=True):
    r = subprocess.run(['git'] + list(a), capture_output=True, text=True)
    if check and r.returncode != 0:
        print('GIT_HATA', ' '.join(a))
        print(r.stdout)
        print(r.stderr)
        sys.exit(1)
    return r.stdout


remote = sys.argv[1] if len(sys.argv) > 1 else 'cloud'
up = remote + '/akis-cc2'
print('dal:', git('rev-parse', '--abbrev-ref', 'HEAD').strip())
print('--- yalniz yerelde olan commitler ---')
print(git('log', '--oneline', up + '..HEAD'))
print('--- yalniz GitHubda olan commitler ---')
print(git('log', '--oneline', '--stat', 'HEAD..' + up))
theirs = set(f for f in git('diff', '--name-only', 'HEAD...' + up).split() if f)
mine = set(f for f in git('diff', '--name-only').split() if f)
risky = sorted(f for f in theirs if f in mine or f.startswith('Saved/Claude/') or f == 'CLAUDE_KOS.cmd')
if risky:
    print('BIRLESTIRME_DURDU: GitHubdaki commitler su dosyalara da dokunuyor:')
    for f in risky:
        print('  ', f)
    sys.exit(1)
r = subprocess.run(['git', '-c', 'user.name=Claude-Cowork', '-c', 'user.email=noreply@anthropic.com', 'merge', '--no-edit', up],
                   capture_output=True, text=True)
print(r.stdout)
print(r.stderr)
if r.returncode != 0:
    subprocess.run(['git', 'merge', '--abort'])
    print('BIRLESTIRME_DURDU: git merge basarisiz, geri alindi.')
    sys.exit(1)
print(git('log', '--oneline', '-3'))
print('BIRLESTIRME_TAMAM')
