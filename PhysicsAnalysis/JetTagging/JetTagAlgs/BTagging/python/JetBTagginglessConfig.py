"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg, JetParticleAssociationByVertexAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg, BTagTrackAugmenterByVertexAlgCfg
from BTagging.BTagConfig import _get_flip_config
from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
from FlavorTagInference.FlavorTagNNConfig import MultifoldGNNCfg
from BTagging.BTagToolConfig import BTagToolCfg
from JetTagTools.JetFitterVariablesFactoryConfig import JetFitterVariablesFactoryCfg
from BTagging.JetSecVtxFindingAlgConfig import JetSecVtxFindingAlgCfg
from BTagging.JetSecVertexingAlgConfig import JetSecVertexingAlgCfg


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
        if ByVertex:
            acc.merge(BTagTrackAugmenterByVertexAlgCfg(
                cfgFlags,
                TrackCollection='InDetTrackParticles',
                PrimaryVertexCollectionName=pv_col,
                prefix=trackAugmenterPrefix,
            ))
          
            
        else:
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
                print("Mario: runnning ", str(dzCut), " ", str(useMinZ0Vertex))
                acc.merge(JetParticleAssociationByVertexAlgCfg(
                    ConfigFlags = cfgFlags,
                    JetCollection = JetCollection,
                    InputParticleCollection = trackCollection,
                    OutputParticleDecoration = JetTrackAssociator,
                    dzCut = dzCut,
                    useMinZ0Vertex = useMinZ0Vertex,
                ))
                print("Mario worked")
                # here I need to add all the possible remapping
                # find out how the re
                if useMinZ0Vertex:
                    '''
                    args['remapping'].setdefault(
                    'BTagTrackToJetAssociator', 'TracksForBTagging_' + str(dzCut) + '_' + 'exclusive_assoc')
                    args['remapping'].setdefault(
                    'GN2v01_pb', 'GN2v01_' + str(dzCut) + '_' + 'exclusive_pb')
                    '''
                    args["remapping"]={'BTagTrackToJetAssociator':'TracksForBTagging_' + str(dzCut) + '_' + 'exclusive_assoc',
                                        'GN2v01_pb': 'GN2v01_' + str(dzCut) + '_' + 'exclusive_pb'}
                    '''
                    args['remapping'].setdefault(
                    'GN2v01_pc', 'GN2v01_' + str(dzCut) + '_' + 'exclusive_pc')
                    args['remapping'].setdefault(
                    'GN2v01_pu', 'GN2v01_' + str(dzCut) + '_' + 'exclusive_pu')
                    args['remapping'].setdefault(
                    'GN2v01_ptau', 'GN2v01_' + str(dzCut) + '_' + 'exclusive_ptau')
                    '''
                  
                else:
                    args["remapping"]={'BTagTrackToJetAssociator': 'TracksForBTagging_' + str(dzCut) + '_' + 'inclusive_assoc',
                                        'GN2v01_pb': 'GN2v01_' + str(dzCut) + '_' + 'inclusive_pb'}
                    '''
                    args['remapping'].setdefault(
                    'BTagTrackToJetAssociator', 'TracksForBTagging_' + str(dzCut) + '_' + 'inclusive_assoc')
                    
                    args['remapping'].setdefault(
                    'GN2v01_pb', 'GN2v01_' + str(dzCut) + '_' + 'inclusive_pb')
                    
                    args['remapping'].setdefault(
                    'GN2v01_pc', 'GN2v01_' + str(dzCut) + '_' + 'inclusive_pc')
                    args['remapping'].setdefault(
                    'GN2v01_pu', 'GN2v01_' + str(dzCut) + '_' + 'inclusive_pu')
                    args['remapping'].setdefault(
                    'GN2v01_ptau', 'GN2v01_' + str(dzCut) + '_' + 'inclusive_ptau')
                    '''

                if '/GN2v01/' in dirname:
                    args['tag_requirements'] = {'nonzeroTracks'}
                    
                acc.merge(MultifoldGNNCfg(**args))
                '''
                # I think we can remove flip taggers and here we need to find a way to 
                # add flip taggers
                if cfgFlags.BTagging.RunFlipTaggers and networks.get('flip', True):
                    for flip_config in _get_flip_config(dirname):
                        acc.merge(MultifoldGNNCfg(**args, FlipConfig=flip_config))
                '''

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

    SetupScheme = ''
    jetcol_no_suffix = jet.replace("Jets","")

    acc = ComponentAccumulator()
    options = {}

    options['BTagTool'] = acc.popToolsAndMerge(BTagToolCfg(
        flags, ['SV1'], pv_col, SetupScheme))

    SecVertexers = ['SV1','JetFitter'] 
    if flags.BTagging.RunFlipTaggers:
        SecVertexers += ['JetFitterFlip','SV1Flip']
        
    secVtxFinderxAODBaseNameList = [] 

    OutputFilesJFVxname = "JFVtx"
    OutputFilesJFVxFlipname = "JFVtxFlip"
    OutputFilesSVname = "SecVtx"
    OutputFilesSVFlipname = 'SecVtxFlip'

    jetFitterVF = acc.popToolsAndMerge(JetFitterVariablesFactoryCfg('JFVarFactory'))

    VxSecVertexInfoNameList = []
    BTagCollection = f'BTagging_{jetcol_no_suffix}'

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
            BTaggingCollection = BTagCollection,
            JetCollection = jet,
            TrackCollection = trackCollection,
            PrimaryVertexCollectionName = pv_col,
            SVFinder = sv, 
        ))

    # Add secondary vertices to jet
    options = {}
    options.setdefault('SecVtxFinderxAODBaseNameList', secVtxFinderxAODBaseNameList)
    options.setdefault('vxPrimaryCollectionName', pv_col)
    options.setdefault('JetFitterVariableFactory', jetFitterVF)
    options['JetSecVtxLinkName'] = jet + '.' + OutputFilesSVname
    options['JetJFVtxLinkName'] = jet + '.' + OutputFilesJFVxname
    options['JetCollectionName'] = jet
    options['BTagVxSecVertexInfoNames'] = []
    for sv in SecVertexers:
        options['BTagVxSecVertexInfoNames'].append(sv + 'VxSecVertexInfo_' + jetcol_no_suffix)

    if flags.BTagging.RunFlipTaggers:
        options['JetJFFlipVtxLinkName'] = jet + '.' + OutputFilesJFVxFlipname
        options['JetSecVtxFlipLinkName'] = jet + '.' + OutputFilesSVFlipname


    acc.addEventAlgo(CompFactory.Analysis.JetTagVertexDecoratorAlg(**options))

    return acc
