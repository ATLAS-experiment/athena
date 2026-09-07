# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from FlavorTagInference.FoldDecoratorConfig import FoldDecoratorCfg

from os.path import commonpath
from pathlib import PurePath
from warnings import warn
import re

_onnx_to_triton_map = {
    "BTagging/20250527/GN3V01/antikt4empflow/network.onnx"           : "BTagging_network_93a858f5c730",
    "BTagging/20231205/GN2v01/antikt4empflow/network_fold0.onnx"     : "BTagging_network_fold0_4812578c733e",
    "BTagging/20231205/GN2v01/antikt4empflow/network_fold1.onnx"     : "BTagging_network_fold1_9280d77c131c",
    "BTagging/20231205/GN2v01/antikt4empflow/network_fold2.onnx"     : "BTagging_network_fold2_25c6ad03db10",
    "BTagging/20231205/GN2v01/antikt4empflow/network_fold3.onnx"     : "BTagging_network_fold3_0558b4924c49",
    "BTagging/20250213/GN3V00/antikt4empflow/network.onnx"           : "BTagging_network_cce6be90efd1",
    "BTagging/20250213/GN3PflowMuonsV00/antikt4empflow/network.onnx" : "BTagging_network_d2138c4252e6",
    "BTagging/20240925/GN2Xv02/antikt10ufo/network.onnx"             : "BTagging_network_09c2dddf15bf",
    "BTagging/20250310/GN2XTauV00/antikt10ufo/network.onnx"          : "BTagging_network_e8d5e9a3059b",
    "BTagging/20250912/GN3XPV01/antikt10ufo/network.onnx"            : "BTagging_network_08105bb8c1d6",
    "BTagging/20260805/GN3EPCLV01/antikt4empflow/network.onnx"       : "BTagging_network_c87686aa79c5",
    # "BTagging/20230705/gn2xv01/antikt10ufo/network.onnx"           : "BTagging_network_9f8aadb82b76", # This model is commented out because at the time of submitting, it did not work on Triton. The code falls back to direct ONNX reading
    "JetCalibTools/CalibArea-00-04-83/CalibrationFactors/bbJESJMS_calibFactors_R22_MC20_CSSKUFO_bJR10v00Ext_20250212.onnx"  : "JetCalibTools_bbJESJMS_calibFactor_80138d800ac5",
    "JetCalibTools/CalibArea-00-04-83/CalibrationFactors/bbJESJMS_calibFactors_R22_MC20MC23_CSSKUFO_bJR10v01_20250212.onnx" : "JetCalibTools_bbJESJMS_calibFactor_fefb85f452f9",
}
# NNFiles should be a list of paths to NN files. master switch for using Triton for NN inference is 
# flags.BTagging.UseTriton. If all of the files are in _onnx_to_triton_map, then the returned 
# sharing service will be configured to use Triton. Otherwise it will fall back to ONNX.
def addAndReturnSharingSvc(flags, ca, NNFiles):
    if flags.BTagging.UseTriton and set(NNFiles).issubset(set(_onnx_to_triton_map.keys())):
        svc = CompFactory.FlavorTagInference.NNSharingTritonSvc(
            'FTagNNSharingTritonSvc',
            TritonPathsMap = _onnx_to_triton_map,
            TritonTimeout = 0.0,
            TritonPort = 443,
            TritonUrl = 'iaasdemo.ml4phys.com',
            TritonUseSSL = True,
        )
    else:
        svc = CompFactory.FlavorTagInference.NNSharingOnnxSvc('FTagNNSharingOnnxSvc')
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
        nnSharingService=addAndReturnSharingSvc(
            flags = flags, 
            ca = acc, 
            NNFiles=[NNFile],
        ),
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
        foldHashName='jetFoldRankHash',
        electrons='',
        muons='',
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
        nnSharingService=addAndReturnSharingSvc(
            flags = flags, 
            ca = acc, 
            NNFiles=nnFilePaths,
        ),
        defaultZeroTracks=default_zero_tracks,
    )

    # Don't bother scheduling the multifold config if there's only
    # one.  This is arguably uglier than using multifold for
    # everything, but groomed jets currently don't have a jetRankHash,
    # and also don't use multifold (for now). So doing it this way
    # lets us support large-R and small-R jets in the same function.
    if len(nnFilePaths) == 1:
        nn_filepath = nnFilePaths[0]
        Tool = CompFactory.FlavorTagInference.GNNTool
        path_defaults = _defaultsFromPaths(nnFilePaths)

        bonusargs = dict(
            name='unifold',
            nnFile=nn_filepath,
        )
        if nn_filepath in path_defaults:
            bonusargs['defaultOutputValues'] = path_defaults[nn_filepath]

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
            muonContainer=muons,
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

    GN2HLv01_fold_defaults = [
        {
            'GN2HLv01_pb':  0.514302135,
            'GN2HLv01_pc':  0.068148732,
            'GN2HLv01_pu':  0.012127459,
            'GN2HLv01_ptau':  0.40542167,
        }
    ]

    defaults = {}
    fold_re = re.compile('network_fold([0-9]+)')
    for path in nn_paths:
        if '/GN2v01/' in path:
            fold = int(fold_re.search(path).group(1))
            defaults[path] = gn2v01_fold_defaults[fold]
        if '/GN2HL/' in path:
            defaults[path] = GN2HLv01_fold_defaults[0]
    return defaults


def getDependencySet(tagger_name: str, override: set[str] | None = None) -> set[str]:
    """Return the dependency modifier set for a given tagger.

    The tagger naming convention encodes which additional physics inputs
    or decorations are required to run a particular flavour-tagging model.
    This function translates the tagger name into the corresponding set
    of dependency modifier characters.

    Each modifier indicates that certain reconstructed objects or
    decorations must be available in the event before the tagger can run.

    The currently defined modifier characters are:

    - ``X`` : Xbb-style tagger for large-R jets.
    - ``L`` : Track-Lepton decoration (generic lepton-related information).
    - ``E`` : Electron inputs associated to the jet.
    - ``M`` : Muon inputs associated to the jet.
    - ``MC``: Muon inputs associated to the jet based on cone-matching
    - ``P`` : Pflow inputs associated to the jet
    - ``R`` : Jet-calibration decorators for regression inputs

    ``P`` needs no algorithm of its own, the pflow inputs are already
    there, so it has no entry in ``_addDepsByTagger``.

    Parameters
    ----------
    tagger_name : str
        Name of the flavour-tagging model (e.g. ``"GN3EPCLV01"``).
    override : set[str] | None, optional
        Override the hardcoded list of tagger names and dependencies
        and simply return the set which is provided here. This is a
        dev option and should not be used in the main inference. 
        By default None

    Returns
    -------
    set[str]
        Set of dependency modifier characters describing the inputs
        required by the tagger.

    Raises
    ------
    KeyError
        If the provided ``tagger_name`` is not present in the internal
        tagger dependency registry.
    """

    # Check for override
    if override:
        return override

    # Define the dependencies of each tagger in a dict
    tagger_dep_dict: dict[str, set[str]] = {
        # Small-R jet taggers
        "GN2v01": {},
        "GN3V00": {},
        "GN3MuonsV00": {"L"},
        "GN3PflowV00": {"P"},
        "GN3PflowMuonsV00": {"L"},
        "GN3PflowMuonsChargeV00": {"L"},
        "GN3PflowMuonsElectronsHybridV00": {"L", "E"},
        "GN3V01": {"L", "E"},
        "GN3EPCLV01": {"E", "L"},
        "GN3V02": {"E", "M"},

        # Small-R jet regression
        "bJR4v01": {"E", "L", "MC", "R"},

        # Run 4 small-R jet taggers
        "GN2HL": {},

        # Large-R jet taggers
        "gn2xv00": {"X"},
        "gn2xv01": {"X"},
        "gn2xwithmassv00": {"X"},
        "GN2Xv02": {"X", "L"},
        "GN2Xv00": {"X"},
        "GN2XTauV00": {"X", "L"},
        "GN3XV00": {"X"},
        "GN3XPV01": {"X"},
    }

    if tagger_name not in tagger_dep_dict:
        available = ", ".join(sorted(tagger_dep_dict))
        raise KeyError(
            f"Unknown tagger '{tagger_name}'. Available taggers are: {available}\n\n"
            "Please check the name and add the tagger to the dict in getDependencySet "
            "if it is a newly deployed tagger!"
        )
    return tagger_dep_dict[tagger_name]


def PassThroughModelCfg(flags, JetCollection,
                        TrackCollection='InDetTrackParticles',
                        variableRemapping=None,
                        electrons='Electrons',
                        muons='',
                        jsonPath=None,
                        nameSuffix=''):
    """Configure a pass-through model for jet and constituent variables.

    jsonPath: PathResolver-resolvable path (relative to DATAPATH) or an
    absolute path to the PassThrough JSON. If falsy (None/""), returns
    an empty ComponentAccumulator.  Callers must pass the path
    explicitly; the derivation config owns which JSON to use (e.g.
    FTAG1LITE's small-R vs large-R).

    The JSON may specify scalar jet_variables and/or constituent
    variables (tracks, electrons, muons, flows). Constituent loading
    uses the existing GNN loader infrastructure (TracksLoader,
    ElectronsLoader, MuonsLoader, FlowElementsLoader).

    nameSuffix: appended to the svc/tool/alg names so more than one
    instance can be scheduled for the same jet collection.

    variableRemapping: dict mapping default link names to actual names,
        e.g. {"BTagTrackToJetAssociator": "GhostTrack",
               "FTagElectrons": "GhostFTagSelectedElectrons",
               "FTagMuons": "GhostFTagMuons"}
    """
    json_path = jsonPath
    if not json_path:
        return ComponentAccumulator()

    acc = ComponentAccumulator()
    FTI = CompFactory.FlavorTagInference

    remap = variableRemapping or {}

    # Unique svc/tool names per jet collection so multiple instances
    # can coexist (e.g. small-R + large-R running side-by-side).
    svc = FTI.PassThroughModelSvc(
        f'FTagPassThroughSvc_{JetCollection}{nameSuffix}',
        JsonFile=json_path,
        VariableRemapping=remap,
    )
    acc.addService(svc)

    tool = FTI.GNNTool(
        name=f'passthrough_decorator_{JetCollection}{nameSuffix}',
        nnFile='passthrough',
        nnSharingService=svc,
        variableRemapping=remap,
    )

    acc.addEventAlgo(
        FTI.JetTagDecoratorAlg(
            name=f'FtagPassThrough_{JetCollection}{nameSuffix}_Jet',
            container=JetCollection,
            constituentContainer=TrackCollection,
            electronContainer=electrons,
            muonContainer=muons,
            decorator=tool,
        )
    )

    return acc
