# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from FlavorTagDiscriminants.FoldDecoratorConfig import FoldDecoratorCfg

from os.path import commonpath
from pathlib import PurePath
from warnings import warn


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

# name of a flag we use in a few places
NONZERO_TRACKS = 'nonzeroTracks'

def FlavorTagNNCfg(
        flags,
        BTaggingCollection,
        TrackCollection,
        NNFile,
        FlipConfig="STANDARD",
        variableRemapping={}):

    FTD = CompFactory.FlavorTagDiscriminants
    alg = FTD.BTagDecoratorAlg
    alg_args = {}

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
        acc.addEventAlgo(
            FTD.CountTrackParticleAlg(
                f'CountTrackParticleAlg{BTaggingCollection}',
                links=f'{BTaggingCollection}.BTagTrackToJetAssociator',
                minimumLinks=flags.BTagging.minTracksForAFT726Workaround,
                flag=f'{BTaggingCollection}.{NONZERO_TRACKS}',
            )
        )
        alg = FTD.BTagConditionalDecoratorAlg
        alg_args = dict(tagFlag=NONZERO_TRACKS)

    else:
        raise ValueError("FlavorTagNNCfg: Wrong NNFile extension. Please check the NNFile argument")

    name = '_'.join([nn_name.lower(), BTaggingCollection])

    # Ensure different names for standard and flip taggers
    if FlipConfig != "STANDARD":
        name = name + FlipConfig

    veto_list = getStaticTrackVars(TrackCollection)
    veto_list += getUndeclaredBtagVars(BTaggingCollection)

    decorAlg = alg(
        name=name,
        container=BTaggingCollection,
        constituentContainer=TrackCollection,
        decorator=decorator,
        undeclaredReadDecorKeys=veto_list,
        **alg_args,
    )

    # -- create the association algorithm
    acc.addEventAlgo(decorAlg)

    return acc


def MultifoldGNNCfg(
        flags,
        BTaggingCollection=None,
        TrackCollection=None,
        FlipConfig="STANDARD",
        nnFilePaths=None,
        remapping={},
        useBTaggingObject=None,
        JetCollection=None,
        conditions=set(),
        defaultOutputValues={},
):
    if nnFilePaths is None:
        raise ValueError('nnFilePaths must be specified')
    common = commonpath(nnFilePaths)
    nn_name = '_'.join(PurePath(common).with_suffix('').parts)
    algname = f'{nn_name}_{FlipConfig}'

    if TrackCollection is None:
        raise ValueError('TrackCollection must be specified')

    veto_list = getStaticTrackVars(TrackCollection)

    acc = ComponentAccumulator()

    if JetCollection is not None:
        acc.merge(
            FoldDecoratorCfg(
                flags,
                jetCollection=JetCollection
            )
        )

    tp_assoc = 'BTagTrackToJetAssociator'
    ip_assoc = 'TracksForBTagging'
    tag_flag = NONZERO_TRACKS
    min_links = flags.BTagging.minTracksForAFT726Workaround

    FTD = CompFactory.FlavorTagDiscriminants

    if BTaggingCollection is not None:
        if conditions:
            Alg = FTD.BTagConditionalDecoratorAlg
            if len(conditions) > 1:
                raise ValueError(f'{conditions=} must have size 0 or 1')
            alg_args = dict(tagFlag=next(iter(conditions)))
        else:
            Alg = FTD.BTagDecoratorAlg
            alg_args = {}
        if tag_flag in conditions:
            remapped_tp = remapping.get(tp_assoc, tp_assoc)
            acc.addEventAlgo(
                FTD.CountTrackParticleAlg(
                    f'CountTrackParticleAlg{BTaggingCollection}',
                    links=f'{BTaggingCollection}.{remapped_tp}',
                    minimumLinks=min_links,
                    flag=f'{BTaggingCollection}.{tag_flag}',
                )
            )
        trackLinkType = 'TRACK_PARTICLE'
        veto_list += getUndeclaredBtagVars(BTaggingCollection)
        container = BTaggingCollection
    elif JetCollection is not None:
        remapping.setdefault(tp_assoc, ip_assoc)
        if conditions:
            Alg = FTD.JetTagConditionalDecoratorAlg
            alg_args = dict(tagFlags=list(conditions))
        else:
            Alg = FTD.JetTagDecoratorAlg
            alg_args = {}
        if tag_flag in conditions:
            acc.addEventAlgo(
                FTD.CountIParticleAlg(
                    f'CountTrackParticleAlg{JetCollection}',
                    links=f'{JetCollection}.{remapping[tp_assoc]}',
                    minimumLinks=min_links,
                    flag=f'{JetCollection}.{tag_flag}',
                )
            )
        trackLinkType = 'IPARTICLE'
        algname += '_Jet'
        container = JetCollection
    else:
        raise ValueError(
            'b-tagging or jet collection is required,'
            f' {BTaggingCollection=}, {JetCollection=}' )

    # we don't remove this outright because it will complicate
    # sweeping between branches.
    if useBTaggingObject is not None:
        warn(f'the option {useBTaggingObject=} is deprecated', stacklevel=2)


    acc.addEventAlgo(
        Alg(
            name=algname,
            container=container,
            constituentContainer=TrackCollection,
            decorator=FTD.MultifoldGNNTool(
                name=f'{algname}_tool',
                foldHashName='jetFoldHash',
                nnFiles=nnFilePaths,
                flipTagConfig=FlipConfig,
                variableRemapping=remapping,
                nnSharingService=addAndReturnSharingSvc(flags, acc),
                trackLinkType=trackLinkType,
                defaultOutputValues=defaultOutputValues,
            ),
            undeclaredReadDecorKeys=veto_list,
            **alg_args
        )
    )

    return acc
