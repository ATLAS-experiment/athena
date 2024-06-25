# DerivationFrameworkJetEtMiss

This package contains the derivation formats (JETMX) needed for Jet/Etmiss performance studies. 

In Run 3 the derivation framework moved to the component accumulator. The config files can now be found in the python directory, not the share directory anymore!

## How to run: 

`Derivation_tf.py --inputAODFile aod.pool.root --outputDAODFile test.pool.root --formats JETM1 JETM2 ...`

Test file: /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc20\_13TeV.410470.PhPy8EG\_A14\_ttbar\_hdamp258p75\_nonallhad.recon.AOD.e6337\_s3681\_r13167/AOD.27162646.\_000001.pool.root.1

## JETMX formats

* `JETM1.py`:  MC calibrations (MC-JES, GSC) and in situ calibrations (eta-intercalibration, MJB, JER), trigger jet studies
* `JETM2.py`: MC only for tagger developments, JetDef R&D (rho, towers, ...), this is a merged format of JETM8 and JETM13  
* `JETM3.py`: *in situ* Z+jets calibration
* `JETM4.py`: *in situ* gamma+jets
* `JETM5.py`: random cones in zero bias data
* `JETM6.py`: tagging scale factors
* `JETM7.py`: by-vertex jet reconstruction
* `JETM10.py`: MET trigger studies
* `JETM11.py`: MET trigger studies in e+mu events
* `JETM12.py` : E/p studies in W to tau + nu events
* `JETM14.py`: MET trigger studies in single lepton events
