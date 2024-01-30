@ Mateus Hufnagel - mateus.hufnagel@cern.ch

# LArClusterCellDumper package:
This package runs the EventReader algorithm, that generates a NTuple with electrons, its clusters, cells, rawChannels, digits and online calibration constants (OFCs, ramps, pedestal, shapes). That algorithm  was developed to aid ML calorimeter studies related to cross-talk.

The input file can be any ESD/AOD, but require some containers that are not forwarded in standard reconstruction, as: RawChannels, Digits, Hits. So, the NTuple data has links between digits, cells, clusters and electrons.

## 1. Get the package

Perform an Athena sparse checkout
```
setupATLAS
lsetup git
git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git
cd athena
git atlas addpkg LArClusterCellDumper
```
## 2. Build
The algorithm is currently tested and working in R21 and R22. From R22, it's working properly on Athena,22.0.44.
```
cd ..
mkdir build
cd build
asetup Athena,22.0.44
cmake ../athena/Projects/WorkDir/
make
source x86_64-centos7-gcc11-opt/setup.sh
```
## 3. Run
The package can run using the `share/EventReader_crosstalk_jobOptions.py` file. On a new folder, after setting up the build, this file can be modified to customize the output NTuple root file. This is done with simple flags:
    
    - runLocal                  : True, if running small tests on lxplus.
    - inputDataPath             : path to ESD/AOD files on lxplus/cernBox.
    - isMC                      : True, case MC file, or False, for data.
    - numOfEvents               : -1, for maximum number of events of files.
    - noBadCh                   : If True, skip the cells tagged as badCells/channels.
    - dumpOnlySingleElectrons   : True, if false, dump electrons from Z->ee. If True, dont care about tag and Probe selection (increase statistics).

The output file name can be set on `Create output file`. At `Algorithm setup and execution`, the containers names can be also customized, like other parameters.

### 3.1 Helpers
Some helper files can be found on share/ folder for reading the NTuple, convert cell indexes, gains into readable variables. Also, for some quick custom reconstruction to generate ESD input files, from HITS/RAW ones.

## 4. ML cross-talk studies

The data on output ntuples are currently being used as input for cross-talk studies using machine learning. This ongoing studies are documented and coded on a local gitlab repository: https://gitlab.cern.ch/mhufnage/xtalkDatasetAnalysis.

