# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG 
from JetToolHelpers.HelperConfig import HistoInputCfg

def qgTagAlgCfg(configFlags, **kwargs):

    acc = ComponentAccumulator()

    # add syst service
    sysService = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    acc.addService(sysService)

    # jet container name
    jets_container = "AntiKt4EMPFlowJets"

    # BJT config file
    config_file = kwargs['cfg_file']

    # input WP file
    wps_file = kwargs['wps_file']
    wps_histo = "QGTransformer_ConstScore__" + kwargs['WP'] + "WP"

    # configure the histogram reader
    tool_args = {}

    histo2D = HistoInputCfg(configFlags, "histo2D",
                            inputFile = wps_file,
                            histName = wps_histo,
                            varX = "pt", varY = "eta",
                            InterpType="None")

    tool_args.setdefault("HistoReader2D", histo2D)

    # pick the BJT tool
    tool_args.setdefault("ConfigFile", config_file)
    tool_args.setdefault("ContainerName", jets_container)
    tool = acc.popToolsAndMerge(BJTToolCfg(kwargs, **tool_args))

    # configure the BJT algo
    algo_args = {}
    algo_args.setdefault("tagger", tool)
    algo_args.setdefault("jets", jets_container)
    acc.addEventAlgo(CompFactory.BJT.BoostedJetTaggerAlg("qgAlg", **algo_args))

    return acc

def WZTagAlgCfg(configFlags, **kwargs):

    acc = ComponentAccumulator()

    # add syst service
    sysService = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    acc.addService(sysService)

    # jet container name
    jets_container = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    # BJT config file
    config_file = kwargs['cfg_file']
    config_file += "WTagger_AntiKt10UFOSoftDrop_" + kwargs['generation'] + "_FixSigEff" + kwargs['WP'] + ".dat"

    # pick the BJT tool
    tool_args = {}
    tool_args.setdefault("ConfigFile", config_file)
    tool_args.setdefault("ContainerName", jets_container)
    tool = acc.popToolsAndMerge(BJTToolCfg(kwargs, **tool_args))

    # configure the BJT algo
    algo_args = {}
    algo_args.setdefault("tagger", tool)
    algo_args.setdefault("jets", jets_container)
    acc.addEventAlgo(CompFactory.BJT.BoostedJetTaggerAlg("WTagAlg", **algo_args))

    return acc

def TopTagAlgCfg(configFlags, **kwargs):

    acc = ComponentAccumulator()

    # add syst service
    sysService = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    acc.addService(sysService)

    # jet container name
    jets_container = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    # BJT config file
    config_file = kwargs['cfg_file']
    config_file += "TopTagger_AntiKt10UFOSoftDrop_" + kwargs['generation'] + "_FixSigEff" + kwargs['WP'] + ".dat"

    # pick the BJT tool
    tool_args = {}
    tool_args.setdefault("ConfigFile", config_file)
    tool_args.setdefault("ContainerName", jets_container)
    tool = acc.popToolsAndMerge(BJTToolCfg(kwargs, **tool_args))

    # configure the BJT algo
    algo_args = {}
    algo_args.setdefault("tagger", tool)
    algo_args.setdefault("jets", jets_container)
    acc.addEventAlgo(CompFactory.BJT.BoostedJetTaggerAlg("TopTagAlg", **algo_args))

    return acc

def BJTToolCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("CalibArea", "Local")
    kwargs.setdefault("IsMC", 1)
    kwargs.setdefault("OutputLevel", DEBUG)

    if flags['tagger'] == 'WZ':
        acc.setPrivateTools(CompFactory.SmoothedWZTagger(**kwargs))
    elif flags['tagger'] == 'Top':
        acc.setPrivateTools(CompFactory.SmoothedTopTagger(**kwargs))
    elif flags['tagger'] == 'qg':
        acc.setPrivateTools(CompFactory.BJT.qgTagger(**kwargs))

    return acc
