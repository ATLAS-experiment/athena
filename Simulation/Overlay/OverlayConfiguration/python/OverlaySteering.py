#!/usr/bin/env python
"""Main steering for MC+MC and MC+data overlay

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
from AthenaConfiguration.Enums import LHCPeriod
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from DigitizationConfig.DigitizationParametersConfig import writeDigitizationParameters

from OverlayCopyAlgs.OverlayCopyAlgsConfig import \
    CopyCaloCalibrationHitContainersCfg, CopyJetTruthInfoCfg, CopyPileupParticleTruthInfoCfg, CopyMcEventCollectionCfg, \
    CopyTrackRecordCollectionsCfg
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg


def OverlayMainCfg(configFlags):
    """Main overlay steering configuration"""

    # Construct our accumulator to run
    acc = MainServicesCfg(configFlags)
    acc.merge(PoolReadCfg(configFlags))
    acc.merge(OverlayMainContentCfg(configFlags))
    return acc


def OverlayMainContentCfg(configFlags):
    """Main overlay content"""

    acc = writeDigitizationParameters(configFlags)

    # Add event info overlay
    if not configFlags.Overlay.FastChain:
        acc.merge(EventInfoOverlayCfg(configFlags))

    # Add truth overlay (needed downstream)
    if not configFlags.Overlay.FastChain and (getEnabledDetectors(configFlags) or configFlags.Digitization.EnableTruth):
        acc.merge(CopyMcEventCollectionCfg(configFlags))
    if configFlags.Digitization.EnableTruth:
        acc.merge(CopyJetTruthInfoCfg(configFlags))
        acc.merge(CopyPileupParticleTruthInfoCfg(configFlags))
        acc.merge(CopyCaloCalibrationHitContainersCfg(configFlags))
        if not configFlags.Overlay.FastChain:
            acc.merge(CopyTrackRecordCollectionsCfg(configFlags))

    # Beam spot reweighting
    if configFlags.Digitization.InputBeamSigmaZ > 0:
        from BeamEffects.BeamEffectsAlgConfig import BeamSpotReweightingAlgCfg
        acc.merge(BeamSpotReweightingAlgCfg(configFlags))

    # Inner detector
    if configFlags.Detector.EnableBCM:
        from InDetOverlay.BCMOverlayConfig import BCMOverlayCfg
        acc.merge(BCMOverlayCfg(configFlags))
    if configFlags.Detector.EnablePixel:
        from InDetOverlay.PixelOverlayConfig import PixelOverlayCfg
        acc.merge(PixelOverlayCfg(configFlags))
    if configFlags.Detector.EnableSCT:
        from InDetOverlay.SCTOverlayConfig import SCTOverlayCfg
        acc.merge(SCTOverlayCfg(configFlags))
    if configFlags.Detector.EnableTRT:
        from InDetOverlay.TRTOverlayConfig import TRTOverlayCfg
        acc.merge(TRTOverlayCfg(configFlags))

    # ITk
    if configFlags.Detector.EnableITkPixel:
        from InDetOverlay.ITkPixelOverlayConfig import ITkPixelOverlayCfg
        acc.merge(ITkPixelOverlayCfg(configFlags))
    if configFlags.Detector.EnableITkStrip:
        from InDetOverlay.ITkStripOverlayConfig import ITkStripOverlayCfg
        acc.merge(ITkStripOverlayCfg(configFlags))
    if configFlags.Detector.EnablePLR:
        from InDetOverlay.PLR_OverlayConfig import PLR_OverlayCfg
        acc.merge(PLR_OverlayCfg(configFlags))

    # HGTD
    if configFlags.Detector.EnableHGTD:
        from HGTD_Overlay.HGTD_OverlayConfig import HGTD_OverlayCfg
        acc.merge(HGTD_OverlayCfg(configFlags))

    # Calorimeters
    if configFlags.Detector.EnableLAr:
        from LArDigitization.LArDigitizationConfig import LArOverlayCfg, LArSuperCellOverlayCfg
        acc.merge(LArOverlayCfg(configFlags))
        if configFlags.Detector.EnableL1Calo:
            if configFlags.Overlay.DataOverlay:
                pass  # TODO: not supported for now
            else:
                from TrigT1CaloSim.TTL1OverlayConfig import LArTTL1OverlayCfg
                acc.merge(LArTTL1OverlayCfg(configFlags))
                if configFlags.GeoModel.Run in [LHCPeriod.Run3]:
                    acc.merge(LArSuperCellOverlayCfg(configFlags))

    if configFlags.Detector.EnableTile:
        from TileSimAlgs.TileDigitizationConfig import TileDigitizationCfg, TileOverlayTriggerDigitizationCfg
        acc.merge(TileDigitizationCfg(configFlags))
        if configFlags.Detector.EnableL1Calo:
            if configFlags.Overlay.DataOverlay:
                pass  # TODO: not supported for now
            else:
                from TrigT1CaloSim.TTL1OverlayConfig import TileTTL1OverlayCfg
                acc.merge(TileTTL1OverlayCfg(configFlags))
                acc.merge(TileOverlayTriggerDigitizationCfg(configFlags))

    # Muon system
    from MuonConfig.MuonOverlayConfig import MuonOverlayCfg
    acc.merge(MuonOverlayCfg(configFlags))
    
    # Add MT-safe PerfMon
    if configFlags.PerfMon.doFastMonMT or configFlags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        acc.merge(PerfMonMTSvcCfg(configFlags))

    # Track overlay
    if configFlags.Overlay.doTrackOverlay:
        #need this to ensure that the ElementLinks to the PRDs are handled correctly (since the name is hardcoded in the converters)
        from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
        acc.merge(TrkEventCnvSuperToolCfg(configFlags))
        if configFlags.Detector.GeometryITk:
           from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyITkTrackCollectionsCfg, CopyITkPixelClusterContainerCfg, CopyITkStripClusterContainerCfg
           acc.merge(CopyITkTrackCollectionsCfg(configFlags))
           acc.merge(CopyITkPixelClusterContainerCfg(configFlags))
           acc.merge(CopyITkStripClusterContainerCfg(configFlags))
        else:
           from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyTrackCollectionsCfg,CopyPixelClusterContainerCfg, CopySCT_ClusterContainerCfg,\
            CopyTRT_DriftCircleContainerCfg
           acc.merge(CopyTrackCollectionsCfg(configFlags))
           acc.merge(CopyPixelClusterContainerCfg(configFlags))
           acc.merge(CopySCT_ClusterContainerCfg(configFlags))
           acc.merge(CopyTRT_DriftCircleContainerCfg(configFlags))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    if configFlags.Output.doWriteRDO:
        acc.merge(SetupMetaDataForStreamCfg(configFlags, "RDO"))
    if configFlags.Output.doWriteRDO_SGNL:
        acc.merge(SetupMetaDataForStreamCfg(configFlags, "RDO_SGNL"))

    return acc
