# LAr EM and HEC sampling fractions

## Input evgen

### LAr EM
The sampling fractions input electrons are generated in release 21.6 as single electrons with a momentum of 50 GeV, injected at a radius of r=1.5m for the barrel and distance of z=3.7405m for the endcap
```
setupATLAS -c centos7
asetup 21.6.31,AthGeneration
Gen_tf.py  --ecmEnergy=13000 --firstEvent=1 --maxEvents=10 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom50000_Radius1500000_eta_0_140 --outputEVNTFile=mc.PG_pid11_Mom50000_Radius1500000_eta_0_140.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --firstEvent=1 --maxEvents=10 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom50000_Z3740500_bec_eta_135_350 --outputEVNTFile=mc.PG_pid11_Mom50000_Z3740500_bec_eta_135_350.EVNT.pool.root
```
Input files with 100k events can be found in:
```
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/LArEM/mc.PG_pid11_Mom50000_Radius1500000_eta_0_140.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/LArEM/mc.PG_pid11_Mom50000_Z3740500_bec_eta_135_350.EVNT.pool.root
```

### HEC
The sampling fractions input particles are generated in release 21.6 as single electrons, positrons and photons with a momentum of 100 GeV, injected at distances of z=4.3195m and z=5.1750m which correspond to the front faces of the two HEC wheels
```
setupATLAS -c centos7
asetup 21.6.31,AthGeneration
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom100000_Z4319500_bec_eta_150_330 --outputEVNTFile=mc.PG_pid11_Mom100000_Z4319500_bec_eta_150_330.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid-11_Mom100000_Z4319500_bec_eta_150_330 --outputEVNTFile=mc.PG_pid-11_Mom100000_Z4319500_bec_eta_150_330.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid22_Mom100000_Z4319500_bec_eta_150_330 --outputEVNTFile=mc.PG_pid22_Mom100000_Z4319500_bec_eta_150_330.EVNT.pool.root

Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom100000_Z5175000_bec_eta_160_330 --outputEVNTFile=mc.PG_pid11_Mom100000_Z5175000_bec_eta_160_330.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid-11_Mom100000_Z5175000_bec_eta_160_330 --outputEVNTFile=mc.PG_pid-11_Mom100000_Z5175000_bec_eta_160_330.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid22_Mom100000_Z5175000_bec_eta_160_330 --outputEVNTFile=mc.PG_pid22_Mom100000_Z5175000_bec_eta_160_330.EVNT.pool.root
```

Input files with 10k events each can be found in:
```
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/HEC/mc.PG_pid-11_Mom100000_Z4319500_bec_eta_150_330.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/HEC/mc.PG_pid-11_Mom100000_Z5175000_bec_eta_160_330.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/HEC/mc.PG_pid11_Mom100000_Z4319500_bec_eta_150_330.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/HEC/mc.PG_pid11_Mom100000_Z5175000_bec_eta_160_330.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/HEC/mc.PG_pid22_Mom100000_Z4319500_bec_eta_150_330.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/HEC/mc.PG_pid22_Mom100000_Z5175000_bec_eta_160_330.EVNT.pool.root
```

### FCal
The sampling fractions input particles are generated in release 21.6 as single electrons, with a momentum of 40 GeV, injected at distances of z=4.7135m, z=5.1733m and z=5.6478 which correspond to the front faces of the three FCal modules
```
setupATLAS -c centos7
asetup 21.6.31,AthGeneration
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom40000_Z4713500_bec_eta_350_380 --outputEVNTFile=mc.PG_pid11_Mom40000_Z4713500_bec_eta_350_380.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom40000_Z5173300_bec_eta_350_380 --outputEVNTFile=mc.PG_pid11_Mom40000_Z5173300_bec_eta_350_380.EVNT.pool.root
Gen_tf.py  --ecmEnergy=13000 --maxEvents=10000 --randomSeed=1234 --jobConfig=athena/Simulation/Tools/CaloSamplingFractionAnalysis/share/PG_pid11_Mom40000_Z5647800_bec_eta_350_380 --outputEVNTFile=mc.PG_pid11_Mom40000_Z5647800_bec_eta_350_380.EVNT.pool.root
```

Input files with 10k events each can be found in:
```
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/FCal/mc.PG_pid11_Mom40000_Z4713500_bec_eta_350_380.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/FCal/mc.PG_pid11_Mom40000_Z5173300_bec_eta_350_380.EVNT.pool.root
/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions/FCal/mc.PG_pid11_Mom40000_Z5647800_bec_eta_350_380.EVNT.pool.root
```

## G4 Simulation
Simulation is run with calibration hits in batches of 5000 events per job with the following simulation command
```
Sim_tf.py \
--CA \
--multithreaded \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-04' \
--physicsList "$physlist" \
--simulator 'FullG4MT_QS' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoVCalibrationHits,SimulationConfig.disablePhotonRussianRoulette,SimulationConfig.disableNeutronRussianRoulette,SimulationConfig.disableFrozenShowersFCalOnly' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile "$inputEVNT" \
--outputHITSFile "$outfile_job" \
--maxEvents $nevents \
--skipEvent $skip \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False
```

For the LAr EM, HEC and FCal input files this can be done interactively in a shell with the commands below
```
export eosdir=/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions
export G4version=11.3
export PhysList=FTFP_BERT_ATL
export resultdir=$PWD/"$G4version"-$PhysList
mkdir -p $resultdir

get_files run_LAr_SamplingFraction_simulation.sh
for file in $eosdir/LArEM/mc.PG_pid11_Mom50000_*.EVNT.pool.root; do outfile=$resultdir/$(basename $file); run_LAr_SamplingFraction_simulation.sh $file ${outfile/EVNT.pool.root/HITS.pool.root} $PhysList 40000 ;done
for file in $eosdir/HEC/mc.PG_pid*_Mom100000_*EVNT.pool.root; do outfile=$resultdir/$(basename $file); run_LAr_SamplingFraction_simulation.sh $file ${outfile/EVNT.pool.root/HITS.pool.root} $PhysList 5000;done
for file in $eosdir/FCal/mc.PG_pid11_Mom40000_*EVNT.pool.root; do outfile=$resultdir/$(basename $file); run_LAr_SamplingFraction_simulation.sh $file ${outfile/EVNT.pool.root/HITS.pool.root} $PhysList 5000;done
```

## LAr EM NTuple creation and analysis
For a sufficient precision, ~40k electrons in the barrel and ~40k electrons in the endcap are needed for LAr EM.

```
export G4version=11.3
export PhysList=FTFP_BERT_ATL
export resultdir=$PWD/"$G4version"-$PhysList
athena.py --filesInput="$resultdir/mc.PG_pid11_Mom50000_Radius1500000*.HITS.pool.root" CaloSamplingFractionAnalysis/LArEMSamplingFractionConfig.py
mv LArEM_SF.root $resultdir/LArEM_SF_barrel.root

athena.py --filesInput="$resultdir/mc.PG_pid11_Mom50000_Z37*.HITS.pool.root" CaloSamplingFractionAnalysis/LArEMSamplingFractionConfig.py
mv LArEM_SF.root $resultdir/LArEM_SF_endcap.root

get_files LarEMSamplingFraction_analysis.C
root -b -q 'LarEMSamplingFraction_analysis.C("'$resultdir/LArEM_SF_barrel.root'","'$resultdir/LArEM_SF_endcap.root'")'
mv SF_LAr_barrel.* SF_LAr_endcap.* $resultdir/
```

## HEC NTuple creation and analysis
For a sufficient precision, 5k events per pdgid and Z position are needed, so in total 30k events

```
export G4version=11.3
export PhysList=FTFP_BERT_ATL
export resultdir=$PWD/"$G4version"-$PhysList
for file in $resultdir/mc.PG_pid*Mom100000_Z[45]*HITS.pool.root;do echo $file;athena.py --filesInput="$file" CaloSamplingFractionAnalysis/LArEMSamplingFractionConfig.py;a=$file;b=${a/Z4319500_bec_eta_150_330.HITS/HECfwh.NTUP};c=${b/Z5175000_bec_eta_160_330.HITS/HECrwh.NTUP};mv LArEM_SF.root $c;done

get_files HEC_SF_analysis
root -b -q HEC_SF_analysis/init.C 'HEC_SF_analysis/store_eta.C("'$G4version'","'$PhysList'","'$PWD'")' 'HEC_SF_analysis/get_SF.C("'$G4version'","'$PhysList'")'
```

## FCal NTuple creation and analysis
```
export G4version=11.3
export PhysList=FTFP_BERT_ATL
export resultdir=$PWD/"$G4version"-$PhysList

get_files LarFCalSamplingFraction_analysis.py
for file in $resultdir/mc.PG_pid*Mom40000_Z*_bec_eta_350_380.HITS.pool.root;do echo $file;athena.py --filesInput="$file" CaloSamplingFractionAnalysis/LArFCalSamplingFractionConfig.py;a=$file;b1=${a/Z4713500_bec_eta_350_380.HITS.pool/fcal1.aan};b2=${b1/Z5173300_bec_eta_350_380.HITS.pool/fcal2.aan};b3=${b2/Z5647800_bec_eta_350_380.HITS.pool/fcal3.aan};mv LArFCal_SF.root $b3;python LarFCalSamplingFraction_analysis.py $b3 -o ${b3/aan.root/txt};done
```                                                                                                                                                                 

# Tile sampling fractions
The measured hit energy in MC is converted to the EM scale using the sampling fraction (Etot/Evis)
determined from the test beam simulation of 100 GeV electrons incident at the center of the front face of the TileCal module at 20 degrees with a beam spot size of 2 cm.
Since the regularly spaced scintillating tiles are oriented vertically,
the electron response varies periodically with the impact point.
Thus, the sampling fraction is periodic and depends on the coordinate of the impact point along the front face of the calorimeter (Z).
The average sampling fraction is obtained by fitting its variation along the Z-direction with a simple periodic function (see more details [here](https://cds.cern.ch/record/962065/files/tilecal-pub-2006-006.pdf))

The sampling fraction at different incident angles can be extracted using the following command (first you need to setup athena):
```
run_Tile_SamplingFraction_simulation.sh 2000 +20 -20
```
Here, the first argument specifies the number of events per incident angle (theta), followed by the list of theta values (in degrees). 
At the end, this will produce the `TileSamplingFractions.root` file containing a TGraph of the inverse sampling fraction as a function of theta.
The values are also printed in `TileSamplingFraction_analysis.log`, and the periodic function fit for each theta angle can be seen in the files like `hist+20.png`.


# All simulation+analysis steps together for all LAr calorimeters should run out-of-the box with the following command
```
run_all_SamplingFraction_simulation.sh 5000
```
