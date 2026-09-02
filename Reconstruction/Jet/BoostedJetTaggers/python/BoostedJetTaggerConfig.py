# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from JetToolHelpers.HelperConfig import HistoInputCfg

def qgTagAlgCfg(configFlags, 
                WP='50', 
                calib_path='/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/BoostedJetTaggers/QGConstituentTagger/May2025',
                config_file='QGTagger_AntiKt04PFlow_Transformer.dat',
                run='run2',
                addSFs=True,
                **kwargs):
    if WP not in ['10', '20', '30', '40', '50', '60', '70', '80', '90']:
        raise ValueError(f"Invalid WP {WP} provided. Valid WPs are: 10, 20, 30, 40, 50, 60, 70, 80, 90")

    acc = ComponentAccumulator()

    # add syst service
    sysService = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    acc.addService(sysService)

    kwargs.setdefault("tagger", 'qg')
    
    # jet container name
    jets_container = "AntiKt4EMPFlowJets"

    config_file = calib_path + '/' + config_file
    wp_suffix = WP + "WP"

    # input WP file
    wps_file = 'QGTagger_WPs_' + run + '.root'
    wps_histo = "QGTransformer_ConstScore__" + WP + "WP"

    # configure the histogram reader
    tool_args = {}

    histo2D = HistoInputCfg(configFlags, "histo2D",
                            inputFile = calib_path + '/' + wps_file,
                            histName = wps_histo,
                            varX = "pt", varY = "eta",
                            InterpType="None")

    tool_args.setdefault("HistoReader2D", histo2D)

    # configure the kinematic range
    tool_args.setdefault("jetPtMin", 20.)
    tool_args.setdefault("jetPtMax", 2000.)
    tool_args.setdefault("jetEtaMax", 4.5)
    
    # JSSTaggerBase write keys
    tool_args.setdefault("TaggedName", f"Tagged{wp_suffix}")
    tool_args.setdefault("ValidPtRangeLowName", f"ValidPtRangeLow{wp_suffix}")
    tool_args.setdefault("ValidEtaRangeName", f"ValidEtaRange{wp_suffix}")
    tool_args.setdefault("ValidJetContentName", f"ValidJetContent{wp_suffix}")
    tool_args.setdefault("ValidEventContentName", f"ValidEventContent{wp_suffix}")
    tool_args.setdefault("PassMassName", f"PassMass{wp_suffix}")
    
    # qgTagger-specific write keys
    tool_args.setdefault("ValidKinRangeName",f"ValidKinRange{wp_suffix}")
    tool_args.setdefault("PassScoreName",f"PassScore{wp_suffix}")
    tool_args.setdefault("CutScoreName",f"Cut_Score{wp_suffix}")

    # pick the BJT tool
    tool_args.setdefault("name", f"qgTagger_{wp_suffix}")
    tool_args.setdefault("ConfigFile", config_file)
    tool_args.setdefault("ContainerName", jets_container)
    tool = acc.popToolsAndMerge(BJTToolCfg(kwargs, **tool_args))

    # sf tool cfg
    if addSFs:
        # histo name
        sfs_histo = "QGTransformer_ConstScore__" + WP + "WP"

        # configure the histogram reader
        sfs_tool_args = {}

        histo2D = {}
        for parton in ['quark', 'gluon']:
            for eff in ['eff', 'ineff']:
                # input SF file
                sfs_file = 'SF_' + eff + '_' + parton + '_' + run + '.root'

                histo2D[parton + '_' + eff] = HistoInputCfg(configFlags, "histo2D",
                                                            inputFile = calib_path + '/' + sfs_file,
                                                            histName = sfs_histo,
                                                            varX = "pt", varY = "eta",
                                                            InterpType="None")
                sfs_tool_args.setdefault("HistoReader2D_" + parton + "_" + eff, histo2D[parton + '_' + eff])

        # pick the BJT tool
        sfs_tool_args.setdefault("name", f"qgTaggerSF_{wp_suffix}")
        sfs_tool_args.setdefault("JetContainer", jets_container)
        sfs_tool_args.setdefault("TaggedName", f"QGTransformer_Tagged{wp_suffix}")
        sfs_tool_args.setdefault("Efficiency", f"QGTransformer_Efficiency{wp_suffix}")
        sfs_tool_args.setdefault("Inefficiency", f"QGTransformer_Inefficiency{wp_suffix}")
        sfs_tool_args.setdefault("truthLabelName", "PartonTruthLabelID")
        sfs_tool_args.setdefault("jetPtMin", 20.)
        sfs_tool_args.setdefault("jetPtMax", 2000.)
        sfs_tool_args.setdefault("jetEtaMax", 4.5)
        sfs_tool = acc.popToolsAndMerge(BJTSFToolCfg(kwargs, **sfs_tool_args))

    # configure the BJT algo
    algo_args = {}
    algo_args.setdefault("tagger", tool)
    if addSFs:
        algo_args.setdefault("scalefactor", sfs_tool)
    algo_args.setdefault("jets", jets_container)
    acc.addEventAlgo(CompFactory.BJT.BoostedJetTaggerAlg(f"qgAlg_{wp_suffix}", **algo_args))

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

    if flags['tagger'] == 'WZ':
        acc.setPrivateTools(CompFactory.SmoothedWZTagger(**kwargs))
    elif flags['tagger'] == 'Top':
        acc.setPrivateTools(CompFactory.SmoothedTopTagger(**kwargs))
    elif flags['tagger'] == 'qg':
        acc.setPrivateTools(CompFactory.BJT.qgTagger(**kwargs))

    return acc

def BJTSFToolCfg(flags, **kwargs):

    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.BJT.ScaleFactors(**kwargs))

    return acc
