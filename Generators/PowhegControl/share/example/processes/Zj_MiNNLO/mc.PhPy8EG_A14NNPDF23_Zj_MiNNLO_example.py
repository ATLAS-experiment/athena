# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# Powheg Zj MiNNlOPS setup starting from ATLAS defaults
#--------------------------------------------------------------
include("PowhegControl/PowhegControl_Zj_MiNNLO_Common.py")
# allowed decay modes: ["z > e+ e-", "z > mu+ mu-", "z > tau+ tau-", "z > b b~"]
PowhegConfig.decay_mode = "z > mu+ mu-"
# These integration settings are only for testing purposes
# They should be increased to 20000-100000 for gridpack production
PowhegConfig.ncall1     = 2000
PowhegConfig.ncall2     = 2000
PowhegConfig.nubound    = 2000

PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 Z+j MiNNLO production with A14 NNPDF2.3 tune"
evgenConfig.keywords    = [ "SM", "Z", "1jet" ]
evgenConfig.contact     = [ "Andrej.Saibel@cern.ch" ]
