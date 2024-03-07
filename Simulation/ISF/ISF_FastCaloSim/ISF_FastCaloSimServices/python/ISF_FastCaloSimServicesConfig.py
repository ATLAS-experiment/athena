"""ComponentAccumulator service configuration for ISF_FastCaloSimServices

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from RngComps.RngCompsConfig import AthRNGSvcCfg
from ISF_Services.ISF_ServicesConfig import TruthServiceCfg
from ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig import FastCaloSimCaloExtrapolationCfg


def PunchThroughClassifierCfg(flags, name="ISF_PunchThroughClassifier", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ScalerConfigFileName"     , "FastCaloSim/MC23/TFCSparam_mpt_classScaler_v04.xml" )
    kwargs.setdefault("NetworkConfigFileName"     , "FastCaloSim/MC23/TFCSparam_mpt_classNet_v04.json" )
    kwargs.setdefault("CalibratorConfigFileName"    , "FastCaloSim/MC23/TFCSparam_mpt_classCalib_v04.xml")
    acc.setPrivateTools(CompFactory.ISF.PunchThroughClassifier(name, **kwargs))
    return acc


def PunchThroughToolCfg(flags, name="ISF_PunchThroughTool", **kwargs):
    from BarcodeServices.BarcodeServicesConfig import BarcodeSvcCfg
    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import EnvelopeDefSvcCfg
    acc = ComponentAccumulator()
    if "PunchThroughClassifier" not in kwargs:
        kwargs.setdefault("PunchThroughClassifier", acc.addPublicTool(acc.popToolsAndMerge(PunchThroughClassifierCfg(flags))))

    kwargs.setdefault("FilenameLookupTable"     , "FastCaloSim/MC23/TFCSparam_mpt_v07.root")
    kwargs.setdefault("FilenameInverseCdf"      , "FastCaloSim/MC23/TFCSparam_mpt_inverseCdf_v07.xml")
    kwargs.setdefault("FilenameInversePca"      , "FastCaloSim/MC23/TFCSparam_mpt_inversePca_v07.xml")
    kwargs.setdefault("EnergyFactor"            , [ 0.98,  0.831, 0.896, 0.652, 0.717, 1., 0.877, 0.858, 0.919 ]    )
    kwargs.setdefault("DoAntiParticles"         , [ 0,   1,    0,     1,     1,     0,   0,    0,    0 ]    )
    kwargs.setdefault("PunchThroughInitiators"  , [ 211, 321, 311, 310, 130, 2212, 2112]        )
    kwargs.setdefault("InitiatorsMinEnergy"     , [ 65536, 65536, 65536, 65536, 65536, 65536, 65536]                                         )
    kwargs.setdefault("InitiatorsEtaRange"      , [ -3.2,   3.2 ]                               )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("CorrelatedParticle"      , []    )
    kwargs.setdefault("FullCorrelationEnergy"   , [ 100000., 100000., 100000., 100000.,      0., 100000., 100000., 100000., 100000.]    )
    kwargs.setdefault("MinEnergy"               , [   938.3,   135.6,     50.,     50.,   105.7,   939.6, 493.7,   497.6,   497.6 ]    )
    kwargs.setdefault("MaxNumParticles"         , [      -1,      -1,      -1,      -1,      -1,    -1,     -1,     -1,     -1 ]    )
    kwargs.setdefault("BarcodeSvc", acc.getPrimaryAndMerge(BarcodeSvcCfg(flags)).name)
    kwargs.setdefault("EnvelopeDefSvc", acc.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags)).name)
    kwargs.setdefault("BeamPipeRadius", 500.)
    acc.setPrivateTools(CompFactory.ISF.PunchThroughTool(name, **kwargs))
    return acc


def EmptyCellBuilderToolCfg(flags, name="ISF_EmptyCellBuilderTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.EmptyCellBuilderTool(name, **kwargs))
    return acc


def FastHitConvertToolCfg(flags, name="ISF_FastHitConvertTool", **kwargs):
    from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg

    acc = ComponentAccumulator()
    mergeable_collection_suffix = "_FastCaloSim"
    region = "CALO"

    EMB_hits_bare_collection_name = "LArHitEMB"
    EMB_hits_merger_input_property = "LArEMBHits"
    acc1, EMB_hits_collection_name = CollectionMergerCfg(
        flags,
        EMB_hits_bare_collection_name,
        mergeable_collection_suffix,
        EMB_hits_merger_input_property,
        region)
    acc.merge(acc1)

    EMEC_hits_bare_collection_name = "LArHitEMEC"
    EMEC_hits_merger_input_property = "LArEMECHits"
    acc2, EMEC_hits_collection_name = CollectionMergerCfg(
        flags,
        EMEC_hits_bare_collection_name,
        mergeable_collection_suffix,
        EMEC_hits_merger_input_property,
        region)
    acc.merge(acc2)

    FCAL_hits_bare_collection_name = "LArHitFCAL"
    FCAL_hits_merger_input_property = "LArFCALHits"
    acc3, FCAL_hits_collection_name = CollectionMergerCfg(
        flags,
        FCAL_hits_bare_collection_name,
        mergeable_collection_suffix,
        FCAL_hits_merger_input_property,
        region)
    acc.merge(acc3)

    HEC_hits_bare_collection_name = "LArHitHEC"
    HEC_hits_merger_input_property = "LArHECHits"
    acc4, HEC_hits_collection_name = CollectionMergerCfg(
        flags,
        HEC_hits_bare_collection_name,
        mergeable_collection_suffix,
        HEC_hits_merger_input_property,
        region)
    acc.merge(acc4)

    tile_hits_bare_collection_name = "TileHitVec"
    tile_hits_merger_input_property = "TileHits"
    acc5, tile_hits_collection_name = CollectionMergerCfg(
        flags,
        tile_hits_bare_collection_name,
        mergeable_collection_suffix,
        tile_hits_merger_input_property,
        region)
    acc.merge(acc5)

    kwargs.setdefault("embHitContainername", EMB_hits_collection_name)
    kwargs.setdefault("emecHitContainername", EMEC_hits_collection_name)
    kwargs.setdefault("fcalHitContainername", FCAL_hits_collection_name)
    kwargs.setdefault("hecHitContainername", HEC_hits_collection_name)

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
    acc.merge( TileSamplingFractionCondAlgCfg(flags) )

    kwargs.setdefault("tileHitContainername", tile_hits_collection_name)

    acc.setPrivateTools(CompFactory.FastHitConvertTool(name, **kwargs))
    return acc


def CaloCellContainerFinalizerToolCfg(flags, name="ISF_CaloCellContainerFinalizerTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CaloCellContainerFinalizerTool(name, **kwargs))
    return acc


def CaloCellContainerFCSFinalizerToolCfg(flags, name="ISF_CaloCellContainerFCSFinalizerTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CaloCellContainerFCSFinalizerTool(name, **kwargs))
    return acc


def FastCaloSimV2ToolCfg(flags, name="ISF_FastCaloSimV2Tool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("CaloCellsOutputName", flags.Sim.FastCalo.CaloCellsName)
    kwargs.setdefault("CaloCellMakerTools_setup",    [acc.addPublicTool(acc.popToolsAndMerge(EmptyCellBuilderToolCfg(flags)))])
    kwargs.setdefault("CaloCellMakerTools_release",  [acc.addPublicTool(acc.popToolsAndMerge(CaloCellContainerFCSFinalizerToolCfg(flags))),
                                                      acc.addPublicTool(acc.popToolsAndMerge(FastHitConvertToolCfg(flags)))])
    kwargs.setdefault("FastCaloSimCaloExtrapolation", acc.addPublicTool(acc.popToolsAndMerge(FastCaloSimCaloExtrapolationCfg(flags))))
    kwargs.setdefault("ParamSvc", acc.getPrimaryAndMerge(FastCaloSimV2ParamSvcCfg(flags)).name)
    kwargs.setdefault("RandomSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("RandomStream", "FastCaloSimRnd")

    if "PunchThroughTool" not in kwargs:
        kwargs.setdefault("PunchThroughTool", acc.popToolsAndMerge(PunchThroughToolCfg(flags)))

    if "ParticleTruthSvc" not in kwargs:
        kwargs.setdefault("ParticleTruthSvc", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    acc.setPrivateTools(CompFactory.ISF.FastCaloSimV2Tool(name, **kwargs))
    return acc


###################################################################################################
# Config

def FastCaloSimV2ParamSvcCfg(flags, name="ISF_FastCaloSimV2ParamSvc", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ParamsInputFilename", flags.Sim.FastCalo.ParamsInputFilename)
    kwargs.setdefault("RunOnGPU", flags.Sim.FastCalo.RunOnGPU)
    kwargs.setdefault("ParamsInputObject", "SelPDGID")
    acc.addService(CompFactory.ISF.FastCaloSimV2ParamSvc(name, **kwargs), primary = True)
    return acc


def FastCaloSimV2SvcCfg(flags, name="ISF_FastCaloSimSvcV2", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SimulatorTool", acc.addPublicTool(acc.popToolsAndMerge(FastCaloSimV2ToolCfg(flags))))
    kwargs.setdefault("Identifier", "FastCaloSim")
    acc.addService(CompFactory.ISF.LegacySimSvc(name, **kwargs), primary = True)
    return acc


def DNNCaloSimSvcCfg(flags, name="ISF_DNNCaloSimSvc", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CaloCellsOutputName", flags.Sim.FastCalo.CaloCellsName)
    kwargs.setdefault("CaloCellMakerTools_setup",   [acc.addPublicTool(acc.popToolsAndMerge(EmptyCellBuilderToolCfg(flags)))])
    kwargs.setdefault("CaloCellMakerTools_release", [acc.addPublicTool(acc.popToolsAndMerge(CaloCellContainerFinalizerToolCfg(flags))),
                                                     acc.addPublicTool(acc.popToolsAndMerge(FastHitConvertToolCfg(flags)))]) #DR needed ?
    kwargs.setdefault("ParamsInputFilename", flags.Sim.FastCalo.ParamsInputFilename)
    kwargs.setdefault("FastCaloSimCaloExtrapolation", acc.addPublicTool(acc.popToolsAndMerge(FastCaloSimCaloExtrapolationCfg(flags))))
    kwargs.setdefault("RandomStream", "FastCaloSimRnd")
    kwargs.setdefault("RandomSvc",
                      acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.addService(CompFactory.ISF.DNNCaloSimSvc(name, **kwargs), primary = True)
    return acc
