import json, subprocess, sys, os, numpy as np
sys.path.insert(0,'/home/claude/projects/trumpet-vst/research/spikes')
import regime_map as rm
from an import analyse
SIM = sys.argv[1]  # 'lipsim' or 'rk4'
subset = sys.argv[2] if len(sys.argv)>2 else 'all'
table=json.load(open('/home/claude/projects/trumpet-vst/research/spikes/table_draft.json'))
res=json.load(open('/home/claude/projects/trumpet-vst/research/spikes/regime_map_result.json'))
out=[]
for note,row in zip(table['notes'],res):
    if subset!='all' and note['written'] not in [int(x) for x in subset.split(',')]: continue
    st,n=note['state'],note['partial']
    fres=[m['f_hz'] for m in table['states'][st]]
    pf=rm.poles_file(table,st)
    fl0=rm.ratio(n)*fres[n-1]
    r={'w':note['written'],'v':note['valves'],'n':n,'kind':note['kind']}
    for dyn,k in rm.DYN.items():
        pm=k*rm.pth(n); wav=f'/tmp/claude-1000/-home-claude-projects-trumpet-vst/f855d006-2b49-5a65-8b7d-22066c71447d/scratchpad/rv/w_{SIM}.raw'
        if SIM=='lipsim':
            subprocess.run(['./lipsim',pf,f'fl={fl0}',f'pm={pm}','Ql=20','mu=9','b=12e-3',f'H={rm.h0(n)}','dur=0.8','attack=0.003','yinit=0','fmax=2000','fmin=40',f'wav={wav}','raw=1'],capture_output=True)
        else:
            subprocess.run(['./rk4sim',pf,wav,f'fl={fl0}',f'pm={pm}','Ql=20','mu=9','b=12e-3',f'H={rm.h0(n)}','dur=0.8','attack=0.003','yinit=0','fsint=1.92e6'],capture_output=True)
        a=analyse(wav)
        part = rm.partial_of(a['f0'],fres) if a['rms']>20 else 0
        r[dyn]=(part, a['f0'], a['rms'], row[f'{dyn}_partial'])
    out.append(r); print(r, flush=True)
json.dump(out,open(f'recheck_{SIM}.json','w'))
