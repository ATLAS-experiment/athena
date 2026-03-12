include("EvgenProdTools/StdEvgenSetup.py")
include("Epos4_i/configFile.py")

theApp.EvtMax = 100

from Epos4_i.Epos4_iConf import Epos4
Ep4 = Epos4()
Ep4.BeamMomentum     = -runArgs.ecmEnergy/2.0 #For now, for symmetric collisions
Ep4.TargetMomentum   = runArgs.ecmEnergy/2.0 #For now, for symmetric collisions

energy              = float(runArgs.ecmEnergy) # center-of-mass energy
number_of_events    = int(runArgs.maxEvents)   # number of events


content = build_config_content(energy, number_of_events, hydro=True, centralityClass=0 ) #creating an optns file for epos4 configuration

with open("Epos4.optns", "w") as f:
    f.write(content)

Ep4.InputCard   = "Epos4.optns"
genSeq  += Ep4

