import numpy as np, sys
def analyse(path, fs=48000, frac=1/3):
    x=np.fromfile(path,dtype=np.float32).astype(float)
    seg=x[int(len(x)*(1-frac)):]; seg=seg-seg.mean()
    rms=np.sqrt(np.mean(seg**2))
    if rms<1e-3: return dict(rms=rms,f0=0,h=[])
    n=len(seg); w=np.hanning(n); X=np.abs(np.fft.rfft(seg*w, 16*n)); f=np.fft.rfftfreq(16*n,1/fs)
    # f0 = strongest peak among 40..2000 Hz, then check subharmonic candidates via harmonic sum
    band=(f>40)&(f<2000)
    cands=[]
    for f0 in np.arange(40,2000,0.05)[::20]:
        pass
    i=np.argmax(X*band); fpk=f[i]
    # harmonic-sum f0 estimator over candidates fpk/k
    best=None
    for k in range(1,6):
        fc=fpk/k
        if fc<40: break
        idx=[np.argmax(X[(f>fc*m*0.97)&(f<fc*m*1.03)]) for m in range(1,9)]
        lev=[X[(f>fc*m*0.97)&(f<fc*m*1.03)].max() for m in range(1,9)]
        # a true fundamental must have energy at fc itself
        if lev[0] > 0.05*X.max(): best=fc
    f0=best
    # refine f0 using the peak near f0
    sel=(f>f0*0.97)&(f<f0*1.03); j=np.argmax(X*sel); f0r=f[j]
    h=[20*np.log10(X[(f>f0r*m*0.98)&(f<f0r*m*1.02)].max()+1e-12) for m in range(1,9)]
    h=[round(v-max(h),1) for v in h]
    return dict(rms=round(rms,1), f0=round(f0r,2), h=h)
if __name__=='__main__':
    for p in sys.argv[1:]: print(p, analyse(p))
