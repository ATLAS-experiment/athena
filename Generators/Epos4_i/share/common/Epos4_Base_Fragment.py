## Base config for Epos4
from Epos4_i.Epos4_iConf import Epos4
genSeq += Epos4("Epos4")
evgenConfig.generators += ["Epos4"]

genSeq.Epos4.BeamMomentum     = -runArgs.ecmEnergy/2.0
genSeq.Epos4.TargetMomentum   = runArgs.ecmEnergy/2.0

