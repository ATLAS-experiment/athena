# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Core configuration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Local
from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFindingCfg
from MuonConfig.MuonTrackBuildingConfig import MuonTrackBuildingCfg
from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg

def StandaloneMuonOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    result = ComponentAccumulator()

    # FIXME! Fix for ATLASRECTS-5151. Remove when better solution found.
    from TrkEventCnvTools.TrkEventCnvToolsConfig import (
        TrkEventCnvSuperToolCfg)
    result.merge(TrkEventCnvSuperToolCfg(flags))

    aod_items = []
    if flags.Detector.EnableMM or flags.Detector.EnablesTGC:
        aod_items += ["xAOD::TrackParticleContainer#EMEO_MuonSpectrometerTrackParticles"]
        aod_items += ["xAOD::TrackParticleAuxContainer#EMEO_MuonSpectrometerTrackParticlesAux."]
        aod_items += ["xAOD::MuonSegmentContainer#xAODNSWSegments"]
        aod_items += ["xAOD::MuonSegmentAuxContainer#xAODNSWSegmentsAux."]

    aod_items += ["xAOD::MuonSegmentContainer#NCB_MuonSegments"]
    aod_items += ["xAOD::MuonSegmentAuxContainer#NCB_MuonSegmentsAux."]

    # TrackParticles
    aod_items += ["xAOD::TrackParticleContainer#MuonSpectrometerTrackParticles"]
    aod_items += ["xAOD::TrackParticleAuxContainer#MuonSpectrometerTrackParticlesAux."]
    aod_items += ["xAOD::TrackParticleContainer#MuonSpectrometerOnlyTrackParticles"]
    aod_items += ["xAOD::TrackParticleAuxContainer#MuonSpectrometerOnlyTrackParticlesAux."]

    aod_items += ["xAOD::TrackParticleContainer#MSonlyTracklets"]
    aod_items += ["xAOD::TrackParticleAuxContainer#MSonlyTrackletsAux."]
    aod_items += ["xAOD::VertexContainer#MSDisplacedVertex"]
    aod_items += ["xAOD::VertexAuxContainer#MSDisplacedVertexAux."]

    if flags.Input.isMC or flags.Overlay.DataOverlay:
        # Truth Particle Container
        aod_items += ["xAOD::TruthParticleContainer#MuonTruthParticles"]
        aod_items += ["xAOD::TruthParticleAuxContainer#MuonTruthParticlesAux."]

        # Truth Segment Container
        aod_items += ["xAOD::MuonSegmentContainer#MuonTruthSegments"]
        aod_items += ["xAOD::MuonSegmentAuxContainer#MuonTruthSegmentsAux."]

    # ESD list includes all AOD items
    esd_items = []
    esd_items += aod_items

    # PRDs et al
    if flags.Detector.EnableMM:
        esd_items += ["Muon::MMPrepDataContainer#MM_Measurements"]
        esd_items += ["xAOD::NSWMMTPRDOContainer#*", "xAOD::NSWMMTPRDOAuxContainer#*"]
    if flags.Detector.EnablesTGC:
        esd_items += ["Muon::sTgcPrepDataContainer#STGC_Measurements"]
        esd_items += ["Muon::NSW_PadTriggerDataContainer#NSW_PadTrigger_RDO"]
        esd_items += ["xAOD::NSWTPRDOContainer#*", "xAOD::NSWTPRDOAuxContainer#*"]
        
        
    if flags.Detector.EnableCSC:
        esd_items += ["Muon::CscPrepDataContainer#CSC_Clusters"]
        esd_items += ["Muon::CscStripPrepDataContainer#CSC_Measurements"]
    esd_items += ["Muon::RpcPrepDataContainer#RPC_Measurements"]
    esd_items += ["Muon::TgcPrepDataContainer#TGC_MeasurementsAllBCs"]
    esd_items += ["Muon::MdtPrepDataContainer#MDT_DriftCircles"]

    if flags.Muon.writexAODPRD:
        esd_items += ["xAOD::MdtDriftCircleContainer#xMdtDriftCircles", "xAOD::MdtDriftCircleAuxContainer#xMdtDriftCirclesAux." ]
        esd_items += ["xAOD::MdtTwinDriftCircleContainer#xMdtTwinDriftCircles", "xAOD::MdtTwinDriftCircleAuxContainer#xMdtTwinDriftCirclesAux." ]
        esd_items += ["xAOD::sTgcStripContainer#xAODsTgcStrips", "xAOD::sTgcStripAuxContainer#xAODsTgcStripsAux." ]
        esd_items += ["xAOD::sTgcPadContainer#xAODsTgcPads", "xAOD::sTgcPadAuxContainer#xAODsTgcPadsAux." ]
        esd_items += ["xAOD::sTgcWireContainer#xAODsTgcWires", "xAOD::sTgcWireAuxContainer#xAODsTgcWiresAux." ]
        esd_items += ["xAOD::MMClusterContainer#xAODMMClusters", "xAOD::MMClusterAuxContainer#xAODMMClustersAux." ]
        esd_items += ["xAOD::TgcStripContainer#xTgcStrips", "xAOD::TgcStripAuxContainer#xTgcStripsAux." ]
        esd_items += ["xAOD::RpcStripContainer#xRpcStrips", "xAOD::RpcStripAuxContainer#xRpcStripsAux." ]
        esd_items += ["xAOD::RpcStrip2DContainer#xRpcBILStrips", "xAOD::RpcStrip2DAuxContainer#xRpcBILStripsAux." ]


    # trigger related info for offline DQA
    esd_items += ["Muon::TgcCoinDataContainer#TrigT1CoinDataCollection"]
    esd_items += ["Muon::TgcCoinDataContainer#TrigT1CoinDataCollectionPriorBC"]
    esd_items += ["Muon::TgcCoinDataContainer#TrigT1CoinDataCollectionNextBC"]
    esd_items += ["Muon::TgcCoinDataContainer#TrigT1CoinDataCollectionNextNextBC"]
    esd_items += ["Muon::RpcCoinDataContainer#RPC_triggerHits"]
    esd_items += ["RpcSectorLogicContainer#RPC_SECTORLOGIC"]
    
    # trigger info for RPC time calibration
    if flags.Output.doWriteRDO or flags.Muon.doWriteRpcRDO:
        esd_items += ["RpcPadContainer#RPCPAD"]

    # Segments
    esd_items += ["Trk::SegmentCollection#NCB_TrackMuonSegments"]

    # Tracks
    esd_items += ["TrackCollection#MuonSpectrometerTracks"]
    if flags.Muon.runCommissioningChain:
        esd_items += ["TrackCollection#EMEO_MuonSpectrometerTracks"]
    if flags.Detector.EnableMM or flags.Detector.EnablesTGC:
        esd_items += ["Trk::SegmentCollection#TrackMuonNSWSegments"]

    # Truth
    if flags.Input.isMC:
        esd_items += ["TrackRecordCollection#MuonEntryLayerFilter"]
        esd_items += ["TrackRecordCollection#MuonExitLayerFilter"]

        esd_items += ["PRD_MultiTruthCollection#MDT_TruthMap",
                      "PRD_MultiTruthCollection#RPC_TruthMap", "PRD_MultiTruthCollection#TGC_TruthMap"]
        if flags.Detector.EnableCSC:
            esd_items += ["PRD_MultiTruthCollection#CSC_TruthMap"]
        if flags.Detector.EnablesTGC:
            esd_items += ["PRD_MultiTruthCollection#STGC_TruthMap"]
        if flags.Detector.EnableMM:
            esd_items += ["PRD_MultiTruthCollection#MM_TruthMap"]

        # Track truth
        esd_items += ["DetailedTrackTruthCollection#MuonSpectrometerTracksTruth"]
        esd_items += ["TrackTruthCollection#MuonSpectrometerTracksTruth"]

        if flags.Muon.writeSDOs:
            if flags.Detector.EnableCSC: esd_items+=["CscSimDataCollection#CSC_SDO"]
            esd_items+=["MuonSimDataCollection#MDT_SDO"]
            esd_items+=["MuonSimDataCollection#RPC_SDO"]
            esd_items+=["MuonSimDataCollection#TGC_SDO"]
            if flags.Detector.EnablesTGC: esd_items+=["MuonSimDataCollection#sTGC_SDO"]
            if flags.Detector.EnableMM: esd_items+=["MuonSimDataCollection#MM_SDO"]

    if flags.Output.doWriteESD:
        result.merge(OutputStreamCfg(flags, "ESD", esd_items))
    if flags.Output.doWriteAOD:
        result.merge(OutputStreamCfg(flags, "AOD", aod_items))
    return result


def MuonReconstructionCfg(flags):
    # https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecExample/python/MuonStandalone.py
    from MuonConfig.MuonPrepDataConvConfig import MuonPrepDataConvCfg
    from MuonConfig.MuonRecToolsConfig import MuonTrackScoringToolCfg
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    from MuonConfig.MuonRecToolsConfig import MuonEDMHelperSvcCfg
    from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg

    # Many components need these services, so setup once here.
    result = MuonIdHelperSvcCfg(flags)
    result.merge(MuonEDMHelperSvcCfg(flags))

    # Now setup reconstruction steps
    result.merge(MuonPrepDataConvCfg(flags))
    result.merge(MuonSegmentFindingCfg(flags))
    result.merge(MuonTrackBuildingCfg(flags))
    result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags))
    if flags.Muon.runCommissioningChain:
        result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,
                                                          "MuonStandaloneTrackParticleCnvAlg_EMEO",
                                                          TrackContainerName="EMEO_MuonSpectrometerTracks",
                                                          xAODTrackParticlesFromTracksContainerName="EMEO_MuonSpectrometerTrackParticles"))

    # FIXME - this is copied from the old configuration, but I'm not sure it really belongs here.
    # It's probably better to have as part of TrackBuilding, or Segment building...
    if flags.Input.isMC  or flags.Overlay.DataOverlay:
        # filter TrackRecordCollection (true particles in muon spectrometer)
        if "MuonEntryLayerFilter" not in flags.Input.Collections and \
            ("MuonEntryLayer" in flags.Input.Collections):
            result.addEventAlgo(CompFactory.TrackRecordFilter())
        if "MuonExitLayerFilter" not in flags.Input.Collections and \
            ("MuonExitLayer" in flags.Input.Collections):
            result.addEventAlgo(CompFactory.TrackRecordFilter("TrackRecordFilterMuonExitLayer",
                                                              inputName="MuonExitLayer",
                                                              outputName="MuonExitLayerFilter"))

        # Now tracks
        track_cols = ["MuonSpectrometerTracks"]
        track_colstp = ["MuonSpectrometerTrackParticles"]
        if flags.Muon.runCommissioningChain:
            track_cols += ["EMEO_MuonSpectrometerTracks"]
            track_colstp += ["EMEO_MuonSpectrometerTrackParticles"]

        from MuonConfig.MuonTruthAlgsConfig import MuonDetailedTrackTruthMakerCfg
        result.merge(MuonDetailedTrackTruthMakerCfg(flags, name="MuonStandaloneDetailedTrackTruthMaker",
                                                    TrackCollectionNames=track_cols))

        for i in range(len(track_cols)):
            from TrkConfig.TrkTruthAlgsConfig import TrackTruthSelectorCfg, TrackParticleTruthAlgCfg
            result.merge(TrackTruthSelectorCfg(flags, tracks=track_cols[i]))

            result.merge(TrackParticleTruthAlgCfg(flags, tracks=track_cols[i],
                                                  TrackParticleName=track_colstp[i]))

        # Check if we're making PRDs
        # FIXME - I think we can remove this flag if we shift this to where PRDs are being created. However, this will involve some refactoring, so temporary fix is this.
        if flags.Muon.makePRDs:
            if not flags.Muon.usePhaseIIGeoSetup:
                from MuonConfig.MuonTruthAlgsConfig import MuonTruthAlgsCfg
                result.merge(MuonTruthAlgsCfg(flags))
            else:
                from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
                result.merge(MuonTruthAlgsCfg(flags))

    if flags.Muon.doMSVertex:
        msvertexrecotool = CompFactory.Muon.MSVertexRecoTool(
            MyExtrapolator=result.popToolsAndMerge(
                AtlasExtrapolatorCfg(flags)),
            TGCKey='TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
        the_alg = CompFactory.MSVertexRecoAlg(
            name="MSVertexRecoAlg", MSVertexRecoTool=msvertexrecotool)
        # Not explicitly configuring MSVertexTrackletTool
        result.addEventAlgo(the_alg)

    # FIXME - work around to fix unconfigured public MuonTrackScoringTool
    # It wuould be best to find who is using this tool, and add this configuration there
    # But AFAICS the only parent is MuonCombinedFitTagTool, and it's private there, so I'm a bit confused.
    result.addPublicTool(result.popToolsAndMerge(
        MuonTrackScoringToolCfg(flags)))
    # Ditto
    result.addPublicTool(result.popToolsAndMerge(
        MuonTrackSummaryToolCfg(flags)))

    # Setup output
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        result.merge(StandaloneMuonOutputCfg(flags))
    return result

# Run with python -m MuonConfig.MuonReconstructionConfig
def MuonReconstructionConfigTest(flags=None):

    if flags is None:
        from MuonConfig.MuonConfigUtils import SetupMuonStandaloneConfigFlags
        flags = SetupMuonStandaloneConfigFlags()
  
    from MuonConfig.MuonConfigUtils import SetupMuonStandaloneCA
    cfg = SetupMuonStandaloneCA(flags)

    # Run the actual test.
    acc = MuonReconstructionCfg(flags)
    cfg.merge(acc)

    from SGComps.AddressRemappingConfig import InputRenameCfg
    cfg.merge(InputRenameCfg("TrackCollection",
              "MuonSpectrometerTracks", "MuonSpectrometerTracks_old"))

    cfg.printConfig(withDetails=True)
    # drop faulty remapping
    # the evaluation of MuonSegmentNameFixCfg should happen conditionally instead
    # this is hack that is functioning only because this is top level CA
    oldRemaps = cfg.getService("AddressRemappingSvc").TypeKeyRenameMaps
    cfg.getService("AddressRemappingSvc").TypeKeyRenameMaps = [
        remap for remap in oldRemaps if "Trk::SegmentCollection" not in remap]

    f = open("MuonReconstruction.pkl", "wb")
    cfg.store(f)
    f.close()

    from MuonConfig.MuonConfigUtils import executeTest
    executeTest(cfg)




def MuonNCBTrackCfg(flags, cfg):
   """ 
        This config (made for r24.0 in Nov 2024) is used to:
        1] Switch setup of the segment making in the NSW to loosen constrain on the IP
        2] Adapt and switch off various criteria in TrackSteering/building to be able to reconstruct track from the non-standards (non-collision background) segments 
   """

    #Adapting NCB alg setup for the standard segment maker alg
   cfg.getEventAlgo("MuonSegmentMaker").NSWSegmentMaker.SeedMMStereos=False
   cfg.getEventAlgo("MuonSegmentMaker").NSWSegmentMaker.IPConstraint=False


   #Most of the setup below is to suppress background which now we want to reconstruct
   cfg.getEventAlgo("MuonCreatorAlg").MuonCreatorTool.RequireMSOEforSA=False
   cfg.getEventAlgo("MuonCreatorAlg").MuonCreatorTool.RequireCaloForSA=False

   cfg.getEventAlgo("MuonCombinedMuonCandidateAlg").MuonCandidateTool.ExtrapolationStrategy=1

   #Loosen up segment criteria
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.SegSeedQCut = -2
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.Seg2ndQCut  = -2
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.SegOtherQCut  = -2 #by default already -2
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.UseTightSegmentMatching  = False

   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.TrackBuilderTool.CandidateMatchingTool.DoTrackSegmentMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.MooBuilderTool.CandidateMatchingTool.DoTrackSegmentMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.TrackBuilderTool.CandidateMatchingTool.DoTrackSegmentMatching = False
   #MuPatTrackBuilder.MuonTrackSteering.MooCandidateMatchingTool
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.CandidateMatchingTool.DoTrackSegmentMatching = False
   #MuPatTrackBuilder.MuonTrackSteering.MooTrackBuilderTemplate.MooCandidateMatchingTool
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.TrackRefinementTool.CandidateMatchingTool.DoTrackSegmentMatching = False

   #MuPatTrackBuilder.MuonTrackSteering.MooCandidateMatchingTool.MuonSegmentMatchingTool
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.CandidateMatchingTool.SegmentMatchingTool.UseEndcapExtrapolationMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.CandidateMatchingTool.SegmentMatchingTool.doThetaMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.CandidateMatchingTool.SegmentMatchingTool.doPhiMatching   = False

   #MuPatTrackBuilder.MuonTrackSteering.MooMuonTrackBuilder.MuSt_MooCandidateMatchingTool.MuSt_MuonSegmentMatchingTool
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.TrackBuilderTool.CandidateMatchingTool.SegmentMatchingTool.UseEndcapExtrapolationMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.TrackBuilderTool.CandidateMatchingTool.SegmentMatchingTool.doThetaMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.TrackBuilderTool.CandidateMatchingTool.SegmentMatchingTool.doPhiMatching   = False

   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.MooBuilderTool.CandidateMatchingTool.SegmentMatchingTool.UseEndcapExtrapolationMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.MooBuilderTool.CandidateMatchingTool.SegmentMatchingTool.doThetaMatching = False
   cfg.getEventAlgo("MuPatTrackBuilder").TrackSteering.MooBuilderTool.CandidateMatchingTool.SegmentMatchingTool.doPhiMatching   = False



if __name__ == "__main__":
    MuonReconstructionConfigTest()

