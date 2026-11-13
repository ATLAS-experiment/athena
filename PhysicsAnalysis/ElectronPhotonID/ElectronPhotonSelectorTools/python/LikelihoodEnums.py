# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from RootUtils.LoadEnumHeader import loadEnumHeader
loadEnumHeader('ElectronPhotonSelectorTools/LikelihoodEnums.h')

# make the LikeEnum namespace visible
from ROOT import LikeEnum   # noqa: F401

# pull in the dictionary/headers with the Menu enum definition
from ROOT.LikeEnum import Menu   # noqa: F401
