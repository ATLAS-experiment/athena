# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from ROOT import xAODType  # noqa: F401

from RootUtils.LoadEnumHeader import loadEnumHeader
loadEnumHeader ('xAODBase/ObjectType.h')

# pull in the ObjectType enum dictionary by name - it makes the enum names
# visible directly in the xAODType  namespace
from ROOT.xAODType import ObjectType  # noqa: F401 
