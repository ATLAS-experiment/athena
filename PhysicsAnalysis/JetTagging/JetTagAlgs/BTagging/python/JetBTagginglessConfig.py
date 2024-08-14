"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from FlavorTagDiscriminants.FlavorTagNNConfig import MultifoldGNNCfg

from pathlib import Path

def JetBTagginglessAlgCfg(
        cfgFlags,
        JetCollection,
        pv_col=None,
        trackAugmenterPrefix=None):

    """
    Run flavour tagging on jet collection in derivations.
    """

    JetTrackAssociator = 'TracksForBTagging'
    trackCollection='InDetTrackParticles'

    acc = ComponentAccumulator()

    acc.merge(BTagTrackAugmenterAlgCfg(
        cfgFlags,
        TrackCollection='InDetTrackParticles',
        PrimaryVertexCollectionName=pv_col,
        prefix=trackAugmenterPrefix,
    ))

    acc.merge(JetParticleAssociationAlgCfg(
        cfgFlags,
        JetCollection,
        trackCollection,
        JetTrackAssociator,
    ))

    for networks in cfgFlags.BTagging.NNs.get(JetCollection, []):
        assert len(networks['folds']) > 1
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])

        args = dict(
                flags=cfgFlags,
                JetCollection=JetCollection,
                TrackCollection=trackCollection,
                nnFilePaths=networks['folds'],
                remapping=networks.get('remapping', {}),
        )

        if '/GN2v01/' in dirname:
            args['tag_requirements'] = {'nonzeroTracks'}

        acc.merge(MultifoldGNNCfg(**args))

        return acc
