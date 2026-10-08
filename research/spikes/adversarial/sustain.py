import json,subprocess,sys
sys.path.insert(0,'/home/claude/projects/trumpet-vst/research/spikes')
import regime_map as rm
from concurrent.futures import ThreadPoolExecutor
t=json.load(open('/home/claude/projects/trumpet-vst/research/spikes/table_draft.json'))
res=json.load(open('/home/claude/projects/trumpet-vst/research/spikes/regime_map_result.json'))
jobs=[]
for note,row in zip(t['notes'],res):
    st,n=note['state'],note['partial']; fres=[m['f_hz'] for m in t['states'][st]]
    for dyn,k in rm.DYN.items():
        jobs.append((note['written'],note['valves'],n,note['kind'],dyn,row[f'{dyn}_partial'],st,fres,k*rm.pth(n)))
def run(j):
    w,v,n,kind,dyn,claimed,st,fres,pm=j
    o=subprocess.run(['./lipsim',rm.poles_file(t,st),f'fl={rm.ratio(n)*fres[n-1]}',f'pm={pm}','Ql=20','mu=9','b=12e-3',f'H={rm.h0(n)}','dur=4','attack=0.003','yinit=0','track=1','fmax=2000','fmin=40'],capture_output=True,text=True).stdout.splitlines()
    rows=[json.loads(l) for l in o]
    def at(tt): return min(rows,key=lambda r:abs(r['t']-tt))
    r08,r4=at(0.7),rows[-1]
    return (w,v,n,kind,dyn,claimed,round(r08['rms']),round(r4['rms']),rm.partial_of(r4['f0'],fres) if r4['rms']>20 else 0)
with ThreadPoolExecutor(8) as ex: out=list(ex.map(run,jobs))
bad=[o for o in out if o[5]!=o[8] or (o[7]<0.5*o[6])]
for b in bad: print(b)
print('total',len(out),'flagged',len(bad))
