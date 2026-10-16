# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------


#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 single top t-channel production with offshell tops"
evgenConfig.keywords = ["SM", "top"]
evgenConfig.contact = ["dominic.hirschbuehl@cern.ch,tpelzer@cern.ch"]
evgenConfig.nEventsPerJob = 2000


# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg single-top-dec process
# for s-channel
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_schan_top_dec_Common.py")

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

