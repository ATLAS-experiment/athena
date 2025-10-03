"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg, JetParticleAssociationByVertexAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg, BTagTrackAugmenterByVertexAlgCfg
from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
from FlavorTagInference.FlavorTagNNConfig import MultifoldGNNCfg
from JetTagTools.JetFitterVariablesFactoryConfig import JetFitterVariablesFactoryCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg
from FlavorTagDiscriminants.FTagElectronAssociationConfig import FTagElectronAssociationCfg


from pathlib import Path


def _addDepsByDirname(cfgFlags, dirname: str, jetCollection: str) -> ComponentAccumulator:
    """
    Add additional algorithms based on the dirname of the network files.

    Parameters
    ----------
    cfgFlags : ConfigFlags
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
    if "GN3EPCLV01" in dirname or "Muon" in dirname:
        acc.merge(TrackLeptonDecorationCfg(cfgFlags))
    if "GN3EPCLV01" in dirname or "Electrons" in dirname:
        acc.merge(FTagElectronAssociationCfg(
            cfgFlags,
            jetCollection=jetCollection,
        ))
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
          cfgFlags,
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
            

    if not fast:
        acc.merge(JetTagVertexDecoratorCfg(
            cfgFlags,
            pv_col,
            JetCollection,
            trackCollection,
            JetTrackAssociator,
        ))



    for networks in cfgFlags.BTagging.NNs.get(JetCollection, []):
        assert isinstance(networks['folds'], list)
        dirnames = [Path(path).parent for path in networks['folds']]
        assert len(set(dirnames)) == 1, 'Different folds should be located in the same dir'
        dirname = str(dirnames[0])
        acc.merge(_addDepsByDirname(cfgFlags, dirname, JetCollection))

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
            args['remapping'].setdefault('BTagTrackToJetAssociator', 'GhostTrack')

        if '/GN2v01/' in dirname:
            args['tag_requirements'] = {'nonzeroTracks'}
        acc.merge(MultifoldGNNCfg(**args))

        # add flip taggers
        if cfgFlags.BTagging.RunFlipTaggers and networks.get('flip', True):
            for flip_config in _get_flip_config(dirname):
                acc.merge(MultifoldGNNCfg(**args, FlipConfig=flip_config))
             

    return acc

def JetBTagginglessByVertexAlgCfg(
        cfgFlags,
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
        cfgFlags,
        TrackCollection='InDetTrackParticles',
        PrimaryVertexCollectionName=pv_col,
        prefix=trackAugmenterPrefix,
        dzCut=max(dzCut_vec),
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

        # we want to run this for different cuts in z0, inclusive/exclusive at the same time
        for dzCut in dzCut_vec:
            for useMinZ0Vertex in useMinZ0Vertex_vec:
                acc.merge(JetParticleAssociationByVertexAlgCfg(
                    ConfigFlags = cfgFlags,
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
                args["remapping"] = {'BTagTrackToJetAssociator':'TracksForBTagging' + dz_suffix + "assoc",
                                      'GN2v01_pb': 'GN2v01' + dz_suffix + "pb",
                                      'GN2v01_pc': 'GN2v01' + dz_suffix + "pc",
                                      'GN2v01_pu': 'GN2v01' + dz_suffix + "pu",
                                      'GN2v01_ptau': 'GN2v01' + dz_suffix + "ptau",
                                      'GN2v01_TrackOrigin': 'GN2v01' + dz_suffix + 'TrackOrigin',
                                      'GN2v01_VertexIndex': 'GN2v01' + dz_suffix + 'VertexIndex',
                                      'GN2v01_TrackLinks': 'GN2v01' + dz_suffix + 'TrackLinks',
                                      'btagIp_': 'btagIp_ByVertex1_'}

                if '/GN2v01/' in dirname:
                    args['tag_requirements'] = {'nonzeroTracks'}
                    
                acc.merge(MultifoldGNNCfg(**args, dz_suffix=dz_suffix))

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
