# DerivationFrameworkTop

This package contains the derivation formats (TOPX) needed for top quark studies. 

## How to run: 

`Derivation_tf.py --inputAODFile aod.pool.root --outputDAODFile test.pool.root --formats TOPQ7 ...`

Test file: /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc20\_13TeV.410470.PhPy8EG\_A14\_ttbar\_hdamp258p75\_nonallhad.recon.AOD.e6337\_s3681\_r13167/AOD.27162646.\_000001.pool.root.1

## TOPX formats

* `TOPQ7.py`:  Contains all variables from PHYS and the three leading "jets" built from partons not originating from the tops or their decay products. A skimming algorithm for boosted ttbar events (mttbar > 700 GeV) is also implemented
