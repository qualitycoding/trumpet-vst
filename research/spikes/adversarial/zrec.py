import numpy as np
d=np.loadtxt('/home/claude/projects/trumpet-vst/research/spikes/freour2022_open_modes.txt',skiprows=1)
s=d[:,0]+1j*d[:,1]; C=d[:,2]+1j*d[:,3]
f=np.linspace(20,2000,200000); w=2j*np.pi*f
Z=sum(c/(w-sk)+np.conj(c)/(w-np.conj(sk)) for c,sk in zip(C,s))
m=20*np.log10(abs(Z))
from scipy.signal import argrelmax
i=argrelmax(m)[0]
print([(round(f[j],1),round(m[j],1)) for j in i])
print('min near 120Hz', f[np.argmin(m[(f>80)&(f<200)])+np.argmax(f>80)], m[(f>80)&(f<200)].min())
print('|Z| at 2000Hz dB',m[-1],'phase',np.angle(Z[-1]))
# implied Zc if C2022 = Zc*C2020 with |C4|2020 ~ 2245-2492
for c4 in (2245,2492): print('Zc implied',abs(C[3])/c4)
rho,c=1.2041*293.15/300.15, 331.3*np.sqrt(1+27/273.15)
for zc in (abs(C[3])/2245,abs(C[3])/2492): print('r mm', 1e3*np.sqrt(rho*c/np.pi/zc))
