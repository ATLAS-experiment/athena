
evgenConfig.description = "Simple EPOS4 production."
evgenConfig.keywords = [ "minbias" ]
evgenConfig.contact  = [ "andrii.verbytskyi@mpp.mpg.de", "paulina.majchrzak@cern.ch" ]
evgenConfig.nEventsPerJob = 10000
evgenConfig.generators += ["Epos4"]

theApp.EvtMax = 100


include("Epos4_i/Epos4_Base_Fragment_pp.py")

# ----------------------------------------------------------------------
# Drop checks
# ----------------------------------------------------------------------

# Currently dropped. To be changed in subsequent iterations!!
if hasattr(fixSeq, "FixHepMC"):
   fixSeq.remove(FixHepMC())

if hasattr(testSeq, "TestHepMC"):
   testSeq.remove(TestHepMC())

