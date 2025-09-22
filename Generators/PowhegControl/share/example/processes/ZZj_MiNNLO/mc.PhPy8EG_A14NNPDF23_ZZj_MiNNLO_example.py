# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 ZZj MiNNLO production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "diboson", "ZZ"]
evgenConfig.contact = ["guglielmo.frattari@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg ZZ process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_ZZj_MiNNLO_Common.py")

# --------------------------------------------------------------
# parameters for integration precision - very low values for quick testing!
# --------------------------------------------------------------
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
PowhegConfig.PDF = 304400


# --------------------------------------------------------------
# setting the decay channel
# --------------------------------------------------------------
# Possible values (PowhegControl syntax):
# z z > e- e+ mu- mu+,
# z z > e- e+ tau- tau+
# z z > mu- mu+ tau- tau+
# z z > l- l+ l'- l'+,
# z z > e- e+ e- e+
# z z > mu- mu+ mu- mu+
# z z > tau- tau+ tau- tau+
# z z > l- l+ l- l+,
# z z > e- e+ e- e+ / mu- mu+ mu- mu+,
# z z > e- e+ vl vl~
# z z > mu- mu+ vl vl~
# z z > tau- tau+ vl vl~
# z z > l- l+ vl vl~
# z z > e- e+ vl vl~ / mu- mu+ vl vl~
# z z (+ w w) > e- e+ vl vl~
# z z (+ w w) > mu- mu+ vl vl~
# z z (+ w w) > tau- tau+ vl vl~
# z z (+ w w) > l- l+ vl vl~
# z z (+ w w) > e- e+ vl vl~ / mu- mu+ vl vl~
# z z > j j j' j'
# z z > j j j j
# z z > l- l+ j j
# z z > j j vl vl~
# --------------------------------------------------------------
PowhegConfig.decay_mode = "z z > l- l+ l'- l'+"

# --------------------------------------------------------------
# Renormalisation / factorisation scales settings
# --------------------------------------------------------------
# fixedcale : 0 if 0 use dynamical scale below (set by whichscale), if 1 scale is fixed to Z-boson mass (leave this to 0 when MiN(N)LO is used)
# whichscale : 0 = sqrt(M_ZZ^2+pt_ZZ^2), 1 = M_ZZ (leave this to 1 when MiN(N)LO is used)
# All MiN(N)LO scale settings are taken care of internally
# --------------------------------------------------------------
PowhegConfig.fixedscale = "0"
PowhegConfig.whichscale = "1"

# --------------------------------------------------------------
# minlo or nnlops - uncomment each line to activate them
# --------------------------------------------------------------
# from the manual: "Note that Minlo overwrites any other running scale choice. The NNLOPS
# works only if the Minlo option is on. Finally, if Minlo is switched off, one
# needs to use a Born suppression factor, or, alternatively, a cut in the phase-space
# generation, in order to make the cross-section finite. The latter cut can be set
# through the token bornktmin."
# --------------------------------------------------------------
PowhegConfig.minlo    = "1"
PowhegConfig.minnlo   = "1" # 1 activates MiNNLOps

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
