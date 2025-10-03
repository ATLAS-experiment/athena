inputRDO="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/trig-val/PhaseIIUpgrade/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1"
xclbinPath="/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F110/kernels.hw.xclbin"
bdfid="0000:c3:00.1"

nEvents="100"
threads=1

## parsing flags
while [ $# -ge 1 ];do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage ; fi ; inputRDO="$2"  ; shift ;;
        -x  | --xclbin )        if [ $# -lt 2 ] ; then usage ; fi ; xclbinPath="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage ; fi ; nEvents="$2"   ; shift ;;
        -b  | --bdfid )         if [ $# -lt 2 ] ; then usage ; fi ; bdfid="$2" ; shift ;;
        -t  | --threads )       if [ $# -lt 2 ] ; then usage ; fi ; threads=${2} ; shift ;;
        -h  | --help )          usage 0 ;;
        *) shift ;;
        esac
        shift
    done

## checking valid inputs
if [ -z "$inputRDO" ]; then usage ; fi

athena.py --imf --perfmon=fastmonmt --threads=${threads} --evtMax=${nEvents}  --filesInput=${inputRDO} TriggerJobOpts/runHLT.py Trigger.enabledSignatures=[\"Muon\",\"Jet\",\"Tau\",\"MET\"] FPGADataPrep.doCodeType=F1X0 Trigger.EFTrackPipeline=\"F100\" Trigger.useActsTracking=True ITk.doTruth=False Tracking.doTruth=False IOVDb.GlobalTag=OFLCOND-MC21-SDR-RUN4-03 Acts.doMonitoring=True Exec.FPE=1 FPGADataPrep.doF110=True FPGADataPrep.bdfID=\"${bdfid}\" FPGADataPrep.xclbin=\"${xclbinPath}\