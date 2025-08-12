# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

## Alg to refit DHNL signal vertices, with the aim of testing the re-fit procedure for speed and accuracy 
def TrackHitAssignementAlgCfg(flags, name="TrackHitAssignementAlg", **kwargs):
    acc = ComponentAccumulator()
    theAlg = CompFactory.TrackHitAssignementAlg(name,**kwargs)
    acc.addEventAlgo(theAlg,primary=True)
    return acc 


