# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def HGTD_RecoCfg(flags):
    """Configures HGTD track extension
    (currently only decorates tracks with relevant info) """
    result = ComponentAccumulator()

    # We can do clusterization using Acts algorithm
    # In so, we then need to schedule the cluster EDM converter
    # to provide the same inputs to the downstream algorithms that
    # still do not support ACTS reconstruction
    if not flags.HGTD.doActs:
        from HGTD_Config.HGTD_PrepRawDataFormationConfig import PadClusterizationCfg
        result.merge(PadClusterizationCfg(flags))
    else:
        from ActsConfig.ActsClusterizationConfig import ActsHgtdClusterizationAlgCfg
        result.merge(ActsHgtdClusterizationAlgCfg(flags))
            
        from InDetConfig.InDetPrepRawDataFormationConfig import HGTDXAODToInDetClusterConversionCfg
        result.merge(HGTDXAODToInDetClusterConversionCfg(flags))
        
    from HGTD_Config.HGTD_TrackTimeExtensionConfig import TrackTimeExtensionCfg
    result.merge(TrackTimeExtensionCfg(flags))

    from HGTD_Config.HGTD_TrackTimeExtensionConfig import TrackTimeDefAndQualityAlgCfg
    result.merge(TrackTimeDefAndQualityAlgCfg(flags))

    from HGTD_Config.HGTD_VertexTimeConfig import VertexTimeAlgCfg
    result.merge(VertexTimeAlgCfg(flags))

    return result
