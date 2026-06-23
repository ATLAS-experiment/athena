evgenConfig.description = "Simple EPOS4 production of O+O collisions, with hydrodynamic evolution, and impact parameter b=0."
evgenConfig.keywords = [ "minbias" ]
evgenConfig.contact  = [ "andrii.verbytskyi@mpp.mpg.de", "paulina.majchrzak@cern.ch" ]
evgenConfig.nEventsPerJob = 10000
evgenConfig.generators += ["Epos4"]

include("Epos4_i/configFile.py")
energy                = float(runArgs.ecmEnergy)                                        # center-of-mass energy
number_of_events      = int(getattr(runArgs, "maxEvents", evgenConfig.nEventsPerJob))   # number of events
laproj = 8                                      # projectile atomic number 
maproj = 16                                    # projectile mass number
latarg = 8                                     # target atomic number
matarg = 16                                    # target mass number

content = build_config_content(energy, number_of_events, laproj, maproj, latarg, matarg, hydro=True) # creating an optns file for epos4 configuration
with open("Epos4.optns", "w") as f:
    f.write(content)


include("Epos4_i/Epos4_Base_Fragment.py")

if hasattr(fixSeq, "FixHepMC"):
   fixSeq.remove(FixHepMC())

if hasattr(testSeq, "TestHepMC"):
   testSeq.remove(TestHepMC())

