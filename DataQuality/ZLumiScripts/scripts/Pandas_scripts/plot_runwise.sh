#!/bin/bash

# script to re-create all run-dependent Z counting plots for a full year
# run as
# ./plot_runwise.sh year [update]
# where year is a required argument of the data taking year (22, 23, 24, 25)
# update=0/1 is an option to skip runs for with the plot directory exists

baseindir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs"
baseoutdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots"

if [[ $# -ge 1 ]]; then
    year=$1
    echo "INFO: Redoing run-wise plots for Year $year"
else
    echo "ERROR: Usage: ./plot_yearwise.sh year"
    echo "ERROR: where year is a required argument of the data taking year (22, 23, 24, 25)"
    exit 1
fi

if [[ $# -lt 2 ]] || [[ $2 -gt 0 ]]; then
    update=1
    echo "INFO: Will keep Plots for existing runs and only add new ones"
else
    echo "INFO: Will overwrite all run-dependent Plots on Z counting EOS"
    update=0
fi

dataset=data${year}_13p6TeV
indir=${baseindir}/${dataset}/physics_Main/

filelist=$(ls $indir)
for file in $filelist; do
    filename=${file/run_/}
    run_number=${filename/.csv/}

    outdir=${baseoutdir}/${dataset}/${run_number}
    
    if [[ -d ${outdir} ]] && [[ $update == 1 ]]; then
	echo "Directory for run $run_number already exists, skipping"
	continue
    fi

    echo Plotting for Run $run_number
    mkdir -p $outdir

    # LB-dependence
    python ../../python/plotting/efficiency.py --infile $indir$file --outdir $outdir
    python ../../python/plotting/luminosity.py --infile $indir$file --outdir $outdir
    python ../../python/plotting/luminosity.py --absolute --infile $indir$file --outdir $outdir

    # mu-dependence
    python ../../python/plotting/efficiency.py --usemu --infile $indir$file --outdir $outdir
    python ../../python/plotting/luminosity.py --usemu --infile $indir$file --outdir $outdir

done

