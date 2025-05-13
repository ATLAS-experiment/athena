# MS Vertex Validation

This validation framework creates ntuples from AODs of heavy scalar llp samples. It searches for truth particles with configurable pdgID as the llp and stores its properties, including the decay vertex position and its lifetime. Additionally, information related to reconstructed muon spectrometer (MS) displaced vertices, the MS tracklets, and MS segments are saved.

## Getting the Code
Following the [ATLAS tutorial](https://atlassoftwaredocs.web.cern.ch/athena/git/env-setup/), setup the ATLAS and Git environment before checking out the package:
```
setupATLAS   
lsetup git
git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git
cd athena
git atlas addpkg MuonVertexValidation
```

### Compile
Then setup athena, compile, and source:
```
mkdir ../build
cd ../build
asetup main,latest,Athena
cmake ../athena/Projects/WorkDir/
make
source x8*/setup.sh
```

## Running
The two main functionalities of this package are the ntuple creation and validation plot production.

### Ntuple maker
The job configurations of the ntuple maker are defined in `python/MuonVertexValidationRun.py`. After sourcing the package, the ntuple maker is run via:
```
cd ../run
python -m MuonVertexValidation.MuonVertexValidationRun -i <path to input AOD files> -o <path to the output root file> --maxEvents <number of events>
```
Note that a list of input files can be passed (for example via globbing) and by default `--maxEvents=-1`.

#### Running on the grid
The ntuple maker can run on the grid via the pathena tool. To run, set up atlas as usual, then call:
```
lsetup rucio
lsetup panda
pathena --trf "python -m MuonVertexValidation.MuonVertexValidationRun \
        --inputFile=%IN \
        --outputFile=%OUT.NTUP.root" \
        --inDS <your dataset> \ 
        --outDS=user.<username>.<output dataset name> \
        --nFiles <number of pool files to run over> \
        --nFilesPerJob <number of files per job> \
        --mergeOutput \
        --nCore 8 \
```

### Validation plots 
The plotting routines are defined in `util/`. The package offers two main functionalities: 
- `makeValidationPlots.cxx` produces validation plots for a single dataset
- `makeValidationPlotsComparison.cxx` combines the output of the former for an arbitrary number of datasets on to a single comparison plot. A ratio plot is additionally made when exactly two datasets are passed.   

During the build of the package, an executable program is complied for each of these functionalities which can be run via (use quotation marks if the arguments contains spaces):
```
makeValidationPlots <path to ntuple> <output directory> <name of the ntuple tree>
``` 
and
```
makeValidationPlotsComparison <path to dataset1 Histograms.root> <dataset1 label> <path to dataset2 Histograms.root> <dataset2 label> <path to output directory>
```
Note that the output directory does not need to exist apriori but will override an existing one.
Plots will be saved as `.pdf` files and also in a `Histograms.root` file at the specified output directory.