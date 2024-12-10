# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 top pair plus Z boson production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "top"]
evgenConfig.contact = ["tpelzer@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg ttZ process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_ttZ_Common.py")

# --------------------------------------------------------------
# Settings
# --------------------------------------------------------------
# define the decay mode
PowhegConfig.decay_mode = "t t~ z > all" # inclusive is the default
#PowhegConfig.decay_mode = "t t~ z > b j j b~ j j emu+ emu-"
#PowhegConfig.decay_mode = "t t~ z > b l+ vl b~ l- vl~ emu+ emu-"
#PowhegConfig.decay_mode = "t t~ z > b emu+ vemu b~ emu- vemu~ emu+ emu-"
#PowhegConfig.decay_mode = "t t~ z > semileptonic emu+ emu-"
#PowhegConfig.decay_mode = "t t~ z > undecayed"
## for handling top decays with MadSpin
#PowhegConfig.decay_mode = "t t~ z > all [MadSpin]"
#PowhegConfig.MadSpin_decays= ["decay t > w+ b, w+ > l+ vl", "decay t~ > w- b~, w- > l- vl~"]
#PowhegConfig.MadSpin_process= "generate p p > t t~ z [QCD]" # this process is default - can be changed (for studies)
## additional MadSpin parameters to improve the integration
#PowhegConfig.MadSpin_max_weight_ps_point= 1000
#PowhegConfig.MadSpin_Nevents_for_max_weight= 250

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
