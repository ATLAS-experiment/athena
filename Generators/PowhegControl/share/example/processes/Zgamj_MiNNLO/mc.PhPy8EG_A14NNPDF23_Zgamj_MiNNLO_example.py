

# --------------------------------------------------------------
# EVGEN configuration
# --------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 Zgamj MiNNLOps production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "diboson", "Zgamma", "1jet"]
evgenConfig.contact = ["aonan.wang@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg WWj process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_Zgamj_MiNNLO_Common.py")

# --------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
# --------------------------------------------------------------

# --------------------------------------------------------------
# parameters for integration precision - very low values for quick testing!
# --------------------------------------------------------------
#  PowhegConfig.lhans1       = "261000"
#  PowhegConfig.lhans2       = "261000"
PowhegConfig.pt_j1_cut = 1
PowhegConfig.pt_a_cut = 5
PowhegConfig.xupbound = 2
PowhegConfig.mintupbratlim = 1000
PowhegConfig.ubexcess_correct = 1
PowhegConfig.withdamp = 1
PowhegConfig.ewscheme = 1
PowhegConfig.ncall1       = 500000
PowhegConfig.ncall2       = 500000
PowhegConfig.nubound      = 500000

# --------------------------------------------------------------
# setting the decay channel
# --------------------------------------------------------------
#  Possible values (PowhegControl syntax):
#  "z > e+ e-",
#  "z > mu+ mu-",
#  "z > tau+ tau-",
#  "z > e+ e- / mu+ mu-",
#  "z > l+ l-",
#  "z > vl vl~"  
# --------------------------------------------------------------
PowhegConfig.decay_mode = "z > e+ e-"

#  PowhegConfig.decay_mode = "z > vl vl~"
#  PowhegConfig.m_lepg_cut = 0
#  PowhegConfig.invmass_min = 0.001
#  PowhegConfig.pta_suppfact = 80
#  PowhegConfig.ptnunu_suppfact = 150
#  PowhegConfig.sum_over_families = 0

# --------------------------------------------------------------
# ANOMALOUS COUPLINGS 
# only works for "z > vl vl~"
# --------------------------------------------------------------
#  PowhegConfig.anomcoup = 1
#  PowhegConfig.hZ1 = 0
#  PowhegConfig.hZ2 = 0
#  PowhegConfig.hZ3 = 0
#  PowhegConfig.hZ4 = 0
#  PowhegConfig.hg1 = 0
#  PowhegConfig.hg2 = 0
#  PowhegConfig.hg3 = 0
#  PowhegConfig.hg4 = 0
#  PowhegConfig.anommode = 0

#  if using anommode = 1
#  PowhegConfig.anommode = 1
# --------------------------------------------------------------
# running scale
# --------------------------------------------------------------
# 0: sqrt(M_Z^2+pt_gamma^2) 
# 1: M_llgamma
#leave this to 1 when MiN(N)LO is used
# --------------------------------------------------------------
PowhegConfig.fixedscale = "0"
PowhegConfig.whichscale = "0"

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
# PowhegConfig.minnlo       = "1"

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

# --------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
# --------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
