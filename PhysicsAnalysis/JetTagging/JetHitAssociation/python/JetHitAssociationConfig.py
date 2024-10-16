# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Needed to get name of split probability container
from InDetConfig.TrackRecoConfig import ClusterSplitProbabilityContainerName

# We need to add this algorithm to get the TrackMeasurementValidationContainer
from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPixelPrepDataToxAODCfg, InDetSCT_PrepDataToxAODCfg

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetHitAssociationCfg(flags, name="JetHitAssociation", **kwargs):

    acc = ComponentAccumulator()
    
    isMC = flags.Input.isMC

        

    acc.merge(
        InDetPixelPrepDataToxAODCfg(
            flags,
            ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(flags),
            WriteSiHits=isMC,
            WriteSDOs=isMC,       
            # see ATR-27293 for discussion on why this was disabled
            WriteNNinformation=False
        )
    )

    acc.merge(
        InDetSCT_PrepDataToxAODCfg(
            flags,
            WriteSiHits=isMC,
            WriteSDOs=isMC	     
        )
    )

    acc.addEventAlgo(
        CompFactory.JetHitAssociation(
            "JetHitAssociation",
            jetCollectionName = flags.BTagging.Trackless_JetCollection,
            jetPtThreshold = flags.BTagging.Trackless_JetPtMin,
            dRmatchHitToJet = flags.BTagging.Trackless_dR
        )
    )

    return acc

