# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, MetadataCategory
from AthenaCommon.SystemOfUnits import GeV


def TCAL1TrackToolsCfg(flags, **kwargs):
    """ Configure the TrackTools tool """

    acc = ComponentAccumulator()

    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    kwargs.setdefault('ParticleCaloExtensionTool', acc.popToolsAndMerge(
        ParticleCaloExtensionToolCfg(flags)))

    kwargs.setdefault('IsCollision', flags.Beam.Type is BeamType.Collisions)

    TrackTools = CompFactory.TileCal.TrackTools
    acc.setPrivateTools(TrackTools(**kwargs))

    return acc


def TCAL1TileCellsDecoratorCfg(flags, Prefix="TCAL1_", **kwargs):
    """ Configure the Tile Cells decorator tool """

    acc = ComponentAccumulator()
    kwargs.setdefault("CellsEnergy", Prefix + "cells_energy")
    kwargs.setdefault("CellsEt", Prefix + "cells_et")
    kwargs.setdefault("CellsEta", Prefix + "cells_eta")
    kwargs.setdefault("CellsPhi", Prefix + "cells_phi")
    kwargs.setdefault("CellsGain", Prefix + "cells_gain")
    kwargs.setdefault("CellsBad", Prefix + "cells_bad")
    kwargs.setdefault("CellsSampling", Prefix + "cells_sampling")
    kwargs.setdefault("CellsTime", Prefix + "cells_time")
    kwargs.setdefault("CellsQuality", Prefix + "cells_quality")
    kwargs.setdefault("CellsSinTh", Prefix + "cells_sinTh")
    kwargs.setdefault("CellsCosTh", Prefix + "cells_cosTh")
    kwargs.setdefault("CellsCotTh", Prefix + "cells_cotTh")
    kwargs.setdefault("CellsX", Prefix + "cells_x")
    kwargs.setdefault("CellsY", Prefix + "cells_y")
    kwargs.setdefault("CellsZ", Prefix + "cells_z")
    kwargs.setdefault("CellsR", Prefix + "cells_r")
    kwargs.setdefault("CellsDx", Prefix + "cells_dx")
    kwargs.setdefault("CellsDy", Prefix + "cells_dy")
    kwargs.setdefault("CellsDz", Prefix + "cells_dz")
    kwargs.setdefault("CellsDr", Prefix + "cells_dr")
    kwargs.setdefault("CellsVolume", Prefix + "cells_volume")
    kwargs.setdefault("CellsDeta", Prefix + "cells_deta")
    kwargs.setdefault("CellsDphi", Prefix + "cells_dphi")
    kwargs.setdefault("CellsSide", Prefix + "cells_side")
    kwargs.setdefault("CellsSection", Prefix + "cells_section")
    kwargs.setdefault("CellsModule", Prefix + "cells_module")
    kwargs.setdefault("CellsTower", Prefix + "cells_tower")
    kwargs.setdefault("CellsSample", Prefix + "cells_sample")
    kwargs.setdefault("CellsPmt1Ros", Prefix + "cells_pmt1_ros")
    kwargs.setdefault("CellsPmt2Ros", Prefix + "cells_pmt2_ros")
    kwargs.setdefault("CellsPmt1Drawer", Prefix + "cells_pmt1_drawer")
    kwargs.setdefault("CellsPmt2Drawer", Prefix + "cells_pmt2_drawer")
    kwargs.setdefault("CellsPmt1Channel", Prefix + "cells_pmt1_channel")
    kwargs.setdefault("CellsPmt2Channel", Prefix + "cells_pmt2_channel")
    kwargs.setdefault("CellsPmt1Energy", Prefix + "cells_pmt1_energy")
    kwargs.setdefault("CellsPmt2Energy", Prefix + "cells_pmt2_energy")
    kwargs.setdefault("CellsPmt1Time", Prefix + "cells_pmt1_time")
    kwargs.setdefault("CellsPmt2Time", Prefix + "cells_pmt2_time")
    kwargs.setdefault("CellsPmt1Quality", Prefix + "cells_pmt1_quality")
    kwargs.setdefault("CellsPmt2Quality", Prefix + "cells_pmt2_quality")
    kwargs.setdefault("CellsPmt1Qbit", Prefix + "cells_pmt1_qbit")
    kwargs.setdefault("CellsPmt2Qbit", Prefix + "cells_pmt2_qbit")
    kwargs.setdefault("CellsPmt1Bad", Prefix + "cells_pmt1_bad")
    kwargs.setdefault("CellsPmt2Bad", Prefix + "cells_pmt2_bad")
    kwargs.setdefault("CellsPmt1Gain", Prefix + "cells_pmt1_gain")
    kwargs.setdefault("CellsPmt2Gain", Prefix + "cells_pmt2_gain")
    acc.addPrivateTools(CompFactory.DerivationFramework.TileCellsDecorator(name="TileCellsDecorator", **kwargs))
    return acc


def TCAL1TileCellsMuonDecoratorCfg(flags, **kwargs):
    """ Configure the Tile Cells Muon decorator augmentation tool """

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    kwargs.setdefault('name', 'TCAL1TileCellsMuonDecorator')
    prefix = kwargs.pop('Prefix', 'TCAL1_')
    kwargs.setdefault('IsoCone', 0.4)
    kwargs.setdefault('DeltaRCones', [0.2, 0.4])
    kwargs.setdefault('ClusterContainer', "CaloCalTopoClusters")
    # Moved logic of setting WriteDecorHandleKeys to python to make configuration clearer
    kwargs.setdefault("SelectedMuon", prefix + "SelectedMuon")
    kwargs.setdefault("Etrkcone", prefix + "etrkcone" + str(int(kwargs['IsoCone'] * 100)))
    kwargs.setdefault("CellsMuonX", prefix + "cells_muon_x")
    kwargs.setdefault("CellsMuonY", prefix + "cells_muon_y")
    kwargs.setdefault( "CellsMuonZ", prefix + "cells_muon_z")
    kwargs.setdefault("CellsMuonEta", prefix + "cells_muon_eta")
    kwargs.setdefault("CellsMuonPhi", prefix + "cells_muon_phi")
    kwargs.setdefault("CellsToMuonDx", prefix + "cells_to_muon_dx")
    kwargs.setdefault("CellsToMuonDy", prefix + "cells_to_muon_dy")
    kwargs.setdefault("CellsToMuonDz", prefix + "cells_to_muon_dz")
    kwargs.setdefault("CellsToMuonDeta", prefix + "cells_to_muon_deta")
    kwargs.setdefault("CellsToMuonDphi", prefix + "cells_to_muon_dphi")
    kwargs.setdefault("CellsMuonDx", prefix + "cells_muon_dx")
    kwargs.setdefault("CellsMuonDeDx", prefix + "cells_muon_dedx")
    if len(kwargs['ClusterContainer']) > 0:
        kwargs.setdefault("LArEnergyInCone", [prefix + "elarcone" + str(int(x*100)) for x in kwargs['DeltaRCones']])

    kwargs.setdefault('SelectMuons', flags.Beam.Type is BeamType.Collisions)
    kwargs.setdefault('MinMuonPt', 10 * GeV)
    kwargs.setdefault('MaxAbsMuonEta', 1.7)
    kwargs.setdefault('MaxRelETrkInIsoCone', 100000)

    kwargs.setdefault('TrackTools', acc.popToolsAndMerge(TCAL1TrackToolsCfg(flags)) )

    kwargs.setdefault('CellsDecorator', acc.popToolsAndMerge(TCAL1TileCellsDecoratorCfg(flags,Prefix=prefix)))

    kwargs.setdefault('TracksInConeTool', CompFactory.xAOD.TrackParticlesInConeTool())

    TileCellsMuonDecorator = CompFactory.DerivationFramework.TileCellsMuonDecorator
    acc.addPublicTool(TileCellsMuonDecorator(**kwargs), primary = True)

    return acc


def TCAL1StringSkimmingToolCfg(flags, **kwargs):
    """Configure TCAL1 derivation framework skimming tool"""

    prefix = kwargs.pop('Prefix', 'TCAL1_')

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    selectionExpression = ""
    if flags.Beam.Type is BeamType.Collisions:
        selectionExpression = f'(Muons.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500 + 0.4 * Muons.neflowisol20) / Muons.pt < 0.18 && Muons.{prefix}SelectedMuon'
    else:
        selectionExpression = 'abs(Muons.eta) < 1.7'

    skimmingExpression = f'count({selectionExpression}) > 0'

    kwargs.setdefault('name', 'TCAL1StringSkimmingTool')
    kwargs.setdefault('expression', skimmingExpression)
    kwargs.setdefault('TrigDecisionTool', tdt)

    xAODStringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool
    acc.addPublicTool(xAODStringSkimmingTool(**kwargs), primary = True)

    return acc


def TCAL1MuonTPThinningToolCfg(flags, streamName, **kwargs):
    """Configure TCAL1 derivation framework thinning tool"""

    acc = ComponentAccumulator()

    kwargs['StreamName'] = streamName
    kwargs.setdefault('name', 'TCAL1MuonTPThinningTool')
    kwargs.setdefault('MuonKey', 'Muons')
    kwargs.setdefault('InDetTrackParticlesKey', 'InDetTrackParticles')

    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg
    muonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(flags, **kwargs))

    acc.addPublicTool(muonTPThinningTool, primary = True)

    return acc


def TCAL1KernelCfg(flags, name='TCAL1Kernel', **kwargs):
    """Configure the TCAL1 derivation framework driving algorithm (kernel)"""

    acc = ComponentAccumulator()

    prefix = kwargs.pop('Prefix', 'TCAL1_')
    streamName = kwargs.pop('StreamName', 'StreamDAOD_TCAL1')

    # Common augmentations
    triggerListsHelper = kwargs.pop('TriggerListsHelper', 'TriggerListsHelper')
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = triggerListsHelper))

    deltaRCones = kwargs.pop('DeltaRCones', [0.2, 0.4])
    cellsMuonDecorator = acc.getPrimaryAndMerge( TCAL1TileCellsMuonDecoratorCfg(flags, Prefix=prefix, DeltaRCones=deltaRCones) )
    kwargs.setdefault('AugmentationTools', [cellsMuonDecorator])

    skimmingTool = acc.getPrimaryAndMerge(TCAL1StringSkimmingToolCfg(flags, Prefix=prefix))
    kwargs.setdefault('SkimmingTools', [skimmingTool])

    thinningTool = acc.getPrimaryAndMerge(TCAL1MuonTPThinningToolCfg(flags, streamName=streamName))
    kwargs.setdefault('ThinningTools', [thinningTool])

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, **kwargs))

    return acc


def TCAL1Cfg(flags):
    """Configure the TCAL1 derivation framework"""

    TCAL1Prefix = 'TCAL1_'
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TCAL1TriggerListsHelper = TriggerListsHelper(flags)

    deltaRCones = [0.2, 0.4]
    acc = ComponentAccumulator()
    acc.merge(TCAL1KernelCfg(flags, name="TCAL1Kernel", StreamName="StreamDAOD_TCAL1", Prefix=TCAL1Prefix,  TriggerListsHelper=TCAL1TriggerListsHelper, DeltaRCones=deltaRCones))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TCAL1SlimmingHelper = SlimmingHelper("TCAL1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TCAL1SlimmingHelper.SmartCollections = ['EventInfo', 'Muons', 'AntiKt4EMTopoJets', 'AntiKt4EMPFlowJets', 'MET_Baseline_AntiKt4EMTopo', 'MET_Baseline_AntiKt4EMPFlow', 'PrimaryVertices', 'BTagging_AntiKt4EMPFlow']

    TCAL1ExtraVariables = f'Muons.{TCAL1Prefix}etrkcone40'

    TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_'.join(['', 'energy', 'et', 'eta', 'phi', 'gain', 'bad', 'time', 'quality'])
    TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_'.join(['', 'sampling', 'sinTh', 'cosTh', 'cotTh', 'x', 'y', 'z'])

    TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_'.join(['', 'side', 'section', 'module', 'tower', 'sample'])
    TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_'.join(['', 'r', 'dx', 'dy', 'dz', 'dr', 'dphi', 'deta', 'volume'])

    TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_muon_'.join(['', 'dx', 'dedx', 'x', 'y', 'z', 'eta', 'phi'])
    TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_to_muon_'.join(['', 'dx', 'dy', 'dz', 'deta', 'dphi'])

    for pmt in ['pmt1', 'pmt2']:
        TCAL1ExtraVariables += f'.{TCAL1Prefix}cells_{pmt}_'.join(['', 'ros', 'drawer', 'channel', 'energy', 'time', 'quality', 'qbit', 'bad', 'gain'])

    for drCone in deltaRCones:
        TCAL1ExtraVariables += f'.{TCAL1Prefix}elarcone{int(drCone*100)}'

    TCAL1SlimmingHelper.ExtraVariables = [TCAL1ExtraVariables]

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TCAL1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TCAL1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TCAL1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TCAL1TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TCAL1SlimmingHelper)      

    TCAL1ItemList = TCAL1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TCAL1", ItemList=TCAL1ItemList, AcceptAlgs=["TCAL1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TCAL1", AcceptAlgs=["TCAL1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
