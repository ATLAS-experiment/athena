# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsAlignStoreProviderAlgCfg(flags, name="AlignStoreProviderAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LoadDetectorVolumeSvc", False)

    from AthenaConfiguration.Enums import ProductionStep
    ### Disable the Volume & TrackingGeometry service in simulation
    if flags.Common.ProductionStep  == ProductionStep.Simulation:       
        kwargs.setdefault("LoadTrackingGeoSvc" , False)
        kwargs.setdefault("LoadDetectorVolumeSvc" , False)
    else:
        kwargs.setdefault("LoadDetectorVolumeSvc", False)
        kwargs.setdefault("LoadTrackingGeoSvc", True)

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    kwargs.setdefault("TrackingGeometrySvc", result.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags)) 
                                                                         if kwargs["LoadTrackingGeoSvc"] else "")
    from ActsGeometry.DetectorVolumeSvcCfg import DetectorVolumeSvcCfg
    kwargs.setdefault("DetectorVolumeSvc", result.getPrimaryAndMerge(DetectorVolumeSvcCfg(flags))
                                                                      if kwargs["LoadDetectorVolumeSvc"] else "")

    the_alg = CompFactory.ActsTrk.AlignStoreProviderAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def DetectorAlignCondAlgCfg(flags, name="DetectorAlignCondAlg", **kwargs):
    result = ComponentAccumulator()

    from AthenaConfiguration.Enums import ProductionStep
    ### Disable the Volume & TrackingGeometry service in simulation
    if flags.Common.ProductionStep  == ProductionStep.Simulation:       
        kwargs.setdefault("LoadTrackingGeoSvc" , False)
        kwargs.setdefault("LoadDetectorVolumeSvc" , False)
    else:
        kwargs.setdefault("LoadDetectorVolumeSvc", False)
        kwargs.setdefault("LoadTrackingGeoSvc", True)

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    kwargs.setdefault("TrackingGeometrySvc", result.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags)) 
                                                                         if kwargs["LoadTrackingGeoSvc"] else "")
    from ActsGeometry.DetectorVolumeSvcCfg import DetectorVolumeSvcCfg
    kwargs.setdefault("DetectorVolumeSvc", result.getPrimaryAndMerge(DetectorVolumeSvcCfg(flags))
                                                                      if kwargs["LoadDetectorVolumeSvc"] else "")

    the_alg = CompFactory.ActsTrk.DetectorAlignCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result


### Setups the Acts alignment for the Pixel detectors
def PixelAlignCondAlgCfg(flags):
    result = ComponentAccumulator()
    from ROOT.ActsTrk import DetectorType 

    if flags.Detector.GeometryITkPixel:
        from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelAlignCondAlgCfg
        result.merge(ITkPixelAlignCondAlgCfg(flags))
        result.merge(DetectorAlignCondAlgCfg(flags,
                                             name="ActsDetAlignmentCondAlgITkPixel",
                                             InputTransforms="ITkPixelAlignmentStore",
                                             ActsTransforms="ActsITkPixelAlignmentStore",
                                             DetectorType=DetectorType.Pixel))
        result.merge(ActsAlignStoreProviderAlgCfg(flags,
                                                  name="ActsDetAlignmentAlgITkPixel",
                                                  CondAlignStore="ActsITkPixelAlignmentStore",
                                                  EventAlignStore="ActsITkPixelAlignmentStore",
                                                  DetectorType=DetectorType.Pixel))

    elif flags.Detector.GeometryPixel:
        from PixelConditionsAlgorithms.PixelConditionsConfig import PixelAlignCondAlgCfg
        result.merge(PixelAlignCondAlgCfg(flags))
        result.merge(DetectorAlignCondAlgCfg(flags,
                                             name="ActsDetAlignmentCondAlgPixel",
                                             InputTransforms="PixelAlignmentStore",
                                             ActsTransforms="ActsPixelAlignmentStore",
                                             DetectorType=DetectorType.Pixel))
        result.merge(ActsAlignStoreProviderAlgCfg(flags,
                                                  name="ActsDetAlignmentAlgPixel",
                                                  CondAlignStore="ActsPixelAlignmentStore",
                                                  EventAlignStore="ActsPixelAlignmentStore",
                                                  DetectorType=DetectorType.Pixel))
    return result

### Setups the Acts alignment for the Sct detectors
def SctAlignCondAlgCfg(flags):
    result = ComponentAccumulator()
    from ROOT.ActsTrk import DetectorType 

    if flags.Detector.GeometryITkStrip:
        from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripAlignCondAlgCfg
        result.merge(ITkStripAlignCondAlgCfg(flags))
        result.merge(DetectorAlignCondAlgCfg(flags,
                                             name="ActsDetAlignmentCondAlgITkSct",
                                             InputTransforms="ITkStripAlignmentStore",
                                             ActsTransforms="ActsITkStripAlignmentStore",
                                             DetectorType=DetectorType.Sct))
        result.merge(ActsAlignStoreProviderAlgCfg(flags,
                                                  name="ActsDetAlignmentAlgITkSct",
                                                  CondAlignStore="ActsITkStripAlignmentStore",
                                                  EventAlignStore="ActsITkStripAlignmentStore",
                                                  DetectorType=DetectorType.Sct))
    elif flags.Detector.GeometrySCT:
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_AlignCondAlgCfg
        result.merge(SCT_AlignCondAlgCfg(flags))
        result.merge(DetectorAlignCondAlgCfg(flags,
                                                name="ActsDetAlignmentCondAlgSct",
                                                InputTransforms="SCTAlignmentStore",
                                                ActsTransforms="ActsSCTAlignmentStore",
                                                DetectorType=DetectorType.Sct))

        result.merge(ActsAlignStoreProviderAlgCfg(flags,
                                                  name="ActsDetAlignmentAlgSct",
                                                  CondAlignStore="ActsSCTAlignmentStore",
                                                  EventAlignStore="ActsSCTAlignmentStore",
                                                  DetectorType=DetectorType.Sct))
    return result

#### Setup the Geometry context algorithm
def ActsGeometryContextAlgCfg(flags, name="GeometryContextAlg", **kwargs):
    result = ComponentAccumulator()
    AlignmentStores = []
    
    from AthenaConfiguration.Enums import ProductionStep
    ### Pixel & Sct Readout geometry is not setup for simulation. Skip it
    if flags.Common.ProductionStep  != ProductionStep.Simulation:
        result.merge(SctAlignCondAlgCfg(flags))
        result.merge(PixelAlignCondAlgCfg(flags))
        if flags.Detector.GeometryITkPixel: AlignmentStores += ["ActsITkPixelAlignmentStore"] 
        if flags.Detector.GeometryITkStrip: AlignmentStores += ["ActsITkStripAlignmentStore"]    
        if flags.Detector.GeometryPixel: AlignmentStores += ["ActsPixelAlignmentStore"]
        if flags.Detector.GeometrySCT: AlignmentStores += ["ActsSCTAlignmentStore"]

    ### Parse the alignment from the new muon geomodel
    if flags.Muon.usePhaseIIGeoSetup:
        from MuonGeoModelR4.MuonGeoModelConfig import MuonAlignStoreCfg
        result.merge(MuonAlignStoreCfg(flags))
      
        if flags.Detector.GeometryMDT:  AlignmentStores += ["MdtActsAlignContainer"]
        if flags.Detector.GeometryRPC:  AlignmentStores += ["RpcActsAlignContainer"]
        if flags.Detector.GeometryTGC:  AlignmentStores += ["TgcActsAlignContainer"]
        if flags.Detector.GeometrysTGC: AlignmentStores += ["sTgcActsAlignContainer"]
        if flags.Detector.GeometryMM:   AlignmentStores += ["MmActsAlignContainer"]

    kwargs.setdefault("AlignmentStores", AlignmentStores)

    the_alg = CompFactory.ActsTrk.GeometryContextAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
