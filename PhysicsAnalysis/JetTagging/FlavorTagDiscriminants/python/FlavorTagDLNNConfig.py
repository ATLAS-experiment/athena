# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

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

def FlavorTagDLNNCfg(
        flags,
        BTaggingCollection,
        TrackCollection,
        NNFile,
        FlipConfig="STANDARD",
        variableRemapping={}):

    alg = CompFactory.FlavorTagInference.BTagDecoratorAlg

    acc = ComponentAccumulator()

    NNFile_extension = NNFile.split(".")[-1]
    nn_opts = dict(
        NNFile=NNFile,
        flipTagConfig=FlipConfig,
        variableRemapping=variableRemapping)
    if NNFile_extension == "json":
        nn_name = NNFile.replace("/", "_").replace("_network.json", "")
        decorator = acc.popToolsAndMerge(DL2ToolCfg(flags, **nn_opts))
    else:
        raise ValueError("FlavorTagDLNNCfg: Wrong NNFile extension. Please check the NNFile argument")

    name = '_'.join(['FtagNN', nn_name.lower(), BTaggingCollection])

    # Ensure different names for standard and flip taggers
    if FlipConfig != "STANDARD":
        name = name + FlipConfig

    veto_list = getUndeclaredBtagVars(BTaggingCollection)

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

