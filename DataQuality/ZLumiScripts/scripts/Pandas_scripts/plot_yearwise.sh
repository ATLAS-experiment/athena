#!/bin/bash


indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/"
outdir="/eos/home-j/jnewell/data23_13p6TeV_physics_plot_officialgrl/yearwise_run3/"
dir_2022="data22_13p6TeV/physics_Main_MC23a/"
dir_2023="data23_13p6TeV/physics_Main_officialgrl/"

for year in run3
do

    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    python -u ../../python/plotting/yearwise_luminosity.py --year $year --comp --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
    python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --year $year --comp --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
    for channel in Zee Zmumu Zll
    do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
        python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --absolute --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
	python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
	python -u ../../python/plotting/yearwise_efficiency.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
	python -u ../../python/plotting/yearwise_efficiency_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
    done
done

