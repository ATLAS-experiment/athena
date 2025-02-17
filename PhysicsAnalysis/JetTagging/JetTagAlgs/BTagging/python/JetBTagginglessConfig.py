"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from BTagging.BTagConfig import _get_flip_config
from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
from FlavorTagDiscriminants.FlavorTagNNConfig import MultifoldGNNCfg

from pathlib import Path


def JetBTagginglessAlgCfg(
          cfgFlags,
          JetCollection,
          pv_col='PrimaryVertices',
          trackAugmenterPrefix=None,
          fast=False):

    """
    Run flavour tagging on jet collection in derivations.
    """

    JetTrackAssociator = 'TracksForBTagging'
    trackCollection='InDetTrackParticles'

    acc = ComponentAccumulator()

    if fast:
        acc.merge(
            _fastCfg(
                cfgFlags,
                tc=trackCollection,
                pv=pv_col,
                pfx=trackAugmenterPrefix,
            )
        )
    else:
        acc.merge(BTagTrackAugmenterAlgCfg(
            cfgFlags,
            TrackCollection='InDetTrackParticles',
            PrimaryVertexCollectionName=pv_col,
            prefix=trackAugmenterPrefix,
        ))

    for networks in cfgFlags.BTagging.NNs.get(JetCollection, []):
        assert isinstance(networks['folds'], list)
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])

        if 'Muon' in dirname:
            acc.merge(TrackLeptonDecorationCfg(cfgFlags))

        args = dict(
             flags=cfgFlags,
             JetCollection=JetCollection,
             TrackCollection=trackCollection,
             nnFilePaths=networks['folds'],
             remapping=networks.get('remapping', {}),
        )

        if foldHashName := networks.get('hash'):
            args['foldHashName'] = foldHashName

        if networks.get('cone_association'):
            acc.merge(JetParticleAssociationAlgCfg(
                cfgFlags,
                JetCollection,
                trackCollection,
                JetTrackAssociator,
            ))
        else:
            args['remapping'].setdefault(
                'BTagTrackToJetAssociator', 'GhostTrack')

        if '/GN2v01/' in dirname:
            args['tag_requirements'] = {'nonzeroTracks'}

        acc.merge(MultifoldGNNCfg(**args))

        # add flip taggers
        if cfgFlags.BTagging.RunFlipTaggers and networks.get('flip', True):
            for flip_config in _get_flip_config(dirname):
                acc.merge(MultifoldGNNCfg(**args, FlipConfig=flip_config))

    return acc


def _fastCfg(flags, pv, tc, pfx):
    acc = ComponentAccumulator()
    name = f'PoorMansAugmenter_{tc}_{pv}_{pfx}'
    prefix = pfx or 'btagIp_'
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.PoorMansIpAugmenterAlg(
            name=name,
            trackContainer=tc,
            primaryVertexContainer=pv,
            prefix=prefix
        )
    )
    return acc
