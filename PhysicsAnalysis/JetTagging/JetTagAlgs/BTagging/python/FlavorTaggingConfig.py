"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg, JetParticleAssociationByVertexAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg, BTagTrackAugmenterByVertexAlgCfg
from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
from FlavorTagInference.FlavorTagNNConfig import MultifoldGNNCfg
from FlavorTagInference.FlavorTagNNConfig import getDependencySet
from JetTagTools.JetFitterVariablesFactoryConfig import JetFitterVariablesFactoryCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg
from FlavorTagDiscriminants.FTagElectronAssociationConfig import FTagElectronAssociationCfg
from JetTagDerivationUtils.CopyJetParentInfoConfig import (
    CopyJetParentInfoCfg
)

from pathlib import Path
import re


_parent_collections = {
    'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets': 'AntiKt10UFOCSSKJets'
}


def _addDepsByDirname(flags, dirname: str, jetCollection: str) -> ComponentAccumulator:
    """
    Add additional algorithms based on the dirname of the network files.

    Parameters
    ----------
    flags : ConfigFlags
        The configuration flags for.
    dirname : str
        The directory name where the network files are located.
    jetCollection : str
        The name of the jet collection to which the additional algorithms will be applied.

    Returns
    -------
    ComponentAccumulator
        An accumulator containing the additional algorithms based on the dirname.
    """
    acc = ComponentAccumulator()

    modset = getDependencySet(dirname.split('/')[-2])

    if "L" in modset:
        acc.merge(TrackLeptonDecorationCfg(flags))
    if "E" in modset:
        acc.merge(FTagElectronAssociationCfg(
            flags,
            jetCollection=jetCollection,
        ))
    # TODO: Need to add "M" here
    if "X" in modset:
        acc.merge(
            CopyJetParentInfoCfg(
                flags,
                jetCollection,
                parents=_parent_collections[jetCollection]
            )
        )
    return acc

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
    if 'gn1' in nn_path or 'gn2' in nn_path or 'gn3' in nn_path:
        return ['SIMPLE_FLIP']
    else:
        return []


def FlavorTaggingCfg(
          flags,
          JetCollection,
          pv_col='PrimaryVertices',
          trackAugmenterPrefix=None,
          fast=False,
          JetTrackAssociator='TracksForBTagging',
          trackCollection='InDetTrackParticles',
          ):

    """
    Run flavour tagging on jet collection in derivations.
    """

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
            JetCollection,
            trackCollection,
            JetTrackAssociator,
        ))



    for networks in flags.BTagging.NNs.get(JetCollection, []):
        assert isinstance(networks['folds'], list)
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])

        # assume there are no special dependencies for jetmet regression
        if re.compile('.*/CalibArea(-[0-9]{2}){3}/.*').match(dirname):
            modset = set()
        else:
            acc.merge(_addDepsByDirname(flags, dirname, JetCollection))
            modset = getDependencySet(dirname.split('/')[-2])

        args = dict(
             flags=flags,
             JetCollection=JetCollection,
             TrackCollection=trackCollection,
             nnFilePaths=networks['folds'],
             remapping=networks.get('remapping', {}),
             electrons=('Electrons' if 'E' in modset else ''),
             muons=('Muons' if 'M' in modset else ''),
        )

        if foldHashName := networks.get('hash'):
            args['foldHashName'] = foldHashName


        if networks.get('cone_association'):
            acc.merge(JetParticleAssociationAlgCfg(
                flags,
                JetCollection,
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
            for flip_config in _get_flip_config(dirname):
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
        acc.merge(_addDepsByDirname(flags, dirname, JetCollection))

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
