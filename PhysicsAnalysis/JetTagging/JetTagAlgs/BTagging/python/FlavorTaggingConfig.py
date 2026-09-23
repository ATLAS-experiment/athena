"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg, JetParticleAssociationByVertexAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg, BTagTrackAugmenterByVertexAlgCfg
from FlavorTagInference.FlavorTagNNConfig import MultifoldGNNCfg
from FlavorTagInference.FlavorTagNNConfig import (
    REGRESSION_CALIBRATION_SCALE,
    TaggerDependenciesCfg,
    getDependencySet,
    getFlipConfigs,
    resolveTaggerName,
)
from JetTagTools.JetFitterVariablesFactoryConfig import JetFitterVariablesFactoryCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg
from JetTagDerivationUtils.CalibratedCopyTaggingConfig import (
    CalibratedCopyCfg,
    FtagScoreCopyCfg,
    copyCollectionName,
)

from pathlib import Path
import re


# Poor man's impact parameter definitions a tagger can ask for, keyed by
# the prefix its decorations are written under. The prefix names the
# definition, so every variant needs its own entry here.
_ip_definitions = {
    'poormanIp_': False,
    'poormanIpD0_': True,
}

# The standard impact parameters, written by BTagTrackAugmenterAlg rather
# than by the poor man's augmenter.
_default_ip_prefix = 'btagIp_'

# Constituent groups that can override the general ip_prefix.
_ip_prefix_groups = ('tracks', 'electrons', 'muons')

# The lepton associations declare their jet decorations on the base
# container, the other support algorithms on the jet container. The
# scheduler only connects a read to a write of the same type.
_iparticle_decorations = frozenset(
    {'FTagElectrons', 'FTagMuons', 'FTagMuonsConeMatched'})


def _supportDecorations(nns, jetTrackAssociator: str, fast: bool) -> dict:
    """
    Jet decorations the support algorithms write for a set of taggers,
    keyed by the container type they are declared on.

    Follows the algorithms scheduled by ``_FlavorTaggingChainCfg`` and
    ``TaggerDependenciesCfg``, so it has to be kept in sync with them. Taggers
    on a calibrated copy read these through the copy's parent store,
    where the scheduler cannot connect them to their producers.
    """
    decorations = set() if fast else {'SecVtx'}
    for networks in nns:
        dirname = str(Path(networks['folds'][0]).parent)
        modset = getDependencySet(resolveTaggerName(dirname, networks))
        decorations.add(
            jetTrackAssociator if networks.get('cone_association')
            else 'GhostTrack'
        )
        if 'E' in modset:
            decorations.add('FTagElectrons')
        if 'M' in modset:
            decorations.add('FTagMuons')
        if 'MC' in modset:
            decorations.add('FTagMuonsConeMatched')
        if 'R' in modset:
            decorations.add(f'{REGRESSION_CALIBRATION_SCALE}_pt')
        if 'X' in modset:
            decorations.add('jetRank')
    return {
        name: ('xAOD::IParticleContainer' if name in _iparticle_decorations
               else 'xAOD::JetContainer')
        for name in decorations
    }


def FlavorTaggingCfg(
          flags,
          JetCollection,
          pv_col='PrimaryVertices',
          **kwargs,
          ):

    """
    Run flavour tagging on jet collection in derivations.

    Taggers listed under flags.BTagging.CalibratedCopies run on a
    shallow copy with a frozen calibration and their opted-in outputs
    are copied back to the original collection (aft/open-tasks#105);
    all other taggers run directly on the original collection.
    """

    copy_taggers = flags.BTagging.CalibratedCopies.get(JetCollection, {})
    frozen, direct = [], []
    for networks in flags.BTagging.NNs.get(JetCollection, []):
        dirname = str(Path(networks['folds'][0]).parent)
        tagger = resolveTaggerName(dirname, networks)
        (frozen if tagger in copy_taggers else direct).append(networks)

    acc = _FlavorTaggingChainCfg(
        flags, JetCollection, pv_col=pv_col, nns=direct, **kwargs)
    if frozen:
        acc.merge(CalibratedCopyCfg(
            flags,
            JetCollection,
            supportDecorations=_supportDecorations(
                frozen,
                jetTrackAssociator=kwargs.get(
                    'JetTrackAssociator', 'TracksForBTagging'),
                fast=kwargs.get('fast', False),
            ),
        ))
        # support algs run on the original: the shallow copy reads their
        # decorations through its parent store
        acc.merge(_FlavorTaggingChainCfg(
            flags, copyCollectionName(JetCollection), pv_col=pv_col,
            nns=frozen, supportCollection=JetCollection, **kwargs))
        acc.merge(FtagScoreCopyCfg(flags, JetCollection))
    return acc


def _FlavorTaggingChainCfg(
          flags,
          JetCollection,
          nns,
          pv_col='PrimaryVertices',
          trackAugmenterPrefix=None,
          fast=False,
          JetTrackAssociator='TracksForBTagging',
          trackCollection='InDetTrackParticles',
          supportCollection=None,
          ):

    """
    Schedule the flavour tagging chain on a jet collection.

    The taggers to schedule are given in nns, so that a subset can be
    run on its own, e.g. on a frozen-calibration copy. Support
    algorithms (track association, secondary vertexing, tagger
    dependencies) run on supportCollection when given, e.g. the source
    collection of a shallow copy which shares its aux store.
    """

    support = supportCollection or JetCollection

    acc = ComponentAccumulator()
    if fast:
        acc.merge(
            _fastCfg(
                flags,
                tc=trackCollection,
                pv=pv_col,
                pfx=trackAugmenterPrefix,
            )
        )
    else:
        acc.merge(BTagTrackAugmenterAlgCfg(
            flags,
            TrackCollection=trackCollection,
            PrimaryVertexCollectionName=pv_col,
            prefix=trackAugmenterPrefix,
        ))

    if not fast:
        acc.merge(JetTagVertexDecoratorCfg(
            flags,
            pv_col,
            support,
            trackCollection,
            JetTrackAssociator,
        ))



    for networks in nns:
        assert isinstance(networks['folds'], list)
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])
        tagger_name = resolveTaggerName(dirname, networks)

        # Keep the no-dependency shortcut only for bJR10 large-R regression.
        # bJR4 regression requires lepton inputs and must resolve deps.
        is_calibarea = re.compile('.*/CalibArea(-[0-9]{2}){3}/.*').match(dirname)
        is_bjr10 = tagger_name.startswith('bJR10')
        if is_calibarea and is_bjr10:
            modset = set()
        else:
            acc.merge(TaggerDependenciesCfg(flags, tagger_name, support))
            modset = getDependencySet(tagger_name)

        args = dict(
             flags=flags,
             JetCollection=JetCollection,
             TrackCollection=trackCollection,
             nnFilePaths=networks['folds'],
             remapping=dict(networks.get('remapping', {})),
             dependencies=modset,
        )

        # Taggers trained on the poor man's impact parameters read their
        # IP inputs from a second set of decorations, written alongside
        # the standard ones. A group can ask for a different prefix than
        # the tracks with '<group>_ip_prefix'.
        _checkIpPrefixKeys(networks)
        if ip_prefix := networks.get('ip_prefix'):
            acc.merge(_ipInputsCfg(flags, ip_prefix, pv_col, trackCollection))
            args['remapping'].setdefault('btagIp_', ip_prefix)
        for group in _ip_prefix_groups:
            if group_prefix := networks.get(f'{group}_ip_prefix'):
                acc.merge(_ipInputsCfg(flags, group_prefix, pv_col,
                                       trackCollection))
                args['remapping'].setdefault(
                    f'{group}_ip_prefix', group_prefix)

        if foldHashName := networks.get('hash'):
            args['foldHashName'] = foldHashName


        if networks.get('cone_association'):
            acc.merge(JetParticleAssociationAlgCfg(
                flags,
                support,
                trackCollection,
                JetTrackAssociator,
            ))
        else:
            args['remapping'].setdefault('BTagTrackToJetAssociator', 'GhostTrack')

        if any(tag in dirname for tag in ['/GN2v01/', '/GN2HL/']):
            args['tag_requirements'] = {'nonzeroTracks'}
        acc.merge(MultifoldGNNCfg(**args))

        # add flip taggers
        if flags.BTagging.RunFlipTaggers and networks.get('flip', True):
            for flip_config in getFlipConfigs(dirname):
                acc.merge(MultifoldGNNCfg(**args, FlipConfig=flip_config))

    return acc

def JetBTagginglessByVertexAlgCfg(
        flags,
        JetCollection,
        pv_col='PrimaryVertices',
        trackAugmenterPrefix=None,
        dzCut_vec=[10],
        useMinZ0Vertex_vec=[True]):

    """
    Run flavour tagging on ByVertex jet collection in derivations.
    """

    JetTrackAssociator = 'TracksForBTagging'
    trackCollection='InDetTrackParticles'

    acc = ComponentAccumulator()

    acc.merge(BTagTrackAugmenterByVertexAlgCfg(
        flags,
        TrackCollection='InDetTrackParticles',
        PrimaryVertexCollectionName=pv_col,
        prefix=trackAugmenterPrefix,
        dzCut=max(dzCut_vec),
    ))        

    for networks in flags.BTagging.NNs.get(JetCollection, []):
        assert isinstance(networks['folds'], list)
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])
        tagger_name = resolveTaggerName(dirname, networks)
        acc.merge(TaggerDependenciesCfg(flags, tagger_name, JetCollection))

        args = dict(
             flags=flags,
             JetCollection=JetCollection,
             TrackCollection=trackCollection,
             nnFilePaths=networks['folds'],
             remapping=networks.get('remapping', {}),
        )

        if foldHashName := networks.get('hash'):
            args['foldHashName'] = foldHashName

        # we want to run this for different cuts in z0, inclusive/exclusive at the same time
        for dzCut in dzCut_vec:
            for useMinZ0Vertex in useMinZ0Vertex_vec:
                acc.merge(JetParticleAssociationByVertexAlgCfg(
                    flags,
                    JetCollection = JetCollection,
                    InputParticleCollection = trackCollection,
                    OutputParticleDecoration = JetTrackAssociator,
                    dzCut = dzCut,
                    useMinZ0Vertex = useMinZ0Vertex,
                ))

                if useMinZ0Vertex:
                    dz_suffix = '_' + str(dzCut) + '_' + 'exclusive_'

                else:
                    dz_suffix = '_' + str(dzCut) + '_' + 'inclusive_'

                # Remap variables
                tagger = flags.BTagging.AK4TaggerName
                args["remapping"] = {
                    'BTagTrackToJetAssociator':'TracksForBTagging' + dz_suffix + "assoc",
                    tagger + '_TrackLinks': tagger + dz_suffix + 'TrackLinks',
                    tagger + '_pb': tagger + dz_suffix + 'pb',
                    tagger + '_pc': tagger + dz_suffix + 'pc',
                    tagger + '_pu': tagger + dz_suffix + 'pu',
                    tagger + '_ptau': tagger + dz_suffix + 'ptau'}

                if flags.GeoModel.Run <= LHCPeriod.Run3:
                    args["remapping"].update({
                        tagger + '_TrackOrigin': tagger + dz_suffix + 'TrackOrigin',
                        tagger + '_VertexIndex': tagger + dz_suffix + 'VertexIndex'})

                if any(tag in dirname for tag in ['/GN2v01/', '/GN2HL/']):
                    args['tag_requirements'] = {'nonzeroTracks'}

                acc.merge(MultifoldGNNCfg(**args, suffix=dz_suffix))

    return acc

def _checkIpPrefixKeys(networks):
    valid = {'ip_prefix'} | {f'{g}_ip_prefix' for g in _ip_prefix_groups}
    for key in networks:
        if key.endswith('ip_prefix') and key not in valid:
            raise ValueError(
                f'unsupported IP prefix setting {key!r}, expected one of '
                f'{sorted(valid)}')
    if 'ip_prefix' in networks and 'tracks_ip_prefix' in networks:
        raise ValueError(
            "set either 'ip_prefix' or 'tracks_ip_prefix', not both")


def _ipInputsCfg(flags, prefix, pv, tc):
    """Schedule whatever writes the IP decorations under prefix."""
    if prefix == _default_ip_prefix:
        return ComponentAccumulator()
    if prefix not in _ip_definitions:
        raise ValueError(
            f'unknown IP prefix {prefix!r}, expected {_default_ip_prefix!r} '
            f'or one of {sorted(_ip_definitions)}')
    return _fastCfg(flags, pv=pv, tc=tc, pfx=prefix)


def _fastCfg(flags, pv, tc, pfx):
    acc = ComponentAccumulator()
    name = f'PoorMansAugmenter_{tc}_{pv}_{pfx}'
    prefix = pfx or _default_ip_prefix
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.PoorMansIpAugmenterAlg(
            name=name,
            trackContainer=tc,
            primaryVertexContainer=pv,
            prefix=prefix,
            d0_modification=_ip_definitions.get(prefix, False)
        )
    )
    return acc

def JetTagVertexDecoratorCfg(flags, pv_col, jet, trackCollection, JetTrackAssociator,):

    jetcol_no_suffix = jet.replace("Jets","")

    acc = ComponentAccumulator()

    acc.merge(JetParticleAssociationAlgCfg(
        flags,
        jet,
        trackCollection,
        JetTrackAssociator,
    ))

    SecVertexers = ['SV1']
    if flags.BTagging.RunFlipTaggers:
        SecVertexers += ['SV1Flip']

    secVtxFinderxAODBaseNameList = []

    OutputFilesSVname = "SecVtx"
    OutputFilesSVFlipname = 'SecVtxFlip'

    jetFitterVF = acc.popToolsAndMerge(JetFitterVariablesFactoryCfg('JFVarFactory'))

    VxSecVertexInfoNameList = []

    for sv in SecVertexers:
        BTagVxSecVertexInfoName = sv + 'VxSecVertexInfo_' + jetcol_no_suffix
        VxSecVertexInfoNameList.append(BTagVxSecVertexInfoName)
        secVtxFinderxAODBaseNameList.append(sv)
        AlgName = (jetcol_no_suffix + '_' + sv).lower()

        acc.merge(JetSecVtxFindingAlgCfg(
            flags,
            BTagVxSecVertexInfoName = BTagVxSecVertexInfoName,
            SVAlgName = AlgName + '_secvtxfinding',
            JetCollection = jet,
            PrimaryVertexCollectionName = pv_col,
            SVFinder = sv,
            TracksToTag = JetTrackAssociator,
        ))

        acc.merge(JetSecVertexingAlgCfg(
            flags,
            BTagVxSecVertexInfoName = BTagVxSecVertexInfoName,
            SVAlgName = AlgName + '_secvtx',
            JetCollection = jet,
            TrackCollection = trackCollection,
            PrimaryVertexCollectionName = pv_col,
            SVFinder = sv, 
        ))

    # Add secondary vertices to jet
    options = {
        'name': f'JetTagVertexDecoratorAlg{jet}'
    }
    options.setdefault('SecVtxFinderxAODBaseNameList', secVtxFinderxAODBaseNameList)
    options.setdefault('vxPrimaryCollectionName', pv_col)
    options.setdefault('JetFitterVariableFactory', jetFitterVF)
    options['JetSecVtxLinkName'] = jet + '.' + OutputFilesSVname
    options['JetCollectionName'] = jet
    options['BTagVxSecVertexInfoNames'] = []
    for sv in SecVertexers:
        options['BTagVxSecVertexInfoNames'].append(sv + 'VxSecVertexInfo_' + jetcol_no_suffix)

    if flags.BTagging.RunFlipTaggers:
        options['JetSecVtxFlipLinkName'] = jet + '.' + OutputFilesSVFlipname


    acc.addEventAlgo(CompFactory.Analysis.JetTagVertexDecoratorAlg(**options))
    return acc
