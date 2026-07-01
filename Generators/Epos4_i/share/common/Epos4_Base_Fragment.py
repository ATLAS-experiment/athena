include("EvgenProdTools/StdEvgenSetup.py")
include("Epos4_i/configFile.py")

theApp.EvtMax = 10000 

from Epos4_i.Epos4_iConf import Epos4
Ep4 = Epos4()
Ep4.ArgsRandomSeed       = runArgs.randomSeed 
Ep4.InputCard   = "Epos4.optns"
genSeq  += Ep4

