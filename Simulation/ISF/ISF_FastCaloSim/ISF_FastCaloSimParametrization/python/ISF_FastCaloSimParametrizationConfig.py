"""ComponentAccumulator config of tools for ISF_FastCaloSimParametrization

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def ISF_HitAnalysisCfg(flags, name="ISF_HitAnalysis",
                       NTruthParticles=1, saveAllBranches=False,
                       doG4Hits=False, doClusterInfo=False,
                       outputGeoFileName=None, **kwargs):
    result = ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge( LArGMCfg(flags) )
    kwargs.setdefault("CaloDetDescrManager", "CaloDetDescrManager")

    from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
    result.merge( TileSamplingFractionCondAlgCfg(flags) )
    kwargs.setdefault("TileSamplingFraction", "TileSamplingFraction")

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    kwargs.setdefault("TileCablingSvc", result.getPrimaryAndMerge(TileCablingSvcCfg(flags)).name)

    kwargs.setdefault("NtupleFileName", 'ISF_HitAnalysis')
    kwargs.setdefault("GeoFileName", 'ISF_Geometry')
    histOutputArray = ["ISF_HitAnalysis DATAFILE='%s' OPT='RECREATE'" % (flags.Output.HISTFileName)] # FIXME top level directory name
    if outputGeoFileName:
        histOutputArray += ["ISF_Geometry DATAFILE='%s' OPT='RECREATE'" % (outputGeoFileName)] # FIXME top level directory name
    result.addService(CompFactory.THistSvc(Output=histOutputArray))
    kwargs.setdefault("NTruthParticles", NTruthParticles)

    from FastCaloSim.FastCaloSimFactoryNew import NITimedExtrapolatorCfg
    kwargs.setdefault("Extrapolator", result.addPublicTool(result.popToolsAndMerge(NITimedExtrapolatorCfg(flags))))
    kwargs.setdefault("CaloCoordinateTool", result.addPublicTool(CompFactory.TBCaloCoordinate("TBCaloCoordinate")))
    kwargs.setdefault("CaloEntrance", 'InDet::Containers::InnerDetector') #FIXME should this be configurable?
    kwargs.setdefault("FastCaloSimCaloExtrapolation", result.addPublicTool(result.popToolsAndMerge(FastCaloSimCaloExtrapolationCfg(flags))))

    kwargs.setdefault("CaloBoundaryR", 1148.0)
    kwargs.setdefault("CaloBoundaryZ", 3550.0) #before: 3475.0
    kwargs.setdefault("CaloMargin", 100) #=10cm
    kwargs.setdefault("SaveAllBranches", saveAllBranches) #FIXME
    kwargs.setdefault("DoAllCells", False)
    kwargs.setdefault("DoLayers", True)
    kwargs.setdefault("DoLayerSums", True)
    kwargs.setdefault("DoG4Hits", doG4Hits) #FIXME
    kwargs.setdefault("DoClusterInfo", doClusterInfo) #FIXME
    kwargs.setdefault("TimingCut", 999999)

    result.addService(CompFactory.PartPropSvc(InputFile="PDGTABLE.MeV"))

    result.addEventAlgo(CompFactory.ISF_HitAnalysis(name,**kwargs))
    return result

def FastCaloSimCaloTransportationCfg(flags, name="FastCaloSimCaloTransportation", **kwargs):
    acc = ComponentAccumulator()
    from FastCaloSim.FastCaloSimFactoryNew import NITimedExtrapolatorCfg
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(NITimedExtrapolatorCfg(flags))))
    kwargs.setdefault("CaloEntrance", 'InDet::Containers::InnerDetector')
    acc.setPrivateTools(CompFactory.FastCaloSimCaloTransportation(name, **kwargs))
    return acc

def FastCaloSimCaloExtrapolationCfg(flags, name="FastCaloSimCaloExtrapolation", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CaloBoundaryR", [1148.0, 120.0, 41.0])
    kwargs.setdefault("CaloBoundaryZ", [3550.0, 4587.0, 4587.0])
    kwargs.setdefault("CaloGeometryHelper", acc.addPublicTool(acc.popToolsAndMerge(FastCaloSimGeometryHelperCfg(flags))))
    kwargs.setdefault("CaloTransportation", acc.addPublicTool(acc.popToolsAndMerge(FastCaloSimCaloTransportationCfg(flags))))
    acc.setPrivateTools(CompFactory.FastCaloSimCaloExtrapolation(name, **kwargs))
    return acc

def FastCaloSimGeometryHelperCfg(flags, name="FastCaloSimGeometryHelper", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.FastCaloSimGeometryHelper(name, **kwargs))
    return acc

def CaloCellContainerSDCfg(flags, name='CaloCellContainerSD', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault ('NoVolumes', True)
    kwargs.setdefault("OutputCollectionNames", ["DefaultCaloCellContainer"])
    # Add ISF_FastHitConvertTool, which will also take care 
    # of creating mergable FastCaloSim HIT collections
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastHitConvertToolCfg
    kwargs.setdefault("FastHitConvertTool",  acc.addPublicTool(acc.popToolsAndMerge(FastHitConvertToolCfg(flags))))
    acc.setPrivateTools(CompFactory.CaloCellContainerSDTool(name, **kwargs))
    return acc

def ISF_FastCaloSimParametrization_SimPreInclude(flags):
    flags.Sim.RecordStepInfo=True
    from SimulationConfig.SimEnums import VertexSource,LArParameterization,CalibrationRun
    #No vertex smearing
    flags.Sim.VertexSource=VertexSource.AsGenerated
    # Deactivated G4Optimizations
    #MuonFieldOnlyInCalo
    flags.Sim.MuonFieldOnlyInCalo=False
    #NRR
    flags.Sim.NRRThreshold=False
    flags.Sim.NRRWeight=False
    #PRR
    flags.Sim.PRRThreshold=False
    flags.Sim.PRRWeight=False
    #Frozen Showers
    flags.Sim.LArParameterization=LArParameterization.NoFrozenShowers
    flags.Sim.CalibrationRun=CalibrationRun.DeadLAr
    flags.GeoModel.Align.LegacyConditionsAccess = False

def PostIncludeISF_FastCaloSimParametrizationConditions(flags, cfg):
    from IOVDbSvc.IOVDbSvcConfig import addOverride
    cfg.merge(addOverride(flags, "/LAR/BadChannels/BadChannels", tag="LARBadChannelsBadChannels-MC-empty", db="COOLOFL_LAR/OFLP200"))
    cfg.merge(addOverride(flags, "/TILE/OFL02/STATUS/ADC", tag="TileOfl02StatusAdc-EmptyBCh", db="COOLOFL_TILE/OFLP200"))


def PostIncludeISF_FastCaloSimParametrizationDigi(flags, cfg):
    # TODO write an OutputStreamConfig.addToRDO method?
    RDO_ItemList = [
        "ISF_FCS_Parametrization::FCS_StepInfoCollection#MergedEventSteps",
        "LArHitContainer#*",
        "TileHitVector#*"
    ]
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, "RDO", RDO_ItemList))

    puAlg = cfg.getEventAlgo("StandardPileUpToolsAlg")
    puAlg.PileUpTools["LArPileUpTool"].CrossTalk = False
    puAlg.PileUpTools["TileHitVecToCntTool"].HitTimeFlag = 1
    puAlg.PileUpTools["TileHitVecToCntTool"].usePhotoStatistics = False

    cfg.getEventAlgo("TileDigitsMaker").IntegerDigits = True

    PostIncludeISF_FastCaloSimParametrizationConditions(flags,cfg)


def PostIncludeISF_FastCaloSimParametrizationReco(flags, cfg):
    ESD_ItemList = [
        "ISF_FCS_Parametrization::FCS_StepInfoCollection#MergedEventSteps",
        "LArHitContainer#*",
        "McEventCollection#TruthEvent",
        "TileHitVector#*",
        "TrackRecordCollection#CaloEntryLayer",
        "TrackRecordCollection#MuonEntryLayer"
    ]
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD
    cfg.merge(addToESD(flags, ESD_ItemList))

    PostIncludeISF_FastCaloSimParametrizationConditions(flags,cfg)
