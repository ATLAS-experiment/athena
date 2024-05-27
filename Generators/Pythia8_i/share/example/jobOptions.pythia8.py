include("GeneratorUtils/StdEvgenSetup.py")

svcMgr.MessageSvc.OutputLevel = INFO

from Pythia8_i.Pythia8_iConf import Pythia8_i
topAlg += Pythia8_i()
topAlg.Pythia8_i.CollisionEnergy = 7000
topAlg.Pythia8_i.Commands += ['HardQCD:all = on']

include("GeneratorUtils/postJO.DumpMC.py")
