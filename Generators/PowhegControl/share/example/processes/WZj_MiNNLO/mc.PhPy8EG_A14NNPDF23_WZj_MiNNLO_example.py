# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 WZj MiNNLO production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "diboson", "WZ"]
evgenConfig.contact = ["aonan.wang@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg WZ process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_WZj_MiNNLO_Common.py")

# --------------------------------------------------------------
# parameters for integration precision - very low values for quick testing!
# --------------------------------------------------------------
#  PowhegConfig.lhans1       = "261400"
#  PowhegConfig.lhans2       = "261400"
PowhegConfig.bornktmin = "0.26d0"
PowhegConfig.xupbound = "2d0"
PowhegConfig.mintupbratlim = "1d3"
PowhegConfig.ubexcess_correct = "1"
PowhegConfig.withdamp = "1"
PowhegConfig.parallelstage = "1"
PowhegConfig.ewscheme = "1"
PowhegConfig.ncall1       = "30"
PowhegConfig.ncall2       = "30"
PowhegConfig.nubound      = "30"


# --------------------------------------------------------------
# setting the decay channel
# --------------------------------------------------------------
# Possible values (PowhegControl syntax):
#  "w- z > e- ve~ e+ e-"
#  "w- z > e- ve~ mu+ mu-"
#  "w- z > e- ve~ tau+ tau-"
#  "w- z > mu- vm~ e+ e-"
#  "w- z > mu- vm~ mu+ mu-"
#  "w- z > mu- vm~ tau+ tau-"
#  "w- z > tau- vt~ e+ e-"
#  "w- z > tau- vt~ mu+ mu-"
#  "w- z > tau- vt~ tau+ tau-"
#  "w+ z > e+ ve e+ e-"
#  "w+ z > e+ ve mu+ mu-"
#  "w+ z > e+ ve tau+ tau-"
#  "w+ z > mu+ vm e+ e-"
#  "w+ z > mu+ vm mu+ mu-"
#  "w+ z > mu+ vm tau+ tau-"
#  "w+ z > tau+ vt e+ e-"
#  "w+ z > tau+ vt mu+ mu-"
#  "w+ z > tau+ vt tau+ tau-"
# --------------------------------------------------------------
PowhegConfig.decay_mode = "w+ z > mu+ vm e+ e-"

# --------------------------------------------------------------
# running scale
# --------------------------------------------------------------
# 0: fixed scale m(W) + m(Z)
# 1: 0.5 (m(W) +m(Z))
# (leave this to 0 when MiN(N)LO is used) All MiN(N)LO scale settings are taken care of internally
# --------------------------------------------------------------
PowhegConfig.runningscales = "0"

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

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
