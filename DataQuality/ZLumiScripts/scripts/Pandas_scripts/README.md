# Introduction

This is the main repository of processing scripts and documentation for the Z counting analysis framework.

Scripts provided here cover (roughly in sequence of execution):
* storage of reduced histogram outputs to our repository
* conversion of histograms to CSV files (with efficiencies and luminosity values)
* plotting scripts for both individual fills and entire years (with appropriate averaging)

# Analysis framework

The Z counting Analysis framework has important code in three separate locations:

1. The core analysis is performed in the Athena framework under DataQualityTools (https://gitlab.cern.ch/atlas/athena/-/tree/main/DataQuality/DataQualityTools, specifically src/DQTGlobalWZFinderAlg.cxx). Here Z events are selected and histograms produced that are later used to calculate the data-driven single-lepton efficiencies and the Z-counting luminosity. The code is run automatically at Tier0 as part of the DQ framework, but may also be run separately on the grid over AODs.
(Some partially obsolete information on how to run the main code can be found here [https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi) - to be checked and migrated.)

2. The Monte Carlo correction factors require running the core DQ components from 1., but the output needs to be processed further with the tools from a separate git project (https://gitlab.cern.ch/z-counting/monte-carlo). The results are then inserted into the Python files from point 3 to process the HIST->CSV.

3. CSV files are created using scripts under ZLumiScripts (https://gitlab.cern.ch/atlas/athena/-/tree/main/DataQuality/ZLumiScripts/scripts/Pandas_scripts) using the inputs from 1. and 2. Here there are also the scripts to produce plots for the luminosity and efficiency using the CSV files. The primary script is dqt_zlumi_pandas.py script that produces a single CSV file containing all information for one ATLAS run. Each row of the CSV file corresponds to a single luminosity block, and contains all information for both channels; such as the number of reconstructed Zs per channel, trigger efficiency, reconstruction efficiency, luminosity, as well as the arithmetic mean of the Zee and Zmumu luminosities and all auxiliary official information (livetime, pileup, luminosity, GRL).

## Other related useful links

https://atlas-datasummary.web.cern.ch/2025/runsum.py?style=table

https://cernbox.cern.ch/files/spaces/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Reports

# EOS space

The Run 3 intermediate and processed files are in the following directories:

| Files 	 | Location 								 | Information 					 |
| :--- 	 	 | :--- 								 | :--- 					 |
| Grid outputs 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/GridOutputs/	 | Raw outputs as we get them from the grid jobs (mostly unused these days) |
| Merged outputs | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/ | Reduced histograms after DQHistogramMerge step, or as copied from official DQ /eos or rucio|
| CSV outputs 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/	 | Single csv file produced per run 		 |
| Plots 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/	 | Plots for full Run3, years and each run |
| Reports 	 | /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Reports/	 | Summary LaTeX beamer slides with a summary of the most important plots |

In times of transitions, there may be multiple versions stored. The 'best knowledge' will then be typically soft-linked to the directories mentioned above.

Directories can also be opened in a browser using https://cernbox.cern.ch/files/spaces/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3

# Semi-automatic full-chain running to update CVSs and Plots with latest runs

Login to LXPLUS as several inputs and outputs are from EOS.  A more-or-less
recent version of Athena (e.g. 24.0.89) needs to be setup for the
scripts to be found and work. As of rel 24, releases work directly on LXPLUS ALMA9.

## First time only

You first have to checkout the latest
scripts and compile - this will take 1-2 minutes.
So only the very first time do

```
setupATLAS
mkdir zcounting
cd zcounting
asetup 24.0.89,Athena
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

There is an include line in DataQuality/ZLumiScripts/python/plotting/luminosity.py near the top that needs to be changed for local running, see comment in code.

## Regular setup

```
setupATLAS
cd zcounting # or where your code is
asetup 24.0.89,Athena
source build/x86_64-el9-gcc13-opt/setup.sh
cd athena/DataQuality/ZLumiScripts/scripts/Pandas_scripts/
```

## HIST copy and HIST->CSV

The primary script is `process_Z_Counting.sh`. Usage:
```
./process_Z_Counting.sh [year] [update HISTs 0/1] [update CSVs 0/1] [update temp GRL]
```

Explanation of options:

* [year]: the last digits of the year to process, e.g. '25' for 2025
* [update HISTs]: 1 - copy HIST output of new runs only; 0 - overwrite all HIST output (will take long and definitely require grid certificate and tools)
* [update CSVs 0/1]: 1 - create CSVs of new runs only; 0 - overwrite all CSV output (few hours run time, required e.g. to apply new GRL or MCCF)
* [update temp GRL]: 1 - re-create temporary GRL for the ongoing year; 0 - reuse old temporary GRL (for testing only) or use official GRL (for prior years)

E.g. the typical run to perform a minimal update of 2025 with the latest extra runs is
```
./process_Z_Counting.sh 25 1 1 1
```

Look at the output and ensure all is going well! The output has been
mostly slimmed down to the minimum. The steps performed by the script are:
1. check `/eos/atlas/atlastier0/rucio/${dataset}/physics_Main/` for the list of runs taken, exclude runs listed in /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/${dataset}/physics_Main/skipruns
2. cross-check the list with available HIST files in ZCounting MergedOutputs and make a reduced copy of HISTs. Only the latest runs are available on the atlastier0 EOS location and in case, rucio may need to be invoked if the runs were already deleted from eos (requires rucio and grid setup beforehand).
3. Creating temporary PHYS_StandardGRL_All_Good GRL, empty runs are removed and excluded from further processing
4. HIST->CVS processing is launched. Output is a bit more noisy and more things can go wrong here as Lumi information is loaded from the database, runs may have problems not covered by the GRL etc.

## Update of Plots

The plotting scripts actually perform a decent amount of
analysis-level work. Important points are:

* Application of selections for minimal LB livetime and minimal run
  livetime (for year-dependent plots) - these are steered globally
  with two variables `lblivetimecut` and `runlivetimecut` in the
  `python_tools.py` file
* Averaging over 20 LBs for run-dependent plots or sorting data into mu-bins
* Computation and display of arithmetic mean / pol0 fit mean / median
  and the respective standard deviation or 68% bands

Due to significant amount of files being opened, the runtime may be
long, so be patient. There is an overall plot update/remake script, usage:
```
./plot_all.sh [quiet 0/1] [update 0/1] [year]
```

Explanation of options:
* [quiet]: 1 - filter most output (recommended); 0 - dump a lot of output to the screen
* [update]: 1 - only add run-dependent plots if the directory doesn't yet exist; 0 - redo all run-dependent plots (needed when input CSVs have changed)
* [year]: the last digits of the year to process, e.g. '25' for 2025

E.g. the typical run to perform an update of 2025 is
```
./plot_all.sh 1 1 25
```

This script will work through the steps as lined out in the following subsections (no need to run these commands separately!)

### Update of runwise plots

The entry point is the script `plot_runwise.sh` that loops over all CSV
files for a given year and plots efficiency and luminosity against LB
and pileup.


### Update of yearwise plots

```
./plot_yearwise.sh
```

Default arguments are "25 22_23_24 run3", which will update the plots for 2025, the 2022+2023+2024 period and full Run 3 plots.

This step will write both plots and a run-summary CSV file into the
directory structure under
/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/

You can also do the plots for a more limited selection, e.g. only 2022 plots with 
```
./plot_yearwise.sh 22
```

### Preparation of plot summary slides

As the amount of plots produced is very large, a script is provided to
prepare LaTeX beamer slides that summarises: overview of Run 3 and 2022-2024 and the 'current' year (as given by the argument);
one page with plots for each run of the 'current year'

```
./make_latexslides.sh [year]
```

The output will be a PDF file with the name Zcounting_year20XX_YYYYMMDD.pdf, where
XX is the year specified and the YYYYMMDD is today's processing date, e.g. 241225 for X-mas day 2024.
When useful, these files can be copied to the Reports directory.

# GRLs

A GRL is required for the HIST->CSV step.

Official GRLs are taken from: /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/

While data taking is progressing, the official GRL updates are usually slow and instead a preliminary GRL is created 'on-the-fly' using the script grl_maker.py is included.
The procedure is part of the process_Z_Counting.sh script, so the instructions below generally do NOT need to be executed manually.
The produced 'latest' GRL is copied to EOS and available at (e.g.) /eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/${dataset}/latest_GRL.xml

## Further Technical GRL info

Creating a custom GRL loops through a list of runs and produces a .xml file for each run. These can then be merged into a single grl by moving all files to a single directory and running merge_goodrunslists e.g.:
```
merge_goodrunslists grls/
```

# Notes on status of Run 3 processing status

Several updates were made as Run 3 progresses, this tries to track what has been used for files stored on EOS

HIST in MergedOutputs:

* 2022 were processed by Sam on the grid
* most of 2023 were taken from official Tier0 DQ processing (either DQ EOS directories or grid, see above), some 2023 runs were manually processed on the grid, either because the Z-counting framework was not yet fully functional or electron triggers were changed (from run 451896 onwards) (451896, 451936, 451949, 452028, 452163, 452202, 452241, 452463, 452533, 452573, 452624, 452640, 452660, 452669, 452696, 452726, 452785, 452787, 452799, 452843, 452872)
* all of 2024 and 2025 were continuously copied from the official Tier0 DQ processing in the DQ EOS directories

Default CSVOutputs are:
* 2022, 2023, 2024 are processed with the matching MC23a,d,e and official GRL (some prior versions still stored in separate directories, but will be removed eventually)
* 2025 is processed with MC23e (2024 conditions) and temporary GRL


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

