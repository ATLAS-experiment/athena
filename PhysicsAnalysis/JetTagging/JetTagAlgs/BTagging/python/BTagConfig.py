"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

Main configuration of flavour tagging algorithms.
The low and high level tagging algorithms are scheduled here.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod, HIMode
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
from JetHitAssociation.JetHitAssociationConfig import JetHitAssociationCfg
from TrackHitAssignement.TrackHitAssignementAlgCfg import TrackHitAssignementAlgCfg
from BTagging.FlavorTaggingConfig import FlavorTaggingCfg


def GetTaggerTrainingMap(flags, jet_col):
    """This function defines the networks used for the different jet collections."""
    if flags.GeoModel.Run >= LHCPeriod.Run4 and "AntiKt10UFOCSSKSoftDropBeta100Zcut10" not in jet_col:
        return [
            "BTagging/20240918/gn2hl/antikt4emtopo/network.onnx",
        ]
    
    networks_by_jet_col = {
        "AntiKt4EMPFlow": [
            "BTagging/20220314/dipsLoose/antikt4empflow/network.json",  # input to DL1dv01
            "BTagging/20220509/dl1dLoose/antikt4empflow/network.json",  # 2023 pre-rec DL1dv01
        ],
        "AntiKt4EMTopo": [
            "BTagging/201903/rnnip/antikt4empflow/network.json",
            "BTagging/201903/dl1r/antikt4empflow/network.json",
            "BTagging/20210824r22/dl1r/antikt4empflow/network.json",
            "BTagging/20220314/dipsLoose/antikt4empflow/network.json",  # input to DL1dv01
            "BTagging/20220509/dl1dLoose/antikt4empflow/network.json",  # 2023 pre-rec DL1dv01
        ],
        "AntiKtVR30Rmax4Rmin02Track": [
            "BTagging/201903/rnnip/antiktvr30rmax4rmin02track/network.json",
            "BTagging/201903/dl1r/antiktvr30rmax4rmin02track/network.json",
            "BTagging/20230208/dipsLoose/antiktvr30rmax4rmin02track/network.json",  # r22 training for VR track jets
            "BTagging/20230307/DL1dv01/antiktvr30rmax4rmin02track/network.json",  # 2023 pre-rec DL1dv01
            "BTagging/20230307/gn2v00/antiktvr30rmax4rmin02track/network.onnx",
        ],
        "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA": [
            "BTagging/20231205/GN2v01/antikt4empflow/network_fold0.onnx", # fold 0 of the GN2v01 (safe for HLT jets)
        ],
        "AntiKt4EMPFlowByVertex": [ # ByVertex added 
            #"BTagging/20231205/GN2v01/antikt4empflow/network_fold0.onnx",
            "BTagging/20220314/dipsLoose/antikt4empflow/network.json"
        ]
    }
    
    networks_by_jet_col["AntiKt4EMPFlowCustomVtx"] = networks_by_jet_col["AntiKt4EMPFlow"] # PFlow jet with custom vertex definition used in HIGG1D1 
    networks_by_jet_col["AntiKt4HI"] = networks_by_jet_col["AntiKt4EMPFlow"]
    return networks_by_jet_col[jet_col]


def RetagRenameInputContainerCfg(suffix, JetCollectionShort, tracksKey='InDetTrackParticles', addRenameMaps=None):
    acc=ComponentAccumulator()
    remapSvc = CompFactory.AddressRemappingSvc("AddressRemappingSvc")
    jc = JetCollectionShort
    s = suffix
    tc = tracksKey
    jac = 'xAOD::JetAuxContainer'
    tpac = 'xAOD::TrackParticleAuxContainer'

    vars = ["jetFoldHash",
            "jetFoldHash_noHits",
            "jetFoldRankHash",
            "BTagTrackToJetAssociator",
            "JFVtx",
            "JFVtxFlip",
            "SecVtx",
            "SecVtxFlip",
            "SV1_masssvx",
            "SV1_efracsvx",
            "SV1_energyTrkInJet",
            "SV1_dstToMatLay",
            "SV1_N2Tpair",
            "SV1_NGTinSvx",
            "SV1_L3d",
            "SV1_Lxy",
            "SV1_deltaR",
            "SV1_isDefaults",
            "SV1_normdist",
            "SV1_significance3d",
            "SV1_correctSignificance3d",
            "SV1_TrackParticleLinks",
            "SV1_badTracksIP",
            "SV1_vertices",
            "SV1Flip_masssvx",
            "SV1Flip_efracsvx",
            "SV1Flip_energyTrkInJet",
            "SV1Flip_dstToMatLay",
            "SV1Flip_N2Tpair",
            "SV1Flip_NGTinSvx",
            "SV1Flip_L3d",
            "SV1Flip_Lxy",
            "SV1Flip_deltaR",
            "SV1Flip_isDefaults",
            "SV1Flip_normdist",
            "SV1Flip_significance3d",
            "SV1Flip_correctSignificance3d",
            "SV1Flip_TrackParticleLinks",
            "SV1Flip_badTracksIP",
            "SV1Flip_vertices",
            "GN2v01_pb",
            "GN2v01_pc",
            "GN2v01_pu",
            "GN2v01_ptau",
            "GN2v01SimpleFlip_pb",
            "GN2v01SimpleFlip_pc",
            "GN2v01SimpleFlip_pu",
            "GN2v01SimpleFlip_ptau",
            "GN2v01_TrackOrigin",
            "GN2v01_VertexIndex",
            "GN2v01_TrackLinks",
            "GN2v01SimpleFlip_TrackOrigin",
            "GN2v01SimpleFlip_VertexIndex",
            "GN2v01SimpleFlip_TrackLinks",
            "TracksForBTagging",
            "TracksForBTaggingOverPtThreshold",
            "MuonsForBTagging",
            "MuonsForBTaggingOverPtThreshold"
            ]

    remapSvc.TypeKeyRenameMaps += [ f'{jac}#{jc}Jets.{var}->{jc}Jets.{var}_{s},' for var in vars]
    remapSvc.TypeKeyRenameMaps += [
        f'{tpac}#{tc}.TrackCompatibility->{tc}.TrackCompatibility_{s}',
        f'{tpac}#{tc}.btagIp_d0->{tc}.btagIp_d0_{s}',
        f'{tpac}#{tc}.btagIp_z0SinTheta->{tc}.btagIp_z0SinTheta_{s}',
        f'{tpac}#{tc}.btagIp_d0Uncertainty->{tc}.btagIp_d0Uncertainty_{s}',
        f'{tpac}#{tc}.btagIp_z0SinThetaUncertainty->{tc}.btagIp_z0SinThetaUncertainty_{s}',
        f'{tpac}#{tc}.btagIp_trackMomentum->{tc}.btagIp_trackMomentum_{s}',
        f'{tpac}#{tc}.btagIp_trackDisplacement->{tc}.btagIp_trackDisplacement_{s}',
        f'{tpac}#{tc}.btagIp_invalidIp->{tc}.btagIp_invalidIp_' + '_retag',
        f'{tpac}#{tc}.JetFitter_TrackCompatibility_antikt4empflow->{tc}.JetFitter_TrackCompatibility_antikt4empflow_{s}'
    ]

    # add extra mappings if present
    if addRenameMaps:
        remapSvc.TypeKeyRenameMaps += addRenameMaps

    acc.addService(remapSvc)
    acc.addService(
        CompFactory.ProxyProviderSvc(
            ProviderNames = [ "AddressRemappingSvc" ]
        )
    )

    return acc


def BTagRecoSplitCfg(flags, JetCollection=['AntiKt4EMPFlowJets']):
    """
    Run flavour tagging algorithms during reconstruction (AOD or ESD production).
    """

    result = ComponentAccumulator()
 
    if flags.Reco.EnableHI:   
        JetCollection=['AntiKt4HIJets']     
        if flags.Reco.HIMode is not HIMode.HI:
            JetCollection.extend(['AntiKt4EMTopoJets','AntiKt4EMPFlowJets'])


    # Can only configure b-tagging for collisions; not cosmics, etc.
    if flags.Beam.Type is not BeamType.Collisions:
        return result

    #Track Augmenter
    result.merge(BTagTrackAugmenterAlgCfg(flags))
    # loop over jet collections and schedule btagging algorithms
    for jc in JetCollection:
        result.merge(
            FlavorTaggingCfg(
                cfgFlags = flags,
                JetCollection = jc,
            )
        )

    # Invoking the algorithm saving hits in the vicinity of jets, with proper flags
    if flags.BTagging.Trackless:
        result.merge(JetHitAssociationCfg(flags))
        result.merge(TrackHitAssignementAlgCfg(flags))
        BTaggingAODList = _track_measurement_list('JetAssociatedPixelClusters')
        BTaggingAODList += _track_measurement_list('JetAssociatedSCTClusters')
        result.merge(addToAOD(flags, BTaggingAODList))
    if flags.BTagging.savePixelHits:
        result.merge(JetHitAssociationCfg(flags))
        result.merge(TrackHitAssignementAlgCfg(flags))
        result.merge(
            addToAOD(
              flags,
              _track_measurement_list("ITkPixelMeasurements" if flags.Detector.GeometryITk else "PixelClusters")
            )
        )
    if flags.BTagging.saveSCTHits:
        result.merge(JetHitAssociationCfg(flags))
        result.merge(TrackHitAssignementAlgCfg(flags))
        result.merge(
            addToAOD(
              flags,
              _track_measurement_list("ITkStripMeasurements" if flags.Detector.GeometryITk else "SCT_Clusters")
            )
        )

    return result

def _track_measurement_list(container_name):
    return [
        f'xAOD::TrackMeasurementValidationContainer#{container_name}',
        f'xAOD::TrackMeasurementValidationAuxContainer#{container_name}Aux.'
    ]
