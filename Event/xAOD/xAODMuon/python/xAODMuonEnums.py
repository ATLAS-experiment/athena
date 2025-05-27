# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ROOT import xAODMuonEnums   # noqa: F401

# pull in the MuonType enum dictionary by name - it makes the enum names
# (and also other enums names from the same dictionary)
# visible directly in the xAODMuonEnums namespace
from ROOT.xAODMuonEnums import MuonType  # noqa: F401 
