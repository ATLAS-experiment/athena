evgenConfig.description = "Simple EPOS4 production of p+Pb collisions, with hydrodynamic evolution, and impact parameter b=0."
evgenConfig.keywords = [ "minbias" ]
evgenConfig.contact  = [ "andrii.verbytskyi@mpp.mpg.de", "paulina.majchrzak@cern.ch" ]
evgenConfig.nEventsPerJob = 10000
evgenConfig.generators += ["Epos4"]

include("Epos4_i/configFile.py")
energy                = float(runArgs.ecmEnergy)                                        # center-of-mass energy
number_of_events      = int(getattr(runArgs, "maxEvents", evgenConfig.nEventsPerJob))   # number of events
list_noDecayParticles = "20 2130 -2130 1330 -1330 2330 -2330 3331 -3331 2230 -2230 1130 -1130"

laproj = 1                                     # projectile atomic number 
maproj = 1                                    # projectile mass number
latarg = 82                                     # target atomic number
matarg = 208                                    # target mass number

content = build_config_content(energy, number_of_events, laproj, maproj, latarg, matarg, hydro=True) # creating an optns file for epos4 configuration
with open("Epos4.optns", "w") as f:
    f.write(content)


include("Epos4_i/Epos4_Base_Fragment.py")

# EvtGen fragment
include("EvtGen_i/EvtGen_Fragment.py")
evgenConfig.auxfiles+=['inclusive.pdt']
genSeq.EvtInclusiveDecay.allowAllKnownDecays=True


#Disable energy and momentum test due to internal generator problem
TestHepMC.EnergyImbalanceTest = False
TestHepMC.MomImbalanceTest    = False
