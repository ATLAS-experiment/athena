# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 Drell-Yan Vector LeptoQuark production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["BSM", "leptoquark"]
evgenConfig.contact = ["tpelzer@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg Drell-Yan Vector LeptoQuark process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_DY_VLQ_modified_Common.py")

# --------------------------------------------------------------
# Relevant parameters for this process
# --------------------------------------------------------------
# 4321 Model Parameters
PowhegConfig.g4 = 1.0 # SU(4) coupling strength g4
PowhegConfig.betaL3x3 = 1.0 # Relative coupling strength to left-handed fermions (3rd generation)
PowhegConfig.betaR3x3 = 1.0 # Relative coupling strength to right-handed fermions (3rd generation)
PowhegConfig.MU1 = 2e3 # Mass of vector leptoquark U1
PowhegConfig.MGp = 2e3 # Mass of the coloron Gp
# General Leptoquark (LQ) Parameters
PowhegConfig.SM = 1 # Include SM contribution
PowhegConfig.LQ = 0 # Include basic LQ contributions
PowhegConfig.LQ_Int = 0 # Include the interference between the SM and the LQ contributions
PowhegConfig.mass_t = 172.5 # top-quark (running) mass
PowhegConfig.mass_low = 2000 # lower limit for dilepton mass
PowhegConfig.mass_high = 2100 # upper limit for dilepton mass
#PowhegConfig.runningscale = 1
#PowhegConfig.new_damp = 1
#PowhegConfig.hnew_damp = 0.5
#PowhegConfig.hdamp = 1.0

# --------------------------------------------------------------
# Integration settings
# --------------------------------------------------------------
PowhegConfig.ncall1 		= 10000
PowhegConfig.ncall2 		= 10000
PowhegConfig.nubound 		= 10000


# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
# Setting the appropriate number of final state particles for the main31 routine
genSeq.Pythia8.Commands += [ 'Powheg:NFinal = 2' ]
