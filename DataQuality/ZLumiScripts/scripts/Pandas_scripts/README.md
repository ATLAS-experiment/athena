# Introduction

This is the main repository of processing scripts for the Z counting analysis framework. This includes the conversion of histograms to CSV files and the subsequent plotting scripts for producing luminosity and efficiency plots over both individual fills and entire years.

# Analysis framework

The Analysis framework for ZC in Run3 is maintained in three separate locations:

1. The core analysis is performed in the athena framework under DataQualityTools (https://gitlab.cern.ch/atlas/athena/-/tree/main/DataQuality/DataQualityTools). Here Z events are selected and histograms produced in order to calculate the Zlumi and data-driven single-lepton efficiencies.

2. The monte carlo correction factors are calculated in a separate git project (https://gitlab.cern.ch/z-counting/monte-carlo). These values are then used in the creation of csv files.

3. CSV files are created in the athena framework under ZlumiScripts (https://gitlab.cern.ch/atlas/athena/-/tree/main/DataQuality/ZLumiScripts/scripts/Pandas_scripts). These csv files are created using the histograms produced by the DQTGlobalWZFinderAlg with the monte carlo correction factors produced separately. Here there are also scripts to produce plots for the luminosity and efficiency using the csv files.

This framework runs over the merged outputs produced by running the DQTGlobalWZFinder algorithm over primary AODs. Information on how to run the main code can be found here [https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi).

Once the merging step has been performed (an example of which is below), the dqt_zlumi_pandas.py script can be ran over the merged output to produce a single csv file containing all information.
 
Each row of the csv file corresponds to a single luminosity block, and contains all information for both channels; such as the number of reconstructed Zs per channel, trigger efficiency, reconstruction efficiency, luminosity, as well as the arithmeric mean of the Zee and Zmumu luminosities and all axuilliary official information (livetime, pileup, luminosity, GRL).
```
ls <grid_output>/* > tomerge.txt
DQHistogramMerge.py tomerge.txt tree_<run_number>.root
```
# EOS space

The Run 3 outputs can be found at the following directories:

| Files 	 | Location 								 | Information 					 |
| :--- 	 	 | :--- 								 | :--- 					 |
| Grid outputs 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/GridOutputs/	 | Raw outputs as we get them from the grid jobs |
| Merged outputs | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/ | After DQHistogramMerge step 			 |
| CSV outputs 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/	 | Single csv file produced per run 		 |

## Note on GRL for 2023 data taking
Official GRLS may be found in: /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data23_13p6TeV/

For a more up-to-date preliminary grl, the script grl_maker.py is included. This loops through a custom list of runs, producing a .xml file for each run producing Lumi Block ranges that contain good physics data. These can then be merged into a single grl by moving all files to a single directory and running merge_goodrunslists e.g.:

```
merge_goodrunslists grls/
```
There are two files which must be changed when using a different GRL. The first is in run_code.sh where the grl used in dqt_zlumi_pandas.py can be specified for each year. The second is in the python_tools.py script in the python/plotting/ directory, which is used by many of the plotting scripts.

For 2023, the default is the latest official GRL, found at:

Official GRL - /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml (up to run 456749)

The preliminary GRL can be used when the GRL for the most recent runs had not been released and is made using the method shown above. The latest version can be found at:

Preliminary GRL - /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/data23_13p6TeV_grl.xml (up to run 456685)

## Note on CSVOutputs for 2023:

2023 CSV Outputs are divided into two directories:

physics_Main_customgrl/

physics_Main_officialgrl/

This contains CSV files processed with MC23a MCCFs using either the preliminary GRL (used to stay up to date with latest ATLAS runs) or the official GRL.
  
Preliminary GRL - /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/data23_13p6TeV_grl.xml (up to run 456685)

Official GRL - /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml (up to run 456749)

## Note on Merged Outputs for 2023:

The Merged Outputs directory contains 2022 and 2023 data separately: MergedOutputs/HighMu/data22_13p6TeV/ and MergedOutputs/HighMu/data23_13p6TeV/
Files stored in data23_13p6TeV/ can be gathered from multiple locations:
1. The rucio eos directory. For the latest 2023 physics data this is located at: /eos/atlas/atlastier0/rucio/data23_13p6TeV/physics_Main/0045*/data23_13p6TeV.*.physics_Main.merge.HIST.f*/
Older files are preiodically removed from here so files are gathered by other means.
2. Download from the grid. Physics_main hist files are also stored on the grid and can be downloaded and used as they are. It is recommended to copy only the DQTGlobalWZFinder folder from the root files to save space. Scripts are provided to simplify this (process_rucio_files.sh and copySelective.py)
3. Manually processing AOD files using the WZFinder (Z-counting) framework. This is used in the special case that the Z-counting is not functioning automatically at Tier-0. For example, when electron triggers were changed (from run 451896 onwards) the trigger lists were not immediately changed in the WZFinder script. This resulted in a number of runs missing Z Lumi data in the electron channel (451896, 451936, 451949, 452028, 452163, 452202, 452241, 452463, 452533, 452573, 452624, 452640, 452660, 452669, 452696, 452726, 452785, 452787, 452799, 452843, 452872)

# Running the HIST to CSV code
Using a single 2022 run (tree_430580.root) as an illustrative example:
```
source setup.sh
grl="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunLists/data22_13p6TeV/20220902/data22_13p6TeV.periodF_DetStatus-v108-pro28_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"
infile="tree_430580.root"
campaign=mc23a
outdir="<your_output_directory>"
update=on

python -u dqt_zlumi_pandas.py --dblivetime --useofficial --grl $grl --infile $infile --campaign $campaign --outdir $outdir --update $update
```
campaign: Determines the Monte Carlo Correction Factors to be used in the code. These are found in zlumi_mc_cf.py in the python/tools/ directory.

update: Set to "on" to run over all available hist files and overwrite any csv files currently in the outdir. Set to off to only run over hist files that don't have csv files in the outdir.

# Running over the entire dataset
_Note_: The input (indir) and output (out_dir) directories will need to be changed at the top of the script run_code.sh.
```
batch=local # Set to "local" to run all jobs locally in a loop

for year in 23
do
    ./run_code.sh $year $batch $update
done
```

# Making single run plots
*Note*: The output directory (outdir) will need to be changed inside plotting/efficiency.py and plotting/luminiosity.py. Both of these scripts calculate an average over successive bunches of 20 luminosity blocks to increase statistical precision. All other plots use the single-LB luminosity, and not the 20 LB merged value, when calculating the integrated luminosity of an LHC fill/pileup bin.
```
# Time dependent efficiency and luminosity plots
python plotting/efficiency.py --infile <input_csv_file> --outdir <output directory>
python plotting/luminosity.py --infile <input_csv_file> --outdir <output directory>
python plotting/luminosity.py --absolute --infile <input_csv_file> --outdir <output directory>

# Pileup dependent efficiency and luminosity plots
python plotting/efficiency.py --usemu --infile <input_csv_file> --outdir <output directory>
python plotting/luminosity.py --usemu --infile <input_csv_file> --outdir <output directory>

# Kinematic plots - this plotting script takes the histograms in the merged root file rather than the produced csv
python plotting/plot_kinematics.py --infile $infile
```

## Making plots for the entire Run 3 dataset
_Note_: The input (indir) and output (outdir) directories will need to be changed in plot_yearwise.sh.

As well as defining input and output directories, individual input directories must be given for each year, i.e. dir_2022, dir_2023.

 This is laid out such that dir_2022 and dir_2023 share a parent directory but data for each year is stored in separate subdirectories. 

For example, Run 3 data is stored in the CSVOutputs directory with subdirectoried data22_13p6TeV/ and data23_13p6TeV/.

Produce plots covering the full 2022, 2023 or Run 3 datasets by using plot_yearwise.sh.

This provides distributions showing the time and pileup dependence of luminosity across the full year (or years) as well as time and pileup dependence of single lepton trigger and reconstruction efficiencies.

The code can loop through individual years using:
```
for year in 22 23
```
Or it can run over the full Run 3 dataset using:
```
for year in run3
```

The full list of commands then loops across each channel as well as making channel comparison plots between electron and muon channels:
```
for year in 23
do
    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    python -u plotting/yearwise_luminosity.py --year $year --comp --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
    python -u plotting/yearwise_luminosity_vs_mu.py --year $year --comp --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
    for channel in Zee Zmumu Zll
    do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        python -u plotting/yearwise_luminosity.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
        python -u plotting/yearwise_luminosity.py --channel $channel --year $year --absolute --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
        python -u plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
	python -u ../../python/plotting/yearwise_efficiency.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
	python -u ../../python/plotting/yearwise_efficiency_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir --dir_2022 $dir_2022 --dir_2023 $dir_2023
    done
done
```

This can also be done by running:
```
./plot_yearwise.sh
```

# Making separate sets of plots for all runs in one directory at once
__Note_: The input (indir) and output (outdir) directories will need to be changed in this script.

```
./plot_all.sh
```
Loops over all .csv files in a given directory and plots efficiency and luminosity against LB and pileup.