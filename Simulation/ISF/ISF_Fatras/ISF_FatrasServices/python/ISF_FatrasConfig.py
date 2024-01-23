# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = "New configuration for the ISF_FatrasSimTool"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg
from ISF_Tools.ISF_ToolsConfig import ParticleHelperCfg
from ISF_Services.ISF_ServicesConfig import (
    ATLFAST_ParticleBrokerSvcCfg, TruthServiceCfg
)
from ISF_Geant4Tools.ISF_Geant4ToolsConfig import G4RunManagerHelperCfg
from RngComps.RandomServices import dSFMT, AthRNGSvcCfg

def TrkExRndSvcCfg(flags):
    seed = 'TrkExRnd OFFSET 0 12412330 37849324'
    return dSFMT(seed)


def TrkExRndSvcMTCfg(flags):
    return AthRNGSvcCfg(flags)


def FatrasRndSvcCfg(flags):
    seed = 'FatrasRnd OFFSET 0 81234740 23474923'
    return dSFMT(seed)


################################################################################
# HIT CREATION SECTION
################################################################################


#   Fatras Hadronic Interaction Processor
#   hadronic interaction creator
def fatrasHitCreatorPixelCfg(flags, name="ISF_FatrasHitCreatorPixel", **kwargs):
    """Return ISF_FatrasHitCreatorPixel configured with ComponentAccumulator"""
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    bare_collection_name = "PixelHits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "PixelHits"
    region = "ID"

    result, hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)
    kwargs.setdefault("IdHelperName", 'PixelID')
    kwargs.setdefault("CollectionName", hits_collection_name)

    kwargs.setdefault("ConditionsTool", "")
    kwargs.setdefault("UseConditionsTool", False)

    result.setPrivateTools(CompFactory.iFatras.HitCreatorSilicon(name=name, **kwargs))
    return result


def fatrasHitCreatorSCTCfg(flags, name="ISF_FatrasHitCreatorSCT", **kwargs):
    """Return ISF_FatrasHitCreatorSCT configured with ComponentAccumulator"""
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    bare_collection_name = "SCT_Hits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "SCTHits"
    region = "ID"

    result, hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)
    kwargs.setdefault("IdHelperName", 'SCT_ID')
    kwargs.setdefault("CollectionName", hits_collection_name)

    kwargs.setdefault("ConditionsTool", "")
    kwargs.setdefault("UseConditionsTool", False)

    result.setPrivateTools(CompFactory.iFatras.HitCreatorSilicon(name=name, **kwargs))
    return result


def fatrasHitCreatorTRTCfg(flags, name="ISF_FatrasHitCreatorTRT", **kwargs):
    """Return ISF_FatrasHitCreatorTRT configured with ComponentAccumulator"""
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    bare_collection_name = "TRTUncompressedHits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "TRTUncompressedHits"
    region = "ID"

    result, hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)
    kwargs.setdefault("CollectionName", hits_collection_name)

    kwargs.setdefault("StrawStatusSummaryTool", "")
    result.setPrivateTools(CompFactory.iFatras.HitCreatorTRT(name=name, **kwargs))
    return result


def fatrasPileupHitCreatorPixelCfg(flags, name="ISF_FatrasPileupHitCreatorPixel", **kwargs):
    """Return ISF_FatrasHitCreatorPixel configured for pileup with ComponentAccumulator"""
    kwargs.setdefault("CollectionName", "PileupPixelHits")
    return fatrasHitCreatorPixelCfg(flags, name, **kwargs)


def fatrasPileupHitCreatorSCTCfg(flags, name="ISF_FatrasPileupHitCreatorSCT", **kwargs):
    """Return ISF_FatrasHitCreatorSCT configured for pileup with ComponentAccumulator"""
    kwargs.setdefault("CollectionName", "PileupSCT_Hits")
    return fatrasHitCreatorSCTCfg(flags, name, **kwargs)


def fatrasPileupHitCreatorTRTCfg(flags, name="ISF_FatrasPileupHitCreatorTRT", **kwargs):
    """Return ISF_FatrasHitCreatorTRT configured with ComponentAccumulator"""
    kwargs.setdefault("CollectionName", "PileupTRTUncompressedHits")
    return fatrasHitCreatorTRTCfg(flags, name, **kwargs)


################################################################################
# TRACK CREATION SECTION
################################################################################
def fatrasSimHitCreatorIDCfg(flags, name="ISF_FatrasSimHitCreatorID", **kwargs):
    """Return ISF_FatrasSimHitCreatorID configured with ComponentAccumulator"""

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()
    kwargs.setdefault("PixelHitCreator", result.addPublicTool(result.popToolsAndMerge(fatrasHitCreatorPixelCfg(flags))))
    kwargs.setdefault("SctHitCreator", result.addPublicTool(result.popToolsAndMerge(fatrasHitCreatorSCTCfg(flags))))
    kwargs.setdefault("TrtHitCreator", result.addPublicTool(result.popToolsAndMerge(fatrasHitCreatorTRTCfg(flags))))
    kwargs.setdefault("OutputLevel", flags.Exec.OutputLevel)
    result.setPrivateTools(CompFactory.iFatras.SimHitCreatorID(name=name, **kwargs))
    return result


def fatrasPileupSimHitCreatorIDCfg(flags, name="ISF_FatrasPileupSimHitCreatorID", **kwargs):
    """Return ISF_FatrasSimHitCreatorID configured for pileup with ComponentAccumulator"""

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()
    kwargs.setdefault("PixelHitCreator", result.addPublicTool(result.popToolsAndMerge(fatrasPileupHitCreatorPixelCfg(flags))))
    kwargs.setdefault("SctHitCreator", result.addPublicTool(result.popToolsAndMerge(fatrasPileupHitCreatorSCTCfg(flags))))
    kwargs.setdefault("TrtHitCreator", result.addPublicTool(result.popToolsAndMerge(fatrasPileupHitCreatorTRTCfg(flags))))
    return fatrasSimHitCreatorIDCfg(flags, name, **kwargs)


def fatrasSimHitCreatorMSCfg(flags, name="ISF_FatrasSimHitCreatorMS", **kwargs):
    """Return ISF_FatrasSimHitCreatorMS configured with ComponentAccumulator"""

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')


    result = ComponentAccumulator()
    mergeable_collection_suffix = "_Fatras"
    region = "MUON"

    mdt_bare_collection_name="MDT_Hits"
    mdt_merger_input_property="MDTHits"
    mdt_result, mdt_hits_collection_name = CollectionMergerCfg(flags,
                                                               mdt_bare_collection_name,
                                                               mergeable_collection_suffix,
                                                               mdt_merger_input_property,
                                                               region)
    result.merge(mdt_result)

    rpc_bare_collection_name="RPC_Hits"
    rpc_merger_input_property="RPCHits"
    rpc_result, rpc_hits_collection_name = CollectionMergerCfg(flags,
                                                               rpc_bare_collection_name,
                                                               mergeable_collection_suffix,
                                                               rpc_merger_input_property,
                                                               region)
    result.merge(rpc_result)

    tgc_bare_collection_name="TGC_Hits"
    tgc_merger_input_property="TGCHits"
    tgc_result, tgc_hits_collection_name = CollectionMergerCfg(flags,
                                                               tgc_bare_collection_name,
                                                               mergeable_collection_suffix,
                                                               tgc_merger_input_property,
                                                               region)
    result.merge(tgc_result)

    csc_hits_collection_name = ""
    if flags.Detector.EnableCSC:
        csc_bare_collection_name="CSC_Hits"
        csc_merger_input_property="CSCHits"
        csc_result, csc_hits_collection_name = CollectionMergerCfg(flags,
                                                                   csc_bare_collection_name,
                                                                   mergeable_collection_suffix,
                                                                   csc_merger_input_property,
                                                                   region)
        result.merge(csc_result)

    stgc_hits_collection_name = ""
    if flags.Detector.EnablesTGC:
        stgc_bare_collection_name="sTGC_Hits"
        stgc_merger_input_property="sTGCHits"
        stgc_result, stgc_hits_collection_name = CollectionMergerCfg(flags,
                                                                     stgc_bare_collection_name,
                                                                     mergeable_collection_suffix,
                                                                     stgc_merger_input_property,
                                                                     region)
        result.merge(stgc_result)

    mm_hits_collection_name = ""
    if flags.Detector.EnableMM:
        mm_bare_collection_name="MM_Hits"
        mm_merger_input_property="MMHits"
        mm_result, mm_hits_collection_name = CollectionMergerCfg(flags,
                                                                 mm_bare_collection_name,
                                                                 mergeable_collection_suffix,
                                                                 mm_merger_input_property,
                                                                 region)
        result.merge(mm_result)

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)

    kwargs.setdefault("Extrapolator" , result.addPublicTool(result.popToolsAndMerge(fatrasExtrapolatorCfg(flags))))

    kwargs.setdefault("MDTCollectionName", mdt_hits_collection_name)
    kwargs.setdefault("RPCCollectionName", rpc_hits_collection_name)
    kwargs.setdefault("TGCCollectionName", tgc_hits_collection_name)
    kwargs.setdefault("CSCCollectionName", csc_hits_collection_name)
    kwargs.setdefault("sTGCCollectionName", stgc_hits_collection_name)
    kwargs.setdefault("MMCollectionName", mm_hits_collection_name)

    Muon__MuonTGMeasurementTool = CompFactory.Muon.MuonTGMeasurementTool
    muon_tgmeasurement_tool = Muon__MuonTGMeasurementTool(name='MuonTGMeasurementTool',
                                                          UseDSManager=True)
    kwargs.setdefault("MeasurementTool", muon_tgmeasurement_tool)

    result.setPrivateTools(CompFactory.iFatras.SimHitCreatorMS(name=name, **kwargs))
    return result


def fatrasPdgG4ParticleCfg(flags, name="ISF_FatrasPdgG4Particle", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    result.setPrivateTools(CompFactory.iFatras.PDGToG4Particle(name=name, **kwargs))
    return result

######################################################################################
# validation & process sampling
######################################################################################


def fatrasPhysicsValidationToolCfg(flags, name="ISF_FatrasPhysicsValidationTool", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()
    kwargs.setdefault("ValidationStreamName", "ISFFatras")

    result.setPrivateTools(CompFactory.iFatras.PhysicsValidationTool(name=name, **kwargs))
    return result


def fatrasParticleDecayHelperCfg(flags, name="ISF_FatrasParticleDecayHelper", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    seed = 'FatrasG4 OFFSET 0 23491234 23470291'
    result.merge(dSFMT(seed))
    kwargs.setdefault("RandomNumberService", result.getService("AtDSFMTGenSvc").name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)
    kwargs.setdefault("G4RandomStreamName", flags.Sim.Fatras.G4RandomStreamName)
    kwargs.setdefault("ValidationMode", flags.Sim.ISF.ValidationMode)

    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)).name)

    if "TruthRecordSvc" not in kwargs:
        kwargs.setdefault("ParticleTruthSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    kwargs.setdefault("PDGToG4ParticleConverter", result.addPublicTool(result.popToolsAndMerge(fatrasPdgG4ParticleCfg(flags))))
    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))
    kwargs.setdefault("G4RunManagerHelper", result.addPublicTool(result.popToolsAndMerge(G4RunManagerHelperCfg(flags))))
    result.setPrivateTools(CompFactory.iFatras.G4ParticleDecayHelper(name=name, **kwargs))
    return result


################################################################################
# Extrapolator
################################################################################
# the definition of an extrapolator (to be cleaned up)

def fatrasEnergyLossUpdatorCfg(flags, name="ISF_FatrasEnergyLossUpdator", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)

    kwargs.setdefault("UsePDG_EnergyLossFormula", True)
    kwargs.setdefault("EnergyLossDistribution", 2)

    from TrkConfig.AtlasExtrapolatorToolsConfig import AtlasEnergyLossUpdatorCfg
    kwargs.setdefault("EnergyLossUpdator", result.popToolsAndMerge(AtlasEnergyLossUpdatorCfg(flags)))

    result.setPrivateTools(CompFactory.iFatras.McEnergyLossUpdator(name=name, **kwargs))
    return result


# Combining all in the MaterialEffectsUpdator
def fatrasMaterialUpdatorCfg(flags, name="ISF_FatrasMaterialUpdator", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)
    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)).name)

    if "TruthRecordSvc" not in kwargs:
        kwargs.setdefault("TruthRecordSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    # @TODO retire once migration to TrackingGeometry conditions data is complete
    if not flags.Sim.ISF.UseTrackingGeometryCond:
        if 'TrackingGeometrySvc' not in kwargs:
            from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
            kwargs.setdefault("TrackingGeometrySvc", result.getPrimaryAndMerge(TrackingGeometrySvcCfg(flags)).name)
            kwargs.setdefault("TrackingGeometryReadKey", '')
    else:
        if 'TrackingGeometryReadKey' not in kwargs:
            from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
            acc = TrackingGeometryCondAlgCfg(flags)
            geom_cond_key = acc.getPrimary().TrackingGeometryWriteKey
            result.merge(acc)
            kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)

    # hadronic interactions
    kwargs.setdefault("HadronicInteraction", True)

    kwargs.setdefault("HadronicInteractionProcessor", result.addPublicTool(result.popToolsAndMerge(fatrasG4HadIntProcessorCfg(flags))))

    # energy loss
    kwargs.setdefault("EnergyLoss", True)
    kwargs.setdefault("EnergyLossUpdator", result.addPublicTool(result.popToolsAndMerge(fatrasEnergyLossUpdatorCfg(flags))))

    # mutiple scattering
    kwargs.setdefault("MultipleScattering", True)
    from TrkConfig.AtlasExtrapolatorToolsConfig import fatrasMultipleScatteringUpdatorCfg
    kwargs.setdefault("MultipleScatteringUpdator", result.addPublicTool(result.popToolsAndMerge(fatrasMultipleScatteringUpdatorCfg(flags))))

    # photon conversion
    kwargs.setdefault("PhotonConversionTool", result.addPublicTool(result.popToolsAndMerge(fatrasConversionCreatorCfg(flags))))

    # the validation output
    kwargs.setdefault("ValidationMode", flags.Sim.ISF.ValidationMode)
    kwargs.setdefault("BremPhotonValidation", False)
    kwargs.setdefault("EnergyDepositValidation", False)

    kwargs.setdefault("MomentumCut", flags.Sim.Fatras.MomCutOffSec)
    kwargs.setdefault("MinimumBremPhotonMomentum", flags.Sim.Fatras.MomCutOffSec)

    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))

    kwargs.setdefault("ProcessSamplingTool", result.addPublicTool(result.popToolsAndMerge(fatrasProcessSamplingToolCfg(flags))))

    kwargs.setdefault("ParticleDecayHelper", result.addPublicTool(result.popToolsAndMerge(fatrasParticleDecayHelperCfg(flags))))

    # MCTruth Process Code
    kwargs.setdefault("BremProcessCode", 3)  # TODO: to be taken from central definition

    result.setPrivateTools(CompFactory.iFatras.McMaterialEffectsUpdator(name=name, **kwargs))
    return result


def fatrasExtrapolatorCfg(flags, name="ISF_FatrasExtrapolator", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    # Charged Transport Tool
    # assign the tools
    from TrkConfig.AtlasExtrapolatorToolsConfig import FastSimNavigatorCfg
    kwargs.setdefault("Navigator", result.addPublicTool(result.popToolsAndMerge(FastSimNavigatorCfg(flags))))

    kwargs.setdefault("MaterialEffectsUpdators", [result.addPublicTool(result.popToolsAndMerge(fatrasMaterialUpdatorCfg(flags)))])

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    kwargs.setdefault("Propagators",
                      [result.addPublicTool(result.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, name="ISF_FatrasChargedPropagator")))])

    from TrkConfig.TrkExSTEP_PropagatorConfig import fatrasSTEP_PropagatorCfg
    kwargs.setdefault("STEP_Propagator", result.addPublicTool(result.popToolsAndMerge(fatrasSTEP_PropagatorCfg(flags))))

    # Fatras specific: stop the trajectory
    kwargs.setdefault("StopWithNavigationBreak", True)
    kwargs.setdefault("StopWithUpdateKill", True)
    kwargs.setdefault("RobustSampling", True)
    kwargs.setdefault("ResolveMuonStation", True)
    kwargs.setdefault("UseMuonMatApproximation", True)

    result.setPrivateTools(CompFactory.Trk.TimedExtrapolator(name=name, **kwargs))
    return result


################################################################################
# SIMULATION TOOL and SERVICE
################################################################################
def fatrasKinematicFilterCfg(flags, name="ISF_FatrasKinematicFilter", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("MaxEtaSymmetric", 10.)
    kwargs.setdefault("MinMomentum", flags.Sim.Fatras.MomCutOffSec)

    result.setPrivateTools(CompFactory.ISF.KinematicParticleFilter(name=name, **kwargs))
    return result


def fatrasConversionCreatorCfg(flags, name="ISF_FatrasConversionCreator", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)

    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)).name)

    if "TruthRecordSvc" not in kwargs:
        kwargs.setdefault("TruthRecordSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))

    kwargs.setdefault("PhysicsProcessCode", 14)  # TODO: to be taken from central definition
    kwargs.setdefault("ValidationMode", flags.Sim.ISF.ValidationMode)

    result.setPrivateTools(CompFactory.iFatras.PhotonConversionTool(name=name, **kwargs))
    return result


def fatrasG4HadIntProcessorCfg(flags, name="ISF_FatrasG4HadIntProcessor", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)

    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)).name)

    if "TruthRecordSvc" not in kwargs:
        kwargs.setdefault("TruthRecordSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))

    kwargs.setdefault("ValidationMode", flags.Sim.ISF.ValidationMode)
    kwargs.setdefault("MomentumCut", flags.Sim.Fatras.MomCutOffSec)

    kwargs.setdefault("G4RunManagerHelper", result.addPublicTool(result.popToolsAndMerge(G4RunManagerHelperCfg(flags))))

    result.setPrivateTools(CompFactory.iFatras.G4HadIntProcessor(name=name, **kwargs))
    return result


#   Fatras Hadronic Interaction Processor
def fatrasParametricHadIntProcessorCfg(flags, name="ISF_FatrasParametricHadIntProcessor", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", flags.Sim.Fatras.RandomStreamName)

    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)).name)

    if "TruthRecordSvc" not in kwargs:
        kwargs.setdefault("TruthRecordSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    kwargs.setdefault("HadronicInteractionScaleFactor", flags.Sim.Fatras.HadronIntProb)
    kwargs.setdefault("MinimumHadronicInitialEnergy", flags.Sim.Fatras.MomCutOffSec)
    kwargs.setdefault("MinimumHadronicOutEnergy", flags.Sim.Fatras.MomCutOffSec)
    kwargs.setdefault("HadronicInteractionValidation", False)
    kwargs.setdefault("PhysicsProcessCode", 121)  # TODO: to be taken from central definition
    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))
    kwargs.setdefault("ValidationMode", flags.Sim.ISF.ValidationMode)

    result.setPrivateTools(CompFactory.iFatras.HadIntProcessorParametric(name, **kwargs))
    return result


def fatrasProcessSamplingToolCfg(flags, name="ISF_FatrasProcessSamplingTool", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)

    # truth record
    if "TruthRecordSvc" not in kwargs:
        kwargs.setdefault("TruthRecordSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    # decays
    kwargs.setdefault("ParticleDecayHelper", result.addPublicTool(result.popToolsAndMerge(fatrasParticleDecayHelperCfg(flags))))

    # photon conversion
    kwargs.setdefault("PhotonConversionTool", result.addPublicTool(result.popToolsAndMerge(fatrasConversionCreatorCfg(flags))))

    # Hadronic interactions
    kwargs.setdefault("HadronicInteractionProcessor", result.addPublicTool(result.popToolsAndMerge(fatrasG4HadIntProcessorCfg(flags))))
    kwargs.setdefault("HadronicInteraction", True)

    # Validation Tool
    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))
    kwargs.setdefault("ValidationMode", flags.Sim.ISF.ValidationMode)

    result.setPrivateTools(CompFactory.iFatras.ProcessSamplingTool(name=name, **kwargs))
    return result


def fatrasTransportToolCfg(flags, name="ISF_FatrasSimTool", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    if "SimHitCreatorID" not in kwargs:
        kwargs.setdefault("SimHitCreatorID", result.addPublicTool(result.popToolsAndMerge(fatrasSimHitCreatorIDCfg(flags))))

    kwargs.setdefault("SimHitCreatorMS", result.addPublicTool(result.popToolsAndMerge(fatrasSimHitCreatorMSCfg(flags))))

    kwargs.setdefault("ParticleDecayHelper", result.addPublicTool(result.popToolsAndMerge(fatrasParticleDecayHelperCfg(flags))))

    kwargs.setdefault("ParticleHelper", result.addPublicTool(result.popToolsAndMerge(ParticleHelperCfg(flags))))

    publicKinFilter = result.addPublicTool(result.popToolsAndMerge(fatrasKinematicFilterCfg(flags)))
    kwargs.setdefault("TrackFilter", publicKinFilter)
    kwargs.setdefault("NeutralFilter", publicKinFilter)
    kwargs.setdefault("PhotonFilter", publicKinFilter)

    kwargs.setdefault("Extrapolator", result.addPublicTool(result.popToolsAndMerge(fatrasExtrapolatorCfg(flags))))

    kwargs.setdefault("PhysicsValidationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPhysicsValidationToolCfg(flags))))

    kwargs.setdefault("ProcessSamplingTool", result.addPublicTool(result.popToolsAndMerge(fatrasProcessSamplingToolCfg(flags))))

    kwargs.setdefault("OutputLevel", flags.Exec.OutputLevel)
    kwargs.setdefault("ValidationOutput", flags.Sim.ISF.ValidationMode)

    kwargs.setdefault("RandomNumberService", result.getPrimaryAndMerge(FatrasRndSvcCfg(flags)).name)

    result.setPrivateTools(CompFactory.iFatras.TransportTool(name=name, **kwargs))
    return result


def fatrasPileupSimToolCfg(flags, name="ISF_FatrasPileupSimTool", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("SimHitCreatorID", result.addPublicTool(result.popToolsAndMerge(fatrasPileupSimHitCreatorIDCfg(flags))))

    result.setPrivateTools(result.popToolsAndMerge(fatrasTransportToolCfg(flags, name, **kwargs)))

    return result


# FatrasSimulatorTool
def fatrasSimulatorToolSTCfg(flags, name="ISF_FatrasSimulatorToolST", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    if "IDSimulationTool" not in kwargs or "SimulationTool" not in kwargs:
        publicTransportTool = result.addPublicTool(result.popToolsAndMerge(fatrasTransportToolCfg(flags, **kwargs)))
        kwargs.setdefault("IDSimulationTool", publicTransportTool)
        kwargs.setdefault("SimulationTool", publicTransportTool)

    result.setPrivateTools(CompFactory.ISF.FatrasSimTool(name, **kwargs))
    return result


def fatrasPileupSimulatorToolSTCfg(flags, name="ISF_FatrasPileupSimulatorToolST", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    kwargs.setdefault("IDSimulationTool", result.addPublicTool(result.popToolsAndMerge(fatrasPileupSimToolCfg(flags))))
    kwargs.setdefault("SimulationTool", result.addPublicTool(result.popToolsAndMerge(fatrasTransportToolCfg(flags))))

    result.setPrivateTools(result.popToolsAndMerge(fatrasSimulatorToolSTCfg(flags, name, **kwargs)))
    return result


# FatrasSimulationSvc
def fatrasSimServiceIDCfg(flags, name="ISF_FatrasSimSvc", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()
    kwargs.setdefault("Identifier", "Fatras")
    kwargs.setdefault("SimulatorTool", result.addPublicTool(result.popToolsAndMerge(fatrasSimulatorToolSTCfg(flags))))
    result.addService(CompFactory.ISF.LegacySimSvc(name, **kwargs), primary = True)
    return result


def fatrasPileupSimServiceIDCfg(flags, name="ISF_FatrasPileupSimSvc", **kwargs):
    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    toolAcc = ComponentAccumulator()
    kwargs.setdefault("SimulatorTool", toolAcc.addPublicTool(toolAcc.popToolsAndMerge(fatrasPileupSimulatorToolSTCfg(flags))))
    result = fatrasSimServiceIDCfg(flags, name, **kwargs)
    result.merge(toolAcc)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.Files = defaultTestFiles.HITS_RUN2
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    acc.popToolsAndMerge(fatrasTransportToolCfg(flags))

    print("INFO_FatrasConfig: Dumping config flags")
    flags.dump()
    print("INFO_FatrasConfig: Print config details")
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.store(open('fatrassimtool.pkl', 'wb'))

    sc = acc.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
