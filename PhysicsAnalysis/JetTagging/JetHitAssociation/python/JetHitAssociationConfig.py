# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Needed to get name of split probability container
from InDetConfig.TrackRecoConfig import ClusterSplitProbabilityContainerName

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetHitAssociationCfg(flags, name="JetHitAssociation", **kwargs):

    acc = ComponentAccumulator()
    
    isMC = flags.Input.isMC
  
    if flags.Detector.GeometryITk:
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            ITkPixelPrepDataToxAODCfg as PixelPrepDataToxAODCfg, 
            ITkStripPrepDataToxAODCfg as StripPrepDataToxAODCfg
        )
    else:
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            InDetPixelPrepDataToxAODCfg as PixelPrepDataToxAODCfg, 
            InDetSCT_PrepDataToxAODCfg as StripPrepDataToxAODCfg
        )
  
    acc.merge(
      PixelPrepDataToxAODCfg(
        flags,
        ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(flags),
        WriteSiHits=isMC,
        WriteSDOs=isMC,       
        # see ATR-27293 for discussion on why this was disabled
        WriteNNinformation=False
      )
    )
    acc.merge(
      StripPrepDataToxAODCfg(
        flags,
        WriteSiHits=isMC,
        WriteSDOs=isMC	     
      )
    )
    acc.addEventAlgo(
      CompFactory.JetHitAssociation(
        "JetHitAssociation",
        inputPixHitCollectionName = ("ITkPixelMeasurements" if flags.Detector.GeometryITk else "PixelClusters"),
        inputSCTHitCollectionName = "ITkStripMeasurements" if flags.Detector.GeometryITk else "SCT_Clusters",
        jetCollectionName = flags.BTagging.Trackless_JetCollection,
        jetPtThreshold = flags.BTagging.Trackless_JetPtMin,
        dRmatchHitToJet = flags.BTagging.Trackless_dR
      )
    )

    return acc

