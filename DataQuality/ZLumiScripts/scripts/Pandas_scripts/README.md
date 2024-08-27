# Introduction

This is the main repository of processing scripts and documentation for the Z counting analysis framework.

Scripts provided here cover (roughly in sequence of execution):
* storage of reduced histogram outputs to our repository
* conversion of histograms to CSV files (with efficiencies and luminosity values)
* plotting scripts for both individual fills and entire years (with appropriate averaging)

# Analysis framework

The Z counting Analysis framework is maintained in three separate locations:

1. The core analysis is performed in the athena framework under DataQualityTools (https://gitlab.cern.ch/atlas/athena/-/tree/main/DataQuality/DataQualityTools, specifically src/DQTGlobalWZFinderAlg.cxx). Here Z events are selected and histograms produced that are later used to calculate the data-driven single-lepton efficiencies and the Zlumi. The code is run automatically at Tier0 as part of the DQ framework, but may also be run separately on the grid over AODs.
(Some partially obsolete information on how to run the main code can be found here [https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi) - to be checked and migrated.)

2. The Monte Carlo correction factors require running the core DQ components from 1., but the output needs to be processed further with the tools from a separate git project (https://gitlab.cern.ch/z-counting/monte-carlo). The results are then inserted into the Python files from point 3 to process the HIST->CSV.

3. CSV files are created using scripts under ZlumiScripts (https://gitlab.cern.ch/atlas/athena/-/tree/main/DataQuality/ZLumiScripts/scripts/Pandas_scripts) using the inputs from 1. and 2. Here there are also the scripts to produce plots for the luminosity and efficiency using the csv files. The primary script is dqt_zlumi_pandas.py script that produces a single csv file containing all information for one ATLAS run. Each row of the csv file corresponds to a single luminosity block, and contains all information for both channels; such as the number of reconstructed Zs per channel, trigger efficiency, reconstruction efficiency, luminosity, as well as the arithmeric mean of the Zee and Zmumu luminosities and all axuilliary official information (livetime, pileup, luminosity, GRL).

## Other related useful links

https://atlas-datasummary.web.cern.ch/2024/runsum.py?style=table

# EOS space

The Run 3 intermediate and processed files are in the following directories:

| Files 	 | Location 								 | Information 					 |
| :--- 	 	 | :--- 								 | :--- 					 |
| Grid outputs 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/GridOutputs/	 | Raw outputs as we get them from the grid jobs (mostly unused these days) |
| Merged outputs | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/ | Reduced histograms after DQHistogramMerge step, or as copied from official DQ /eos or rucio|
| CSV outputs 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/	 | Single csv file produced per run 		 |
| Plots 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/	 | Plots for full Run3, years and each run |


# Semi-automatic full-chain running to update CVSs and Plots with latest runs

Loging to LXPLUS as several inputs and outputs are from EOS.  A more-or-less
recent version of Athena (e.g. 23.0.56) needs to be setup for the
scripts to be found and work. As the rel 23 were built for Centos7, but LXPLUS now runs ALMA9, you need to setup a container, the prompt and directories look a bit strange then...

## First time only

You first have to checkout the latest
scripts and compile - this will take a while.
(Not sure if maybe this could be avoided in case no changes on
scripts are needed). So only the very first time do

```
setupATLAS
mkdir zcounting
cd zcounting
asetup 24.0.58,Athena
lsetup git
git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git
cd athena/
git checkout 24.0
git atlas addpkg DataQualityTools
git atlas addpkg ZLumiScripts
cd ..
mkdir build
cd build/
cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filters.txt ../athena/Projects/WorkDir
make -j
source x86_64-el9-gcc13-opt/setup.sh
cd ..
cd athena/DataQuality/ZLumiScripts/scripts/Pandas_scripts/
```

The best is to do this in your home directory on AFS. Increase quote if necessary https://resources.web.cern.ch/resources/Manage/AFS/Settings.aspx (takes 5 seconds)

## Regular setup

```
setupATLAS
cd zcounting # or where your code is
asetup 24.0.58,Athena
source build/x86_64-el9-gcc13-opt/setup.sh
cd athena/DataQuality/ZLumiScripts/scripts/Pandas_scripts/
```

## HIST copy and HIST->CSV

The primary script is `process_Z_Counting.sh`. Usage:
```
./process_Z_Counting.sh [year] [update HISTs 0/1] [update CSVs 0/1]
```

E.g. the typical run to update 2024 with the latest extra runs is
```
./process_Z_Counting.sh 24 1 1
```
If the either of the two last arguments are 0, then old HIST and CVS files are recreated.

Look at the output and ensure all is going well! The output has been
mostly slimmed down to the minimum. The steps performed by the script are:
1. check `/eos/atlas/atlastier0/rucio/${dataset}/physics_Main/` for the list of runs taken
2. cross-check the list with available HIST files in ZCounting MergedOutputs and make a reduced copy of HISTs. Only the latest runs are available on the atlastier0 EOS location and in case, rucio may need to be envoked (requires rucio and grid setup beforehand)
3. Creating temporary PHYS_StandardGRL_All_Good GRL, empty runs are removed and excluded from further processing
4. HIST->CVS processing is launched. Output is a bit more noisy and more things can go wrong here as Limu information is loded from the database, runs may have problems not covered by the GRL etc.

## Update of Yearwise and Run 3 plots

```
./plot_yearwise.sh
```

Default arguments are "24 run3", which will update the 2024 and full
Run 3 plots.

This step will write both plots and a run-summary CSV file into the directory structure under /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/

You can also do e.g. only 2022 plots with 
```
./plot_yearwise.sh 22
```

## Update of Runwise plots (TODO)


The entry point is the script  `plot_all.sh`.

Note this is script requires changing of input/output locations and a bit more testing.

It will loops over all .csv files in a given directory and plots efficiency and luminosity against LB and pileup.


# GRLs

A GRL is required for the HIST->CSV step.

Official GRLs are taken from: /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/

While data taking is progessing, the official GRL updates are usually slow and instead a preliminary GRL is created 'on-the-fly' using the script grl_maker.py is included.
The procedure is part of the process_Z_Counting.sh script, so the instructions below generally do NOT need to be executed manually.
The produced 'latest' GRL is copied to EOS and available at (e.g.) /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/data24_13p6TeV/latest_GRL.xml

## Further Technical GRL info

Creating a cusom GRL loops through a list of runs and produces a .xml file for each run. These can then be merged into a single grl by moving all files to a single directory and running merge_goodrunslists e.g.:
```
merge_goodrunslists grls/
```

# Notes on status of Run 3 processing status

Several updates were mdae as Run 3 progresses, this tries to track what has been used for files stored on EOS

2022 MergedOutputs were processed by Sam on the grid, CSV Outputs are processed with MC21a MCCFs and the official GRL

2023 CSV Outputs are processed with MC23a MCCFs and the official GRL, other preliminary versions have been removed
Most 2023 were taken from official Tier0 DQ processing, either the DQ EOS directories /eos/atlas/atlastier0/rucio/data23_13p6TeV/physics_Main/*/data23_13p6TeV.*.physics_Main.merge.HIST.f*, or downloaded from the grid. Only the DQTGlobalWZFinder folder from the root files is sored to save space, scripts provided were used (process_rucio_files.sh and copySelective.py)

A series of 2023 runs were manually processed on the grid from AOD files using the WZFinder (Z-counting) framework. This was either because the Z-counting framework was not yet fully functional or electron triggers were changed (from run 451896 onwards) and not immediately updated. This resulted in a number of runs missing Z Lumi data in the electron channel (451896, 451936, 451949, 452028, 452163, 452202, 452241, 452463, 452533, 452573, 452624, 452640, 452660, 452669, 452696, 452726, 452785, 452787, 452799, 452843, 452872)

# Store of technical/verbose information that could be reduced or (re)moved

## How to merge HIST outputs

When running the DQ framework on AODs on the grid yourself

```
ls <grid_output>/* > tomerge.txt
DQHistogramMerge.py tomerge.txt tree_<run_number>.root
```

## Running the HIST to CSV code for a single run

Using a single 2022 run (tree_430580.root) as an illustrative example:
```
source setup.sh
grl="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunLists/data22_13p6TeV/20220902/data22_13p6TeV.periodF_DetStatus-v108-pro28_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"
infile="tree_430580.root"
campaign=mc23a
outdir="<your_output_directory>"

python -u dqt_zlumi_pandas.py --dblivetime --useofficial --grl $grl --infile $infile --campaign $campaign --outdir $outdir --update $update
```
campaign: Determines the Monte Carlo Correction Factors to be used in the code. These are found in zlumi_mc_cf.py in the python/tools/ directory.

update: Set to "on" to run over all available hist files and overwrite any csv files currently in the outdir. Set to off to only run over hist files that don't have csv files in the outdir.

## Running over the entire dataset
_Note_: The input (indir) and output (out_dir) directories will need to be changed at the top of the script run_code.sh.
```
year - defines the dataset to be processed

updateC - Set to 0 to run over all available hist files and overwrite any csv files currently in the outdir. Set to 1 to only run over hist files that don't have csv files in the outdir.

grlname - Can be used to define a specific grl location instead of using pre-defined grl locations given in the script

for year in 23
do
    ./run_code.sh $year $updateC $grlname
done
```

## Making single run plots

*Note*: The output directory (outdir) will need to be changed inside plotting/efficiency.py and plotting/luminiosity.py. Both of these scripts calculate an average over successive bunches of 20 luminosity blocks to increase statistical precision. All other plots use the single-LB luminosity, and not the 20 LB merged value, when calculating the integrated luminosity of an LHC fill/pileup bin.
```
### Time dependent efficiency and luminosity plots
python plotting/efficiency.py --infile <input_csv_file> --outdir <output directory>
python plotting/luminosity.py --infile <input_csv_file> --outdir <output directory>
python plotting/luminosity.py --absolute --infile <input_csv_file> --outdir <output directory>

### Pileup dependent efficiency and luminosity plots
python plotting/efficiency.py --usemu --infile <input_csv_file> --outdir <output directory>
python plotting/luminosity.py --usemu --infile <input_csv_file> --outdir <output directory>

### Kinematic plots - this plotting script takes the histograms in the merged root file rather than the produced csv
python plotting/plot_kinematics.py --infile $infile
```

