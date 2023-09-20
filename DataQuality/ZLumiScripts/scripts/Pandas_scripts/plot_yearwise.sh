#!/bin/bash


indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data23_13p6TeV/temp/physics_Main_customgrl3/"
outdir="/eos/home-j/jnewell/data23_13p6TeV_physics_plot_customgrl3/yearwise_run3/"

for year in run3
do
    if [ $year == 22 ];
    then 
	#indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/"
	indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/temp/physics_Main_MC23a/"
    elif [ $year == 23 ];
    then 
	indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data23_13p6TeV/temp/physics_Main_customgrl3/"
    fi
    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    python -u ../../python/plotting/yearwise_luminosity.py --year $year --comp --indir $indir --outdir $outdir
    python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --year $year --comp --indir $indir --outdir $outdir
    for channel in Zee Zmumu Zll
    do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --indir $indir --outdir $outdir
        python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --absolute --indir $indir --outdir $outdir
	python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir
	python -u ../../python/plotting/yearwise_efficiency.py --channel $channel --year $year --indir $indir --outdir $outdir
	python -u ../../python/plotting/yearwise_efficiency_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir
    done
done

