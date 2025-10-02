# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# make the xAOD namespace accessible
from ROOT import xAOD  # noqa: F401

# pull in the EgammaType enum dictionary by name - it makes the enum names
# visible directly in EgammaParameters namespace
from ROOT.xAOD.EgammaParameters import EgammaType  # noqa: F401 
