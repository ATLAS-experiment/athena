include("Sherpa_i/Base_Fragment.py")
include("Sherpa_i/PDF4LHC21.py")

evgenConfig.description = "Sherpa 3.0.x example JO, Z+0,1-jet production."
evgenConfig.keywords = [ "2lepton" ]
evgenConfig.contact  = [ "atlas-generators-sherpa@cern.ch", "chris.g@cern.ch"]
evgenConfig.nEventsPerJob = 10000

genSeq.Sherpa_i.RunCard="""
 ME_SIGNAL_GENERATOR: Amegic

PROCESSES:
- 93 93 -> 11 -11 93{1}:
  Order: {QCD: 0, EW: 2}
  CKKW: 20

SELECTORS:
- [Mass, 11, -11, 40, E_CMS]
"""

genSeq.Sherpa_i.OpenLoopsLibs = []
genSeq.Sherpa_i.ExtraFiles = []
genSeq.Sherpa_i.NCores = 1
