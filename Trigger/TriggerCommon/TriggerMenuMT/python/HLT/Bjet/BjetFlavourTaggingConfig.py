#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# standard b-tagging
from BTagging.JetParticleAssociationAlgConfig import JetParticleAssociationAlgCfg
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from BTagging.BTagLegacyConfig import BTagAlgsCfg

# fast btagging
from FlavorTagInference.FlavorTagNNConfig import getStaticTrackVars
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg

def flavourTaggingCfg( flags, inputJets, inputVertex, inputTracks,
                       inputMuons = ""):

    # because Cfg functions internally re-append the 'Jets' string
    inputJetsPrefix = inputJets.replace("bJets","b")

    acc = ComponentAccumulator()


    #Track Augmenter
    acc.merge(BTagTrackAugmenterAlgCfg(
        flags,
        TrackCollection=inputTracks,
        PrimaryVertexCollectionName=inputVertex
    ))

    #Run new Run3 taggers, i.e. DL1, RNNIP, DL1r
    nnList = [

        # These are trigger-specific trainings
        #
        # Trigger GN1 training
        'BTagging/20220813trig/gn1/antikt4empflow/network.onnx',
        #Trigger GN2 training
        'BTagging/20240122trig/gn2/antikt4empflow/SmallPrec.onnx'
    ]


    acc.merge(BTagAlgsCfg(
        inputFlags=flags,
        JetCollection=inputJetsPrefix,
        nnList=nnList,
        trackCollection=inputTracks,
        muons=inputMuons,
        primaryVertices=inputVertex,
        AddedJetSuffix='Jets',
        SecVertexers = [],
    ))

    return acc

def fastFlavourTaggingCfg( flags, inputJets, inputVertex, inputTracks, isPFlow=False, fastDipsMinimumPt=None, doXbbtagLargeRJet = False):
    """
    This function tags jets directly: there is no  b-tagging object
    """

    ca = ComponentAccumulator()

    # first add the track augmentation
    jet_name = inputJets
    if isPFlow:
        if doXbbtagLargeRJet:
            ca.merge(OnlineBeamspotAugmenterCfg(flags))

            ca.merge(
                OnlineIpAugmenterCfg(
                    flags,
                    tracks = inputTracks,
                    vertices = inputVertex,
                    trackIpPrefix='XbbIp_'
                ))

        else:
            ca.merge(
                BTagTrackAugmenterAlgCfg(
                    flags,
                    TrackCollection=inputTracks,
                    PrimaryVertexCollectionName=inputVertex,
                )
            )
    else:
        trackIpPrefix='simpleIp_'
        ca.merge(
            OnlineIpAugmenterCfg(
            flags,
            tracks=inputTracks,
            vertices=inputVertex,
            trackIpPrefix=trackIpPrefix
            )
        )
        if inputVertex:
            ca.merge(
            BTagTrackAugmenterAlgCfg(
                flags,
                TrackCollection=inputTracks,
                PrimaryVertexCollectionName=inputVertex,
            )
        )

    # now we associate the tracks to the jet
    ## JetParticleAssociationAlgCfg uses a shrinking cone.
    if doXbbtagLargeRJet:
        tracksOnJetDecoratorName = "GhostTrack_ftf"
    else:
        tracksOnJetDecoratorName = "TracksForMinimalJetTag"
        pass_flag = f'{tracksOnJetDecoratorName}_isValid'
        ca.merge(
            JetParticleAssociationAlgCfg(
                flags,
                JetCollection=jet_name,
                InputParticleCollection=inputTracks,
                OutputParticleDecoration=tracksOnJetDecoratorName,
                MinimumJetPt=fastDipsMinimumPt,
                MinimumJetPtFlag=pass_flag
            )
        )

    # Now we have to add an algorithm that tags the jets with dips
    # The input and output remapping is handled via a map in DL2.
    #
    # The file above adds fastDIPSnoPV20220211_p*, we'll call them
    # fastDips_p* on the jet.
    if isPFlow:
        if doXbbtagLargeRJet:
            dl2_configs = [
                [
                    'BTagging/20230705/gn2xv01/antikt10ufo/network.onnx',
                    {
                        'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                        'btagIp_': 'XbbIp_',
                    }
                ],
                [
                    'BTagging/20250702trig/GN2XTrig/antikt10empflow/network.onnx',
                    {
                        'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                        'btagIp_': 'XbbIp_',
                    }
                ],
            ]
        else: 
            dl2_configs=[
                [
                    'BTagging/20211216trig/dips/AntiKt4EMPFlow/network.json',
                    {
                        'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    }
                ],
                [
                    'BTagging/20211215trig/fastDips/antikt4empflow/network.json',
                    {
                        'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    },
                ],

                [
                    'BTagging/20230331trig/gn1/antikt4empflow/network.onnx',
                    {
                        'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    },
                ],
                                [
                 'BTagging/20240122trig/gn2/antikt4empflow/Small.onnx',
                {
                    'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    **{f'p{x}': f'tlaGN220240122_p{x}' for x in 'cub'},
                }
                ],   
            ]
    else:
        dl2_configs=[
            [
                'BTagging/20220211trig/fastDips/antikt4empflow/network.json',
                {
                    'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    **{f'fastDIPSnoPV20220211_p{x}': f'fastDips_p{x}' for x in 'cub'},
                    'btagIp_': trackIpPrefix,
                }
            ],
            [
                'BTagging/20230327trig/gn1/antikt4emtopo/network.onnx',
                {
                    'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    **{f'GN120230327_p{x}': f'fastGN120230327_p{x}' for x in 'cub'},
                    'btagIp_': trackIpPrefix,
                }
            ],
            [
                'BTagging/20231122trig/dipz/antikt4emtopo/network.json',
                {
                    'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    'btagIp_': trackIpPrefix,
                }
            ],

        ]
        if inputVertex: 
            dl2_configs += [
                [
                 'BTagging/20230331trig/gn1/antikt4empflow/network.onnx',
                {
                    'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    **{f'GN120230331_p{x}': f'fastGN120230331_p{x}' for x in 'cub'}
                },   
                ],
                [
                 'BTagging/20240122trig/gn2/antikt4empflow/Small.onnx',
                {
                    'BTagTrackToJetAssociator': tracksOnJetDecoratorName,
                    **{f'p{x}': f'fastGN220240122_p{x}' for x in 'cub'},
                }
                ],

                [ 'BTagging/20240216trig/gn2/antikt4emtopo/GNTau.onnx',
                    { 'BTagTrackToJetAssociator': tracksOnJetDecoratorName
                    , 'GN2_ptau' : 'fastGNTau20240216_ptau'
                    , 'GN2_pu' : 'fastGNTau20240216_pu'
                    }
                ],

                [ 'BTagging/20250813trig/uht1/antikt4emtopo/UHT1.onnx',
                    { 'BTagTrackToJetAssociator': tracksOnJetDecoratorName
                    , 'FastGNTau_ptau' : 'fastUHT120250605_ptau'
                    , 'FastGNTau_pu'   : 'fastUHT120250605_pu'
                    , 'FastGNTau_pc'   : 'fastUHT120250605_pc'
                    , 'FastGNTau_pb'   : 'fastUHT120250605_pb'
                    }
                ]   
            ]

    # not all the keys that the NN requests are declaired. This will
    # cause an algorithm stall if we don't explicetly tell it that it
    # can ignore some of them.
    missingKeys = getStaticTrackVars(inputTracks)

    for nnFile, variableRemapping in dl2_configs:
        nnAlgo = nnFile.replace('/','_').split('.')
        nnAlgoKey = nnAlgo[0]
        nnAlgoext = nnAlgo[1]
        toolDict = {
            "json": CompFactory.FlavorTagDiscriminants.DL2Tool,
            "onnx": CompFactory.FlavorTagInference.GNNTool
        }
        if not doXbbtagLargeRJet:
            tag_flags = {pass_flag}
        else:
            tag_flags = {}

        if nnAlgoext == 'onnx':
            defaults = _triggerDefaultsFromPath(nnFile)
            extra = dict(
                defaultOutputValues=defaults,
                defaultZeroTracks=(True if defaults else False),
            )
        else:
            extra = {}

        ca.addEventAlgo(
            CompFactory.FlavorTagInference.JetTagConditionalDecoratorAlg(
                name='_'.join([
                    'simpleJetTagAlg',
                    jet_name,
                    inputTracks,
                    nnAlgoKey,
                ]),
                container=jet_name,
                constituentContainer=inputTracks,
                undeclaredReadDecorKeys=missingKeys,
                tagFlags=list(tag_flags),
                decorator=toolDict[nnAlgoext](
                    name='_'.join([
                        'simpleDipsToJet',
                        nnAlgoKey,
                    ]),
                    nnFile=nnFile,
                    variableRemapping=variableRemapping,
                    # note that the tracks are associated to the jet as
                    # and IParticle container.
                    trackLinkType='IPARTICLE',
                    defaultOutputValue=0,
                    **extra,
                ),
            )
        )
    return ca

def OnlineBeamspotAugmenterCfg(flags,pfx='online'):
    ca = ComponentAccumulator()

    ca.merge(BeamSpotCondAlgCfg(flags))
    decnames = {
        f'{n}Key':n.replace('beam',f'EventInfo.{pfx}Beam')
        for n in [
            'beamPosX','beamPosY','beamPosZ',
            'beamPosSigmaX','beamPosSigmaY','beamPosSigmaZ','beamPosSigmaXY',
            'beamTiltXZ','beamTiltYZ',
            'beamStatus'
            ]
    }
    ca.addEventAlgo(CompFactory.xAODMaker.EventInfoBeamSpotDecoratorAlg(
        'EventInfoOnlineBeamSpotDecorator',
        **decnames
        )
    )

    return ca

def OnlineIpAugmenterCfg(
        flags, tracks, vertices='',
        beamSpotPrefix='online',
        trackIpPrefix='simpleIp_'):
    ca = ComponentAccumulator()

    decnames = {
        n:n.replace('beam',f'EventInfo.{beamSpotPrefix}Beam').replace('spotSigma','Pos').replace('spotCovariance','PosSigma')
        for n in [
            'beamspotSigmaX','beamspotSigmaY','beamspotSigmaZ','beamspotCovarianceXY',
        ]
    }
    
    ca.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.PoorMansIpAugmenterAlg(
            name='_'.join([
                'SimpleTrackAugmenter',
                tracks,
                vertices,
                beamSpotPrefix,
                trackIpPrefix,
            ]).replace('__','_').rstrip('_'),
            trackContainer=tracks,
            primaryVertexContainer=vertices,
            prefix=trackIpPrefix,
            **decnames
        )
    )
    return ca

def _triggerDefaultsFromPath(nn_path):
    """Special overrides for AFT-726

    In more recent ONNX versions we can't run a lot of versions of GN2
    on zero track jets. The workaround here is to hardcode the values
    we'd normally get out of the GNN in this case.
    """

    if '20230331trig' in nn_path:
        # pflow GN1
        return {
            'GN120230331_pu': 1.0,
        }
    elif '20240122trig' in nn_path:
        # pflow and emtopo GN2
        return {
            'pb': 0.0973031148314476,
            'pc': 0.21328286826610565,
            'pu': 0.6894140243530273,
        }
    elif '20230327trig' in nn_path:
        # emtopo GN1
        return {
            'GN120230327_pu': 1.0
        }
    elif '20230331trig' in nn_path:
        # emtopo GN1, with input vertex
        return {
            'GN120230331_pu': 1.0
        }
    elif '20240216trig' in nn_path:
        # emtopo GNtau
        return {
            'GN2_ptau': 0.18229439854621887,
            'GN2_pu': 0.8177056312561035,
        }
    elif '20230705/gn2xv01' in nn_path:
        # GN2X
        return {
            'GN2Xv01_phbb': 0.524639487266540527, 
            'GN2Xv01_ptop': 0.24834538996219635, 
            'GN2Xv01_pqcd': 0.101029679179191589,
        }
    return {}
