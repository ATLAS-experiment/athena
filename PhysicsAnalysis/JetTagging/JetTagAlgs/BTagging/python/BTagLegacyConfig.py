"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

Main configuration of flavour tagging algorithms.
The low and high level tagging algorithms are scheduled here.
"""

#! LEGACY: this is used by the legacy b-tagging in trigger and derivation
#! to be removed when migrating to modern b-tagging

from pathlib import Path
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg
from BTagging.JetBTaggingAlgConfig import JetBTaggingAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from FlavorTagDiscriminants.BTagJetAugmenterAlgConfig import BTagJetAugmenterAlgCfg
from FlavorTagDiscriminants.BTagMuonAugmenterAlgConfig import BTagMuonAugmenterAlgCfg
from FlavorTagInference.FlavorTagNNConfig import FlavorTagNNCfg, MultifoldGNNCfg
from FlavorTagDiscriminants.FlavorTagDLNNConfig import FlavorTagDLNNCfg
from BTagging.FlavorTaggingConfig import _get_flip_config

def GNN_or_DL_cfg(nn_path):
    """Use the correct configuration for the NN based on the path"""
    return FlavorTagNNCfg if ('GN' in nn_path or 'gn' in nn_path) else FlavorTagDLNNCfg

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
    if BTagCollection is None:
        BTagCollection = f'BTagging_{jet}'

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
    if VxSecVertexInfoNameList:
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
        NN_cfg_func = GNN_or_DL_cfg(nn_path)
        output_remapping={}
        if  '20240122trig' in nn_path:
            output_remapping={
                'pb': 'GN220240122_pb',
                'pc': 'GN220240122_pc',
                'pu': 'GN220240122_pu',
            }

        result.merge(
            NN_cfg_func(
                inputFlags,
                BTaggingCollection=BTagCollection,
                JetCollection=jetcol,
                TrackCollection=trackCollection,
                NNFile=nn_path,
                variableRemapping=output_remapping)
        )
        # add flip taggers if requested
        if inputFlags.BTagging.RunFlipTaggers:
            for flip_config in _get_flip_config(nn_path):
                result.merge(
                    NN_cfg_func(
                        inputFlags,
                        BTaggingCollection=BTagCollection,
                        JetCollection=jetcol,
                        TrackCollection=trackCollection,
                        NNFile=nn_path,
                        FlipConfig=flip_config,
                    )
                )

    # multifold models, at the moment this is only supported via inputFlags
    for networks in inputFlags.BTagging.NNs.get(jetcol, []):
        assert isinstance(networks['folds'], list) 
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])

        # skip ghost association: not suppoted on the BTagging object
        if not networks.get('cone_association'):
            continue

        args = dict(
            flags=inputFlags,
            BTaggingCollection=BTagCollection,
            TrackCollection=trackCollection,
            nnFilePaths=networks['folds'],
            remapping=networks.get('remapping', {}),
            JetCollection=jetcol,
        )
        if foldHashName := networks.get('hash'):
            args['foldHashName'] = foldHashName


        # disable GN2v01 if there are 0 tracks
        if '/GN2v01/' in dirname:
            args['tag_requirements'] = {'nonzeroTracks'}

        # run the standard (unflipped tagger)
        result.merge(MultifoldGNNCfg(**args))

        # add flip taggers
        if inputFlags.BTagging.RunFlipTaggers and networks.get('flip', True):
            for flip_config in _get_flip_config(dirname):
                result.merge(MultifoldGNNCfg(**args, FlipConfig=flip_config))

    return result
