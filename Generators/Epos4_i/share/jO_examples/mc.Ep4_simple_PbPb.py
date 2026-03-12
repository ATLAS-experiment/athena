evgenConfig.description = "Simple EPOS4 production of Pb+Pb collisions, w/o hydrodynamic evolution, with impact parameter b=0."
evgenConfig.keywords = [ "Pb+Pb collisions" ]
evgenConfig.contact  = [ "andrii.verbytskyi@mpp.mpg.de", "paulina.majchrzak@cern.ch" ]
evgenConfig.nEventsPerJob = 10000
evgenConfig.generators += ["Epos4"]

include("Epos4_i/Epos4_Base_Fragment_PbPb.py")

#To be fixed (TestHepMC is currently not working)
if hasattr(fixSeq, "FixHepMC"):
   fixSeq.remove(FixHepMC())

if hasattr(testSeq, "TestHepMC"):
   testSeq.remove(TestHepMC())

