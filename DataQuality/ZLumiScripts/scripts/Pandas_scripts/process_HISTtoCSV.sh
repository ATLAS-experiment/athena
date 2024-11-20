#!/usr/bin/env bash

if [[ $# -lt 1 ]]; then
    echo "ERROR: Need to give year to process: 22, 23, 24, or 25"
    exit 1
fi
if [[ $1 -ge 22 && $1 -le 24 ]]; then
    year=$1
else
    echo "ERROR: Bad year $1 given to process: use 22, 23, 24, 25"
    exit 1
fi

campaign=mc23a # could be improved
dataset=data${year}_13p6TeV

GRLCVMFS=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists
indir=/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/${dataset}/
outdir=/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/${dataset}/physics_Main/

if [[ $year -eq 22 ]]; then
    grl=${GRLCVMFS}/data22_13p6TeV/20230207/data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns.xml
    lumitag=OflLumi-Run3-003
    tag=OflLumiAcct-Run3-003
elif [[ $year -eq 23 ]]; then
    grl=${GRLCVMFS}/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml
    lumitag=OflLumi-Run3-004
    tag=OflLumiAcct-Run3-004
elif [[ $year -eq 24 ]]; then
    grl=${GRLCVMFS}/data24_13p6TeV/20241118/data24_13p6TeV.periodsEtoO_DetStatus-v130-pro36-08_MERGED_PHYS_StandardGRL_All_Good_25ns.xml
    lumitag=OflLumi-Run3-005
    tag=OflLumiAcct-Run3-005
elif [[ $year -eq 25 ]]; then
    grl=${indir}/latest_GRL.xml
    lumitag=OflLumi-Run3-006
    tag=OflLumiAcct-Run3-006
fi
if [[ $# -ge 3 ]]; then
    grl=$3
fi

echo "INFO: Processing year 20${year} with GRL ${grl} and MCCF ${campaign} from ${indir} to ${outdir}"
if [[ $# -lt 2 ]] || [[ $2 -gt 0 ]]; then
    update=1
    echo "INFO: Will keep existing CSVs and only add new ones"
else
    echo "INFO: Will overwrite all CSVs on Z counting EOS"
    update=0
fi

for infilename in `ls $indir |grep ".*HIST.*.root"`; do
    run=${infilename//[^0-9]/}
    if [[ -f `echo ${outdir}*${run}*` ]] && [[ $update == 1 ]]; then
	echo "CSV for run $run already exists, moving on"
	continue
    fi
    python -u dqt_zlumi_pandas.py --tag $tag --lumitag $lumitag --dblivetime --useofficial --grl $grl --infile ${indir}${infilename} --campaign $campaign --outdir ${outdir}
done
