
evgenConfig.description = "Simple EPOS4 production."
evgenConfig.keywords = [ "minbias" ]
evgenConfig.contact  = [ "andrii.verbytskyi@mpp.mpg.de", "paulina.majchrzak@cern.ch" ]
evgenConfig.nEventsPerJob = 10000
evgenConfig.generators += ["Epos4"]

include("Epos4_i/configFile.py")
energy                = float(runArgs.ecmEnergy)                                        # center-of-mass energy
number_of_events      = int(getattr(runArgs, "maxEvents", evgenConfig.nEventsPerJob))   # number of events
list_noDecayParticles = "110"                                                           # the most basic noDecayList

content = build_config_content(energy, number_of_events, hydro=True, centralityClass=0, list_of_particle_ids=list_noDecayParticles) #creating an optns file for epos4 configuration
with open("Epos4.optns", "w") as f:
    f.write(content)


include("Epos4_i/Epos4_Base_Fragment.py")

#To be fixed (TestHepMC is currently not working)
if hasattr(fixSeq, "FixHepMC"):
   fixSeq.remove(FixHepMC())

if hasattr(testSeq, "TestHepMC"):
   testSeq.remove(TestHepMC())

