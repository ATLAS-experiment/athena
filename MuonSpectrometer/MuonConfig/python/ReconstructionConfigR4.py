# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def MsTrkRecoChainConfig(flags):
    result = ComponentAccumulator()
    ### Acts -> Trk track conversion
    from MuonTrackFindingAlgs.TrackFindingConfig import MuonActsToTrkConvCfg
    result.merge(MuonActsToTrkConvCfg(flags))

    ### Convert Trk -> xAOD
    from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
    result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,"MuonTrkToxAODTrackParticleCnvR4",
                                                   TrackContainerName="MsTracksTrkCnv",
                                                   xAODTrackParticlesFromTracksContainerName="MsTrackParticlesFromTrkR4"))

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
    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg, MuidSaTagMakerAlg, \
                                                        StandaloneTrackPartCnvCfg, MuonCreatorAlgCfg
    result.merge(MSTrackFinderAlgCfg(flags))
    
    ### MuTag conversion to share same format as the combined chain
    result.merge(StandaloneTrackPartCnvCfg(flags))
    result.merge(MuidSaTagMakerAlg(flags))

    MuonTags = ["MuonTagsSA"]
    ### Combined reconstruction chain
    if flags.Reco.EnableTracking:
        from MuonTrackFindingAlgs.TrackFindingConfig import MuonInDetTrackSelectionAlgCfg, MuonSegmentTaggingAlgCfg
        result.merge(MuonInDetTrackSelectionAlgCfg(flags))
        result.merge(MuonSegmentTaggingAlgCfg(flags))
        MuonTags+= ["SegmentTags"]

    #### create the xAOD muons
    result.merge(MuonCreatorAlgCfg(flags, name = "MuonActsCreatorAlg",
                                          TagKeys=MuonTags))

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonToTruthAssocAlgCfg
    result.merge(MuonToTruthAssocAlgCfg(flags))
    result.merge(MsTrkRecoChainConfig(flags))
    return result