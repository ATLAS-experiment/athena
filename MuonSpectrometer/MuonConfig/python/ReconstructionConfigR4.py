# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

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

    
    #Add the xAOD muon PRD containers to the ESD output 
    exclude = ["", "mdtTrkPrdLink", "rpcTrkPrdLink", "tgcTrkPrdLink"]
    for cont_t, cont_name in [("MdtDriftCircle", "xMdtDriftCircles"),
                              ("MdtTwinDriftCircle", "xMdtTwinDriftCircles"),
                              ("sTgcStrip", "xAODsTgcStrips"),
                              ("sTgcPad", "xAODsTgcPads"),
                              ("sTgcWire", "xAODsTgcWires"),
                              ("MMCluster", "xAODMMClusters"),
                              ("TgcStrip", "xTgcStrips"),
                              ("RpcStrip", "xRpcStrips" ),
                              ("RpcStrip2D", "xRpcBILStrips"),
                              ("CombinedMuonStrip", "CombinedMuonPrds" )]:
        esd_items+=[f"xAOD::{cont_t}Container#{cont_name}",
                    "xAOD::{cont_t}AuxContainer#{cont_name}Aux{exclude}".format(
                        cont_t = cont_t,cont_name = cont_name,
                        exclude = ".-".join(exclude))]
    
    #add the SDOs to the ESD output if requested 
    if flags.Muon.writeSDOs:
        for item in ["MDT_SDO","RPC_SDO","TGC_SDO","MM_SDO","sTGC_SDO"]:
            esd_items += [f"xAOD::MuonSimHitContainer#{item}", f"xAOD::MuonSimHitAuxContainer#{item}Aux."]
    
    
    #add the segments to the AOD output
    aod_items += ["xAOD::MuonSegmentContainer#MuonSegmentsFromR4"]
    aod_items += ["xAOD::MuonSegmentAuxContainer#MuonSegmentsFromR4Aux.-localSegPars.-parentSegment.-localSegCov"]

    #add MS track and track particle containers to the ESD output
    aod_items += ["xAOD::TrackParticleContainer#MsTrackParticlesR4"]
    aod_items += ["xAOD::TrackParticleAuxContainer#MsTrackParticlesR4Aux."]

    # convert the Acts muon track to its xAOD representation and add it to the ESD output
    if flags.Output.doWriteESD:
        from ActsConfig.ActsEventCnvConfig import ActsToXAODTrackConverterAlgCfg
        result.merge(ActsToXAODTrackConverterAlgCfg(flags, name = "MsTracksToXAODTrackConverterAlg", InputActsTracksLocation = "MsTracks", OutputActsTracksLocation="MsTracks"))
        esd_items += MuonTrackContainersToPersistify("Ms")

    #for now add also the muon container here. This should go into the combined config at some point
    aod_items += ["xAOD::MuonContainer#MuonsR4"]

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
                MuonSpectrometerTrackParticleLocation="MsTrackParticlesR4",
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
    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg, MuidSaTagMakerAlgCfg, \
                                                        StandaloneTrackPartCnvCfg, MuonCreatorAlgCfg
    result.merge(MSTrackFinderAlgCfg(flags))
    
    ### MuTag conversion to share same format as the combined chain
    result.merge(StandaloneTrackPartCnvCfg(flags))


    MuonTags = ["MuonTagsSA"]
    ### Combined reconstruction chain
    if flags.Reco.EnableTracking:
        from MuonTrackFindingAlgs.TrackFindingConfig import MuonInDetTrackSelectionAlgCfg, MuonSegmentTaggingAlgCfg, BeamSpotPreparatorAlgCfg
        result.merge(BeamSpotPreparatorAlgCfg(flags))
        result.merge(MuonInDetTrackSelectionAlgCfg(flags))
        result.merge(MuonSegmentTaggingAlgCfg(flags))
        MuonTags+= ["SegmentTags"]
    
    result.merge(MuidSaTagMakerAlgCfg(flags, ExtrapolateToIP = flags.Reco.EnableTracking,
                                             RefitWithBeamSpot =flags.Reco.EnableTracking))

    #### create the xAOD muons
    result.merge(MuonCreatorAlgCfg(flags, name = "MuonActsCreatorAlg",
                                          TagKeys=MuonTags))

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonToTruthAssocAlgCfg
    result.merge(MuonToTruthAssocAlgCfg(flags))
    result.merge(MsTrkRecoChainConfig(flags))
    if flags.Output.doWriteAOD or flags.Output.doWriteESD:
        result.merge(StandaloneMuonOutputCfg(flags))
    return result