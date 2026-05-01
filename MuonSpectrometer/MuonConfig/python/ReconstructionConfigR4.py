# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration



from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
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
    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg, MuonActsToTrkConvCfg
    result.merge(MSTrackFinderAlgCfg(flags))

    return result
    ### Acts -> Trk track conversion
    result.merge(MuonActsToTrkConvCfg(flags, TracksLocation="MsTracksR4"))
    ### Convert Trk -> xAOD
    from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
    result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,"MuonXAODParticleConvFromHoughR4",
                                                   TrackContainerName="MsTracksR4",
                                                   xAODTrackParticlesFromTracksContainerName="MsTrackParticlesR4"))

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
