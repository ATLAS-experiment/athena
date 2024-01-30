# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

def secondStageBjetTrackingCfg(flags, inputRoI: str, inputVertex: str, inputJets: str) -> ComponentAccumulator:

    log = logging.getLogger("BJetTrackingConfig")
    
    #safety measure to ensure we get the right instance of flags
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    trkflags = getFlagsForActiveConfig(flags, "bjet", log)

    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
    seq = InDetTrigSequence(trkflags, 
                            trkflags.Tracking.ActiveConfig.input_name, 
                            rois   = inputRoI,
                            inView = "VDVInDetFTF")
    acc = seq.sequence("FastTrackFinder")
    acc.merge(seq.sequenceAfterPattern())

    verifier = CompFactory.AthViews.ViewDataVerifier(name = 'VDVsecondStageBjetTracking',
                                                     DataObjects = {('xAOD::VertexContainer', f'StoreGateSvc+{inputVertex}'),
                                                                    ('xAOD::JetContainer', f'StoreGateSvc+{inputJets}')} )
    acc.addEventAlgo(verifier)

    return acc
