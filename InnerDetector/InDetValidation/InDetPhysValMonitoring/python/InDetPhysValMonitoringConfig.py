#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetPhysValMonitoringConfig.py
@author T. Strebler
@date 2021-08-30
@brief Main CA-based python configuration for InDetPhysValMonitoring
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def extractCollectionPrefix(track_collection_name):
    suffixes = ['Tracks', 'TrackParticles']
    for suffix in suffixes:
        if track_collection_name.endswith(suffix):
            return track_collection_name.removesuffix(suffix)
    return track_collection_name


def HistogramDefinitionSvcCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    if flags.Detector.GeometryID:
        if flags.PhysVal.IDPVM.doValidateLargeD0Tracks or flags.PhysVal.IDPVM.doValidateMergedLargeD0Tracks:
            kwargs.setdefault("DefinitionSource", "InDetPVMPlotDefLRT.xml")
        else:
            kwargs.setdefault("DefinitionSource", "InDetPVMPlotDefRun2.xml")
    elif flags.Detector.GeometryITk:
        kwargs.setdefault("DefinitionSource", "InDetPVMPlotDefITK.xml")
    kwargs.setdefault("DefinitionFormat", "text/xml")
    histoSvc = CompFactory.HistogramDefinitionSvc(**kwargs)
    acc.addService(histoSvc)
    return acc


def InDetRttTruthSelectionToolCfg(
        flags, name="InDetRttTruthSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("requireStatus1", True)
    kwargs.setdefault("requireCharged", True)
    kwargs.setdefault("selectedCharge", flags.PhysVal.IDPVM.selectedCharge)
    kwargs.setdefault("requireOnlyPrimary",
                      True if kwargs.pop("OnlyDressPrimaryTracks", True)
                      else False)
    kwargs.setdefault("maxProdVertRadius",
                      flags.PhysVal.IDPVM.maxProdVertRadius)

    if flags.Detector.GeometryID:
        if flags.PhysVal.IDPVM.doValidateLargeD0Tracks or flags.PhysVal.IDPVM.doValidateMergedLargeD0Tracks:
            kwargs.setdefault("maxEta", 3.0)
        else:
            kwargs.setdefault("maxEta", 2.5)
    elif flags.Detector.GeometryITk:
        kwargs.setdefault("maxEta", 4.0)

    kwargs.setdefault("minPt", flags.PhysVal.IDPVM.truthMinPt)
    kwargs.setdefault("ancestorList", flags.PhysVal.IDPVM.ancestorIDs)
    kwargs.setdefault("requireSiHit", flags.PhysVal.IDPVM.requiredSiHits)

    if "radiusCylinder" in kwargs or "zDisc" in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(extrapolator)  # TODO: migrate to private?
        kwargs.setdefault("Extrapolator", extrapolator)
    else:
        kwargs.setdefault("Extrapolator", None)

    acc.setPrivateTools(CompFactory.AthTruthSelectionTool(name, **kwargs))
    return acc


def GoodRunsListSelectionToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    cvmfs = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/'
    grldict = {
        '2015': cvmfs + 'data15_13TeV/20170619/data15_13TeV.periodAllYear_DetStatus-v89-pro21-02_Unknown_PHYS_StandardGRL_All_Good_25ns.xml',
        '2016': cvmfs + 'data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml',
        '2017': cvmfs + 'data17_13TeV/20180619/data17_13TeV.periodAllYear_DetStatus-v99-pro22-01_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml',
        '2018': cvmfs + 'data18_13TeV/20190318/data18_13TeV.periodAllYear_DetStatus-v102-pro22-04_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml',
        '2022': cvmfs + 'data22_13p6TeV/20220902/data22_13p6TeV.periodF_DetStatus-v108-pro28_MERGED_PHYS_StandardGRL_All_Good_25ns.xml'
    }

    acc.setPrivateTools(CompFactory.GoodRunsListSelectionTool(
        name="GoodRunsListSelectionTool",
        GoodRunsListVec=[grldict[p] for p in flags.PhysVal.IDPVM.GRL],
        **kwargs
    ))
    return acc


def InDetVertexTruthMatchToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.InDetVertexTruthMatchTool(**kwargs))
    return acc


def InDetPhysValMonitoringToolCfg(flags, **kwargs):
    from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
        AddDecoratorIfNeededCfg)
    acc = AddDecoratorIfNeededCfg(flags)
    kwargs.setdefault("useTrackSelection", False)
    kwargs.setdefault("EnableLumi", False)

    acc.merge(HistogramDefinitionSvcCfg(flags))

    # if we are running with sumpT(w) hard scatter selection, we need to schedule jet finding
    if flags.PhysVal.IDPVM.hardScatterStrategy in [2, 3]:

        from InDetPhysValMonitoring.addRecoJetsConfig import (
            AddRecoJetsIfNotExistingCfg)
        acc.merge(AddRecoJetsIfNotExistingCfg(
            flags, flags.PhysVal.IDPVM.jetsNameForHardScatter))

    # if we are running with the GNN hard scatter selection, we need to schedule the dependencies
    if flags.PhysVal.IDPVM.hardScatterStrategy == 3:
        from InDetConfig.InDetGNNHardScatterSelectionConfig import (
            GNNSequenceCfg)
        acc.merge(GNNSequenceCfg(flags))

    # if we are running with the HGam hard scatter selection, we need to schedule the NN
    if flags.PhysVal.IDPVM.hardScatterStrategy == 4:
        from DerivationFrameworkHiggs.HIGG1D1CustomVertexConfig import DiPhotonVertexCfg
        acc.merge(DiPhotonVertexCfg(flags))

    if flags.PhysVal.IDPVM.GRL:
        kwargs.setdefault("useGRL", True)
        kwargs.setdefault('GoodRunsListSelectionTool', acc.popToolsAndMerge(
            GoodRunsListSelectionToolCfg(flags)))

    kwargs.setdefault("doIDTIDEPlots", flags.PhysVal.IDPVM.doIDTIDE)
    kwargs.setdefault("setCSVName", flags.PhysVal.IDPVM.setCSVName)

    if flags.PhysVal.IDPVM.doValidateTracksInJets:
        kwargs.setdefault("JetContainerName", flags.PhysVal.IDPVM.jetCollection)
        kwargs.setdefault("FillTrackInJetPlots", True)
        kwargs.setdefault('JetAbsEtaMin', 0)
        kwargs.setdefault('JetAbsEtaMax', flags.PhysVal.IDPVM.JetAbsEtaMax)
        kwargs.setdefault('JetPtMin', flags.PhysVal.IDPVM.JetPtMin)
        kwargs.setdefault('JetPtMax', flags.PhysVal.IDPVM.JetPtMax)
        kwargs.setdefault('maxTrkJetDR', flags.PhysVal.IDPVM.maxTrkJetDR)
        kwargs.setdefault("FillTrackInBJetPlots",
                          flags.PhysVal.IDPVM.doValidateTracksInBJets)
    else:
        kwargs.setdefault("JetContainerName", '')
        kwargs.setdefault("FillTrackInJetPlots", False)

    kwargs.setdefault("doPerAuthorPlots",
                      flags.PhysVal.IDPVM.doPerAuthorPlots)

    if flags.Input.isMC and not flags.PhysVal.IDPVM.doRecoOnly:
        kwargs.setdefault("TruthParticleContainerName", "TruthParticles")
        if 'TruthSelectionTool' not in kwargs:
            kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
                InDetRttTruthSelectionToolCfg(flags)))

        doHyyHSSelection = flags.PhysVal.IDPVM.hardScatterStrategy == 4
        if 'hardScatterSelectionTool' not in kwargs:
            from InDetConfig.InDetHardScatterSelectionToolConfig import (
                InDetHardScatterSelectionToolCfg)
            kwargs.setdefault("hardScatterSelectionTool", acc.popToolsAndMerge(
                InDetHardScatterSelectionToolCfg(
                    flags,
                    RedoHardScatter=not doHyyHSSelection,
                    SelectionMode=flags.PhysVal.IDPVM.hardScatterStrategy,
                    # make sure the HS selection tool picks up the correct jets
                    JetContainer=flags.PhysVal.IDPVM.jetsNameForHardScatter,
                    VertexContainer="PrimaryVertices" if not doHyyHSSelection else "HggPrimaryVertices"
                )))

        if doHyyHSSelection:
            kwargs.setdefault("VertexContainerName", "HggPrimaryVertices")

        kwargs.setdefault("FillTruthToRecoNtuple",
                          flags.PhysVal.IDPVM.doValidateTruthToRecoNtuple)
        kwargs.setdefault("doTruthOriginPlots",
                          flags.PhysVal.IDPVM.doTruthOriginPlots)
        kwargs.setdefault("doHitLevelPlots",
                          flags.PhysVal.IDPVM.doHitLevelPlots)

        # adding the VertexTruthMatchingTool
        if not flags.PhysVal.IDPVM.doIDTIDE:
            kwargs.setdefault("useVertexTruthMatchTool", True)
            kwargs.setdefault("VertexTruthMatchTool", acc.popToolsAndMerge(
                InDetVertexTruthMatchToolCfg(flags)))

        if "trackTruthOriginTool" not in kwargs:
            from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import InDetTrackTruthOriginToolCfg
            kwargs.setdefault("trackTruthOriginTool", acc.popToolsAndMerge(
                InDetTrackTruthOriginToolCfg(flags)))

        # Options for Truth Strategy : Requires full pile-up truth containers for some
        if flags.PhysVal.IDPVM.setTruthStrategy in ['All', 'PileUp']:
            if not("xAOD::TruthPileupEventContainer#TruthPileupEvents"
                   in flags.Input.TypedCollections):
                print('WARNING Truth Strategy for InDetPhysValMonitoring set to %s but TruthPileupEvents are missing in the input' % (flags.PhysVal.IDPVM.setTruthStrategy))
            kwargs.setdefault("PileupSwitch",
                              flags.PhysVal.IDPVM.setTruthStrategy)

        elif flags.PhysVal.IDPVM.setTruthStrategy != 'HardScatter':
            print('WARNING Truth Strategy for for InDetPhysValMonitoring set to invalid option %s; valid flags are ["HardScatter", "All", "PileUp"]' % (
                flags.PhysVal.IDPVM.setTruthStrategy))

    else:
        # disable truth monitoring for data
        kwargs.setdefault("TruthParticleContainerName", '')
        kwargs.setdefault("TruthVertexContainerName", '')
        kwargs.setdefault("TruthEvents", '')
        kwargs.setdefault("TruthPileupEvents", '')
        kwargs.setdefault("TruthSelectionTool", None)
        kwargs.setdefault("FillTruthToRecoNtuple", False)

    if flags.Detector.GeometryITk:
        # Disable vertex container for now
        kwargs.setdefault("doTRTExtensionPlots", False)

    if flags.PhysVal.IDPVM.doTechnicalEfficiency:
        kwargs.setdefault("fillTechnicalEfficiency", True)
        kwargs.setdefault("EtaBins", flags.Tracking.ITkMainPass.etaBins if flags.Detector.GeometryITk
                                     else [-1, 9999.]) # Configurable eta bins for determining a "reconstructable" particle
        kwargs.setdefault("MinNumberClusters", flags.Tracking.ITkMainPass.minClusters if flags.Detector.GeometryITk
                                               else [flags.Tracking.MainPass.minClusters]) # Configurable hits per eta bins for determining a "reconstructable" particle

    # Control the number of output histograms
    kwargs.setdefault("DetailLevel", 200 if flags.PhysVal.IDPVM.doExpertOutput else 100)

    kwargs.setdefault("doPRW", flags.PhysVal.IDPVM.doPRW)

    acc.setPrivateTools(CompFactory.InDetPhysValMonitoringTool(**kwargs))
    return acc


def InDetPhysValMonitoringToolLooseCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    if 'TrackSelectionTool' not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionTool_Loose_Cfg)
        kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionTool_Loose_Cfg(flags)))

    kwargs.setdefault("SubFolder", 'Loose/')
    kwargs.setdefault("useTrackSelection", True)

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name="InDetPhysValMonitoringToolLoose", **kwargs)))
    return acc


def InDetPhysValMonitoringToolTightPrimaryCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    if 'TrackSelectionTool' not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionTool_TightPrimary_Cfg)
        kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionTool_TightPrimary_Cfg(flags)))

    kwargs.setdefault("SubFolder", 'TightPrimary/')
    kwargs.setdefault("useTrackSelection", True)

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name="InDetPhysValMonitoringToolTightPrimary", **kwargs)))
    return acc


def InDetPhysValMonitoringToolHILooseCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    if 'TrackSelectionTool' not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionTool_HILoose_Cfg)
        kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionTool_HILoose_Cfg(flags)))

    kwargs.setdefault("SubFolder", 'HILoose/')
    kwargs.setdefault("useTrackSelection", True)

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name="InDetPhysValMonitoringToolHILoose", **kwargs)))
    return acc


def InDetPhysValMonitoringToolGSFCfg(flags, **kwargs):
    kwargs.setdefault("SubFolder", 'GSF/')
    kwargs.setdefault("TrackParticleContainerName", 'GSFTrackParticles')
    kwargs.setdefault("useTrackSelection", True)
    return InDetPhysValMonitoringToolCfg(
        flags, name="InDetPhysValMonitoringToolGSF", **kwargs)


def InDetPhysValMonitoringToolElectronsCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
        InDetRttTruthSelectionToolCfg(
            flags, name="AthTruthSelectionToolForIDPVM_Electrons",
            pdgId=11,
            minPt=5000.
        )))
    kwargs.setdefault("onlyFillTruthMatched", True)
    kwargs.setdefault("SubFolder", 'Electrons/')

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name='InDetPhysValMonitoringToolElectrons', **kwargs)))
    return acc


def InDetPhysValMonitoringToolMuonsCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
        InDetRttTruthSelectionToolCfg(
            flags, name="AthTruthSelectionToolForIDPVM_Muons",
            pdgId=13,
            minPt=5000.
        )))
    kwargs.setdefault("onlyFillTruthMatched", True)
    kwargs.setdefault("SubFolder", 'Muons/')

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name='InDetPhysValMonitoringToolMuons', **kwargs)))
    return acc


def InDetLargeD0PhysValMonitoringToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("SubFolder", 'LRT/')
    kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
        InDetRttTruthSelectionToolCfg(flags)))
    kwargs.setdefault("TrackParticleContainerName",
                      'InDetLargeD0TrackParticles'
                      if flags.Tracking.storeSeparateLargeD0Container else
                      'InDetTrackParticles')
    kwargs.setdefault("useTrackSelection", True)

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name='InDetPhysValMonitoringToolLargeD0', **kwargs)))
    return acc


def InDetMergedLargeD0PhysValMonitoringToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("SubFolder", 'LRTMerged/')
    kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
        InDetRttTruthSelectionToolCfg(flags)))
    kwargs.setdefault("TrackParticleContainerName",
                      'InDetWithLRTTrackParticles'
                      if flags.Tracking.storeSeparateLargeD0Container else
                      'InDetTrackParticles')
    kwargs.setdefault("useTrackSelection", True)

    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name='InDetPhysValMonitoringToolMergedLargeD0', **kwargs)))
    return acc

def InDetLowPtRoIPhysValMonitoringToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SubFolder", 'LowPtRoI/') #Subfolder
    kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
        InDetRttTruthSelectionToolCfg(flags,minPt=100)))
    kwargs.setdefault("TrackParticleContainerName",
                     'InDetLowPtRoITrackParticles')
    acc.setPrivateTools(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
        flags, name='InDetPhysValMonitoringToolLowPtRoI', **kwargs)))

    return acc


def InDetPhysValMonitoringCfg(flags):
    acc = ComponentAccumulator()

    if flags.PhysVal.IDPVM.doValidateMergedLargeD0Tracks:
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg(flags))

    mons = [(True,
             InDetPhysValMonitoringToolCfg),
            (flags.PhysVal.IDPVM.doValidateMuonMatchedTracks,
             InDetPhysValMonitoringToolMuonsCfg),
            (flags.PhysVal.IDPVM.doValidateElectronMatchedTracks,
             InDetPhysValMonitoringToolElectronsCfg),
            (flags.PhysVal.IDPVM.doValidateLargeD0Tracks,
             InDetLargeD0PhysValMonitoringToolCfg),
            (flags.PhysVal.IDPVM.doValidateMergedLargeD0Tracks,
             InDetMergedLargeD0PhysValMonitoringToolCfg),
            (flags.PhysVal.IDPVM.doValidateLowPtRoITracks,
             InDetLowPtRoIPhysValMonitoringToolCfg),
            (flags.PhysVal.IDPVM.doValidateLooseTracks,
             InDetPhysValMonitoringToolLooseCfg),
            (flags.PhysVal.IDPVM.doValidateTightPrimaryTracks,
             InDetPhysValMonitoringToolTightPrimaryCfg),
            (flags.PhysVal.IDPVM.doValidateHILoose,
             InDetPhysValMonitoringToolHILooseCfg),
            (flags.PhysVal.IDPVM.doValidateGSFTracks,
             InDetPhysValMonitoringToolGSFCfg)
            ]

    tools = []
    for enabled, creator in mons:
        if enabled:
            tools.append(acc.popToolsAndMerge(creator(flags)))

    for col in flags.PhysVal.IDPVM.validateExtraTrackCollections:
        prefix = extractCollectionPrefix(col)
        tools.append(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(
            flags, name='InDetPhysValMonitoringTool'+prefix,
            SubFolder=prefix+'Tracks/',
            TrackParticleContainerName=prefix+'TrackParticles'
        )))

    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    acc.merge(PhysValMonitoringCfg(flags, tools=tools))
    return acc
