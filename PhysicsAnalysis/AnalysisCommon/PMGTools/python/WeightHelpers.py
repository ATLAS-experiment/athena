# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ROOT import PMGTools    # noqa: F401
# pull in the PMGTools dictionary and headers using generatorWeightsPrefix
from ROOT.PMGTools import generatorWeightsPrefix
# make the functions accessible
from ROOT.PMGTools import weightNameCleanup, weightNameWithPrefix

__all__ = ['generatorWeightsPrefix', 'weightNameCleanup', 'weightNameWithPrefix']
