# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------
evgenConfig.nEventsPerJob = 100

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 bblvlv_beta_modified production."
evgenConfig.keywords = ["SM", "bb4l", "offshell", "TOP", "l+jets"]
evgenConfig.contact = ["Andrej.Saibel@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg bblvlv_modified process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_bblvlv_Beta_modified_Common.py")
include("PowhegControl/PowhegControl_bblvlv_Beta_modified_SemiLep_Common.py")

########################

PowhegConfig.ncall1 = 1000
PowhegConfig.ncall1btlbrn = 1000
PowhegConfig.ncall2 = 1000
PowhegConfig.ncall2btlbrn = 1000
PowhegConfig.nubound = 1000

# settings for semileptonic production
PowhegConfigSemiLep.ncall1 = 1000
PowhegConfigSemiLep.ncall1btlbrn = 1000
PowhegConfigSemiLep.ncall2 = 1000
PowhegConfigSemiLep.ncall2btlbrn = 1000
PowhegConfigSemiLep.nubound = 1000

PowhegConfigSemiLep.lhefuborn = 1


# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
# Produce Dileptonic Events
PowhegConfig.generate()
# From DL LHE make semileptonic events
PowhegConfigSemiLep.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg.py")
