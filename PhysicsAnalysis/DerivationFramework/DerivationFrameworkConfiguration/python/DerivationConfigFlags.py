# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Common derivation config flags

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createDerivationConfigFlags():
    dfflags = AthConfigFlags()
    dfflags.addFlag("Derivation.skimmingExpression", "", help="Expression for command-line skimming, SKIM format only")
    dfflags.addFlag("Derivation.dynamicConsumers", [], help="List of containers for command-line skimming, SKIM format only")
    return dfflags
