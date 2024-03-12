@ Mateus Hufnagel - mateus.hufnagel@cern.ch

# LArClusterCellDumper package:
This package holds the `CaloThinCellsInAODAlg` and `EventReaderAlg`.

- `CaloThinCellsInAODAlg`: 

    Runs on reconstruction by set the `flags.Calo.TopoCluster.xtalkInfoDumper` to **True**, as the example below, for Run2 data. 
    ```
    Reco_tf.py --CA "default:True" \
        --AddCaloDigi=True \
        --inputBSFile=data17_13TeV.00329542.physics_MinBias.daq.RAW._lb0245._SFO-4._0001.data \
        --outputAODFile=AOD_data_physicsMain.pool.root \
        --outputESDFile=ESD_tmp.pool.root \
        --preExec "r2a:flags.Digitization.AddCaloDigi=True; flags.Calo.TopoCluster.xtalkInfoDumper=True" \
        --geometryVersion "default:ATLAS-R2-2016-01-00-01" \
        --conditionsTag CONDBR2-BLKPA-RUN2-11 \
        --maxEvents 100
    ```
    - `CaloThinCellsInAODAlgConfig.py`:
        
        - By default, it selects clusters from `CaloCalTopoCluster` container with `ClusterPtCut=1000 MeV` `ClusterEtaCut=1.4`, and map their `CaloCells` for PSB, EMB1, EMB2 and EMB3 layers. 
        
        - This map select `LArDigits` and `LArRawChannels`, then create new thin containers and forward them to AOD. Their name are: `[...]#LArDigitContainer_ClusterThinned`, `[...]#LArRawChannels_ClusterThinned`, `[...]#AllCalo_ClusterThinned`.


Having those containers available in any AOD file, the NTuple dumper can be applied:

- `EventReaderAlg`: 

    That algorithm generates a NTuple with electrons, its clusters, cells, rawChannels, digits and online calibration constants (OFCs, ramps, pedestal, shapes). There are two main electron dumper chains: (1) single electrons and (2) Z->ee selection by Tag and Probe.
    That algorithm  was developed to aid ML studies related to cross-talk between cells on LAr cells.

    The input file can be any ESD/AOD, but require some containers that are not forwarded in standard reconstruction, as: RawChannels, Digits, Hits. So, the NTuple data has links between digits, cells, clusters and electrons.



## 1. Get the package

Perform an Athena sparse checkout
```
mkdir athena_sparse
setupATLAS
lsetup git
git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git
cd athena
git atlas addpkg LArClusterCellDumper
```
## 2. Build
The algorithms works on R24,R25+.
```
cd ..
mkdir build
mkdir run
cd build
asetup Athena,22.0.44
cmake ../athena/Projects/WorkDir/
make
source x86_64-centos7-gcc11-opt/setup.sh
```
## 3. Run
To run locally, you can copy the `/athena/LArCalorimeter/LArClusterCellDumper/python/LArClusterCellDumperConfig.py` to a `run/` folder, and set the properties that controls the dumper's output:

- on `"__main__"`, the `NTuple` **output** file name is set at`THistSvc(Output = ["rec DATAFILE='dumper_output.root', OPT='RECREATE'"])`.
- the **input** file can be set at `dumperFlags.Input.Files`, like any input file.
- number of events: `dumperFlags.Exec.MaxEvents`.
- electron output chain:
    - T&P: `dumpOnlySingleElectrons` set to `False`
    - Single_e: `dumpOnlySingleElectrons` set to `True`
- electron eta cut: `electronEtaCut = 1.4` by default.
- online LAr calib. constants (OFca, OFCb, DSP_thr, pedestal): `getLArCalibConstants` set to `True`.

The containers names can be also customized, like other parameters.


## 4. ML cross-talk studies

The data on output ntuples are currently being used as input for cross-talk studies using machine learning. This ongoing studies are documented and coded on a local gitlab repository: https://gitlab.cern.ch/mhufnage/xtalkDatasetAnalysis, and will be shared in a public ATLAS repo when results starts to come and, also, an ATLAS Note at CDS.