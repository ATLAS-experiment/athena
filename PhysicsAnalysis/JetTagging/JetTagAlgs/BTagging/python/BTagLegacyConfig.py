"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

Main configuration of flavour tagging algorithms.
The low and high level tagging algorithms are scheduled here.
"""

#! LEGACY: this is used by the legacy b-tagging in trigger and derivation
#! to be removed when migrating to modern b-tagging

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from FlavorTagInference.FlavorTagNNConfig import FlavorTagNNCfg
from BTagging.FlavorTaggingConfig import _get_flip_config


def BTagAlgsCfg(
    inputFlags,
    JetCollection,
    nnList=[],
    TaggerList=None,
    SecVertexers=None,
    trackCollection='InDetTrackParticles',
    primaryVertices='PrimaryVertices',
    muons='Muons',
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
    jetcol = JetCollection + AddedJetSuffix

    # Names of element link vectors that are stored on the jet and
    # BTagging object. These are added and read out by the packages
    # that are configured below: in principal you should be able to
    # change these without changing the final b-tagging output.
    JetTrackAssociator = 'TracksForBTagging'
    JetMuonAssociator = 'MuonsForBTagging'

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
            JetCollection = jetcol,
            TrackCollection = trackCollection,
            PrimaryVertexCollectionName = primaryVertices,
            SVFinder = sv, 
        ))

    if inputFlags.BTagging.RunNewVrtSecInclusive:
        #add soft b hadron vertex finder (outside of jets)
        from NewVrtSecInclusiveTool.NewVrtSecInclusiveAlgConfig import (
            NewVrtSecInclusiveAlgTightCfg,
            NewVrtSecInclusiveAlgMediumCfg,
            NewVrtSecInclusiveAlgLooseCfg,
        )
        result.merge(NewVrtSecInclusiveAlgTightCfg(inputFlags))
        result.merge(NewVrtSecInclusiveAlgMediumCfg(inputFlags))
        result.merge(NewVrtSecInclusiveAlgLooseCfg(inputFlags))


    # Add the final taggers based on neural networks
    for nn_path in nnList:
        # add standard (unflipped) taggers
        output_remapping={}
        if  '20240122trig' in nn_path:
            output_remapping={
                'pb': 'GN220240122_pb',
                'pc': 'GN220240122_pc',
                'pu': 'GN220240122_pu',
            }

        result.merge(
            FlavorTagNNCfg(
                inputFlags,
                JetCollection=jetcol,
                TrackCollection=trackCollection,
                NNFile=nn_path,
                variableRemapping=output_remapping)
        )
        # add flip taggers if requested
        if inputFlags.BTagging.RunFlipTaggers:
            for flip_config in _get_flip_config(nn_path):
                result.merge(
                    FlavorTagNNCfg(
                        inputFlags,
                        JetCollection=jetcol,
                        TrackCollection=trackCollection,
                        NNFile=nn_path,
                        FlipConfig=flip_config,
                    )
                )

    return result
