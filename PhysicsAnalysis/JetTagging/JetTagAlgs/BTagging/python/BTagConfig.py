"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

Main configuration of flavour tagging algorithms.
The low and high level tagging algorithms are scheduled here.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod, HIMode
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
from JetHitAssociation.JetHitAssociationConfig import JetHitAssociationCfg
from TrackHitAssignement.TrackHitAssignementAlgCfg import TrackHitAssignementAlg
from BTagging.FlavorTaggingConfig import FlavorTaggingCfg


def GetTaggerTrainingMap(inputFlags, jet_col):
    """This function defines the networks used for the different jet collections."""
    if inputFlags.GeoModel.Run >= LHCPeriod.Run4 and "AntiKt10UFOCSSKSoftDropBeta100Zcut10" not in jet_col:
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
        "AntiKt10UFOCSSKSoftDropBeta100Zcut10": [
            "BTagging/20230413/gn2xv00/antikt10ufo/network.onnx",
            "BTagging/20230413/gn2xwithmassv00/antikt10ufo/network.onnx",
            "BTagging/20230705/gn2xv01/antikt10ufo/network.onnx",
            "BTagging/20240925/GN2Xv02/antikt10ufo/network.onnx",
            "BTagging/20250310/antikt10ufo/GN2XTauV00.onnx",
            "BTagging/20250522/GN3XV00/antikt10ufo/network.onnx",
            "JetCalibTools/CalibArea-00-04-83/CalibrationFactors/MC20_bbJES_ak10csskufo_Sep24_calibFactors.onnx", # bJR10v00
            "JetCalibTools/CalibArea-00-04-83/CalibrationFactors/bbJESJMS_calibFactors_R22_MC20_CSSKUFO_bJR10v00Ext_20250212.onnx", # bJR10v00Ext
            "JetCalibTools/CalibArea-00-04-83/CalibrationFactors/bbJESJMS_calibFactors_R22_MC20MC23_CSSKUFO_bJR10v01_20250212.onnx" # bJR10v01
        ],
        "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA": [
            "BTagging/20220314/dipsLoose/antikt4empflow/network.json",    # input to DL1dv01
            "BTagging/20220509/dl1dLoose/antikt4empflow/network.json",    # 2023 pre-rec DL1dv01
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
    bc = 'xAOD::BTaggingContainer'
    bac = 'xAOD::BTaggingAuxContainer'
    tpac = 'xAOD::TrackParticleAuxContainer'
    remapSvc.TypeKeyRenameMaps += [
        f'{jac}#{jc}Jets.jetFoldHash->{jc}Jets.jetFoldHash_{s}',
        f'{jac}#{jc}Jets.jetFoldHash_noHits->{jc}Jets.jetFoldHash_noHits_{s}',
        f'{jac}#{jc}Jets.jetFoldRankHash->{jc}Jets.jetFoldRankHash_{s}',
        f'{jac}#{jc}Jets.BTagTrackToJetAssociator->{jc}Jets.BTagTrackToJetAssociator_{s}',
        f'{jac}#{jc}Jets.JFVtx->{jc}Jets.JFVtx_{s}',
        f'{jac}#{jc}Jets.JFVtxFlip->{jc}Jets.JFVtxFlip_{s}',
        f'{jac}#{jc}Jets.SecVtx->{jc}Jets.SecVtx_{s}',
        f'{jac}#{jc}Jets.SecVtxFlip->{jc}Jets.SecVtxFlip_{s}',
        f'{jac}#{jc}Jets.btaggingLink->{jc}Jets.btaggingLink_{s}',
        f'{bc}#BTagging_{jc}->BTagging_{jc}_{s}',
        f'{bac}#BTagging_{jc}Aux.->BTagging_{jc}_{s}Aux.',
        f'xAOD::VertexContainer#BTagging_{jc}SecVtx->BTagging_{jc}SecVtx_{s}',
        f'xAOD::VertexAuxContainer#BTagging_{jc}SecVtxAux.->BTagging_{jc}SecVtx_{s}Aux.',
        f'xAOD::BTagVertexContainer#BTagging_{jc}JFVtx->BTagging_{jc}JFVtx_{s}'
        f'xAOD::BTagVertexAuxContainer#BTagging_{jc}JFVtxAux.->BTagging_{jc}JFVtx_{s}Aux.',
        f'{tpac}#{tc}.TrackCompatibility->{tc}.TrackCompatibility_{s}',
        f'{tpac}#{tc}.btagIp_d0->{tc}.btagIp_d0_{s}',
        f'{tpac}#{tc}.btagIp_z0SinTheta->{tc}.btagIp_z0SinTheta_{s}',
        f'{tpac}#{tc}.btagIp_d0Uncertainty->{tc}.btagIp_d0Uncertainty_{s}',
        f'{tpac}#{tc}.btagIp_z0SinThetaUncertainty->{tc}.btagIp_z0SinThetaUncertainty_{s}',
        f'{tpac}#{tc}.btagIp_trackMomentum->{tc}.btagIp_trackMomentum_{s}',
        f'{tpac}#{tc}.btagIp_trackDisplacement->{tc}.btagIp_trackDisplacement_{s}',
        f'{tpac}#{tc}.btagIp_invalidIp->{tc}.btagIp_invalidIp_' + '_retag',
        f'{tpac}#{tc}.JetFitter_TrackCompatibility_antikt4empflow->{tc}.JetFitter_TrackCompatibility_antikt4empflow_{s}',
        f'{jac}#{jc}Jets.TracksForBTagging->{jc}Jets.TracksForBTagging{s}',
        f'{jac}#{jc}Jets.TracksForBTaggingOverPtThreshold->{jc}Jets.TracksForBTaggingOverPtThreshold{s}',
        f'{jac}#{jc}Jets.MuonsForBTagging->{jc}Jets.MuonsForBTagging{s}',
        f'{jac}#{jc}Jets.MuonsForBTaggingOverPtThreshold->{jc}Jets.MuonsForBTaggingOverPtThreshold{s}',
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


def BTagRecoSplitCfg(inputFlags, JetCollection=['AntiKt4EMPFlowJets']):
    """
    Run flavour tagging algorithms during reconstruction (AOD or ESD production).
    """

    result = ComponentAccumulator()
 
    if inputFlags.Reco.EnableHI:   
        JetCollection=['AntiKt4HIJets']     
        if inputFlags.Reco.HIMode is not HIMode.HI:
            JetCollection.extend(['AntiKt4EMTopoJets','AntiKt4EMPFlowJets'])


    # Can only configure b-tagging for collisions; not cosmics, etc.
    if inputFlags.Beam.Type is not BeamType.Collisions:
        return result

    #Track Augmenter
    result.merge(BTagTrackAugmenterAlgCfg(inputFlags))
    # loop over jet collections and schedule btagging algorithms
    for jc in JetCollection:
        result.merge(
            FlavorTaggingCfg(
                cfgFlags = inputFlags,
                JetCollection = jc,
            )
        )

    # Invoking the algorithm saving hits in the vicinity of jets, with proper flags
    if inputFlags.BTagging.Trackless:
        result.merge(JetHitAssociationCfg(inputFlags))
        result.merge(TrackHitAssignementAlg(inputFlags))
        BTaggingAODList = _track_measurement_list('JetAssociatedPixelClusters')
        BTaggingAODList += _track_measurement_list('JetAssociatedSCTClusters')
        result.merge(addToAOD(inputFlags, BTaggingAODList))
    if inputFlags.BTagging.savePixelHits:
        result.merge(JetHitAssociationCfg(inputFlags))
        result.merge(TrackHitAssignementAlg(inputFlags))
        result.merge(
            addToAOD(
              inputFlags,
              _track_measurement_list(("ITk" if inputFlags.Detector.GeometryITk else "") + "PixelClusters")
            )
        )
    if inputFlags.BTagging.saveSCTHits:
        result.merge(JetHitAssociationCfg(inputFlags))
        result.merge(TrackHitAssignementAlg(inputFlags))
        result.merge(
            addToAOD(
              inputFlags,
              _track_measurement_list("ITkStripClusters" if inputFlags.Detector.GeometryITk else "SCT_Clusters")
            )
        )

    return result

def _track_measurement_list(container_name):
    return [
        f'xAOD::TrackMeasurementValidationContainer#{container_name}',
        f'xAOD::TrackMeasurementValidationAuxContainer#{container_name}Aux.'
    ]
