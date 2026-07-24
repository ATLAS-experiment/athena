# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""ComponentAccumulator config of tools for ISF_FastCaloSimParametrization

NOTE: the parametrization-input ntuple algorithm (formerly ISF_HitAnalysis) now
lives in G4FastSimulation as FastCaloSimParamHitAnalysis, using the same
external FastCaloSim transport+extrapolation as the G4 FastCaloSim fast-sim
model. It still runs offline from ESD, via ESDtoNTUP_FCS_Skeleton.py. See
G4FastSimulation.G4FastSimulationConfig.FastCaloSimParamHitAnalysisCfg.
"""
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def FastCaloSimGeometryHelperCfg(flags, name="FastCaloSimGeometryHelper", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.FastCaloSimGeometryHelper(name, **kwargs))
    return acc

# CaloCellContainerSDCfg has moved to the G4AtlasTools package (so that it is
# available in AthSimulation); the SD C++ now lives in G4FastSimulation.
# Re-exported here for back-compat.
from G4AtlasTools.G4AtlasToolsConfig import CaloCellContainerSDCfg  # noqa: F401

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
