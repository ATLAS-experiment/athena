#!/bin/bash


indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/"
outdir="/eos/home-j/jnewell/data23_13p6TeV_physics_plot_customgrl3/yearwise_run3/"
2022_dir="data22_13p6TeV/physics_Main_MC23a/"
2023_dir="data23_13p6TeV/physics_Main_customgrl/"

for year in run3
do

    fi
    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    echo python -u ../../python/plotting/yearwise_luminosity.py --year $year --comp --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
    echo python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --year $year --comp --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
    for channel in Zee Zmumu Zll
    do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        echo python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
        echo python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --absolute --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
	echo python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
	echo python -u ../../python/plotting/yearwise_efficiency.py --channel $channel --year $year --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
	echo python -u ../../python/plotting/yearwise_efficiency_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir --2022_dir $2022_dir --2023_dir $2023_dir
    done
done

