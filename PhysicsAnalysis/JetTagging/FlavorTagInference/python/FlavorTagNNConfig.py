# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from FlavorTagInference.FoldDecoratorConfig import FoldDecoratorCfg

from os.path import commonpath
from pathlib import PurePath
from warnings import warn
import re

def addAndReturnSharingSvc(flags, ca):
    svc = CompFactory.FlavorTagInference.NNSharingSvc('FTagNNSharingSvc')
    ca.addService(svc)
    return svc

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

    # Due to AFT-726 some trigger configurations need custom default
    # values for the zero track cases. We're keeping this
    # conservative: zero track tags are considered light jets.
    #
    # Trigger GN1
    if '20220813trig' in NNFile:
        defout = {
            'GN120220813_pu': 1.0,
            'GN120220813_pb': 0.0,
            'GN120220813_pc': 0.0,
        }
    # trigger GN2
    elif '20240122trig' in NNFile:
        defout = {
            'pu': 0.6712538,
            'pb': 0.10002074,
            'pc': 0.22872543,
        }
    else:
        defout = {}
    defkey = 'defaultOutputValues'
    options[defkey] = defout | options.get(defkey, {})

    gnntool = CompFactory.FlavorTagInference.GNNTool(
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


# name of a flag we use in a few places
NONZERO_TRACKS = 'nonzeroTracks'

def FlavorTagNNCfg(
        flags,
        BTaggingCollection,
        TrackCollection,
        NNFile,
        FlipConfig="STANDARD",
        variableRemapping={}):

    FTI = CompFactory.FlavorTagInference
    alg = FTI.BTagDecoratorAlg

    acc = ComponentAccumulator()

    NNFile_extension = NNFile.split(".")[-1]
    nn_opts = dict(
        NNFile=NNFile,
        flipTagConfig=FlipConfig,
        variableRemapping=variableRemapping)
    if NNFile_extension == "onnx":
        nn_name = NNFile.replace("/", "_").replace(".onnx", "")
        nn_opts["defaultZeroTracks"] = True
        decorator = acc.popToolsAndMerge(GNNToolCfg(flags, **nn_opts))
    else:
        raise ValueError("FlavorTagNNCfg: Wrong NNFile extension. Please check the NNFile argument")

    name = '_'.join(['FtagNN', nn_name.lower(), BTaggingCollection])

    # Ensure different names for standard and flip taggers
    if FlipConfig != "STANDARD":
        name = name + FlipConfig

    veto_list = getStaticTrackVars(TrackCollection)

    decorAlg = alg(
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
        JetCollection,
        TrackCollection,
        nnFilePaths,
        BTaggingCollection=None,
        FlipConfig="STANDARD",
        remapping={},
        useBTaggingObject=None,
        tag_requirements=set(),
        defaultOutputValues={},
        foldHashName='jetFoldRankHash',
        dz_suffix='',
):
    common = commonpath(nnFilePaths)
    nn_name = '_'.join(PurePath(common).with_suffix('').parts)
    algname = 'FtagMultifoldNN_{jc}_{tc}_{nn}_{fc}{dz}'.format(
        jc=JetCollection,
        tc=TrackCollection,
        nn=nn_name,
        fc=FlipConfig,
        dz=dz_suffix,
    )

    default_zero_tracks = NONZERO_TRACKS in tag_requirements
    veto_list = getStaticTrackVars(TrackCollection)

    acc = ComponentAccumulator()

    acc.merge(
        FoldDecoratorCfg(
            flags,
            jetCollection=JetCollection
        )
    )

    tp_assoc = 'BTagTrackToJetAssociator'
    ip_assoc = 'TracksForBTagging'

    FTI = CompFactory.FlavorTagInference

    if BTaggingCollection is not None:
        Alg = FTI.BTagDecoratorAlg
        trackLinkType = 'TRACK_PARTICLE'
        container = BTaggingCollection
    elif JetCollection is not None:
        remapping.setdefault(tp_assoc, ip_assoc)
        Alg = FTI.JetTagDecoratorAlg
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
            decorator=CompFactory.FlavorTagInference.MultifoldGNNTool(
                name=f'{algname}_tool',
                foldHashName=foldHashName,
                nnFiles=nnFilePaths,
                flipTagConfig=FlipConfig,
                variableRemapping=remapping,
                nnSharingService=addAndReturnSharingSvc(flags, acc),
                trackLinkType=trackLinkType,
                defaultOutputValues=defaultOutputValues,
                perFoldDefaultOutputValues=_defaultsFromPaths(nnFilePaths),
                defaultZeroTracks=default_zero_tracks,
            ),
            undeclaredReadDecorKeys=veto_list,
            ExtraInputs=[("xAOD::JetContainer", f"StoreGateSvc+{JetCollection}.jetFoldHash")],
        )
    )

    return acc


def _defaultsFromPaths(nn_paths):
    # these are the values GN2v01 has with zero tracks, see discussion
    # on AFT-726
    gn2v01_fold_defaults = [
        {
            'GN2v01_pb':  0.008461162,
            'GN2v01_pc':  0.013391991,
            'GN2v01_pu':  0.266699642,
            'GN2v01_ptau': 0.711447179,
        },
        {
            'GN2v01_pb':  0.008416064,
            'GN2v01_pc':  0.012780965,
            'GN2v01_pu':  0.266321003,
            'GN2v01_ptau': 0.712482035,
        },
        {
            'GN2v01_pb':  0.008398464,
            'GN2v01_pc':  0.013321628,
            'GN2v01_pu':  0.265400767,
            'GN2v01_ptau': 0.712879181,
        },
        {
            'GN2v01_pb':  0.008461761,
            'GN2v01_pc':  0.012895554,
            'GN2v01_pu':  0.265607148,
            'GN2v01_ptau': 0.713035464,
        }
    ]
    defaults = {}
    fold_re = re.compile('network_fold([0-9]+)')
    for path in nn_paths:
        if '/GN2v01/' in path:
            fold = int(fold_re.search(path).group(1))
            defaults[path] = gn2v01_fold_defaults[fold]
    return defaults

