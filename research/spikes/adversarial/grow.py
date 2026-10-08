import numpy as np,sys
for p in sys.argv[1:]:
    x=np.fromfile(p,dtype=np.float32).astype(float); fs=48000
    r=[np.sqrt(np.mean((x[i:i+4800]-x[i:i+4800].mean())**2)) for i in range(0,len(x)-4800,int(0.5*fs))]
    print(p,[round(v,2) for v in r])
