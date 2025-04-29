#!/usr/bin/env python
"""Main steering for the digitization jobs

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from DigitizationConfig.DigitizationParametersConfig import writeDigitizationMetadata
from RunDependentSimComps.PileUpUtils import pileupInputCollections

from AthenaCommon.Logging import logging
logDigiSteering = logging.getLogger('DigitizationSteering')

def HepMCVersion():
    try:
        from AthenaPython.PyAthena import HepMC3 # noqa: F401
        HepMCVersion=3
    except ImportError:
        HepMCVersion=2
    return HepMCVersion


def DigitizationMainServicesCfg(flags):
    """Configure main digitization services"""
    if flags.Digitization.PileUp:
        if flags.Concurrency.NumThreads > 0:
            logDigiSteering.info("DigitizationMainServicesCfg: Attempting to run pile-up digitization AthenaMT using %s threads!", str(flags.Concurrency.NumThreads))
            logDigiSteering.info("DigitizationMainServicesCfg: Using new PileUpMT code.")
            # raise RuntimeError("DigitizationSteering.DigitizationMainServicesCfg: Running pile-up digitization with AthenaMT is not supported. Please update your configuration.")
            from DigitizationConfig.PileUpMTConfig import PileUpMTAlgCfg
            acc = MainServicesCfg(flags)
            acc.merge(PileUpMTAlgCfg(flags))
        else:
            from DigitizationConfig.PileUpConfig import PileUpEventLoopMgrCfg
            acc = MainServicesCfg(flags, LoopMgr="PileUpEventLoopMgr")
            acc.merge(PileUpEventLoopMgrCfg(flags))
    else:
        acc = MainServicesCfg(flags)

    acc.merge(PoolReadCfg(flags))

    return acc


def DigitizationMainCfg(flags):
    # Construct main services
    acc = DigitizationMainServicesCfg(flags)

    acc.merge(DigitizationMainContentCfg(flags))

    return acc

def DigitizationMainContentCfg(flags):

    acc = ComponentAccumulator()

    acc.merge(writeDigitizationMetadata(flags))

    if not flags.Digitization.PileUp:
        # Old EventInfo conversion
        if "EventInfo" not in flags.Input.Collections:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
            acc.merge(EventInfoCnvAlgCfg(flags,
                                        inputKey="McEventInfo",
                                        outputKey="Input_EventInfo"))

        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
        acc.merge(EventInfoUpdateFromContextAlgCfg(flags))

        # Decorate pile-up values
        from DigitizationConfig.PileUpConfig import NoPileUpMuWriterCfg
        acc.merge(NoPileUpMuWriterCfg(flags))

    # Signal-only truth information
    if flags.Digitization.PileUp:
        if HepMCVersion() == 3:
            from MCTruthSimAlgs.MCTruthSimAlgsConfig import SimpleMergeMcEventCollCfg as MergeMcEventCollCfg
            from MCTruthSimAlgs.MCTruthSimAlgsConfig import InTimeOnlySimpleMergeMcEventCollCfg as InTimeOnlyMergeMcEventCollCfg
            from MCTruthSimAlgs.MCTruthSimAlgsConfig import SignalOnlySimpleMergeMcEventCollCfg as SignalOnlyMergeMcEventCollCfg
        else:
            from MCTruthSimAlgs.MCTruthSimAlgsConfig import MergeMcEventCollCfg
            from MCTruthSimAlgs.MCTruthSimAlgsConfig import InTimeOnlyMcEventCollCfg as InTimeOnlyMergeMcEventCollCfg
            from MCTruthSimAlgs.MCTruthSimAlgsConfig import SignalOnlyMcEventCollCfg as SignalOnlyMergeMcEventCollCfg
        from MCTruthSimAlgs.MCTruthSimAlgsConfig import (
            MergeAntiKt4TruthJetsCfg,
            MergeAntiKt6TruthJetsCfg,
            MergeTruthParticlesCfg,
            MergeMuonEntryLayerCfg,
            MergeCalibHitsCfg,
            MergeHijingParsCfg,
        )
        if flags.Common.ProductionStep is not ProductionStep.FastChain and getEnabledDetectors(flags):
            if flags.Digitization.DigiSteeringConf=="StandardPileUpToolsAlg":
                acc.merge(MergeMcEventCollCfg(flags))
            elif flags.Digitization.DigiSteeringConf=="StandardInTimeOnlyTruthPileUpToolsAlg":
                acc.merge(InTimeOnlyMergeMcEventCollCfg(flags))
            else:
                acc.merge(SignalOnlyMergeMcEventCollCfg(flags))
        if flags.Digitization.EnableTruth:
            puCollections = pileupInputCollections(flags.Digitization.PU.LowPtMinBiasInputCols)
            if "AntiKt4TruthJets" in puCollections:
                acc.merge(MergeAntiKt4TruthJetsCfg(flags))
            if "AntiKt6TruthJets" in puCollections:
                acc.merge(MergeAntiKt6TruthJetsCfg(flags))
            if "TruthPileupParticles" in puCollections:
                acc.merge(MergeTruthParticlesCfg(flags))
            acc.merge(MergeMuonEntryLayerCfg(flags))
            acc.merge(MergeCalibHitsCfg(flags))
            if 'Hijing_event_params' in pileupInputCollections(flags.Digitization.PU.CavernInputCols):
                acc.merge(MergeHijingParsCfg(flags))


    from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
    acc.merge(TruthDigitizationOutputCfg(flags))

    # Beam spot reweighting
    if flags.Common.ProductionStep != ProductionStep.PileUpPresampling and flags.Digitization.InputBeamSigmaZ > 0:
        from BeamEffects.BeamEffectsAlgConfig import BeamSpotReweightingAlgCfg
        acc.merge(BeamSpotReweightingAlgCfg(flags))

    # Inner Detector
    if flags.Detector.EnableBCM:
        from BCM_Digitization.BCM_DigitizationConfig import BCM_DigitizationCfg
        acc.merge(BCM_DigitizationCfg(flags))
    if flags.Detector.EnablePixel:
        from PixelDigitization.PixelDigitizationConfig import PixelDigitizationCfg
        acc.merge(PixelDigitizationCfg(flags))
    if flags.Detector.EnableSCT:
        from SCT_Digitization.SCT_DigitizationConfig import SCT_DigitizationCfg
        acc.merge(SCT_DigitizationCfg(flags))
    if flags.Detector.EnableTRT:
        from TRT_Digitization.TRT_DigitizationConfig import TRT_DigitizationCfg
        acc.merge(TRT_DigitizationCfg(flags))

    # ITk
    if flags.Detector.EnableITkPixel:
        from PixelDigitization.ITkPixelDigitizationConfig import ITkPixelDigitizationCfg
        acc.merge(ITkPixelDigitizationCfg(flags))
    if flags.Detector.EnableITkStrip:
        from StripDigitization.StripDigitizationConfig import ITkStripDigitizationCfg
        acc.merge(ITkStripDigitizationCfg(flags))
    if flags.Detector.EnablePLR:
        from PixelDigitization.PLR_DigitizationConfig import PLR_DigitizationCfg
        acc.merge(PLR_DigitizationCfg(flags))

    # HGTD
    if flags.Detector.EnableHGTD:
        from HGTD_Digitization.HGTD_DigitizationConfig import HGTD_DigitizationCfg
        acc.merge(HGTD_DigitizationCfg(flags))

    # Calorimeter
    if flags.Detector.EnableLAr:
        from LArDigitization.LArDigitizationConfig import LArTriggerDigitizationCfg
        acc.merge(LArTriggerDigitizationCfg(flags))
    if flags.Detector.EnableTile:
        from TileSimAlgs.TileDigitizationConfig import TileDigitizationCfg, TileTriggerDigitizationCfg
        acc.merge(TileDigitizationCfg(flags))
        acc.merge(TileTriggerDigitizationCfg(flags))

    # Muon Spectrometer
    if flags.Detector.EnableMDT:
        from MuonConfig.MDT_DigitizationConfig import MDT_DigitizationDigitToRDOCfg
        acc.merge(MDT_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableTGC:
        from MuonConfig.TGC_DigitizationConfig import TGC_DigitizationDigitToRDOCfg
        acc.merge(TGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableRPC:
        from MuonConfig.RPC_DigitizationConfig import RPC_DigitizationDigitToRDOCfg
        acc.merge(RPC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableCSC:
        from MuonConfig.CSC_DigitizationConfig import CSC_DigitizationDigitToRDOCfg
        acc.merge(CSC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnablesTGC:
        from MuonConfig.sTGC_DigitizationConfig import sTGC_DigitizationDigitToRDOCfg
        acc.merge(sTGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.EnableMM:
        from MuonConfig.MM_DigitizationConfig import MM_DigitizationDigitToRDOCfg
        acc.merge(MM_DigitizationDigitToRDOCfg(flags))

    # LUCID
    if flags.Detector.EnableLucid:
        from LUCID_Digitization.LUCID_DigitizationConfig import LUCID_DigitizationCfg
        acc.merge(LUCID_DigitizationCfg(flags))

    # AFP
    if flags.Detector.EnableAFP:
        from AFP_Digitization.AFP_DigitizationConfig import AFP_DigitizationCfg
        acc.merge(AFP_DigitizationCfg(flags))

    # ALFA
    if flags.Detector.EnableALFA:
        from ALFA_Digitization.ALFA_DigitizationConfig import ALFA_DigitizationCfg
        acc.merge(ALFA_DigitizationCfg(flags))

    # ZDC
    if flags.Detector.EnableZDC:
        from ZDC_SimuDigitization.ZDC_SimuDigitizationConfig import ZDC_DigitizationCfg
        acc.merge(ZDC_DigitizationCfg(flags))

    # Add MT-safe PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        acc.merge(PerfMonMTSvcCfg(flags))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    acc.merge(SetupMetaDataForStreamCfg(flags, "RDO"))

    return acc


def DigitizationMessageSvcCfg(flags):
    """MessageSvc for digitization and overlay"""
    MessageSvc = CompFactory.MessageSvc
    acc = ComponentAccumulator()
    acc.addService(MessageSvc(setError=["HepMcParticleLink"]))
    return acc


def DigitizationTestingPostInclude(flags, acc):
    """Testing digitization post-include"""
    # dump config
    configName = "DigiPUConfigCA" if flags.Digitization.PileUp else "DigiConfigCA"
    from AthenaConfiguration.MainServicesConfig import JobOptionsDumpCfg
    acc.merge(JobOptionsDumpCfg(flags, fileName=f"{configName}.txt"))

    # dump pickle
    with open(f"{configName}.pkl", "wb") as f:
        acc.store(f)
