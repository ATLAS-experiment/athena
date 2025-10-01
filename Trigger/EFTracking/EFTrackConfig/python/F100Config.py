#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def dataPreparation2(flags: AthConfigFlags, signature: str, inView: bool, rois: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if not inView:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                    ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map') ]
        acc.merge(SGInputLoaderCfg(flags, Load=loadRDOs))

    
    from EFTrackingFPGAPipeline.F100IntegrationConfig import FPGADataPreparation

    kwargs = {}
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("RoIs", rois)
    kwargs.setdefault("FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs_"+signature)
    kwargs.setdefault("FPGAEncodedStripKey", "FPGAEncodedStripRDOs_"+signature)
    kwargs.setdefault("FPGAOutputPixelKey", "FPGAFormatPixelClusters_"+signature)
    kwargs.setdefault("FPGAOutputStripKey", "FPGAFormatStripClusters_"+signature)
    kwargs.setdefault('xAODPixelClusterContainer', "FPGAPixelClusters_"+signature)
    kwargs.setdefault('xAODStripClusterContainer', "FPGAStripClusters_"+signature)
    kwargs.setdefault('sortedxAODPixelClusterContainer', "ITkPixelClusters_"+signature)
    kwargs.setdefault('sortedxAODStripClusterContainer', "ITkStripClusters_"+signature)
    kwargs.setdefault("FPGAThreads", 0) #require runtime extraction instead, cannot statically configure in the trigger

    acc.merge(FPGADataPreparation(flags, runStandalone=False, nameSuffix=signature, **kwargs))

    #add pixel spacepoint creation
    from ActsConfig.ActsSpacePointFormationConfig import ActsPixelSpacePointFormationAlgCfg
    acc.merge(ActsPixelSpacePointFormationAlgCfg(flags,name="PixelSPFormation_"+signature,useCache=False, PixelClusters = "ITkPixelClusters_"+signature, PixelSpacePoints = "ITkPixelSpacepoints_"+signature))

def dataPreparation(flags: AthConfigFlags, signature: str, inView: bool, rois: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if not inView:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                    ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map') ]
        acc.merge(SGInputLoaderCfg(flags, Load=loadRDOs))

    
    from EFTrackingFPGAPipeline.F100IntegrationConfig import F1X0IntegrationCfg

    acc.merge(fpga_data_encoding(flags, signature, rois))

    kwarg = {}

    kwarg.setdefault("FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs_"+signature)
    kwarg.setdefault("FPGAEncodedStripKey", "FPGAEncodedStripRDOs_"+signature)
    
    kwarg.setdefault("FPGAOutputPixelKey", "FPGAFormatPixelClusters_"+signature)
    kwarg.setdefault("FPGAOutputStripKey", "FPGAFormatStripClusters_"+signature)
    kwarg.setdefault("FPGAThreads", 0)
    
    acc.merge(F1X0IntegrationCfg(flags, name="F100IntegAlg_"+signature, **kwarg))

    #convert back to 
    acc.merge(fpga_xaod_creation(flags, signature))
    #sort
    acc.merge(fpga_xaod_sort(flags, signature))

    #add pixel spacepoint creation
    from ActsConfig.ActsSpacePointFormationConfig import ActsPixelSpacePointFormationAlgCfg
    acc.merge(ActsPixelSpacePointFormationAlgCfg(flags,name="PixelSPFormation_"+signature,useCache=False, PixelClusters = "ITkPixelClusters_"+signature, PixelSpacePoints = "ITkPixelSpacepoints_"+signature))

    return acc


def fpga_data_encoding(flags, signature, rois) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs = {}

    from EFTrackingFPGAPipeline.F100IntegrationConfig import F100DataEncodingCfg

    from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
    kwargs.setdefault('RegPixelSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
    kwargs.setdefault('RegStripSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))

    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("RoIs", rois)
    kwargs.setdefault("FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs_"+signature)
    kwargs.setdefault("FPGAEncodedStripKey", "FPGAEncodedStripRDOs_"+signature)

    acc.merge(F100DataEncodingCfg(flags, "F100DataEncoding_"+signature, **kwargs))

    return acc

def fpga_xaod_creation(flags, signature) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwarg = {}
    from EFTrackingFPGAPipeline.DataPrepConfig import xAODClusterMakerCfg
    clusterMakerTool = acc.popToolsAndMerge(xAODClusterMakerCfg(flags, 
                                                                name = "xAODClusterMaker_" + signature, 
                                                                PixelClusterContainerKey="FPGAPixelClusters_"+signature,
                                                                StripClusterContainerKey="FPGAStripClusters_"+signature))
    kwarg.setdefault('xAODClusterMaker', clusterMakerTool)
    kwarg.setdefault("FPGAOutputPixelKey", "FPGAFormatPixelClusters_"+signature)
    kwarg.setdefault("FPGAOutputStripKey", "FPGAFormatStripClusters_"+signature)

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F100EDMConversionAlg("F100EDMConversionAlg_"+signature, **kwarg))

    return acc

def fpga_xaod_sort(flags, signature) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs = {}

    kwargs.setdefault('xAODPixelClusterContainer', "FPGAPixelClusters_"+signature)
    kwargs.setdefault('xAODStripClusterContainer', "FPGAStripClusters_"+signature)
    kwargs.setdefault('sortedxAODPixelClusterContainer', "ITkPixelClusters_"+signature)
    kwargs.setdefault('sortedxAODStripClusterContainer', "ITkStripClusters_"+signature)
    
    ClustrerSorting = CompFactory.FPGAClusterSortingAlg("F100ClusterSorting_"+signature,**kwargs)

    # Add the algorithm to the accumulator
    acc.addEventAlgo(ClustrerSorting)

    return acc
