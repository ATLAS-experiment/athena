"""
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

Main configuration of flavour tagging algorithms.
The low and high level tagging algorithms are scheduled here.
"""

from pathlib import Path

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod, HIMode
from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg
from BTagging.JetBTaggingAlgConfig import JetBTaggingAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from FlavorTagDiscriminants.BTagJetAugmenterAlgConfig import BTagJetAugmenterAlgCfg
from FlavorTagDiscriminants.BTagMuonAugmenterAlgConfig import BTagMuonAugmenterAlgCfg
from FlavorTagDiscriminants.FlavorTagNNConfig import (
    FlavorTagNNCfg,
    MultifoldGNNCfg,
)
from JetTagCalibration.JetTagCalibConfig import JetTagCalibCfg
from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
from JetHitAssociation.JetHitAssociationConfig import JetHitAssociationCfg
from TrackHitAssignement.TrackHitAssignementAlgCfg import TrackHitAssignementAlg


def GetTaggerTrainingMap(inputFlags, jet_col):
    """This function defines the networks used for the different jet collections."""
    if inputFlags.GeoModel.Run >= LHCPeriod.Run4 and "AntiKt10UFOCSSKSoftDropBeta100Zcut10" not in jet_col:
        return [
            "BTagging/20221008/dipsrun4/antikt4emtopo/network.json",
            "BTagging/20221017/dl1drun4/antikt4emtopo/network.json",
            "BTagging/20221010/GN1run4/antikt4emtopo/network.onnx",
        ]
    
    networks_by_jet_col = {
        "AntiKt4EMPFlow": [
            "BTagging/201903/rnnip/antikt4empflow/network.json",
            "BTagging/201903/dl1r/antikt4empflow/network.json",
            "BTagging/20210824r22/dl1r/antikt4empflow/network.json",
            "BTagging/20220314/dipsLoose/antikt4empflow/network.json",  # input to DL1dv01
            "BTagging/20220509/dl1dLoose/antikt4empflow/network.json",  # 2023 pre-rec DL1dv01
            "BTagging/20230306/gn2v00/antikt4empflow/network.onnx",
        ],
        "AntiKt4EMPFlowCustomVtx": [ # PFlow jet with custom vertex definition used in HIGG1D1 
            "BTagging/201903/rnnip/antikt4empflow/network.json",
            "BTagging/201903/dl1r/antikt4empflow/network.json",
            "BTagging/20220314/dipsLoose/antikt4empflow/network.json",  # input to DL1dv01
            "BTagging/20220509/dl1dLoose/antikt4empflow/network.json",  # 2023 pre-rec DL1dv01
            "BTagging/20230306/gn2v00/antikt4empflow/network.onnx",
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
        ],
    }

    networks_by_jet_col["AntiKt4HI"] = networks_by_jet_col["AntiKt4EMPFlow"]
    return networks_by_jet_col[jet_col]


def RetagRenameInputContainerCfg(suffix, JetCollectionShort, tracksKey='InDetTrackParticles', addRenameMaps=None):
    
    acc=ComponentAccumulator()
    AddressRemappingSvc, ProxyProviderSvc=CompFactory.getComps("AddressRemappingSvc","ProxyProviderSvc",)
    AddressRemappingSvc = AddressRemappingSvc("AddressRemappingSvc")
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.BTagTrackToJetAssociator->' + JetCollectionShort + 'Jets.BTagTrackToJetAssociator_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.JFVtx->' + JetCollectionShort + 'Jets.JFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.SecVtx->' + JetCollectionShort + 'Jets.SecVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.btaggingLink->' + JetCollectionShort + 'Jets.btaggingLink_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingContainer#BTagging_' + JetCollectionShort + '->BTagging_' + JetCollectionShort + '_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingAuxContainer#BTagging_' + JetCollectionShort + 'Aux.->BTagging_' + JetCollectionShort + '_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexContainer#BTagging_' + JetCollectionShort + 'SecVtx->BTagging_' + JetCollectionShort + 'SecVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexAuxContainer#BTagging_' + JetCollectionShort + 'SecVtxAux.->BTagging_' + JetCollectionShort + 'SecVtx_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexContainer#BTagging_' + JetCollectionShort + 'JFVtx->BTagging_' + JetCollectionShort + 'JFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexAuxContainer#BTagging_' + JetCollectionShort + 'JFVtxAux.->BTagging_' + JetCollectionShort + 'JFVtx_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.TrackCompatibility->' + tracksKey + '.TrackCompatibility_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.btagIp_d0->' + tracksKey + '.btagIp_d0_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.btagIp_z0SinTheta->' + tracksKey + '.btagIp_z0SinTheta_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.btagIp_d0Uncertainty->' + tracksKey + '.btagIp_d0Uncertainty_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.btagIp_z0SinThetaUncertainty->' + tracksKey + '.btagIp_z0SinThetaUncertainty_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.btagIp_trackMomentum->' + tracksKey + '.btagIp_trackMomentum_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.btagIp_trackDisplacement->' + tracksKey + '.btagIp_trackDisplacement_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::TrackParticleAuxContainer#' + tracksKey + '.JetFitter_TrackCompatibility_antikt4empflow->' + tracksKey + '.JetFitter_TrackCompatibility_antikt4empflow_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.TracksForBTagging->' + JetCollectionShort + 'Jets.TracksForBTagging' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.TracksForBTaggingOverPtThreshold->' + JetCollectionShort + 'Jets.TracksForBTaggingOverPtThreshold' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.MuonsForBTagging->' + JetCollectionShort + 'Jets.MuonsForBTagging' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#' + JetCollectionShort + 'Jets.MuonsForBTaggingOverPtThreshold->' + JetCollectionShort + 'Jets.MuonsForBTaggingOverPtThreshold' + suffix]
    
    # add extra mappings if present
    if addRenameMaps:
        AddressRemappingSvc.TypeKeyRenameMaps += addRenameMaps
    
    acc.addService(AddressRemappingSvc)
    acc.addService(ProxyProviderSvc(ProviderNames = [ "AddressRemappingSvc" ]))

    return acc


def BTagRecoSplitCfg(inputFlags, JetCollection=['AntiKt4EMTopo','AntiKt4EMPFlow']):


    result=ComponentAccumulator()
 
    if inputFlags.Reco.EnableHI:   
        JetCollection=['AntiKt4HI']     
        if inputFlags.Reco.HIMode is not HIMode.HI:
            JetCollection.extend(['AntiKt4EMTopo','AntiKt4EMPFlow'])

    # Can only configure b-tagging for collisions; not cosmics, etc.
    if inputFlags.Beam.Type is not BeamType.Collisions:
        return result

    result.merge(JetTagCalibCfg(inputFlags))

    #Track Augmenter
    result.merge(BTagTrackAugmenterAlgCfg(inputFlags))

    # loop over jet collections and schedule btagging algorithms
    for jc in JetCollection:
        result.merge(
            BTagAlgsCfg(
                inputFlags,
                JetCollection=jc,
                nnList=GetTaggerTrainingMap(inputFlags, jc),
                muons='', # muon augmentation isn't thread safe, disable
                renameTrackJets=True,
                AddedJetSuffix='Jets'
            )
        )

    # By default, in Run3 we don't write out BTagging containers in AOD or ESD
    # following allows to write them out when using Reco_tf.py --CA run 3 style configuration

    if inputFlags.Output.doWriteAOD and inputFlags.Jet.WriteToAOD:
     result.merge(addBTagToOutput(inputFlags, JetCollection, toAOD=True, toESD=False))

    if inputFlags.Output.doWriteESD:
     result.merge(addBTagToOutput(inputFlags, JetCollection, toAOD=False, toESD=True))

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
        result.merge(addToAOD(inputFlags, _track_measurement_list("PixelClusters")))
    if inputFlags.BTagging.saveSCTHits:
        result.merge(JetHitAssociationCfg(inputFlags))
        result.merge(TrackHitAssignementAlg(inputFlags))
        result.merge(addToAOD(inputFlags, _track_measurement_list("SCT_Clusters")))

    return result


def _track_measurement_list(container_name):
    return [
        f'xAOD::TrackMeasurementValidationContainer#{container_name}',
        f'xAOD::TrackMeasurementValidationAuxContainer#{container_name}Aux.'
    ]


def BTagAlgsCfg(
    inputFlags,
    JetCollection,
    nnList=[],
    TaggerList=None,
    SecVertexers=None,
    trackCollection='InDetTrackParticles',
    primaryVertices='PrimaryVertices',
    muons='Muons',
    BTagCollection=None,
    renameTrackJets=False,
    AddedJetSuffix='',
):
    """
    This is the main function in this module and does the heavy lifting of 
    scheduling the tagging algorithms for a given jet collection.
    """

    # If things aren't specified in the arguments, we'll read them
    # from the config flags
    if TaggerList is None:
        TaggerList = inputFlags.BTagging.taggerList
    if SecVertexers is None:
        SecVertexers = ['JetFitter', 'SV1']
        if inputFlags.BTagging.RunFlipTaggers:
            SecVertexers += ['JetFitterFlip','SV1Flip']
    jet = JetCollection
    jetcol_no_suffix = JetCollection
    jetcol = JetCollection + AddedJetSuffix
    if renameTrackJets is True:
        jetcol_no_suffix = jet.replace("Track", "PV0Track")
        jetcol = jetcol.replace("Track", "PV0Track")
    if BTagCollection is None:
        BTagCollection = inputFlags.BTagging.OutputFiles.Prefix + jet

    # Names of element link vectors that are stored on the jet and
    # BTagging object. These are added and read out by the packages
    # that are configured below: in principal you should be able to
    # change these without changing the final b-tagging output.
    JetTrackAssociator = 'TracksForBTagging'
    BTagTrackAssociator = 'BTagTrackToJetAssociator'
    JetMuonAssociator = 'MuonsForBTagging'
    BTagMuonAssociator = 'Muons'

    # List of input VxSecVertexInfo containers
    VxSecVertexInfoNameList = []

    #List of secondary vertex finders
    secVtxFinderxAODBaseNameList = []
    result = ComponentAccumulator()

    # Associate tracks to the jet
    result.merge(JetParticleAssociationAlgCfg(
        inputFlags,
        jetcol,
        trackCollection,
        JetTrackAssociator,
    ))

    if muons:
        result.merge(JetParticleAssociationAlgCfg(
            inputFlags, jetcol, muons, JetMuonAssociator))

    # Build secondary vertices
    for sv in SecVertexers:
        BTagVxSecVertexInfoName = sv + 'VxSecVertexInfo_' + jet
        VxSecVertexInfoNameList.append(BTagVxSecVertexInfoName)
        secVtxFinderxAODBaseNameList.append(sv)
        AlgName = (jet + '_' + sv).lower()
        result.merge(JetSecVtxFindingAlgCfg(
            inputFlags,
            BTagVxSecVertexInfoName = BTagVxSecVertexInfoName,
            SVAlgName = AlgName + '_secvtxfinding',
            JetCollection = jetcol,
            PrimaryVertexCollectionName = primaryVertices,
            SVFinder = sv,
            TracksToTag = JetTrackAssociator,
        ))
        result.merge(JetSecVertexingAlgCfg(
            inputFlags,
            BTagVxSecVertexInfoName = BTagVxSecVertexInfoName,
            SVAlgName = AlgName + '_secvtx',
            BTaggingCollection = BTagCollection,
            JetCollection = jetcol,
            TrackCollection = trackCollection,
            PrimaryVertexCollectionName = primaryVertices,
            SVFinder = sv, 
        ))

    # Create the b-tagging object, and run the older b-tagging algorithms
    secVtxFinderTrackNameList = [ BTagTrackAssociator ] * len(SecVertexers)
    result.merge(
        JetBTaggingAlgCfg(
            inputFlags,
            BTaggingCollection=BTagCollection,
            JetCollection=jetcol,
            JetColNoJetsSuffix=jetcol_no_suffix,
            PrimaryVertexCollectionName=primaryVertices,
            TaggerList=TaggerList,
            Tracks=JetTrackAssociator,
            Muons=JetMuonAssociator if muons else '',
            VxSecVertexInfoNameList = VxSecVertexInfoNameList,
            secVtxFinderxAODBaseNameList = secVtxFinderxAODBaseNameList,
            secVtxFinderTrackNameList = secVtxFinderTrackNameList,
            OutgoingTracks=BTagTrackAssociator,
            OutgoingMuons=BTagMuonAssociator,
        )
    )

    if inputFlags.BTagging.RunNewVrtSecInclusive:
        #add soft b hadron vertex finder (outside of jets)
        from NewVrtSecInclusiveTool.NewVrtSecInclusiveAlgConfig import NewVrtSecInclusiveAlgTightCfg,NewVrtSecInclusiveAlgMediumCfg,NewVrtSecInclusiveAlgLooseCfg
        result.merge(NewVrtSecInclusiveAlgTightCfg(inputFlags))
        result.merge(NewVrtSecInclusiveAlgMediumCfg(inputFlags))
        result.merge(NewVrtSecInclusiveAlgLooseCfg(inputFlags))

    # Add some high level information to the b-tagging object we created above
    result.merge(
        BTagJetAugmenterAlgCfg(
            inputFlags,
            BTagCollection=BTagCollection,
            Associator=BTagTrackAssociator,
            TrackCollection=trackCollection,
        )
    )

    # add also Flip tagger information
    if inputFlags.BTagging.RunFlipTaggers:
       result.merge(
           BTagJetAugmenterAlgCfg(
               inputFlags,
               BTagCollection=BTagCollection,
               Associator=BTagTrackAssociator,
               TrackCollection=trackCollection,
               doFlipTagger=True,
           )
       )

    # add muon info
    if muons:
        result.merge(
            BTagMuonAugmenterAlgCfg(
                inputFlags,
                BTagCollection=BTagCollection,
                Associator=BTagMuonAssociator,
                MuonCollection=muons,
            )
        )

    # Add the final taggers based on neural networks
    for nn_path in nnList:
        # add standard (unflipped) taggers
        result.merge(
            FlavorTagNNCfg(
                inputFlags,
                BTaggingCollection=BTagCollection,
                TrackCollection=trackCollection,
                NNFile=nn_path)
        )
        # add flip taggers if requested
        if inputFlags.BTagging.RunFlipTaggers:
            for flip_config in _get_flip_config(nn_path):
                result.merge(
                    FlavorTagNNCfg(
                        inputFlags,
                        BTaggingCollection=BTagCollection,
                        TrackCollection=trackCollection,
                        NNFile=nn_path,
                        FlipConfig=flip_config,
                    )
                )

    # multifold models, at the moment this is only supported via inputFlags
    for networks in inputFlags.BTagging.NNs.get(jetcol, []):
        assert len(networks['folds']) > 1
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])

        args = dict(
            flags=inputFlags,
            BTaggingCollection=BTagCollection,
            TrackCollection=trackCollection,
            nnFilePaths=networks['folds'],
            remapping=networks.get('remapping', {})
        )
        
        # run the standard (unflipped tagger)
        result.merge(MultifoldGNNCfg(**args))

        # add flip taggers
        if inputFlags.BTagging.RunFlipTaggers and networks.get('flip', True):
            for flip_config in _get_flip_config(dirname):
                result.merge(MultifoldGNNCfg(**args, FlipConfig=flip_config))

    return result


def _get_flip_config(nn_path):
    """
    Schedule NN-based IP 'flip' taggers.

    FlipConfig is "STANDARD" by default - for flip tagger set up with
    option "NEGATIVE_IP_ONLY" (flip sign of d0 and use only (flipped)
    positive d0 values).

    Returns a list of flip configurations, or [] for things we don't flip.
    """
    nn_path = nn_path.lower()

    #flipping of DL1r with 2019 taggers does not work at the moment
    if (('dl1d' in nn_path) or ('dl1r' in nn_path and '201903' not in nn_path)):
        return ['FLIP_SIGN']
    if 'rnnip' in nn_path or 'dips' in nn_path:
        return ['NEGATIVE_IP_ONLY']
    if 'gn1' in nn_path or 'gn2' in nn_path:
        return ['SIMPLE_FLIP']
    else:
        return []


def addBTagToOutput(inputFlags, JetCollectionList, toAOD=True, toESD=True):
    """Write out the BTagging containers as defined by JetCollectionList
    """
    result = ComponentAccumulator()

    outlist = []

    for coll in JetCollectionList:
        registerContainer(coll, outlist)

    if toESD:
        result.merge(addToESD(inputFlags, outlist))
    if toAOD:
        result.merge(addToAOD(inputFlags, outlist))

    return result


# ---------------------------------------------------------------------------
# copied from the old BTaggingConfiguration.py
# ---------------------------------------------------------------------------
def registerContainer(JetCollection, bfg):
    Prefix = "BTagging_"
    SV = "SecVtx"
    JFVx = "JFVtx"
    Base = "xAOD::BTaggingContainer#"
    BaseAux = "xAOD::BTaggingAuxContainer#"
    BaseSecVtx = "xAOD::VertexContainer#"
    BaseAuxSecVtx = "xAOD::VertexAuxContainer#"
    BaseJFSecVtx = "xAOD::BTagVertexContainer#"
    BaseAuxJFSecVtx = "xAOD::BTagVertexAuxContainer#"

    author = Prefix + JetCollection # Get correct name with prefix
    bfg.append(Base + author)
    bfg.append(BaseAux + author + 'Aux.')
    # SeCVert
    bfg.append(BaseSecVtx + author + SV)
    bfg.append(BaseAuxSecVtx + author + SV + 'Aux.-vxTrackAtVertex')
    # JFSeCVert
    bfg.append(BaseJFSecVtx + author + JFVx)
    bfg.append(BaseAuxJFSecVtx + author + JFVx + 'Aux.')

