# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SeedingStrategy
from AthenaCommon.Utils.unixtools import find_datafile
from ActsInterop import UnitConstants as ActsUnits
import AthenaCommon.SystemOfUnits as GaudiUnits


def ActsGbtsFtfSeedingTrigToolCfg(flags,name: str = "GbtsFtfActsSeedingTool", **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  if "layerNumberTool" not in kwargs:
    from TrigFastTrackFinder.TrigFastTrackFinderConfig import (
      ITkTrigL2LayerNumberToolCfg)
    kwargs.setdefault("layerNumberTool", acc.popToolsAndMerge(
      ITkTrigL2LayerNumberToolCfg(
        flags, UseNewLayerScheme=True,
        dumpGbtsGeometry=flags.Acts.Gbts.dumpGbtsGeometry,
        geometryDump=flags.Acts.Gbts.geometryDump)))
  
  kwargs.setdefault("DoPhiFiltering", False) #no phi-filtering for full-scan tracking
  kwargs.setdefault("UseBeamTilt", False)
  kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPTSeed)
  kwargs.setdefault("MaxGraphEdges", 3000000)
  kwargs.setdefault("AddTriplets", False)

  isLargeD0 = flags.Tracking.ActiveConfig.isLargeD0
  kwargs.setdefault("ConnectionFileName",
                    "binTables_ITK_RUN4_LRT.txt" if isLargeD0 else
                    "binTables_ITK_RUN4.txt")

  acc.setPrivateTools(CompFactory.GbtsFtfActsSeedingTool(name, **kwargs))
  return acc


# ACTS tools
def ActsPixelGridTripletSeedingToolCfg(
    flags, name: str = "ActsPixelGridTripletSeedingTool",
    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    ## For ITkPixel
    kwargs.setdefault("numSeedIncrement" , float("inf"))
    kwargs.setdefault("deltaZMax" , float("inf"))
    kwargs.setdefault("maxPtScattering", float("inf"))
    kwargs.setdefault("useVariableMiddleSPRange", False)
    kwargs.setdefault("rMax", 320. * ActsUnits.mm)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPTSeed *
                      ActsUnits.GeV / GaudiUnits.GeV)
    kwargs.setdefault("impactMax", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
    kwargs.setdefault("rRangeMiddleSP", [
        [0,0],
        [140, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [70, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [140, 260],
        [0, 0]])
    
    kwargs.setdefault("useHVCollisionRegion",
                      flags.Tracking.ActiveConfig.useHoughVertexFilter)
    kwargs.setdefault("hvCollisionRegionTolerance", 10. * ActsUnits.mm)
    kwargs.setdefault( "inputHoughVtx",
                       "HoughVertices" if flags.Tracking.ActiveConfig.useHoughVertexFilter else "")

    acc.setPrivateTools(CompFactory.ActsTrk.GridTripletSeedingTool(name, **kwargs))
    return acc

def ActsFastPixelSeedingToolCfg(flags,
                                name: str = "ActsFastPixelSeedingTool",
                                **kwargs) -> ComponentAccumulator:
    ## Additional cuts for fast seed configuration
    kwargs.setdefault("sigmaScattering", 2.)
    kwargs.setdefault("maxSeedsPerSpM", 3)
    kwargs.setdefault("collisionRegionMin", -flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("collisionRegionMax", flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("maxPhiBins", 200)
    kwargs.setdefault("gridRMax", 250 * ActsUnits.mm)
    kwargs.setdefault("deltaRMax", 200 * ActsUnits.mm)
    kwargs.setdefault("zBinsCustomLooping" , [2, 10, 3, 9, 6, 4, 8, 5, 7])
    kwargs.setdefault("rRangeMiddleSP", [
             [0.0, 0.0],
             [60.0, 165.0],
             [60.0, 200.0],
             [60.0, 200.0],
             [60.0, 260.0],
             [60.0, 260.0],
             [60.0, 260.0],
             [60.0, 200.0],
             [60.0, 200.0],
             [60.0, 165.0],
             [0.0, 0.0]])

    kwargs.setdefault("zBinNeighborsTop", [
      [0, 0], # -3000, -2000  
      [-1, 0], # -2000, -1400  
      [-1, 0], # -1400, -910
      [-1, 0], # -910, -500 
      [-1, 0], # -500, -250 
      [-1, 1], # -250, 250
      [0, 1], # 250, 500 
      [0, 1], # 500, 910
      [0, 1], # 910, 1400
      [0, 1], # 1400, 2000
      [0, 0] # 2000, 3000
    ])
    kwargs.setdefault("zBinNeighborsBottom", [
      [0, 0], # -3000, -2000
      [0, 1], # -2000, -1400      
      [0, 1], # -1400, -910
      [0, 1], # -910, -500
      [0, 1], # -500, -250
      [0, 0], # -250, 250
      [-1, 0], # 250, 500
      [-1, 0], # 500, 910
      [-1, 0], # 910, 1400
      [-1, 0], # 1400, 2000
      [0, 0] # 2000, 3000
    ])
    
    kwargs.setdefault("zBinEdges", [-3000., -2000, -1400., -910., -500., -250.,  250., 500., 910., 1400., 2000, 3000.])
    kwargs.setdefault("useExperimentCuts", True)

    kwargs.setdefault("deltaRMaxTopSP", 220 * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP", 135 * ActsUnits.mm)

    return ActsPixelGridTripletSeedingToolCfg(flags, name, **kwargs)

def ActsStripGridTripletSeedingToolCfg(flags,
                                       name: str = "ActsStripSeedingTool",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    ## For ITkStrip, change properties that have to be modified w.r.t. the default values
    kwargs.setdefault("doSeedQualitySelection", False)
    # For SpacePointGridConfig
    kwargs.setdefault("gridRMax", 1000. * ActsUnits.mm)
    kwargs.setdefault("deltaRMax", 600. * ActsUnits.mm)
    kwargs.setdefault("impactMax", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPTSeed *
                      ActsUnits.GeV / GaudiUnits.GeV)
    # For SeedfinderConfig
    kwargs.setdefault("rMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("deltaRMinTopSP", 20. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxTopSP", 300. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinBottomSP", 20. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP", 300. * ActsUnits.mm)
    kwargs.setdefault("deltaZMax", 900. * ActsUnits.mm)
    kwargs.setdefault("interactionPointCut", False)
    kwargs.setdefault("zBinsCustomLooping", [7, 8, 6, 9, 5, 10, 4, 11, 3, 12, 2])
    kwargs.setdefault("deltaRMiddleMinSPRange", 30 * ActsUnits.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange", 150 * ActsUnits.mm)
    kwargs.setdefault("useDetailedDoubleMeasurementInfo", True)
    kwargs.setdefault("maxPtScattering", float("inf"))
    # For SeedFilterConfig
    kwargs.setdefault("useDeltaRorTopRadius", False)
    kwargs.setdefault("seedConfirmationInFilter", False)
    kwargs.setdefault("impactWeightFactor", 1.)
    kwargs.setdefault("compatSeedLimit", 4)
    kwargs.setdefault("numSeedIncrement", 1.)
    kwargs.setdefault("seedWeightIncrement", 10100.)
    kwargs.setdefault("maxSeedsPerSpMConf", 100)
    kwargs.setdefault("maxQualitySeedsPerSpMConf", 100)
    # For seeding algorithm
    kwargs.setdefault("zBinNeighborsBottom", [
      (0,0),(0,1),(0,1),(0,1),(0,2),(0,1),(0,0),
      (-1,0),(-2,0),(-1,0),(-1,0),(-1,0),(0,0)] )
    # Any other
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
    kwargs.setdefault("collisionRegionMin", -flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("collisionRegionMax", flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("useHVCollisionRegion",
                      flags.Tracking.ActiveConfig.useHoughVertexFilter)
    kwargs.setdefault("hvCollisionRegionTolerance", 10. * ActsUnits.mm)
    kwargs.setdefault("inputHoughVtx",
                      "HoughVertices" if flags.Tracking.ActiveConfig.useHoughVertexFilter else "")
    
    acc.setPrivateTools(CompFactory.ActsTrk.GridTripletSeedingTool(name, **kwargs))
    return acc

def ActsLargeRadiusStripSeedingToolCfg(flags,
                                        name: str = "ActsLargeRadiusStripSeedingTool",
                                        **kwargs) -> ComponentAccumulator:
    ## LRT-specific seeding cuts
    kwargs.setdefault("interactionPointCut", True)
    kwargs.setdefault("impactMax", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    ## per-pair azimuthal-swing doublet cut: rejects doublets whose azimuthal
    ## separation exceeds what a track with |d0| < impactMax can produce
    kwargs.setdefault("doubletDPhiCut", True)
    kwargs.setdefault("doubletDPhiCap", 0.10)
    ## deltaRMax only affects the grid phi-bin width (the doublet finders use
    ## deltaRMin/MaxTopSP/BottomSP); the bins widen with the impact-parameter
    ## term |asin(impactMax/(gridRMax-deltaRMax)) - asin(impactMax/gridRMax)|,
    ## so this value controls how much phi is enumerated per middle space point
    kwargs.setdefault("deltaRMax", 400. * ActsUnits.mm)
    kwargs.setdefault("collisionRegionMin", -flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("collisionRegionMax", flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange", 75 * ActsUnits.mm)
    kwargs.setdefault("deltaRMinTopSP", 50. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxTopSP", 250. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinBottomSP", 50. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP", 250. * ActsUnits.mm)
    kwargs.setdefault("deltaZMax", 850. * ActsUnits.mm)
    kwargs.setdefault("cotThetaMax", 5.0)
    kwargs.setdefault("maxSeedsPerSpM", 1)
    kwargs.setdefault("maxStripDeltaCotTheta", 0.3)
    kwargs.setdefault("absDeltaEtaWeightFactor", 10.)
    kwargs.setdefault("absDeltaEtaMinImpact", 2.)
    kwargs.setdefault("zBinEdges", [-3000., -2500, -1400., -910., -500., -250.,
                                    250., 500., 910., 1400., 2500, 3000.])
    kwargs.setdefault("zBinsCustomLooping" , [2, 10, 3, 9, 6, 4, 8, 5, 7])
    kwargs.setdefault("useVariableMiddleSPRange", False)
    kwargs.setdefault("zBinNeighborsTop", [
      [0, 0],  # -3000, -2500
      [-1, 0], # -2500, -1400
      [-1, 0], # -1400, -910
      [-1, 0], # -910, -500
      [-1, 0], # -500, -250
      [-1, 1], # -250, 250
      [0, 1],  # 250, 500
      [0, 1],  # 500, 910
      [0, 1],  # 910, 1400
      [0, 1],  # 1400, 2500
      [0, 0]   # 2500, 3000
    ])
    kwargs.setdefault("zBinNeighborsBottom", [
      [0, 0],  # -3000, -2500
      [0, 1],  # -2500, -1400
      [0, 1],  # -1400, -910
      [0, 1],  # -910, -500
      [0, 1],  # -500, -250
      [0, 0],  # -250, 250
      [-1, 0], # 250, 500
      [-1, 0], # 500, 910
      [-1, 0], # 910, 1400
      [-1, 0], # 1400, 2500
      [0, 0]   # 2500, 3000
    ])
    kwargs.setdefault("rRangeMiddleSP", [
      [0.0, 0.0],       # -3000, -2500
      [400.0, 850.0],   # -2500, -1400
      [500.0, 800.0],   # -1400, -910
      [500.0, 800.0],   # -910, -500
      [500.0, 800.0],   # -500, -250
      [500.0, 800.0],   # -250, 250
      [500.0, 800.0],   # 250, 500
      [500.0, 800.0],   # 500, 910
      [500.0, 800.0],   # 910, 1400
      [400.0, 850.0],   # 1400, 2500
      [0.0, 0.0]        # 2500, 3000
    ])

    kwargs.setdefault("seedConfirmation", True)
    kwargs.setdefault("seedConfirmationInFilter", True)
    kwargs.setdefault("zOriginWeightFactor", 1.)
    kwargs.setdefault("maxSeedsPerSpMConf", 1)
    kwargs.setdefault("maxQualitySeedsPerSpMConf", 1)
    kwargs.setdefault("seedConfCentralZMin",              -1400. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralZMax",               1400. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralRMax",                140. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralNTopLargeR",         1)
    kwargs.setdefault("seedConfCentralNTopSmallR",         0)
    kwargs.setdefault("seedConfCentralMinBottomRadius",      0. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMaxZOrigin",        1500. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMinImpact",          200. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMin",              -3000. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMax",               3000. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardRMax",                140. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardNTopLargeR",         1)
    kwargs.setdefault("seedConfForwardNTopSmallR",         0)
    kwargs.setdefault("seedConfForwardMinBottomRadius",    350. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMaxZOrigin",        1500. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMinImpact",          200. * ActsUnits.mm)

    return ActsStripGridTripletSeedingToolCfg(flags, name, **kwargs)

def ActsGbtsLayerToolCfg(flags,
                         name: str = "ActsGbtsLayerTool",
                         **kwargs) -> ComponentAccumulator:
    """The GBTS layer geometry, built from the ITk readout geometry.

    One instance is shared by every GBTS seeding tool: the layer indices it
    hands out have to agree with the connection table they all read.
    """
    acc = ComponentAccumulator()
    kwargs.setdefault("dumpGbtsGeometry", flags.Acts.Gbts.dumpGbtsGeometry)
    kwargs.setdefault("geometryDump", flags.Acts.Gbts.geometryDump)
    acc.setPrivateTools(CompFactory.ActsTrk.GbtsLayerTool(name, **kwargs))
    return acc


def ActsPixelGbtsSeedingToolCfg(flags,
                                name: str = "ActsPixelGbtsSeedingTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if "layerTool" not in kwargs:
        kwargs.setdefault(
            "layerTool",
            acc.popToolsAndMerge(ActsGbtsLayerToolCfg(flags))
        )

    ## For ITkPixel, use default values for ActsTrk::GbtsSeedingTool
    kwargs.setdefault("usePixelLayers", True)
    kwargs.setdefault("useStripLayers", False)
    kwargs.setdefault("connectorInputFile" , find_datafile(flags.Acts.Gbts.connectionTable))
    kwargs.setdefault("lutInputFile" , find_datafile("gbts_ml_pixel_barrel_loose.lut"))
    kwargs.setdefault("minPt" , flags.Tracking.ActiveConfig.minPTSeed *
                      ActsUnits.GeV / GaudiUnits.GeV)
    kwargs.setdefault("d0Max", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("addTriplets", False)

    acc.setPrivateTools(CompFactory.ActsTrk.GbtsSeedingTool(name, **kwargs))
    return acc

def ActsStripGbtsSeedingToolCfg(flags,
                                name: str = "ActsStripGbtsSeedingTool",
                                **kwargs) -> ComponentAccumulator:
    """GBTS strip seeding for the primary pass. ActsLargeRadiusStripGbtsSeedingToolCfg is the LRT one."""
    acc = ComponentAccumulator()
    if "layerTool" not in kwargs:
        kwargs.setdefault(
            "layerTool",
            acc.popToolsAndMerge(ActsGbtsLayerToolCfg(flags))
        )

    kwargs.setdefault("usePixelLayers", False)
    kwargs.setdefault("useStripLayers", True)
    kwargs.setdefault("usePixelConnections", False)
    kwargs.setdefault("useStripConnections", True)
    ## the z0 range comes from the RoI
    kwargs.setdefault("LRTmode", False)
    ## the cluster width cuts are pixel-only
    kwargs.setdefault("useML", False)
    kwargs.setdefault("connectorInputFile", find_datafile(flags.Acts.Gbts.connectionTable))

    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPTSeed *
                      ActsUnits.GeV / GaudiUnits.GeV)
    kwargs.setdefault("d0Max", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    ## the strips have few layers, so also keep the seeds with three space points
    kwargs.setdefault("addTriplets", True)
    ## the strips reach far beyond the pixel default of 550 mm
    kwargs.setdefault("maxOuterRadius", 1100.0)

    acc.setPrivateTools(CompFactory.ActsTrk.GbtsSeedingTool(name, **kwargs))
    return acc

def ActsLargeRadiusStripGbtsSeedingToolCfg(flags,
                                           name: str = "ActsLargeRadiusStripGbtsSeedingTool",
                                           **kwargs) -> ComponentAccumulator:
    ## For ITkStrip LRT, enable LRT mode and use the LRT connector file
    kwargs.setdefault("LRTmode", True)
    kwargs.setdefault("connectorInputFile", find_datafile(flags.Acts.Gbts.connectionTableLrt))
    kwargs.setdefault("filterMaxZ0", 500. * ActsUnits.mm)
    kwargs.setdefault("cutDPhiMax", 0.07)
    kwargs.setdefault("cutDCurvMax", 0.015)
    kwargs.setdefault("tauRatioCut", 0.015)
    kwargs.setdefault("minZ0", -flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("maxZ0", flags.Tracking.ActiveConfig.maxZImpactSeed *
                      ActsUnits.mm / GaudiUnits.mm)
    kwargs.setdefault("minDeltaPhi", 0.01)
    kwargs.setdefault("maxOuterRadius", 1050.0)

    return ActsStripGbtsSeedingToolCfg(flags, name, **kwargs)


def ActsGnnSeedingToolCfg(flags,
                          name: str = "ActsGnnSeedingTool",
                          **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'GnnPipelineTool' not in kwargs:
        from ActsConfig.ActsTrackFindingConfig import ActsGnnPipelineToolCfg
        kwargs.setdefault('GnnPipelineTool', acc.popToolsAndMerge(
            ActsGnnPipelineToolCfg(flags, name="GnnSeedingPipeline")))

    acc.setPrivateTools(CompFactory.ActsTrk.GnnSeedingTool(name, **kwargs))
    return acc

# ACTS algorithm using Athena objects upstream
def ActsPixelSeedingAlgCfg(flags,
                           name: str = 'PixelSeedingAlg',
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    useFastTracking = kwargs.get("useFastTracking", isFastPrimaryPass(flags))

    if "SeedTool" not in kwargs:
        if flags.Tracking.ActiveConfig.PixelSeedingStrategy is SeedingStrategy.Gbts:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
              ActsPixelGbtsSeedingToolCfg(flags)))
        elif flags.Tracking.ActiveConfig.PixelSeedingStrategy is SeedingStrategy.GbtsFtf:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
              ActsGbtsFtfSeedingTrigToolCfg(flags)))
        elif flags.Tracking.ActiveConfig.PixelSeedingStrategy is SeedingStrategy.Gnn:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
              ActsGnnSeedingToolCfg(flags)))
        elif flags.Tracking.ActiveConfig.PixelSeedingStrategy is SeedingStrategy.GridTriplet:
            if useFastTracking:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
                  ActsFastPixelSeedingToolCfg(flags)))
            else:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
                  ActsPixelGridTripletSeedingToolCfg(flags)))

    kwargs.setdefault("useFastTracking", useFastTracking)

    suffix = '_Cached' if flags.Acts.useCache else ''
    pixelSpacePoints = ['ITkPixelSpacePoints' + suffix]
    if flags.Tracking.ActiveConfig.isSecondaryPass:
      pixelSpacePoints = [
        'ITk' + flags.Tracking.ActiveConfig.extension.replace("Acts", "") +
        'PixelSpacePoints' + suffix]
    kwargs.setdefault('InputSpacePoints', pixelSpacePoints)

    kwargs.setdefault('OutputSeeds',
                      f'{flags.Tracking.ActiveConfig.extension}PixelSeeds')
    kwargs.setdefault('UsePixel', True)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkPixelSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
          ActsITkPixelSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.GenericSeedingAlg(
      flags.Tracking.ActiveConfig.extension + name, **kwargs))
    return acc


def ActsStripSeedingAlgCfg(flags,
                           name: str = 'StripSeedingAlg',
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    if "SeedTool" not in kwargs:
        if flags.Tracking.ActiveConfig.StripSeedingStrategy is SeedingStrategy.Gbts:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
              ActsLargeRadiusStripGbtsSeedingToolCfg(flags) if flags.Tracking.ActiveConfig.isLargeD0
              else ActsStripGbtsSeedingToolCfg(flags)))
        elif flags.Tracking.ActiveConfig.StripSeedingStrategy is SeedingStrategy.GbtsFtf:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsLargeRadiusStripGbtsSeedingToolCfg(flags)))
        elif flags.Tracking.ActiveConfig.StripSeedingStrategy is SeedingStrategy.GridTriplet:
            if flags.Tracking.ActiveConfig.isLargeD0:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
                  ActsLargeRadiusStripSeedingToolCfg(flags)))
            else:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(
                  ActsStripGridTripletSeedingToolCfg(flags)))

    suffix = "_Cached" if flags.Acts.useCache else ""
    stripSpacePoints = ['ITkStripSpacePoints' + suffix,
                        'ITkStripOverlapSpacePoints' + suffix]
    if flags.Tracking.ActiveConfig.isSecondaryPass:
      prefix = 'ITk' + flags.Tracking.ActiveConfig.extension.replace("Acts", "")
      stripSpacePoints = [prefix + 'StripSpacePoints' + suffix,
                         prefix + 'StripOverlapSpacePoints' + suffix]
      # Conversion pass does not use overlap space points
      if flags.Tracking.ActiveConfig.extension == 'ActsConversion':
        stripSpacePoints = [prefix + 'StripSpacePoints' + suffix]
    kwargs.setdefault('InputSpacePoints', stripSpacePoints)

    kwargs.setdefault('OutputSeeds',
                      f'{flags.Tracking.ActiveConfig.extension}StripSeeds')
    kwargs.setdefault('UsePixel', False)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkStripSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
          ActsITkStripSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.GenericSeedingAlg(
      flags.Tracking.ActiveConfig.extension + name, **kwargs))
    return acc


def ActsSeedToTrackCnvAlgCfg(flags,
                             name: str = "ActsSeedToTrackCnvAlg",
                             **kwargs: dict) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  kwargs.setdefault(
    'ACTSTracksLocation',
    f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}Tracks')

  acc.addEventAlgo(CompFactory.ActsTrk.SeedToTrackCnvAlg(name, **kwargs))
  return acc


def ActsSeedingCfg(flags) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  processPixels = flags.Tracking.ActiveConfig.useITkPixelSeeding
  processStrips = flags.Tracking.ActiveConfig.useITkStripSeeding

  if processPixels:
    acc.merge(ActsPixelSeedingAlgCfg(flags))
  if processStrips:
    acc.merge(ActsStripSeedingAlgCfg(flags))

  prefix = flags.Tracking.ActiveConfig.extension
  if (flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis):
    if processPixels:
      from ActsConfig.ActsAnalysisConfig import ActsPixelSeedsToTrackParamsAlgCfg
      acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(
        flags, name = prefix + 'PixelSeedsToTrackParamsAlg',
        InputSeedContainerKey = prefix + 'PixelSeeds',
        OutputTrackParamsCollectionKey = prefix + 'PixelEstimatedTrackParams'))

    if processStrips:
      from ActsConfig.ActsAnalysisConfig import ActsStripSeedsToTrackParamsAlgCfg
      acc.merge(ActsStripSeedsToTrackParamsAlgCfg(
        flags, name = prefix + 'StripSeedsToTrackParamsAlg',
        InputSeedContainerKey = prefix + 'StripSeeds',
        OutputTrackParamsCollectionKey = prefix + 'StripEstimatedTrackParams'))

  if flags.Acts.Seeds.doAnalysis:
    if processPixels:
      from ActsConfig.ActsAnalysisConfig import (
        ActsPixelSeedAnalysisAlgCfg, ActsPixelEstimatedTrackParamsAnalysisAlgCfg)
      acc.merge(ActsPixelSeedAnalysisAlgCfg(
        flags, name = prefix + 'PixelSeedAnalysisAlg',
        extension = prefix, InputSeedCollection = prefix + 'PixelSeeds'))

      acc.merge(ActsPixelEstimatedTrackParamsAnalysisAlgCfg(
        flags, name = prefix + 'PixelEstimatedTrackParamsAnalysisAlg',
        extension = prefix,
        InputTrackParamsCollection = prefix + 'PixelEstimatedTrackParams'))

    if processStrips:
      from ActsConfig.ActsAnalysisConfig import (
        ActsStripSeedAnalysisAlgCfg, ActsStripEstimatedTrackParamsAnalysisAlgCfg)
      acc.merge(ActsStripSeedAnalysisAlgCfg(
        flags, name = prefix + 'StripSeedAnalysisAlg',
        extension = prefix, InputSeedCollection = prefix + 'StripSeeds'))

      acc.merge(ActsStripEstimatedTrackParamsAnalysisAlgCfg(
        flags, name = prefix + 'StripEstimatedTrackParamsAnalysisAlg',
        extension = prefix,
        InputTrackParamsCollection = prefix + 'StripEstimatedTrackParams'))

  if flags.Tracking.ActiveConfig.storeTrackSeeds:
    if processPixels:
      acc.merge(ActsStoreTrackSeedsCfg(
        flags, processPixels=True, processStrips=False))
    if processStrips:
      acc.merge(ActsStoreTrackSeedsCfg(
        flags, processPixels=False, processStrips=True))
    if processPixels and processStrips:
      acc.merge(ActsStoreTrackSeedsCfg(
        flags, processPixels=True, processStrips=True))

  return acc


def ActsStoreTrackSeedsCfg(flags,
                           processPixels: bool,
                           processStrips: bool) -> ComponentAccumulator:

  acc = ComponentAccumulator()

  prefix = flags.Tracking.ActiveConfig.extension
  if processPixels and not processStrips:
    prefix = prefix + 'Pixel'
  if processStrips and not processPixels:
    prefix = prefix + 'Strip'
  processBoth = processPixels and processStrips
  tracks = 'SiSPTracksSeedSegments' + prefix + 'Tracks'

  # Convert seed to Acts track
  acc.merge(ActsSeedToTrackCnvAlgCfg(
    flags, name = prefix + 'PixelSeedToTrackCnvAlg',
    EstimatedTrackParametersKey = (
      [prefix + 'PixelEstimatedTrackParams', prefix + 'StripEstimatedTrackParams']
      if processBoth else [prefix + 'EstimatedTrackParams']),
    SeedContainerKey = (
      [prefix + 'PixelSeeds', prefix + 'StripSeeds']
      if processBoth else [prefix + 'Seeds']),
    ACTSTracksLocation = tracks))

  # Truth
  if flags.Tracking.doTruth:
    from ActsConfig.ActsTruthConfig import (
      ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg)
    acc.merge(ActsTrackToTruthAssociationAlgCfg(
      flags, name = prefix + 'SeedTrackToTruthAssociationAlg',
      ACTSTracksLocation = tracks,
      AssociationMapOut = tracks + 'ToTruthParticleAssociation'))

    acc.merge(ActsTrackFindingValidationAlgCfg(
      flags, name = prefix + 'SeedTrackFindingValidationAlg',
      TrackToTruthAssociationMap = tracks + 'ToTruthParticleAssociation'))
            
  # Track Particle creation and persistification
  from InDetConfig.ITkActsParticleCreationConfig import (
    ITkActsTrackParticleCreationCfg)
  acc.merge(ITkActsTrackParticleCreationCfg(
    flags,
    TrackContainers = [tracks],
    TrackParticleContainer = 'SiSPSeedSegments' + prefix + 'TrackParticles'))

  return acc
