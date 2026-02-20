# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 W+c production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "W", "1jet"]
evgenConfig.contact = ["andrii.verbytskyi@mpp.mpg.de"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg Wj process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_Wc_Common.py")
PowhegConfig.CKM_Vud=0.97359
PowhegConfig.CKM_Vus=0.22438
PowhegConfig.CKM_Vcd=0.22438
PowhegConfig.CKM_Vcs=0.97356
PowhegConfig.mass_W=80.385
PowhegConfig.width_Z=2.4952
PowhegConfig.mass_b=4.8
PowhegConfig.mass_c=1.5
PowhegConfig.rwl_group_events=10
PowhegConfig.nubound=5000
PowhegConfig.storeinfo_rwgt=1
PowhegConfig.hdamp=0.1
PowhegConfig.bornktmin=0.0
PowhegConfig.bornsuppfact=10.0
PowhegConfig.doublefsr=0.0
PowhegConfig.ptsqmin=1.0
PowhegConfig.colltest=0
#PowhegConfig.resum_profile=1 #One should check this parameter
PowhegConfig.storeinfo_rwgt=1
PowhegConfig.alphas_from_pdf=1
PowhegConfig.lhapdf6maxsets=10
PowhegConfig.runningscales=1
PowhegConfig.softtest=0
PowhegConfig.ubexcess_correct=1
PowhegConfig.itmx1=1

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
