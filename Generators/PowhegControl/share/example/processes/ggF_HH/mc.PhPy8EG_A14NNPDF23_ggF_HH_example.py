# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 gluon-fusion di-Higgs production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["Higgs", "SMHiggs"]
evgenConfig.contact = ["james.robinson@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg ggF_HH process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_ggF_HH_Common.py")

# --------------------------------------------------------------
# Modify couplings
# --------------------------------------------------------------
#PowhegConfig.chhh = 1.0 # Trilinear Higgs self-coupling [default: 1.0 (SM)]
#PowhegConfig.ct = 1.0 # Top-Higgs Yukawa coupling [default: 1.0 (SM)]
#PowhegConfig.ctt = 0. # Two-top-two-Higgs (tthh) coupling [default: 0. (SM)]
#PowhegConfig.cggh = 0. # Effective gluon-gluon-Higgs coupling [default: 0. (SM)]
#PowhegConfig.cgghh = 0. # Effective two-gluon-two-Higgses coupling [default: 0. (SM)]

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
