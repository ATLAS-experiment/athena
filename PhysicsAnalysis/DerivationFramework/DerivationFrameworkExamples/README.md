# DerivationFrameworkExamples

This package contains a series of examples showing how each aspect of the DAOD building (skimming, slimming, thinning, augmentation, scheduling etc) are done. They also act as unit tests allowing each aspect to be run individually, which is especially useful when migrations are being performed. 

The examples are run as follows:

`Derivation_tf.py --inputAODFile aod.pool.root --outputDAODFile test.pool.root --formats TEST1 TEST3 ...`

The examples are as follows:

1. `TEST1.py`: `TEST1` - demonstration of skimming via a dedicated tool implemented in C++, plus smart slimming
2. `TEST2.py`: `TEST2` - demonstration of skimming via the generic string parsing tool, plus smart slimming
3. `TEST3.py`: `TEST3` - demonstration of thinning via a dedicated tool implemented in C++, plus smart slimming
4. `TEST4.py`: `TEST4` - demonstration of smart slimming using the slimming helper
5. `TEST5.py`: `TEST5` - illustration of object decoration, using an example tool and a CP (muon) tool
6. `TEST6.py`: `TEST6` - illustration of scheduling CPU-heavy operations after a pre-selection skimming step to avoid running expensive calculations for events that will be rejected

For more details please refer to [the manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)

