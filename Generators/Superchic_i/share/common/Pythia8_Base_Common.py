from AthenaCommon import Logging
logger = Logging.logging.getLogger("Superchic_i")

## Shower config for Pythia8 with Superchic for elastic production

from Pythia8_i.Pythia8_iConf import Pythia8_i
genSeq += Pythia8_i("Pythia8")
evgenConfig.generators += ["Pythia8"]

genSeq.Pythia8.LHEFile = genSeq.SuperChicConfig.outputLHEFile()
genSeq.Pythia8.CollisionEnergy = int(runArgs.ecmEnergy)

testSeq.TestHepMC.MaxTransVtxDisp = 1000000
testSeq.TestHepMC.MaxVtxDisp      = 1000000000
