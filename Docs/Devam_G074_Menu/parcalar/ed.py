import sys
def esc(s): return ''.join(c if ord(c)<128 else '\\u%04x'%ord(c) for c in s)
def rw(p, pairs, count=1):
    s=open(p,encoding='utf-8').read()
    for a,b in pairs:
        a2,b2=esc(a),esc(b)
        n=s.count(a2)
        if n!=count and not (count==0 and n>=1):
            raise SystemExit('ANCHOR %s: found %d: %r'%(p,n,a[:120]))
        s=s.replace(a2,b2) if count==0 else s.replace(a2,b2,1)
    open(p,'w',encoding='utf-8',newline='').write(s)
