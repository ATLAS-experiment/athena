# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format, MetadataCategory, HIMode

def RecoSteering(flags):
    """
    Generates configuration of the reconstructions
    """
    from AthenaCommon.Logging import logging
    log = logging.getLogger("RecoSteering")
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # setup input
    acc.flagPerfmonDomain('IO')
    if flags.Input.Format is Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))
        # Decorate EventInfo obj with Beam Spot information
        if flags.Reco.EnableBeamSpotDecoration:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import (
                EventInfoBeamSpotDecoratorAlgCfg)
            acc.merge(EventInfoBeamSpotDecoratorAlgCfg(flags))
        log.info("---------- Configured BS reading")
    else:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
        # Check if running on legacy inputs
        if "EventInfo" not in flags.Input.Collections:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import (
                EventInfoCnvAlgCfg)
            acc.merge(EventInfoCnvAlgCfg(flags))
        log.info("---------- Configured POOL reading")

    acc.flagPerfmonDomain('Truth')
    if flags.Input.isMC:
        # AOD2xAOD Truth conversion
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        acc.merge(GEN_AOD2xAODCfg(flags))
        log.info("---------- Configured AODtoxAOD Truth Conversion")

        # We always want to write pileup truth jets to AOD,
        # irrespective of whether we write jets to AOD in general
        # This is because we cannot rebuild jets from pileup truth
        # particles from the AOD
        from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg
        acc.merge(addTruthPileupJetsToOutputCfg(flags))
        log.info("---------- Configured Truth pileup jet writing")

    # trigger
    acc.flagPerfmonDomain('Trigger')
    if flags.Reco.EnableTrigger:
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
        acc.merge(TriggerRecoCfg(flags))
        log.info("---------- Configured trigger data decoding")

    # calorimeter
    acc.flagPerfmonDomain('Calo')
    if flags.Detector.EnableCalo:
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        acc.merge(CaloRecoCfg(flags))
        log.info("---------- Configured calorimeter reconstruction")

    # ID / ITk
    acc.flagPerfmonDomain('ID')
    if flags.Reco.EnableTracking:
       if flags.Reco.EnableTrackOverlay:
           from TrackOverlayRec.TrackOverlayRecoConfig import TrackOverlayRecoCfg
           acc.merge(TrackOverlayRecoCfg(flags)) 
       else:
           from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
           acc.merge(InDetTrackRecoCfg(flags))

       log.info("---------- Configured tracking")

    # HI
    acc.flagPerfmonDomain('HI')
    if flags.Reco.EnableHI:
        from HIRecConfig.HIRecConfig import HIRecCfg
        acc.merge(HIRecCfg(flags))
        log.info("---------- Configured Heavy Ion reconstruction")
    
    # HGTD
    acc.flagPerfmonDomain('HGTD')
    if flags.Reco.EnableHGTDExtension:
        from HGTD_Config.HGTD_RecoConfig import HGTD_RecoCfg
        acc.merge(HGTD_RecoCfg(flags))
        log.info("---------- Configured HGTD track extension")

    # Muon
    acc.flagPerfmonDomain('Muon')
    if flags.Detector.EnableMuon:
        from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
        acc.merge(MuonReconstructionCfg(flags))
        log.info("---------- Configured muon tracking")

    # EGamma
    acc.flagPerfmonDomain('EGamma')
    if flags.Reco.EnableEgamma:
        from egammaConfig.egammaSteeringConfig import EGammaSteeringCfg
        acc.merge(EGammaSteeringCfg(flags))
        log.info("---------- Configured e/gamma")

    # Caching of CaloExtension for downstream
    # Combined Performance algorithms.
    acc.flagPerfmonDomain('CaloExtension')
    if flags.Reco.EnableCaloExtension:
        from TrackToCalo.CaloExtensionBuilderAlgCfg import (
            CaloExtensionBuilderCfg)
        acc.merge(CaloExtensionBuilderCfg(flags))
        log.info("---------- Configured track calorimeter extension builder")

    # Muon Combined
    acc.flagPerfmonDomain('CombinedMuon')
    if flags.Reco.EnableCombinedMuon:
        from MuonCombinedConfig.MuonCombinedReconstructionConfig import (
            MuonCombinedReconstructionCfg)
        acc.merge(MuonCombinedReconstructionCfg(flags))
        log.info("---------- Configured combined muon reconstruction")

    # TrackParticleCellAssociation
    # add cells crossed by high pt ID tracks
    acc.flagPerfmonDomain('TrackCellAssociation')
    if flags.Reco.EnableTrackCellAssociation:
        from TrackParticleAssociationAlgs.TrackParticleAssociationAlgsConfig import (
            TrackParticleCellAssociationCfg)
        acc.merge(TrackParticleCellAssociationCfg(flags))
        log.info("---------- Configured track particle-cell association")

    # PFlow
    acc.flagPerfmonDomain('PFlow')
    if flags.Reco.EnablePFlow:
        from eflowRec.PFRun3Config import PFCfg
        acc.merge(PFCfg(flags))
        log.info("---------- Configured particle flow")

    # EGamma and CombinedMuon isolation
    acc.flagPerfmonDomain('Isolation')
    if flags.Reco.EnableIsolation:
        from IsolationAlgs.IsolationSteeringConfig import IsolationSteeringCfg
        acc.merge(IsolationSteeringCfg(flags))
        log.info("---------- Configured isolation")

    # jets
    acc.flagPerfmonDomain('Jets')
    if flags.Reco.EnableJet:
        from JetRecConfig.JetRecoSteering import JetRecoSteeringCfg
        acc.merge(JetRecoSteeringCfg(flags))
        log.info("---------- Configured Jets")

    # btagging
    acc.flagPerfmonDomain('FTag')
    if flags.Reco.EnableBTagging:
        from BTagging.BTagConfig import BTagRecoSplitCfg
        acc.merge(BTagRecoSplitCfg(flags))
        log.info("---------- Configured btagging")

    # Tau
    acc.flagPerfmonDomain('Tau')
    if flags.Reco.EnableTau:
        from tauRec.TauConfig import TauReconstructionCfg
        acc.merge(TauReconstructionCfg(flags))
        log.info("---------- Configured tau reconstruction")
        # PFlow tau links
        if flags.Reco.EnablePFlow:
            from eflowRec.PFRun3Config import PFTauFELinkCfg
            acc.merge(PFTauFELinkCfg(flags))
            log.info("---------- Configured particle flow tau FE linking")

    acc.flagPerfmonDomain('Jets')
    if flags.Reco.EnableGlobalFELinking:
        # We also need to build links between the newly
        # created jet constituents (GlobalFE)
        # and electrons,photons,muons and taus
        from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
        acc.merge(PFGlobalFlowElementLinkingCfg(flags))
        log.info("---------- Configured particle flow global linking")

    # MET
    acc.flagPerfmonDomain('MET')
    if flags.Reco.EnableMet:
        from METReconstruction.METRecCfg import METCfg
        acc.merge(METCfg(flags))
        log.info("---------- Configured MET")

    # Calo Rings
    acc.flagPerfmonDomain('CaloRings')
    if flags.Reco.EnableCaloRinger:
        from CaloRingerAlgs.CaloRingerAlgsConfig import CaloRingerSteeringCfg
        acc.merge(CaloRingerSteeringCfg(flags))
        log.info("---------- Configured Calo Rings")

    # AFP
    acc.flagPerfmonDomain('AFP')
    if flags.Detector.EnableAFP:
        from ForwardRec.AFPRecConfig import AFPRecCfg
        acc.merge(AFPRecCfg(flags))
        log.info("---------- Configured AFP reconstruction")

    # Lucid
    acc.flagPerfmonDomain('Lucid')
    if flags.Detector.EnableLucid:
        from ForwardRec.LucidRecConfig import LucidRecCfg
        acc.merge(LucidRecCfg(flags))
        log.info("---------- Configured Lucid reconstruction")

    # ZDC 
    acc.flagPerfmonDomain('ZDC')
    if flags.Reco.EnableZDC:
        from ZdcRec.ZdcRecConfig import ZdcRecCfg
        acc.merge(ZdcRecCfg(flags))
        log.info("---------- Configured ZDC reconstruction")

    # Monitoring
    acc.flagPerfmonDomain('DQM')
    if flags.DQ.doMonitoring:
        from AthenaMonitoring.AthenaMonitoringCfg import (
            AthenaMonitoringCfg, AthenaMonitoringPostprocessingCfg)
        acc.merge(AthenaMonitoringCfg(flags))
        if flags.DQ.doPostProcessing:
            acc.merge(AthenaMonitoringPostprocessingCfg(flags))
        log.info("---------- Configured DQ monitoring")

    # Setup the final post-processing
    acc.flagPerfmonDomain('PostProcessing')
    if flags.Reco.EnablePostProcessing:
        acc.merge(RecoPostProcessingCfg(flags))
        log.info("---------- Configured post-processing")

    # setup output
    acc.flagPerfmonDomain('IO')
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    if flags.Output.doWriteESD:
        # Needed for Trk::Tracks TPCnv
        from TrkEventCnvTools.TrkEventCnvToolsConfigCA import (
            TrkEventCnvSuperToolCfg)
        acc.merge(TrkEventCnvSuperToolCfg(flags))
        # Needed for MetaData
        acc.merge(
            SetupMetaDataForStreamCfg(
                flags,
                "ESD",
                createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TruthMetaData,
                ],
            )
        )
        log.info("ESD ItemList: %s", acc.getEventAlgo(
            "OutputStreamESD").ItemList)
        log.info("ESD MetadataItemList: %s", acc.getEventAlgo(
            "OutputStreamESD").MetadataItemList)
        log.info("---------- Configured ESD writing")

    if flags.Output.doWriteAOD:
        # Needed for MetaData
        acc.merge(
            SetupMetaDataForStreamCfg(
                flags,
                "AOD",
                createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TruthMetaData,
                ],
            )
        )
        log.info("AOD ItemList: %s", acc.getEventAlgo(
            "OutputStreamAOD").ItemList)
        log.info("AOD MetadataItemList: %s", acc.getEventAlgo(
            "OutputStreamAOD").MetadataItemList)
        log.info("---------- Configured AOD writing")

    if flags.Output.doJiveXML:
        from JiveXML.JiveXMLConfig import AlgoJiveXMLCfg
        acc.merge(AlgoJiveXMLCfg(flags))
        log.info("---------- Configured JiveXML writing")
    
    # Set up PerfMon
    acc.flagPerfmonDomain('PerfMon')
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        acc.merge(PerfMonMTSvcCfg(flags))
        log.info("---------- Configured PerfMon")
    
    return acc


def RecoPostProcessingCfg(flags):
    acc = ComponentAccumulator()
    if flags.Reco.PostProcessing.ThinNegativeClusters:
        from ThinningUtils.ThinNegativeEnergyCaloClustersConfig import (
            ThinNegativeEnergyCaloClustersCfg)
        acc.merge(ThinNegativeEnergyCaloClustersCfg(flags))
    if flags.Reco.PostProcessing.TRTAloneThinning:
        from ThinningUtils.ThinTRTStandaloneConfig import (
            ThinTRTStandaloneCfg)
        acc.merge(ThinTRTStandaloneCfg(flags))
    if flags.Reco.PostProcessing.InDetForwardTrackParticleThinning:
        from ThinningUtils.ThinInDetForwardTrackParticlesConfig import (
            ThinInDetForwardTrackParticlesCfg)
        acc.merge(ThinInDetForwardTrackParticlesCfg(flags))
    if flags.Reco.PostProcessing.GeantTruthThinning:
        from ThinningUtils.ThinGeantTruthConfig import (
            ThinGeantTruthCfg)
        acc.merge(ThinGeantTruthCfg(flags))
        pass

    if flags.Output.doWriteAOD and flags.Reco.HIMode in [HIMode.UPC, HIMode.HIP]:
        from HIGlobal.RecordExtraInfoConfig import addMBTS, addSpacePoints
        acc.merge(addMBTS(flags))
        if flags.Reco.HIMode is HIMode.UPC:
            acc.merge(addSpacePoints(flags))

    return acc
