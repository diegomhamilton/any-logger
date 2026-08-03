#!/usr/bin/env python3
"""Small, dependency-free Serori deterministic checks."""
import argparse, hashlib, json, os, re, subprocess, sys
from pathlib import Path

EXIT_OK, EXIT_FINDING, EXIT_INPUT, EXIT_BLOCKED, EXIT_SAFETY = 0, 1, 2, 3, 5

def sha(path):
    h = hashlib.sha256()
    if path.is_file():
        with path.open('rb') as f:
            for chunk in iter(lambda: f.read(65536), b''): h.update(chunk)
    return h.hexdigest()

def root_arg(p):
    root = Path(p).resolve()
    if not root.is_dir(): raise ValueError('project root is not a directory')
    return root

def envelope(kind, project, data, inputs=(), findings=()):
    return {'artifact': {'id': kind + '-' + hashlib.sha256(json.dumps(data, sort_keys=True).encode()).hexdigest()[:16],
                         'schema_version': 1, 'project': project, 'created_by': 'serori-' + kind,
                         'inputs': list(inputs), 'assumptions': [], 'decisions': [], 'risks': [],
                         'evidence': [], 'status': 'draft'}, 'data': data,
            'diagnostics': list(findings),
            'summary': {'passed': 0, 'warnings': 0, 'errors': len(findings), 'skipped': 0}}

def files(root):
    ignored = {'.git', 'build', 'out'}
    return [p for p in root.rglob('*') if p.is_file() and not any(x in ignored for x in p.parts)]

def intake(args):
    root = root_arg(args.project_root)
    ch = (root / args.chibios).resolve() if args.chibios else root / 'libs' / 'ChibiOS'
    revision = None
    if ch.is_dir():
        try: revision = subprocess.check_output(['git','-C',str(ch),'rev-parse','HEAD'], text=True, stderr=subprocess.DEVNULL).strip()
        except (OSError, subprocess.CalledProcessError): pass
    detected = [str(p.relative_to(root)) for p in files(root) if p.name in {'Makefile','halconf.h','chconf.h','mcuconf.h'} or p.suffix in {'.c','.h'}]
    blockers = []
    if not ch.is_dir(): blockers.append('ChibiOS checkout not found')
    if args.strict and not args.board and not args.mcu: blockers.append('target board/MCU not supplied')
    data = {'goal': args.goal, 'target': {'board': args.board, 'mcu': args.mcu, 'toolchain': args.toolchain},
            'chibios': {'root': str(ch.relative_to(root)) if ch.is_relative_to(root) else str(ch), 'revision': revision},
            'constraints': {'memory_bytes': args.memory, 'latency_us': args.latency}, 'protocols': args.protocol,
            'acceptance_criteria': args.acceptance, 'detected_inputs': detected[:500],
            'gate': {'ready_for_mapping': not blockers, 'blockers': blockers}}
    return envelope('project-context', root.name, data, [{'path': str(ch), 'sha256': sha(ch) if ch.is_file() else None}]), EXIT_FINDING if blockers else EXIT_OK

def map_repo(args):
    root = root_arg(args.project_root)
    items=[]
    for p in files(root):
        try: rel=str(p.relative_to(root)); text=p.read_text(errors='ignore') if p.suffix in {'.c','.h','.mk'} or p.name=='Makefile' else ''
        except OSError: continue
        kind='source' if p.suffix=='.c' else 'header' if p.suffix=='.h' else 'build' if 'Makefile' in p.name or p.suffix=='.mk' else 'other'
        items.append({'path':rel,'kind':kind,'sha256':sha(p),'lines':text.count('\n')+1 if text else None})
    needles={'threads':r'\b(?:THD_FUNCTION|chThdCreate\w*)\b','callbacks':r'\b(?:callback|cb)\b','dma':r'\bDMA\w*\b','locks':r'\b(?:chMtx|chSysLock|chSem|chBSem)\w*\b'}
    runtime={k:[] for k in needles}
    for p in files(root):
        if p.suffix not in {'.c','.h'}: continue
        text=p.read_text(errors='ignore')
        for k,pat in needles.items():
            if re.search(pat,text): runtime[k].append(str(p.relative_to(root)))
    data={'files':items,'runtime':runtime,'chibios':str((root/'libs'/'ChibiOS').relative_to(root)) if (root/'libs'/'ChibiOS').exists() else None,
          'blockers': []}
    return envelope('repo-map',root.name,data), EXIT_OK

def evidence(args):
    root=root_arg(args.project_root); diagnostics=[]
    for value in args.path:
        p=(root/value).resolve()
        if not str(p).startswith(str(root)) or not p.exists(): diagnostics.append({'code':'EVIDENCE_PATH','severity':'error','message':str(value)})
    data={'checked_paths':args.path,'gate':{'evidence_complete':not diagnostics,'blockers':[d['message'] for d in diagnostics]}}
    return envelope('evidence',root.name,data), EXIT_FINDING if diagnostics else EXIT_OK

def review(args):
    root=root_arg(args.project_root); findings=[]
    if not (root/'libs'/'ChibiOS').exists(): findings.append({'code':'NO_CHIBIOS','severity':'critical','message':'ChibiOS checkout unavailable'})
    data={'findings':findings,'gate':{'result':'fail' if findings else 'conditional','blocking_findings':[x['code'] for x in findings]}}
    return envelope('review',root.name,data), EXIT_FINDING if findings else EXIT_OK

def main():
    ap=argparse.ArgumentParser(); sub=ap.add_subparsers(dest='cmd',required=True)
    def common(p): p.add_argument('--project-root',required=True); p.add_argument('--output'); p.add_argument('--format',choices=['json','text'],default='json')
    p=sub.add_parser('intake'); common(p); p.add_argument('--goal'); p.add_argument('--board'); p.add_argument('--mcu'); p.add_argument('--toolchain'); p.add_argument('--chibios'); p.add_argument('--memory',type=int); p.add_argument('--latency',type=int); p.add_argument('--protocol',action='append',default=[]); p.add_argument('--acceptance',action='append',default=[]); p.add_argument('--strict',action='store_true'); p.set_defaults(fn=intake)
    p=sub.add_parser('map'); common(p); p.set_defaults(fn=map_repo)
    p=sub.add_parser('evidence'); common(p); p.add_argument('--path',action='append',default=[]); p.set_defaults(fn=evidence)
    p=sub.add_parser('review'); common(p); p.set_defaults(fn=review)
    args=ap.parse_args()
    try: result, code=args.fn(args)
    except ValueError as e: print(json.dumps({'error':str(e)})); return EXIT_INPUT
    out=json.dumps(result,indent=2)
    if args.format=='text': out=result['data'].__repr__()
    if args.output: Path(args.output).write_text(out+'\n')
    else: print(out)
    return code
if __name__=='__main__': sys.exit(main())

