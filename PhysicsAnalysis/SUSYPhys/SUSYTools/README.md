# SUSYTools

**Current Main Developers**:   
michael.holzbock@cern.ch,  
michael.donald.hank@cern.ch,

For questions please use atlas-phys-susy-bgforum-conveners@cern.ch

Note that the newest recommendations are always on the [background forum TWiki page](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BackgroundStudies).  Bugs can be reported on [JIRA](https://its.cern.ch/jira/projects/ATLSUSYBGF).  In case you are starting out, here are some useful tutorials for [GIT](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/GitTutorial) and [CMake](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/CMakeTestProjectInstructions) as they are used in ATLAS.

## Recommended tags

The recommended tags are on the [background forum TWiki page](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BackgroundStudies).

## AnalysisBase / AthAnalysisBase Setup

Set up a recent AnalysisBase release:
```bash
setupATLAS
lsetup git
asetup "AnalysisBase,25.2.23"
```

Or a recent AthAnalysis release::

```bash
setupATLAS
asetup "AthAnalysis,25.2.23"
```
  
For working with code, a sparse checkout is pretty straightforward.  
(In order for this to work, you need your own fork of the athena project, see the [ATLAS git tutorial](https://atlas-software.docs.cern.ch/athena/git/))  

```bash
git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git
```

And then the version of SUSYTools in the release can be checked out via:  
```bash
cd athena
git atlas addpkg SUSYTools
git checkout -b mybranchname upstream/main
```

Then, to compile:  
```bash
cd ..
mkdir build
cd build
cmake ../athena/Projects/WorkDir
make
source x86_64-el9-gcc13-opt/setup.sh
```

Additional packages needed on top of the recommended release are documented in `doc/packages.txt`; for now you can add those to your work area via git atlas addpkg.  Afterwards, be sure to recompile everything:  
```bash   
cmake ../athena/Projects/WorkDir
make -j
source x86_64-el9-gcc13-opt/setup.sh
```

and you are ready to go!

## Running SUSYTools

SUSYTools itself provides just an interface to the CP tools but for testing purposes the packages contains a couple of utilities to process an input file using SUSYTools to illustrate it's usage and/or write out some histogram output for validation (and testing).

### SUSYToolsTester (only AnalysisBase)

The `SUSYToolsTester` just runs over the input file and uses the most common functionalities without really producing any output. An example command to run on data is:

```shell
SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/data18_13TeV.00356250_p5855.PHYS.pool.root maxEvents=500 isData=1 isAtlfast=0 Debug=0
```

This executable is also run in the SUSYTools CI tests (see below).

### SUSYToolsAlg

The SUSYToolsAlg runs over the inputfile and also writes out a set of histograms to a file that can be inspected later on. It needs to be called via a wrapper that differes in AnalyisBase and AthAnalysis.

In AnalysisBase, e.g.

```shell
TestSUSYToolsAlg.py -f PHYS -t mc20e  -m 2000 --dosyst
```

In AthAnalysis, e.g.

```shell
athena SUSYTools/jobOptions.py --evtMax 2000 - --testCampaign mc23a
```

This algorithm is also run in the SUSYTOols ART tests (see below).

## Testing

In total 3 different types of tests for SUSYTools exists: CI tests, ART tests and unit tests.

**Please note:** Due to the unit tests causing a lot of "noise" these have been removed at some point and replaced with CI tests

### CI Tests

These are defined in [AnalysisBase.cmake](https://gitlab.cern.ch/atlas/athena/-/blob/main/AtlasTest/CITest/AnalysisBase.cmake) in the AtlasTest package of athena. They will run the `SUSYToolsTester` on some example input files. The CI tests are run for each MR that affects SUSYTools and obviously needs to be passed before any change will be merged .

### ART Tests

The ATLAS Release Tests (ART) are run essentially daily on the grid. For SUSYTools they are defined by the files located in the [test](test) directory of the SUSYTools package. They run the SUSYToolsAlg and compare the output histograms with respect to some reference histograms. These tests will fail if either running the SUSYToolsAlg does not succeed or the new histograms don't agree with the references.

#### [For ST Developpers] Updating ART Files and References

The input files for the SUSYTools ART tests are located on /cvmfs under

```
/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/
```

This folder is synced from

```
/eos/atlas/atlascerngroupdisk/data-art/grid-input/SUSYTools
```

where the BGF subconveners can upload new input files that can be used in the ART tests.

The files containing the reference files are located on /cvmfs under

```
/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/
```

This folder is synced from

```
/eos/atlas/atlascerngroupdisk/asg-calib/dev/SUSYTools/ART/References
```
Where again the BGF subconveners can upload new files. For bookeeping, any update of the reference files should be documented in [ATLSUSYBGF-648](https://its.cern.ch/jira/browse/ATLSUSYBGF-648).

Any updates to dcube need to be uploaded here:

```
/eos/atlas/atlascerngroupdisk/asg-calib/dev/SUSYTools/ART/xml
```

### Unit Tests

**Reminder:** No unit tests are currently defined for SUSYTools, so this paragraph remains here just for any future reference.

Unit tests now use [CTest](https://cmake.org/Wiki/CMake/Testing_With_CTest).  To run unit tests, simply **go to your build area** and type:

```bash
gmake test
```

More complex testing is best done directly with the `ctest` command.  To see what tests are available:  
```bash
ctest -N
```

To run specific tests (or tests for specific packages), use the `-R` option to restrict running to tests with a string in the name (e.g. `-R SUSYTools`) or `-E` to exclude certain tests (e.g. `-E SUSYToolsTester`).  To get logs / verbose output, use `-V`.

To test locally in an AnalysisBase release, get your favourite benchmark sample (e.g. `mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13145_p5057/` or `data18_13TeV.00358031.physics_Main.deriv.DAOD_PHYS.r13286_p4910_p5057`), and run:

```bash
SUSYToolsTester <myAOD.pool.root> maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 2>&1 | tee log
```

The standard `SUSYToolsTester` code is meant to test the configuration of all the relevant CP tools and apply a minimal analysis flow. It includes the recalibration and correction of all physics objects, the recalculation of the MissingET, the extraction of SFs and other specific functionalities that are enabled based on the DxAOD stream. All systematic variations available in the central registry are tested by default, and a cutflow is displayed for each of them. This can be disabled by setting `NoSyst=1`.

To test locally in an AthAnalysis release, run the test job options as::

```bash
cd ..
athena.py SUSYTools/minimalExampleJobOptions_mc.py
```

which is the athena-friendly equivalent of the `SUSYToolsTester` code above for running on MC.  You can also change "mc" to "data" or "atlfast" to run on data or fast simulation if you would prefer.


#### For expert only: Update the SUSYTools reference files for the CI unit test.


1) Contact atlas-phys-susy-bgforum-conveners@cern.ch
2) If they agree, proceed with:
```bash
cd src/athena
git atlas addpkg SUSYTools (or any other additional packages)
cd ../../build
cmake ../athena/Projects/WorkDir
make -j
source x86_64-centos7-gcc11-opt/setup.sh
ctest 
```
or
```bash
ctest -R SUSYTools_ut_SUSYToolsTester_data_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_data_Run3_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_mc_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_mc_Run3_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_mc23a_Run3_ctest

```
3) Before copying the files, check that all the changes are expected.
```bash
cd $WorkDir_DIR/../PhysicsAnalysis/SUSYPhys/SUSYTools/CMakeFiles/unitTestRun/
cp ut_SUSYToolsTester_data.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_data.ref
cp ut_SUSYToolsTester_data_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_data_Run3.ref
cp ut_SUSYToolsTester_mc.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_mc.ref
cp ut_SUSYToolsTester_mc_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_mc_Run3.ref
cp ut_SUSYToolsTester_mc23a_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_mc23a_Run3.ref
```
Or if you are using AthAnalysis these are the current test:
```bash
ctest -R SUSYTools_ut_SUSYToolsAlg_data_ctest
ctest -R SUSYTools_ut_SUSYToolsAlg_data_Run3_ctest
ctest -R SUSYTools_ut_SUSYToolsAlg_mc_ctest
ctest -R SUSYTools_ut_SUSYToolsAlg_mc_Run3_ctest
```
3) Before copying the files, check that all the changes are expected.
```bash
cd $WorkDir_DIR/../PhysicsAnalysis/SUSYPhys/SUSYTools/CMakeFiles/unitTestRun/
cp ut_SUSYToolsAlg_data.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_data.ref
cp ut_SUSYToolsAlg_data_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_data_Run3.ref
cp ut_SUSYToolsAlg_mc.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_mc.ref
cp ut_SUSYToolsAlg_mc_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_mc_Run3.ref
```
4) Add the new reference file on git:
```bash
cd $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/
git add ut_SUSYToolsTester*
```
Please tag the SUSY bgforum conveners in the athena MR.
