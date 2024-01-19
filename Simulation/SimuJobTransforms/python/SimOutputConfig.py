# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import BeamType

def getStreamEVNT_TR_ItemList(flags):
    #Add to item list
    ItemList = [
        "IOVMetaDataContainer#*",
        "EventInfo#*"
    ]
    from SimulationConfig.SimEnums import CavernBackground
    if flags.Sim.CavernBackground in [CavernBackground.Write, CavernBackground.WriteWorld]:
        ItemList += ["TrackRecordCollection#NeutronBG"]
    else:
        ItemList += ["TrackRecordCollection#CosmicRecord"]
    return ItemList


def getStreamHITS_ItemList(flags):
    #Add to item list
    #TODO - make a separate function (combine with G4AtlasAlg one?)
    ItemList = ["McEventCollection#TruthEvent",
                "JetCollection#*"]

    ItemList+=["xAOD::EventInfo#EventInfo",
               "xAOD::EventAuxInfo#EventInfoAux.",
               "xAOD::EventInfoContainer#*",
               "xAOD::EventInfoAuxContainer#*"]

    if flags.Sim.IncludeParentsInG4Event:
        ItemList += ["McEventCollection#GEN_EVENT"]

    ItemList += ["xAOD::JetContainer#AntiKt4TruthJets",
                 "xAOD::AuxContainerBase!#AntiKt4TruthJetsAux.-constituentLinks.-constituentWeights",
                 "xAOD::JetContainer#AntiKt6TruthJets",
                 "xAOD::AuxContainerBase!#AntiKt6TruthJetsAux.-constituentLinks.-constituentWeights"]

    # pile-up truth particles
    ItemList += ["xAOD::TruthParticleContainer#TruthPileupParticles",
                 "xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux."]

    if 'Hijing_event_params' in flags.Input.Collections:
        ItemList += ["HijingEventParams#Hijing_event_params"]

    if flags.Detector.EnablePixel or  flags.Detector.EnableSCT or \
       flags.Detector.EnableITkPixel or  flags.Detector.EnableITkStrip or flags.Detector.EnablePLR or \
       flags.Detector.EnableHGTD:
        ItemList += ["SiHitCollection#*"]

    if flags.Detector.EnableTRT:
        ItemList += ["TRTUncompressedHitCollection#*"]

    if flags.Detector.EnableID or flags.Detector.EnableITk:
       ItemList += ["TrackRecordCollection#CaloEntryLayer"]

    if flags.Detector.EnableCalo:
        ItemList += ["TrackRecordCollection#MuonEntryLayer"]
        from SimulationConfig.SimEnums import CalibrationRun
        if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            ItemList += ["CaloCalibrationHitContainer#LArCalibrationHitActive",
                         "CaloCalibrationHitContainer#LArCalibrationHitDeadMaterial",
                         "CaloCalibrationHitContainer#LArCalibrationHitInactive",
                         "CaloCalibrationHitContainer#TileCalibHitActiveCell",
                         "CaloCalibrationHitContainer#TileCalibHitInactiveCell",
                         "CaloCalibrationHitContainer#TileCalibHitDeadMaterial"]
        else:
            ItemList += ["CaloCalibrationHitContainer#*"] #TODO be more precise about this case

    if flags.Detector.EnableLAr:
        ItemList += ["LArHitContainer#LArHitEMB",
                     "LArHitContainer#LArHitEMEC",
                     "LArHitContainer#LArHitHEC",
                     "LArHitContainer#LArHitFCAL"]
        if flags.Sim.ISF.HITSMergingRequired.get('CALO', False):
            ItemList += ["LArHitContainer#LArHitEMB_G4",
                         "LArHitContainer#LArHitEMEC_G4",
                         "LArHitContainer#LArHitHEC_G4",
                         "LArHitContainer#LArHitFCAL_G4",
                         "LArHitContainer#LArHitEMB_FastCaloSim",
                         "LArHitContainer#LArHitEMEC_FastCaloSim",
                         "LArHitContainer#LArHitHEC_FastCaloSim",
                         "LArHitContainer#LArHitFCAL_FastCaloSim"]

    if flags.Detector.EnableTile:
        if flags.Beam.Type is BeamType.TestBeam:
            ItemList += ["TBElementContainer#TBElementCnt",
                         "TileHitVector#TileTBHits"]
        ItemList += ["TileHitVector#TileHitVec",
                     "TileHitVector#MBTSHits"]
        if flags.Sim.ISF.HITSMergingRequired.get('CALO', False):
            ItemList += ["TileHitVector#MBTSHits_G4",
                         "TileHitVector#TileHitVec_G4",
                         "TileHitVector#TileHitVec_FastCaloSim"]

    if flags.Detector.EnableRPC:
        ItemList += ["RPCSimHitCollection#*"]

    if flags.Detector.EnableTGC:
        ItemList += ["TGCSimHitCollection#*"]

    if flags.Detector.EnableMDT:
        ItemList += ["MDTSimHitCollection#*"]

    if flags.Detector.EnableCSC:
        ItemList += ["CSCSimHitCollection#*"]

    if flags.Detector.EnablesTGC:
        ItemList += ["sTGCSimHitCollection#*"]

    if flags.Detector.EnableMM:
        ItemList += ["MMSimHitCollection#*"]

    if flags.Detector.EnableMuon:
        ItemList += ["TrackRecordCollection#MuonExitLayer"]

    if flags.Detector.EnableLucid:
        ItemList += ["LUCID_SimHitCollection#*"]

    if flags.Detector.EnableFwdRegion:
        ItemList += ["SimulationHitCollection#*"]

    if flags.Detector.EnableZDC:
        ItemList += ["ZDC_SimFiberHit_Collection#ZDC_SimFiberHit_Collection"]
        from SimulationConfig.SimEnums import CalibrationRun
        if flags.Sim.CalibrationRun in [CalibrationRun.ZDC, CalibrationRun.LArTileZDC]:
            ItemList += ["CaloCalibrationHitContainer#ZDC_CalibrationHits"]

    if flags.Detector.EnableALFA:
        ItemList += ["ALFA_HitCollection#*",
                     "ALFA_ODHitCollection#*"]

    if flags.Detector.EnableAFP:
        ItemList += ["AFP_TDSimHitCollection#*",
                     "AFP_SIDSimHitCollection#*"]

    if flags.Beam.Type is BeamType.Cosmics:
        ItemList += ["TrackRecordCollection#CosmicRecord",
                     "TrackRecordCollection#CosmicPerigee"]

    return ItemList
