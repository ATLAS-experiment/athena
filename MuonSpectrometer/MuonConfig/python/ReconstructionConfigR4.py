# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonTrackContainersToPersistify(prefix : str):
    toESD=[]
    toESD += [f"xAOD::TrackSummaryContainer#{prefix}TrackSummary",
              f"xAOD::TrackSummaryAuxContainer#{prefix}TrackSummaryAux.",
              f"xAOD::TrackStateContainer#{prefix}TrackStates",
              f"xAOD::TrackStateAuxContainer#{prefix}TrackStatesAux.-uncalibratedMeasurement",
              f"xAOD::TrackParametersContainer#{prefix}TrackParameters",
              f"xAOD::TrackParametersAuxContainer#{prefix}TrackParametersAux.",
              f"xAOD::TrackJacobianContainer#{prefix}TrackJacobians",
              f"xAOD::TrackJacobianAuxContainer#{prefix}TrackJacobiansAux.",
              f"xAOD::TrackMeasurementContainer#{prefix}TrackMeasurements",
              f"xAOD::TrackMeasurementAuxContainer#{prefix}TrackMeasurementsAux.",
              f"xAOD::TrackSurfaceContainer#{prefix}TrackStateSurfaces",
              f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackStateSurfacesAux.",
              f"xAOD::TrackSurfaceContainer#{prefix}TrackSurfaces",
              f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackSurfacesAux."]
    return toESD

def StandaloneMuonOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    result = ComponentAccumulator()

    aod_items = []
    esd_items = []

    if flags.Input.isMC or flags.Overlay.DataOverlay:
        # Truth Particle Container
        aod_items += ["xAOD::TruthParticleContainer#MuonTruthParticles"]
        aod_items += ["xAOD::TruthParticleAuxContainer#MuonTruthParticlesAux."]

        # Truth Segment Container
        aod_items += ["xAOD::MuonSegmentContainer#MuonTruthSegments"]
        aod_items += ["xAOD::MuonSegmentAuxContainer#MuonTruthSegmentsAux.-localSegPars"]
    
    from AthenaConfiguration.Enums import Format
    if (flags.Input.isMC or flags.Overlay.DataOverlay) and flags.Input.Format!=Format.BS:
        # filter TrackRecordCollection (true particles in muon spectrometer)
        if "MuonEntryLayerFilter" not in flags.Input.Collections and \
            ("MuonEntryLayer" in flags.Input.Collections):
            result.addEventAlgo(CompFactory.TrackRecordFilter())
        if "MuonExitLayerFilter" not in flags.Input.Collections and \
            ("MuonExitLayer" in flags.Input.Collections):
            result.addEventAlgo(CompFactory.TrackRecordFilter("TrackRecordFilterMuonExitLayer",
                                                              inputName="MuonExitLayer",
                                                              outputName="MuonExitLayerFilter"))
        
        esd_items += ["TrackRecordCollection#MuonEntryLayerFilter"]
        esd_items += ["TrackRecordCollection#MuonExitLayerFilter"]

    
    #add the SDOs to the ESD output if requested 
    if flags.Muon.writeSDOs:
        for item in ["MDT_SDO","RPC_SDO","TGC_SDO","MM_SDO","sTGC_SDO"]:
            esd_items += [f"xAOD::MuonSimHitContainer#{item}", f"xAOD::MuonSimHitAuxContainer#{item}Aux."]

    
    #Add the xAOD muon PRD containers to the ESD output 
    exclude = ["", "mdtTrkPrdLink", "rpcTrkPrdLink", "tgcTrkPrdLink", "segmentFitDriftSign"]
    contList =  [("MdtDriftCircle", "xMdtDriftCircles"),
                 ("MdtTwinDriftCircle", "xMdtTwinDriftCircles"),
                 ("sTgcStrip", "xAODsTgcStrips"),
                 ("sTgcPad", "xAODsTgcPads"),
                 ("sTgcWire", "xAODsTgcWires"),
                 ("MMCluster", "xAODMMClusters"),
                 ("TgcStrip", "xTgcStrips"),
                 ("CombinedMuonStrip", "CombinedMuonPrds")]

    from AthenaConfiguration.Enums import LHCPeriod
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        contList += [ ("RpcStrip", "xRpcStrips" ), ("RpcStrip2D", "xRpcBILStrips") ]
    else:
        contList += [("RpcStrip", "xRpcMeasurements")]

    for cont_t, cont_name in contList:
        esd_items+=[f"xAOD::{cont_t}Container#{cont_name}",
                    "xAOD::{cont_t}AuxContainer#{cont_name}Aux{exclude}".format(
                        cont_t = cont_t,cont_name = cont_name,
                        exclude = ".-".join(exclude))]

    #add the segments to the AOD output
    aod_items += ["xAOD::MuonSegmentContainer#MsSegments"]
    aod_items += ["xAOD::MuonSegmentAuxContainer#MsSegmentsAux.-localSegPars.-parentSegment.-localSegCov"]

    #add MS track and track particle containers to the ESD output
    for trk in ["MuonMsAtIpTrackParticles", "MuonMsTrackParticles","MuonSTACOTrackParticles"]:
        aod_items += [f"xAOD::TrackParticleContainer#{trk}",
                      f"xAOD::TrackParticleAuxContainer#{trk}Aux."]
    # convert the Acts muon track to its xAOD representation and add it to the ESD output
    if flags.Output.doWriteESD:
        from ActsConfig.ActsEventCnvConfig import ActsToXAODTrackConverterAlgCfg
        result.merge(ActsToXAODTrackConverterAlgCfg(flags, name = "MsTracksToXAODTrackConverterAlg", 
                                                    InputActsTracksLocation = "MuonMsTracks", 
                                                    OutputActsTracksLocation="MuonMsTracks"))
        esd_items += MuonTrackContainersToPersistify("MuonMs")
        if flags.Muon.buildMETrack:
            result.merge(ActsToXAODTrackConverterAlgCfg(flags, name = "MsAtIpTracksToXAODTrackConverterAlg", 
                                                        InputActsTracksLocation = "MuonMsAtIpTracks", 
                                                        OutputActsTracksLocation="MuonMsAtIpTracks"))
        esd_items += MuonTrackContainersToPersistify("MuonMsAtIp")
    #for now add also the muon container here. This should go into the combined config at some point
    aod_items += ["xAOD::MuonContainer#Muons"]
    aod_items += ["xAOD::MuonAuxContainerR4#MuonsAux."]
    
    aod_items += ["xAOD::MuonContainer#MuonsLRT"]
    aod_items += ["xAOD::MuonAuxContainerR4#MuonsLRTAux."]

    esd_items += aod_items
    
    if flags.Output.doWriteESD:
        result.merge(OutputStreamCfg(flags, "ESD", esd_items))
    if flags.Output.doWriteAOD:
        result.merge(OutputStreamCfg(flags, "AOD", aod_items))
    return result


def MsTrkRecoChainConfig(flags):
    result = ComponentAccumulator()
    ### Acts -> Trk track conversion
    from MuonTrackFindingAlgs.TrackFindingConfig import MuonActsToTrkConvCfg
    result.merge(MuonActsToTrkConvCfg(flags))
    return result
    ### Schedule the back extrapolation to the  IP
    from MuonCombinedConfig.MuonCombinedReconstructionConfig import  MuonCombinedMuonCandidateAlgCfg
    result.merge(MuonCombinedMuonCandidateAlgCfg(flags,
                name="MuonMSOECandidateAlgR4",
                MuonSpectrometerTrackParticleLocation="MuonMsTrackParticles",
                MuonCandidateLocation="MuonCandidatesR4",
                MSOnlyExtrapolatedTrackLocation="MSOETrksR4"))
            
    #### Create the xAOD::Muon from the MSOE tracks
    from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCreatorAlgCfg
    result.merge(MuonCreatorAlgCfg(flags, 
                                    name="MuonCreatorAlgR4",
                                    TagMaps=[], 
                                    CreateSAmuons = True, 
                                    MakeClusters= False,
                                    MuonContainerLocation="MuonsR4",
                                    MSOnlyExtrapolatedTrackLocation="MSOETrackParticlesR4",
                                    MSOnlyExtrapolatedLocation="MSOEMuonTrackParticlesR4",
                                    MuonCandidateLocation=["MuonCandidatesR4"],
                                    ClusterContainerName=""))
    return result

def MuonCombinedReconstructionConfigR4(flags):
    result = ComponentAccumulator()

    MuonTags = ["MuonTagsSA"]
    ### Combined reconstruction chain
    from MuonCombinedAlgsR4.MuonCombinedAlgsConfigR4 import MuonInDetTrackSelectionAlgCfg, \
                                                        MuonSegmentTaggingAlgCfg, \
                                                        BeamSpotPreparatorAlgCfg, \
                                                        MuonLegacyCaloTagAlgCfg, \
                                                        MuonCombinedStacoAlgCfg, \
                                                        MuonCombinedFitAlgCfg, \
                                                        MuidSaTagMakerAlgCfg, \
                                                        MuonCreatorAlgCfg
    if flags.Reco.EnableTracking: 
        if flags.Muon.buildMETrack: result.merge(BeamSpotPreparatorAlgCfg(flags))
        result.merge(MuonInDetTrackSelectionAlgCfg(flags))
        result.merge(MuonSegmentTaggingAlgCfg(flags))
        MuonTags+= ["SegmentTags"]
        result.merge(MuonCombinedStacoAlgCfg(flags))
        MuonTags+=["MuonTagsSTACO"]
        result.merge(MuonCombinedFitAlgCfg(flags))
        MuonTags+=["MuonTagsMuidCo"]
        if flags.MuonCombined.doCaloTrkMuId:
            result.merge(MuonLegacyCaloTagAlgCfg(flags))
            MuonTags+=["LegacyCaloTags"]
    
    result.merge(MuidSaTagMakerAlgCfg(flags,
                                      CombinedTags="MuonTagsSTACO" if flags.Reco.EnableTracking else ""))

    if flags.Muon.buildMETrack:
        from MuonTrackFindingAlgs.TrackFindingConfig import TrackSummaryLockCfg  
        result.merge(TrackSummaryLockCfg(flags, inContainer="MuonMsAtIpTrackParticles"))
    
    #### create the xAOD muons
    result.merge(MuonCreatorAlgCfg(flags, name = "MuonActsCreatorAlg",
                                          TagKeys=MuonTags))
    return result

def LRT_MuonCombinedReconstructionConfigR4(flags):
    result = ComponentAccumulator()
    if not flags.Reco.EnableTracking or not flags.Acts.doLargeRadius:
        return result
    
    MuonTags = []
    from MuonCombinedAlgsR4.MuonCombinedAlgsConfigR4 import LRT_MuonInDetTrackSelectionAlgCfg, \
                                                        LRT_MuonCombinedStacoAlgCfg, \
                                                        LRT_MuonLegacyCaloTagAlgCfg, \
                                                        LRT_MuonSegmentTaggingAlgCfg, \
                                                        LRT_MuonCombinedFitAlgCfg, \
                                                        MuonCreatorAlgCfg
    from ActsConfig.CaloExtensionBuilderConfig import ActsCaloExtensionBuilderCfg

    result.merge(ActsCaloExtensionBuilderCfg(flags, name="ActsCaloExtensionBuilderLRT",TrackParticleContainerName="InDetLargeD0TrackParticles", CaloExtensionName="CaloExtensionLRT"))
    result.merge(LRT_MuonInDetTrackSelectionAlgCfg(flags))

    result.merge(LRT_MuonSegmentTaggingAlgCfg(flags))
    MuonTags+= ["SegmentTagsLRT"]
    
    result.merge(LRT_MuonCombinedStacoAlgCfg(flags))
    MuonTags+=["MuonTagsSTACOLRT"]
    
    result.merge(LRT_MuonCombinedFitAlgCfg(flags))
    MuonTags+=["MuonTagsMuidCoLRT"]


    if flags.MuonCombined.doCaloTrkMuId:
        result.merge(LRT_MuonLegacyCaloTagAlgCfg(flags))
        MuonTags+=["LegacyCaloTagsLRT"]
    
    result.merge(MuonCreatorAlgCfg(flags, name = "MuonActsCreatorAlg_LRT",
                                      TagKeys=MuonTags, MuonKey="MuonsLRT"))
    return result


def MuonTruthChainCfg(flags):
    result  = ComponentAccumulator()
    if not flags.Input.isMC:
        return result

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
    result.merge(MuonTruthAlgsCfg(flags))

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonActsTruthMatchingCfg, MuonToTruthAssocAlgCfg
    tracks = ["MuonMsTrackParticles"]
    result.merge(MuonActsTruthMatchingCfg(flags, TrackContainer="MuonMsTracks", 
                                          ParticleContainer="MuonMsTrackParticles"))
    if flags.Muon.buildMETrack:
        tracks+=["MuonMsAtIpTrackParticles"]
        result.merge(MuonActsTruthMatchingCfg(flags, TrackContainer="MuonMsAtIpTracks", 
                                                ParticleContainer="MuonMsAtIpTrackParticles"))
    ### MuidCo track candidates
    if flags.Reco.EnableTracking:
        tracks+=["MuonCmbTrackParticles", "InDetTrackParticles"]
        result.merge(MuonActsTruthMatchingCfg(flags, TrackContainer="MuonCmbTracks",
                                                ParticleContainer="MuonCmbTrackParticles", 
                                                isCombined =True))
        if flags.Muon.buildMETrack:
            tracks+=["MuonMeTrackParticles"]
            result.merge(MuonActsTruthMatchingCfg(flags, TrackContainer="MuonMeTracks",
                                                ParticleContainer="MuonMeTrackParticles"))
        if flags.Acts.doLargeRadius:
            tracks+=["MuonCmbTrackParticlesLRT"]
            result.merge(MuonActsTruthMatchingCfg(flags, TrackContainer="MuonCmbTracksLRT",
                                                ParticleContainer="MuonCmbTrackParticlesLRT", 
                                                isCombined =True))
            if flags.Muon.buildMETrack:
                tracks+=["MuonMeTrackParticlesLRT"]
                result.merge(MuonActsTruthMatchingCfg(flags, TrackContainer="MuonMeTracksLRT",
                                                ParticleContainer="MuonMeTrackParticlesLRT"))
        
    result.merge(MuonToTruthAssocAlgCfg(flags, TrackKeys = tracks))
    if flags.Acts.doLargeRadius and flags.Reco.EnableTracking:
        result.merge(MuonToTruthAssocAlgCfg(flags, name = "MuonToTruthAssocAlgLRT",
                                            TrackKeys= tracks, MuonKey="MuonsLRT"))
    return result

### Standard reconstruction chain of phase II
def MuonReconstructionConfig(flags):
    result = ComponentAccumulator()


    ## Prep data creation
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    result.merge(xAODUncalibMeasPrepCfg(flags))
    
    ### Space point formation
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    result.merge(MuonSpacePointFormationCfg(flags))

    ### Segment finding
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    result.merge(MuonPatternRecognitionCfg(flags))

    ### Track building
    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg, \
                                                        StandaloneTrackPartCnvCfg 
    result.merge(MSTrackFinderAlgCfg(flags))
    
    ### MuTag conversion to share same format as the combined chain
    result.merge(StandaloneTrackPartCnvCfg(flags))

    if flags.Muon.scheduleLegacyReco:
        result.merge(MsTrkRecoChainConfig(flags))

    result.merge(MuonCombinedReconstructionConfigR4(flags))
    if flags.Acts.doLargeRadius:
        result.merge(LRT_MuonCombinedReconstructionConfigR4(flags)) 

    if flags.Output.doWriteAOD or flags.Output.doWriteESD:
        result.merge(StandaloneMuonOutputCfg(flags))

    result.merge(MuonTruthChainCfg(flags))
    
    return result