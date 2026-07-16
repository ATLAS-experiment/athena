#!/bin/bash
# ----------------------------------------------------------------------
# RunITkDQMonitoringTest.sh
#
# End-to-end test of the ITk DQ monitoring (ITkAlignmentMonitoring +
# ITkGlobalMonitoring): runs Reco_tf RAWtoALL over a Phase-II MC RDO
# with only the InDet DQ enabled, then runs the han checks and prints
# a summary of the histogram inventory and check verdicts.
#
# Prerequisites (run these first, from your work area):
#   setupATLAS
#   asetup Athena,24.0,latest   (or asetup --restore in the work dir)
#   source <build>/x86_64-*/setup.sh    <-- REQUIRED: without the
#         WorkDir setup the release versions of the monitoring
#         packages are silently used instead of this branch!
#
# Usage:
#   RunITkDQMonitoringTest.sh [-n NEVENTS] [-o OUTDIR]
#     -n NEVENTS   number of events to process (default 10)
#     -o OUTDIR    output/run directory (default ./itk-dq-test)
#
# Notes:
#   * The transform exits with code 66 ("events_lb" count check); this
#     is benign here because global/dataflow monitoring is disabled.
#   * han MUST be given the run directory (run_XXXXXX) as its third
#     argument and a local (non-EOS-fuse) input file; this script
#     handles both.
# ----------------------------------------------------------------------
set -u

NEVENTS=10
OUTDIR="itk-dq-test"
while getopts "n:o:h" opt; do
  case $opt in
    n) NEVENTS=$OPTARG ;;
    o) OUTDIR=$OPTARG ;;
    h) grep '^#' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) exit 1 ;;
  esac
done

# ---- environment sanity checks ---------------------------------------
if [ -z "${AtlasVersion:-}" ]; then
  echo "ERROR: no Athena release set up (run setupATLAS + asetup first)" >&2
  exit 1
fi
if [ -z "${WorkDir_DIR:-}" ]; then
  echo "ERROR: WorkDir not sourced - run 'source <build>/x86_64-*/setup.sh'." >&2
  echo "       Without it the RELEASE monitoring packages are used, not this branch." >&2
  exit 1
fi
if ! python -c "from ITkAlignmentMonitoring import ITkAlignmentMonitoringConfig" 2>/dev/null; then
  echo "ERROR: ITkAlignmentMonitoring python not importable - is the branch built?" >&2
  exit 1
fi

HCFG="$WorkDir_DIR/data/DataQualityConfigurations/collisions_run.hcfg"
if [ ! -f "$HCFG" ]; then
  echo "WARNING: $HCFG not found (DataQualityConfigurations not built?); han step will be skipped" >&2
  HCFG=""
fi

RDO='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1'

mkdir -p "$OUTDIR"
cd "$OUTDIR" || exit 1
echo "=== ITk DQ monitoring test: $NEVENTS events, output in $PWD ==="

# ---- 1) reconstruction with InDet DQ monitoring -----------------------
# Enable DQ but steer off every non-InDet system; disable trigger
# (Phase II RDO has no trigger menu metadata).
PREEXEC='all:'
PREEXEC+='flags.Reco.EnableTrigger=False;'
PREEXEC+='flags.Trigger.triggerConfig="FILE";'
PREEXEC+='flags.DQ.doMonitoring=True;'
PREEXEC+='flags.DQ.useTrigger=False;'
for f in doLVL1CaloMon doLVL1InterfacesMon doCTPMon doHLTMon \
         doPixelMon doSCTMon doTRTMon \
         doLArMon doTileMon doCaloGlobalMon doMuonMon \
         doLucidMon doAFPMon doZDCMon \
         doHIMon doEgammaMon doJetMon doMissingEtMon doJetInputsMon \
         doTauMon doJetTagMon doDataFlowMon doGlobalMon; do
  PREEXEC+="flags.DQ.Steering.${f}=False;"
done
PREEXEC+='flags.DQ.Steering.doInDetMon=True;'
PREEXEC+='flags.DQ.Steering.InDet.doGlobalMon=True;'
PREEXEC+='flags.DQ.Steering.InDet.doAlignMon=True;'
PREEXEC+='flags.DQ.Steering.InDet.doPerfMon=False;'

Reco_tf.py \
  --maxEvents "$NEVENTS" \
  --autoConfiguration 'everything' \
  --conditionsTag 'default:OFLCOND-MC21-SDR-RUN4-03' \
  --postInclude 'all:PyJobTransforms.UseFrontier' \
  --preInclude 'all:Campaigns.PhaseIIPileUp200' \
  --steering 'doRAWtoALL' \
  --preExec "$PREEXEC" \
  --inputRDOFile "$RDO" \
  --outputHISTFile 'ITkDQ.HIST.root' > reco.log 2>&1
rc=$?
if [ $rc -ne 0 ] && [ $rc -ne 66 ]; then
  echo "ERROR: Reco_tf failed with exit code $rc (66 is the only tolerated non-zero); see $PWD/reco.log" >&2
  exit $rc
fi
[ $rc -eq 66 ] && echo "(Reco_tf exit 66: benign events_lb count check)"
if [ ! -f ITkDQ.HIST.root ]; then
  echo "ERROR: no HIST output produced; see $PWD/reco.log" >&2
  exit 1
fi
echo "HIST written: $PWD/ITkDQ.HIST.root"

# ---- 2) han checks -----------------------------------------------------
RUNDIR=$(python - <<'EOF'
import ROOT
f = ROOT.TFile.Open('ITkDQ.HIST.root')
print(next(k.GetName() for k in f.GetListOfKeys() if k.GetName().startswith('run_')))
EOF
)
if [ -n "$HCFG" ]; then
  # han needs a local file (not EOS fuse) and the run dir as path argument
  TMPDIR_LOCAL=$(mktemp -d /tmp/${USER}_itkdq.XXXXXX)
  cp ITkDQ.HIST.root "$TMPDIR_LOCAL/"
  ( cd "$TMPDIR_LOCAL" && han "$HCFG" ITkDQ.HIST.root "$RUNDIR" > han.log 2>&1 )
  if [ -f "$TMPDIR_LOCAL/ITkDQ.HIST_han.root" ]; then
    cp "$TMPDIR_LOCAL/ITkDQ.HIST_han.root" ./ITkDQ.HAN.root
    cp "$TMPDIR_LOCAL/han.log" .
    echo "han result written: $PWD/ITkDQ.HAN.root"
  else
    echo "WARNING: han produced no output; see $TMPDIR_LOCAL/han.log" >&2
  fi
  rm -rf "$TMPDIR_LOCAL"
fi

# ---- 3) summary --------------------------------------------------------
python - "$RUNDIR" <<'EOF'
import sys
import ROOT
rundir = sys.argv[1]

print()
print('================ ITk DQ monitoring summary ================')
f = ROOT.TFile.Open('ITkDQ.HIST.root')
stats = {}
def walk(d, path=""):
    for k in d.GetListOfKeys():
        o = k.ReadObj()
        if o.InheritsFrom('TDirectory'):
            walk(o, path + '/' + k.GetName())
        elif o.InheritsFrom('TH1'):
            t = stats.setdefault(path, [0, 0])
            t[0] += 1
            if o.GetEntries() == 0:
                t[1] += 1
walk(f.Get(rundir))
tot = emp = 0
for d in sorted(stats):
    n, e = stats[d]
    tot += n; emp += e
    print(f'  {d}: {n} histograms' + (f' ({e} EMPTY)' if e else ''))
print(f'  TOTAL: {tot} histograms, {emp} empty')

import os, json
if os.path.exists('ITkDQ.HAN.root'):
    from collections import Counter
    h = ROOT.TFile.Open('ITkDQ.HAN.root')
    counts = Counter()
    nongreen = []
    def walk2(d, p=''):
        for k in d.GetListOfKeys():
            if k.GetClassName() == 'TDirectoryFile':
                walk2(k.ReadObj(), p + '/' + k.GetName())
            elif k.GetName() == 'Results':
                try:
                    s = json.loads(str(k.ReadObj().GetString())).get('Status')
                except Exception:
                    return
                counts[s] += 1
                if s in ('Red', 'Yellow') and p.endswith('_'):
                    nongreen.append((p, s))
    for tree in ('InnerDetector/ITkAlignment', 'InnerDetector/ITkGlobal'):
        d = h.Get(tree)
        if d: walk2(d, tree)
    print()
    print('  han check verdicts (ITk trees):', dict(counts))
    print('  (Undefined = display-only GatherData entries, no check attached)')
    for p, s in nongreen:
        print(f'    {s}: {p}')
print('============================================================')
EOF
