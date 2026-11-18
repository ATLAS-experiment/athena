# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Make sure that the dictionary is loaded.
import ROOT

from RootUtils.LoadEnumHeader import loadEnumHeader
loadEnumHeader('xAODPrimitives/IsolationType.h')
loadEnumHeader('xAODPrimitives/IsolationCorrection.h')

# Declare the xAODIso type.
xAODIso = ROOT.xAOD.Iso

# pull in the IsolationType enum dictionary by name - it makes the enum names
# visible directly in the Iso  namespace
from ROOT.xAOD.Iso import IsolationType  # noqa: F401 
