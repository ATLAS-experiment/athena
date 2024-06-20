# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from os.path import commonpath
from pathlib import PurePath

def addAndReturnSharingSvc(flags, ca):
    svc = CompFactory.FlavorTagDiscriminants.NNSharingSvc('FTagNNSharingSvc')
    ca.addService(svc)
    return svc

def DL2ToolCfg(flags, NNFile, **options):
    acc = ComponentAccumulator()

    # default is "STANDARD" in case of a setup of the standard b-taggers. "NEGATIVE_IP_ONLY" [and "FLIP_SIGN"] if want to set up the flip taggers
    # naming convention, see here: https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/JetTagging/FlavorTagDiscriminants/Root/FlipTagEnums.cxx

    # this map lets us change the names of EDM inputs with respect to
    # the values we store in the saved NN
    remap = {}
    # This is a hack to accomodate the older b-tagging training with
    # old names for variables. We should be able to remove it when we
    # move over to the 2020 / 2021 retraining.
    if '201903' in NNFile and 'dl1' in NNFile:
        for aggragate in ['minimum','maximum','average']:
            remap[f'{aggragate}TrackRelativeEta'] = (
                f'JetFitterSecondaryVertex_{aggragate}AllJetTrackRelativeEta')

    # Similar hack for 21.9-based upgrade training
    if '20221008' in NNFile and 'dips' in NNFile:
        for aggragate in ['InnermostPixelLayer', 'NextToInnermostPixelLayer',
                          'InnermostPixelLayerShared',
                          'InnermostPixelLayerSplit']:
            remap[f'numberOf{aggragate}Hits'] = (
                f'numberOf{aggragate}Hits21p9')

    mkey = 'variableRemapping'
    options[mkey] = remap | options.get(mkey,{})

    dl2 = CompFactory.FlavorTagDiscriminants.DL2Tool(
        name='decorator',
        nnFile=NNFile,
        **options)

    acc.setPrivateTools(dl2)

    return acc

def GNNToolCfg(flags, NNFile, **options):
    acc = ComponentAccumulator()

    # this map lets us change the names of EDM inputs with respect to
    # the values we store in the saved NN
    remap = {}

    if '20221010' in NNFile and 'GN1' in NNFile:
        for aggragate in ['InnermostPixelLayer', 'NextToInnermostPixelLayer',
                          'InnermostPixelLayerShared',
                          'InnermostPixelLayerSplit']:
            remap[f'numberOf{aggragate}Hits'] = (
                f'numberOf{aggragate}Hits21p9')

    mkey = 'variableRemapping'
    options[mkey] = remap | options.get(mkey,{})

    gnntool = CompFactory.FlavorTagDiscriminants.GNNTool(
        name='decorator',
        nnFile=NNFile,
        nnSharingService=addAndReturnSharingSvc(flags, acc),
        **options)

    acc.setPrivateTools(gnntool)

    return acc

def getStaticTrackVars(TrackCollection):
    # some things should not be declared as date dependencies: it will
    # make the trigger sad.
    #
    # In the case of tracking it's mostly static variables that are a
    # problem.
    static_track_vars = [
        'numberOfInnermostPixelLayerHits',
        'numberOfInnermostPixelLayerSharedHits',
        'numberOfInnermostPixelLayerSplitHits',
        'numberOfNextToInnermostPixelLayerHits',
        'numberOfPixelDeadSensors',
        'numberOfPixelHits',
        'numberOfPixelHoles',
        'numberOfPixelSharedHits',
        'numberOfPixelSplitHits',
        'numberOfSCTDeadSensors',
        'numberOfSCTHits',
        'numberOfSCTHoles',
        'numberOfSCTSharedHits',
        'chiSquared',
        'numberDoF',
        'qOverP',
    ]
    return [f'{TrackCollection}.{x}' for x in static_track_vars]

def getUndeclaredBtagVars(BTaggingCollection):
    #
    # In the case of b-tagging we should really declare these
    # variables using WriteDecorHandle, but this is very much a work
    # in progress.
    #
    # We should revisit this once in a while, last time this was
    # checked was:
    #
    #  - 20210602
    #
    undeclared_btag = [
        'JetFitter_N2Tpair',
        'JetFitter_energyFraction',
        'JetFitter_mass',
        'JetFitter_nSingleTracks',
        'JetFitter_nTracksAtVtx',
        'JetFitter_nVTX',
        'JetFitter_significance3d',
        'SV1_L3d',
        'SV1_Lxy',
        'SV1_N2Tpair',
        'SV1_NGTinSvx',
        'SV1_deltaR',
        'SV1_efracsvx',
        'SV1_masssvx',
        'SV1_significance3d',
        'BTagTrackToJetAssociator',
    ]
    return [f'{BTaggingCollection}.{x}' for x in undeclared_btag]

def FlavorTagNNCfg(
        flags,
        BTaggingCollection,
        TrackCollection,
        NNFile,
        FlipConfig="STANDARD",
        variableRemapping={}):

    acc = ComponentAccumulator()

    NNFile_extension = NNFile.split(".")[-1]
    nn_opts = dict(
        NNFile=NNFile,
        flipTagConfig=FlipConfig,
        variableRemapping=variableRemapping)
    if NNFile_extension == "json":
        nn_name = NNFile.replace("/", "_").replace("_network.json", "")
        decorator = acc.popToolsAndMerge(DL2ToolCfg(flags, **nn_opts))
    elif NNFile_extension == "onnx":
        nn_name = NNFile.replace("/", "_").replace(".onnx", "")
        decorator = acc.popToolsAndMerge(GNNToolCfg(flags, **nn_opts))
    else:
        raise ValueError("FlavorTagNNCfg: Wrong NNFile extension. Please check the NNFile argument")

    name = '_'.join([nn_name.lower(), BTaggingCollection])

    # Ensure different names for standard and flip taggers
    if FlipConfig != "STANDARD":
        name = name + FlipConfig

    veto_list = getStaticTrackVars(TrackCollection)
    veto_list += getUndeclaredBtagVars(BTaggingCollection)

    decorAlg = CompFactory.FlavorTagDiscriminants.BTagDecoratorAlg(
        name=name,
        container=BTaggingCollection,
        constituentContainer=TrackCollection,
        decorator=decorator,
        undeclaredReadDecorKeys=veto_list,
    )

    # -- create the association algorithm
    acc.addEventAlgo(decorAlg)

    return acc


def MultifoldGNNCfg(
        flags,
        BTaggingCollection,
        TrackCollection,
        FlipConfig="STANDARD",
        nnFilePaths=None,
        remapping={},
        useBTaggingObject=True,
        JetCollection=None
):
    if nnFilePaths is None:
        raise ValueError('nnFilePaths must be specified')
    common = commonpath(nnFilePaths)
    nn_name = '_'.join(PurePath(common).with_suffix('').parts)
    algname = f'{nn_name}_{FlipConfig}'
    veto_list = getStaticTrackVars(TrackCollection)
    acc = ComponentAccumulator()

    if useBTaggingObject:
        Alg = CompFactory.FlavorTagDiscriminants.BTagDecoratorAlg
        trackLinkType = 'TRACK_PARTICLE'
        veto_list += getUndeclaredBtagVars(BTaggingCollection)
        container = BTaggingCollection
    else:
        if JetCollection is None:
            raise ValueError('JetCollection must be specified if useBTaggingObject is set to False')
        Alg = CompFactory.FlavorTagDiscriminants.JetTagDecoratorAlg
        trackLinkType = 'IPARTICLE'
        algname += '_Jet'
        container = JetCollection

    acc.addEventAlgo(
        Alg(
            name=algname,
            container=container,
            constituentContainer=TrackCollection,
            decorator=CompFactory.FlavorTagDiscriminants.MultifoldGNNTool(
                name=f'{algname}_tool',
                foldHashName='jetFoldHash',
                nnFiles=nnFilePaths,
                flipTagConfig=FlipConfig,
                variableRemapping=remapping,
                nnSharingService=addAndReturnSharingSvc(flags, acc),
                trackLinkType=trackLinkType,
            ),
            undeclaredReadDecorKeys=veto_list,
        )
    )

    return acc
