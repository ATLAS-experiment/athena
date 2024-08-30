#!/bin/bash

indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/"
baseoutdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/"

yearlist=(24 run3)
if [[ $# -ge 1 ]]; then
    yearlist=($@)
fi

for year in ${yearlist[@]}; do
    if [[ $year == "run3" ]]; then
	outdir=${baseoutdir}
    else
	outdir=${baseoutdir}/data${year}_13p6TeV/
    fi

    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    python -u ../../python/plotting/yearwise_luminosity.py --year $year --comp --indir $indir --outdir $outdir
    python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --year $year --comp --indir $indir --outdir $outdir
    
    for channel in Zee Zmumu Zll; do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --indir $indir --outdir $outdir --outcsv
        python -u ../../python/plotting/yearwise_luminosity.py --channel $channel --year $year --absolute --indir $indir --outdir $outdir --outcsv

	python -u ../../python/plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir
    done

    for channel in Zee Zmumu; do
        # Yearwise Efficiencies vs. time and pileup
	python -u ../../python/plotting/yearwise_efficiency.py --channel $channel --year $year --indir $indir --outdir $outdir
	python -u ../../python/plotting/yearwise_efficiency_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir
    done
done

