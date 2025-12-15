

# --------------------------------------------------------------
# EVGEN configuration
# --------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 NLO + PS LL polarized WW"
evgenConfig.keywords = ["SM", "diboson", "WW"]
evgenConfig.contact = ["aonan.wang@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg WWj process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_VV_pol_Common.py")

# --------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
# --------------------------------------------------------------

# --------------------------------------------------------------
# parameters for integration precision - very low values for quick testing!
# --------------------------------------------------------------
#  PowhegConfig.lhans1       = "261400"
#  PowhegConfig.lhans2       = "261400"
PowhegConfig.xupbound = 2
PowhegConfig.ubexcess_correct = 1
PowhegConfig.ncall1       = 30
PowhegConfig.ncall2       = 30
PowhegConfig.nubound      = 50

# --------------------------------------------------------------
# setting the process 
# --------------------------------------------------------------
# Possible values (PowhegControl syntax):
#  "w+ w- > e+ ve mu- vm~",
#  "w+ w- > mu+ vm e- ve~",
#  "w+ w- > tau+ vt e- ve~",
#  "w+ w- > e+ ve tau- vt~",
#  "w+ w- > tau+ vt mu- vm~",
#  "w+ w- > mu+ vm tau- vt~",
#  "w+ w- > e+ ve e- ve~",
#  "w+ w- > mu+ vm mu- vm~",
#  "w+ w- > tau+ vt tau- vt~",
#  "w+ z > e+ ve mu+ mu-",
#  "w+ z > e+ ve tau+ tau-",
#  "w+ z > mu+ vm e+ e-",
#  "w+ z > mu+ vm tau+ tau-",
#  "w+ z > tau+ vt e+ e-",
#  "w+ z > tau+ vt mu+ mu-",
#  "w- z > e- ve~ mu+ mu-",
#  "w- z > e- ve~ tau+ tau-",
#  "w- z > mu- vm~ e+ e-",
#  "w- z > mu- vm~ tau+ tau-",
#  "w- z > tau- vt~ e+ e-",
#  "w- z > tau- vt~ mu+ mu-",
#  "z z > e+ e- mu+ mu-",
#  "z z > e+ e- tau+ tau-",
#  "z z > mu+ mu- e+ e-",
#  "z z > mu+ mu- tau+ tau-",
#  "z z > tau+ tau- e+ e-",
#  "z z > tau+ tau- mu+ mu-",
# --------------------------------------------------------------
PowhegConfig.VVprocess = "w+ w- > e+ ve mu- vm~"
# --------------------------------------------------------------
# setting the process 
# --------------------------------------------------------------
# Possible values (PowhegControl syntax):
#  "unpol-unpol",
#  "unpol-transv",
#  "transv-unpol",
#  "unpol-longit",
#  "longit-unpol",
#  "longit-longit",
#  "transv-transv",
#  "longit-transv",
#  "transv-longit",
#  "unpol-left",
#  "left-unpol",
#  "longit-left",
#  "left-longit",
#  "transv-left",
#  "left-transv",
#  "unpol-right",
#  "right-unpol",
#  "longit-right",
#  "right-longit",
#  "transv-right",
#  "right-transv",
#  "right-left",
#  "left-right",
#  "left-left",
#  "right-right",
# --------------------------------------------------------------
PowhegConfig.polarization = "longit-longit"

# --------------------------------------------------------------
# running scale
# --------------------------------------------------------------
# 0: fixed scale average VV mass
# 1: VV virtuality
# --------------------------------------------------------------
PowhegConfig.runningscale = "0"

# --------------------------------------------------------------
# minlo or nnlops - uncomment each line to activate them
# --------------------------------------------------------------
# from the manual: "Note that Minlo overwrites any other running scale choice. The NNLOPS
# works only if the Minlo option is on. Finally, if Minlo is switched off, one
# needs to use a Born suppression factor, or, alternatively, a cut in the phase-space
# generation, in order to make the cross-section finite. The latter cut can be set
# through the token bornktmin."
# --------------------------------------------------------------
# PowhegConfig.minlo        = "1"
# PowhegConfig.minlo_nnll   = "1"
# PowhegConfig.nnlops       = "1"

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

# --------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
# --------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
