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
        JetCollection,
        TrackCollection,
        NNFile,
        BTaggingCollection=None,
        FlipConfig="STANDARD",
        variableRemapping={}):
    
    if BTaggingCollection is not None:
        warn('BTaggingCollection is deprecated, use JetCollection instead',
             stacklevel=2)
    if JetCollection is None:
        raise ValueError(
            'JetCollection is required,'
            f' {BTaggingCollection=}, {JetCollection=}'
        )
    
    tp_assoc = 'BTagTrackToJetAssociator'
    ip_assoc = 'TracksForBTagging'
    variableRemapping.setdefault(tp_assoc, ip_assoc)
    FTI = CompFactory.FlavorTagInference
    alg = FTI.JetTagDecoratorAlg

    acc = ComponentAccumulator()

    NNFile_extension = NNFile.split(".")[-1]
    nn_opts = dict(
        NNFile=NNFile,
        flipTagConfig=FlipConfig,
        variableRemapping=variableRemapping
    )

    if NNFile_extension == "onnx":
        nn_name = NNFile.replace("/", "_").replace(".onnx", "")
        nn_opts["defaultZeroTracks"] = True
        decorator = acc.popToolsAndMerge(GNNToolCfg(flags, **nn_opts))
    else:
        raise ValueError("FlavorTagNNCfg: Wrong NNFile extension. Please check the NNFile argument")

    name = '_'.join(['FtagNN', nn_name.lower(), JetCollection])

    # Ensure different names for standard and flip taggers
    if FlipConfig != "STANDARD":
        name = name + FlipConfig

    veto_list = getStaticTrackVars(TrackCollection)

    decorAlg = alg(
        name=name,
        container=JetCollection,
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
        electrons='',
        suffix='',
):
    common = commonpath(nnFilePaths)
    nn_name = '_'.join(PurePath(common).with_suffix('').parts)
    algname = 'FtagNN_{jc}_{tc}_{nn}_{fc}{dz}'.format(
        jc=JetCollection,
        tc=TrackCollection,
        nn=nn_name,
        fc=FlipConfig,
        dz=suffix,
    )

    default_zero_tracks = NONZERO_TRACKS in tag_requirements
    veto_list = getStaticTrackVars(TrackCollection)

    acc = ComponentAccumulator()


    FTI = CompFactory.FlavorTagInference
    if JetCollection is None:
        raise ValueError(
            'jet collection is required,'
            f' {BTaggingCollection=}, {JetCollection=}' )
    if BTaggingCollection is not None:
        warn('BTaggingCollection is deprecated,'
             ' use JetCollection instead', stacklevel=2)
    # we don't remove this outright because it will complicate
    # sweeping between branches.
    if useBTaggingObject is not None:
        warn(f'the option {useBTaggingObject=} is deprecated', stacklevel=2)

    tp_assoc = 'BTagTrackToJetAssociator'
    ip_assoc = 'TracksForBTagging'
    remapping.setdefault(tp_assoc, ip_assoc)
    algname += '_Jet'
    container = JetCollection

    toolargs = dict(
        flipTagConfig=FlipConfig,
        variableRemapping=remapping,
        nnSharingService=addAndReturnSharingSvc(flags, acc),
        defaultOutputValues=defaultOutputValues,
        defaultZeroTracks=default_zero_tracks,
    )

    # Don't bother scheduling the multifold config if there's only
    # one.  This is arguably uglier than using multifold for
    # everything, but groomed jets currently don't have a jetRankHash,
    # and also don't use multifold (for now). So doing it this way
    # lets us support large-R and small-R jets in the same function.
    if len(nnFilePaths) == 1:
        Tool = CompFactory.FlavorTagInference.GNNTool
        bonusargs = dict(
            name='unifold',
            nnFile=nnFilePaths[0]
        )

    else:
        Tool = CompFactory.FlavorTagInference.MultifoldGNNTool
        bonusargs = dict(
            name='multifold',
            nnFiles=nnFilePaths,
            foldHashName=foldHashName,
            perFoldDefaultOutputValues=_defaultsFromPaths(nnFilePaths),
        )
        acc.merge(
            FoldDecoratorCfg(
                flags,
                jetCollection=JetCollection
            )
        )

    acc.addEventAlgo(
        FTI.JetTagDecoratorAlg(
            name=algname,
            container=container,
            constituentContainer=TrackCollection,
            electronContainer=electrons,
            decorator=Tool(**toolargs, **bonusargs),
            undeclaredReadDecorKeys=veto_list,
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


def getModifierSet(tagger_name):
    """
    Translate tagger name into a list of dependencies
    """
    # Tagger should be of of the form GN<N><mods>V<M> where:
    # - N is the major version number
    # - mods specify the inputs we run on
    # - M is the minor version number
    tagparse = re.compile('(GN|gn)([0-9])(.*)([vV])([0-9]+)')
    if not (matches := tagparse.match(tagger_name)):
        raise ValueError(f"can't parse {tagger_name}")
    pfx, major, mods, verchar, minor = matches.groups()
    modset = set()

    # first handle the pre-GN3 taggers, things were not well specified
    # at this point
    if int(major) < 3:
        if "Muon" in mods:
            modset.add("M")
        if "Electrons" in mods:
            modset.add("L")
        # GN2X also used leponID
        if "X" in mods:
            if int(minor) == 2 or "Tau" in mods:
                modset.add("L")
        return modset

    # 2025-10-13: also one special case for GN3PflowMuonsV00, which
    # was defined before we had any convention here. The tagger and
    # this exception should ideally be removed soon
    if tagger_name == "GN3PflowMuonsV00":
        return {"L", "P"}

    # See the documentation in
    # https://ftag.docs.cern.ch/reco_algs/taggers/deploy/#naming-conventions
    # or
    # https://gitlab.cern.ch/atlas-flavor-tagging-tools/algorithms/ftag-docs/-/blob/64c70e9770d03a2545271150e163699905d1cba8/docs/reco_algs/taggers/deploy.md#modifiers

    if verchar != "V":
        raise ValueError(
            f"tagger {tagger_name} should use a uppercase V as the version")
    if pfx != "GN":
        raise ValueError(f"Tagger {tagger_name} should start with GN prefix")

    modsetparse = re.compile("[A-Z]")
    modset = set(modsetparse.findall(mods))

    allowed_mods = {
        "X", # Xbb tagger
        "L", # lepton decoration
        "E", # Electrons
        "P", # Particle Flow
        "C", # Charge Tagger (optional, no useful effects)
        "H", # Hybrid model (optional, no useful effects)
    }
    if baddies := modset - allowed_mods:
        raise ValueError(
            f"found forbidden modifiers {baddies} in {tagger_name}"
        )
    return modset
