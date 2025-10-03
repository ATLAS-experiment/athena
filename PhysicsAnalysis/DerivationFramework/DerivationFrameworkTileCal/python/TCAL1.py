# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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

def TCAL1TileCellsMuonDecoratorCfg(flags, **kwargs):
    """ Configure the Tile Cells Muon decorator augmentation tool """

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    kwargs.setdefault('name', 'TCAL1TileCellsMuonDecorator')
    kwargs.setdefault('Prefix', 'TCAL1_')
    kwargs.setdefault('SelectMuons', flags.Beam.Type is BeamType.Collisions)
    kwargs.setdefault('MinMuonPt', 10 * GeV)
    kwargs.setdefault('MaxAbsMuonEta', 1.7)
    kwargs.setdefault('IsoCone', 0.4)
    kwargs.setdefault('MaxRelETrkInIsoCone', 100000)

    kwargs.setdefault('TrackTools', acc.popToolsAndMerge(TCAL1TrackToolsCfg(flags)) )

    TileCellsDecorator = CompFactory.DerivationFramework.TileCellsDecorator
    kwargs.setdefault('CellsDecorator', TileCellsDecorator(Prefix=kwargs['Prefix']))

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
