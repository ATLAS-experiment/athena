#!/bin/bash

#script to make all plots

indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data23_13p6TeV/physics_Main_officialgrl/"
outdir="/eos/home-j/jnewell/data23_13p6TeV_physics_plot_customgrl3/"

filelist=$(ls $indir)
for file in $filelist;
do
	
	filename=${file/run_/}
	run_number=${filename/.csv/}

	mkdir -p $outdir$run_number

	# Kinematic plots
	#python plotting/plot_kinematics.py --infile $infile
	
	# Time dependent efficiency and luminosity plots
	python ../../python/plotting/efficiency.py --infile $indir$file --outdir $outdir
	python ../../python/plotting/luminosity.py --infile $indir$file --outdir $outdir
        python ../../python/plotting/luminosity.py --absolute --infile $indir$file --outdir $outdir

	# Pileup dependent efficiency and luminosity plots
	python ../../python/plotting/efficiency.py --usemu --infile $indir$file --outdir $outdir
	python ../../python/plotting/luminosity.py --usemu --infile $indir$file --outdir $outdir

done

