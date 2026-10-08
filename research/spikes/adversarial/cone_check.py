import sys, numpy as np
sys.path.insert(0,'/home/claude/projects/trumpet-vst/research/spikes')
import tmm_trumpet as t
from scipy.integrate import solve_ivp
t.gamma = lambda f, r: 2j*np.pi*f/t.C0   # lossless
rho,c=t.RHO,t.C0
def webster(f,r1,r2,L):
    # integrate dp/dx = -jw rho/S U, dU/dx = -jw S/(rho c^2) p from output (x=L) to input (x=0), two unit columns
    w=2*np.pi*f; S=lambda x: np.pi*(r1+(r2-r1)*x/L)**2
    def rhs(x,y):
        p=y[0]+1j*y[1]; U=y[2]+1j*y[3]
        dp=-1j*w*rho/S(x)*U; dU=-1j*w*S(x)/(rho*c*c)*p
        return [dp.real,dp.imag,dU.real,dU.imag]
    M=np.zeros((2,2),complex)
    for col,(p0,U0) in enumerate(((1,0),(0,1))):
        s=solve_ivp(rhs,(L,0),[p0,0,U0,0],rtol=1e-11,atol=1e-14)
        y=s.y[:,-1]; M[0,col]=y[0]+1j*y[1]; M[1,col]=y[2]+1j*y[3]
    return M
def ck(f,r1,r2,L):
    k=2*np.pi*f/c; x1=r1*L/(r2-r1); x2=x1+L
    A=(x2/x1)*np.cos(k*L)-np.sin(k*L)/(k*x1)
    D=(x1/x2)*np.cos(k*L)+np.sin(k*L)/(k*x2)
    return A,D
worst=0
for (r1,r2,L) in ((8.25e-3,1.83e-3,0.006),(1.83e-3,4.4e-3,0.065),(5.84e-3,61.5e-3,0.5),(0.03,0.0615,0.05),(5e-3,5.3e-3,0.3)):
    for f in (50,233,466,1000,1500,2000):
        Mt=t.cone(np.array([f]),r1,r2,L)[:,:,0] if t.cone(np.array([f]),r1,r2,L).ndim==3 else t.cone(np.array([f]),r1,r2,L).reshape(2,2)
        Mw=webster(f,r1,r2,L)
        err=np.max(np.abs(Mt-Mw)/np.maximum(np.abs(Mw),1e-30*0+np.abs(Mw).max()*1e-6))
        A,B=ck(f,r1,r2,L) if abs(r2-r1)>1e-7 else (np.nan,np.nan)
        worst=max(worst,err)
        print(f"r1={r1} r2={r2} L={L} f={f}: max rel err TMM vs Webster {err:.2e};  A: {Mt[0,0]:.6f} vs CK {A:.6f};  D: {Mt[1,1]:.6f} vs CK {B:.6f}; det {np.linalg.det(Mt):.6f}")
print('worst',worst)
