# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def RenameHitCollectionsOnReadCfg(flags):

    result = ComponentAccumulator()
    from SGComps.AddressRemappingConfig import InputRenameCfg
    result.merge(InputRenameCfg("McEventCollection","TruthEvent","TruthEventOLD"))

    if flags.Detector.EnableID or  flags.Detector.EnableITk:
        if flags.Detector.EnableBCM:
            result.merge(InputRenameCfg("SiHitCollection","BCMHits","BCMHitsOLD"))
            result.merge(InputRenameCfg("SiHitCollection","BLMHits","BLMHitsOLD"))
        if flags.Detector.EnablePixel:
            result.merge(InputRenameCfg("SiHitCollection","PixelHits","PixelHitsOLD"))
        if flags.Detector.EnableSCT:
            result.merge(InputRenameCfg("SiHitCollection","SCT_Hits","SCT_HitsOLD"))
        if flags.Detector.EnableTRT:
            result.merge(InputRenameCfg("TRTUncompressedHitCollection","TRTUncompressedHits","TRTUncompressedHitsOLD"))
        if flags.Detector.EnableBCMPrime:
            result.merge(InputRenameCfg("SiHitCollection","BCMPrimeHits","BCMPrimeHitsOLD"))
        if flags.Detector.EnableITkPixel:
            result.merge(InputRenameCfg("SiHitCollection","ITkPixelHits","ITkPixelHitsOLD"))
        if flags.Detector.EnableITkStrip:
            result.merge(InputRenameCfg("SiHitCollection","ITkStripHits","ITkStripHitsOLD"))
        if flags.Detector.EnableHGTD:
            result.merge(InputRenameCfg("SiHitCollection","HGTD_Hits","HGTD_HitsOLD"))
        result.merge(InputRenameCfg("TrackRecordCollection","CaloEntryLayer","CaloEntryLayerOLD"))

    if flags.Detector.EnableCalo:
        if flags.Detector.EnableLAr:
            result.merge(InputRenameCfg("LArHitContainer","LArHitEMB","LArHitEMBOLD"))
            result.merge(InputRenameCfg("LArHitContainer","LArHitEMEC","LArHitEMECOLD"))
            result.merge(InputRenameCfg("LArHitContainer","LArHitFCAL","LArHitFCALOLD"))
            result.merge(InputRenameCfg("LArHitContainer","LArHitHEC","LArHitHECOLD"))
            result.merge(InputRenameCfg("LArHitContainer","LArHitMiniFCAL","LArHitMiniFCALOLD"))
            result.merge(InputRenameCfg("CaloCalibrationHitContainer","LArCalibrationHitActive","LArCalibrationHitActiveOLD"))
            result.merge(InputRenameCfg("CaloCalibrationHitContainer","LArCalibrationHitDeadMaterial","LArCalibrationHitDeadMaterialOLD"))
            result.merge(InputRenameCfg("CaloCalibrationHitContainer","LArCalibrationHitInactive","LArCalibrationHitInactiveOLD"))
        if flags.Detector.EnableTile:
            result.merge(InputRenameCfg("TileHitVector","TileHitVec","TileHitVecOLD"))
            result.merge(InputRenameCfg("CaloCalibrationHitContainer","TileCalibHitActiveCell","TileCalibHitActiveCellOLD"))
            result.merge(InputRenameCfg("CaloCalibrationHitContainer","TileCalibHitInactiveCell","TileCalibHitInactiveCellOLD"))
            result.merge(InputRenameCfg("CaloCalibrationHitContainer","TileCalibHitDeadMaterial","TileCalibHitDeadMaterialOLD"))
        if flags.Detector.EnableMBTS:
            result.merge(InputRenameCfg("TileHitVector","MBTSHits","MBTSHitsOLD"))
        result.merge(InputRenameCfg("TrackRecordCollection","MuonEntryLayer","MuonEntryLayerOLD"))

    if flags.Detector.EnableMuon:
        if flags.Detector.EnableCSC:
            result.merge(InputRenameCfg("CSCSimHitCollection","CSC_Hits","CSC_HitsOLD"))
        if flags.Detector.EnableMDT:
            result.merge(InputRenameCfg("MDTSimHitCollection","MDT_Hits","MDT_HitsOLD"))
        if flags.Detector.EnableRPC:
            result.merge(InputRenameCfg("RPCSimHitCollection","RPC_Hits","RPC_HitsOLD"))
        if flags.Detector.EnableTGC:
            result.merge(InputRenameCfg("TGCSimHitCollection","TGC_Hits","TGC_HitsOLD"))
        if flags.Detector.EnablesTGC:
            result.merge(InputRenameCfg("sTGCSimHitCollection","sTGC_Hits", "sTGC_HitsOLD"))
        if flags.Detector.EnableMM:
            result.merge(InputRenameCfg("MMSimHitCollection","MM_Hits", "MM_HitsOLD"))
        result.merge(InputRenameCfg("TrackRecordCollection","MuonExitLayer","MuonExitLayerOLD"))

    #FIXME Add Renaming for Fwd Detector sim hits

    return result
