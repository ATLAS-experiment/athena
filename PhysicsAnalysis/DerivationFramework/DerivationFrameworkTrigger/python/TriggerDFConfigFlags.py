# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createTriggerDFConfigFlags():
    cf = AthConfigFlags()
    cf.addFlag("Derivation.Trigger.outputL1JetRoIs", False)
    return cf
