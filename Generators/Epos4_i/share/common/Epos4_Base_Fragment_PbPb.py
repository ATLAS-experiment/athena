include("EvgenProdTools/StdEvgenSetup.py")
include("Epos4_i/configFile.py")

from Epos4_i.Epos4_iConf import Epos4
Ep4 = Epos4()
Ep4.BeamMomentum     = -runArgs.ecmEnergy/2.0 
Ep4.TargetMomentum   = runArgs.ecmEnergy/2.0 

energy              = float(runArgs.ecmEnergy)  # center-of-mass energy
number_of_events    = int(runArgs.maxEvents)    # number of events
laproj = 82                                     # projectile atomic number 
maproj = 208                                    # projectile mass number
latarg = 82                                     # target atomic number
matarg = 208                                    # target mass number

content = build_config_content(energy, number_of_events, laproj, maproj, latarg, matarg) # creating an optns file for epos4 configuration
with open("Epos4.optns", "w") as f:
    f.write(content)

Ep4.InputCard   = "Epos4.optns"
genSeq  += Ep4

