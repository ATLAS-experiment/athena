#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Joseph Lambert

def compareConfigSeq(seq1, seq2, *, checkOrder=False):
    """Compares two ConfigSequences"""
    blocks1 = seq1._blocks
    blocks2 = seq2._blocks
    print("Block order for each config sequence")
    print("\033[4m{0:<30} {1:<30}\033[0m".format("Sequence1", "Sequence2"))
    for i in range(max(len(blocks1), len(blocks2))):
        name1, name2 = '', ''
        if i < len(blocks1):
            name1 = blocks1[i].__class__.__name__
        if i < len(blocks2):
            name2 = blocks2[i].__class__.__name__
        print(f"{name1:<30} {name2}")
    if not checkOrder:
        print("Sorting blocks by name (will not sort blocks with same name)")
        blocks1.sort(key=lambda x: x.__class__.__name__)
        blocks2.sort(key=lambda x: x.__class__.__name__)
    if len(blocks1) != len(blocks2):
        raise Exception("Number of blocks are different")
    for i in range(len(blocks1)):
        block1 = blocks1[i]
        block2 = blocks2[i]
        name1 = block1.__class__.__name__
        name2 = block2.__class__.__name__
        if name1 != name2:
            raise Exception(f"In position {i} "
                f"the first sequence results in {name1} "
                f"and the second sequence results in {name2}")
        for name in block1.getOptions():
            if name == 'groupName':
                continue
            value1 = block1.getOptionValue(name)
            value2 = block2.getOptionValue(name)
            if value1 != value2:
                raise Exception(f"For block {name1}, the block "
                    f"option {name} the first sequence results in {value1} "
                    f"and the second sequence results in {value2}")


def compareTextBuilder(yamlPath='', *, checkOrder=False) :
    """
    Create a configSequence using provided YAML file and a
    configSequence using ConfigText python commands and compare.

    Will raise an exception if configSequences differ
    """
    # create text config object to build text configurations
    from AnalysisAlgorithmsConfig.ConfigText import TextConfig
    config = TextConfig()

    # CommonServices
    config.addBlock('CommonServices')
    config.setOptions(systematicsHistogram='systematicsList')

    # PileupReweighting
    config.addBlock('PileupReweighting')

    # EventCleaning
    config.addBlock('EventCleaning')
    config.setOptions (runEventCleaning=True)

    # Jets
    config.addBlock('Jets')
    config.setOptions (containerName='AnaJets')
    config.setOptions (jetCollection='AntiKt4EMPFlowJets')
    config.setOptions (runJvtUpdate=False)
    config.setOptions (runNNJvtUpdate=True)
    config.setOptions (recalibratePhyslite=False)
    # Jets.FlavourTagging
    config.addBlock( 'Jets.FlavourTagging')
    config.setOptions (containerName='AnaJets')
    config.setOptions (selectionName='ftag')
    config.setOptions (btagger='GN2v01')
    config.setOptions (btagWP='FixedCutBEff_65')
    config.setOptions (saveScores='All')
    # Jets.FlavourTaggingEventSF
    config.addBlock( 'Jets.FlavourTaggingEventSF')
    config.setOptions (containerName='AnaJets.baselineJvt')
    config.setOptions (btagger='GN2v01')
    # Jets.JVT
    config.addBlock('Jets.JVT', containerName='AnaJets')

    # Large-R jets
    config.addBlock('Jets')
    config.setOptions (containerName='AnaLargeRJets')
    config.setOptions (jetCollection='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets')
    config.setOptions (recalibratePhyslite=False)

    # Electrons
    config.addBlock ('Electrons')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (recalibratePhyslite=False)
    config.setOptions (decorateTruth=True)
    # Electrons.WorkingPoint
    config.addBlock ('Electrons.WorkingPoint')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (selectionName='loose')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (noEffSF=True)
    config.setOptions (identificationWP='LooseBLayerLH')
    config.setOptions (isolationWP='Loose_VarRad')
    config.setOptions (writeTrackD0Z0=True)
    # Electrons.PtEtaSelection
    config.addBlock ('Electrons.PtEtaSelection')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (minPt=10000.0)
    # Electrons.IFFClassification
    config.addBlock ('Electrons.IFFClassification')
    config.setOptions (containerName='AnaElectrons')
    # Electrons.MCTCClassification
    config.addBlock ('Electrons.MCTCClassification')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (prefix='truth_')

    # Photons
    config.addBlock ('Photons', containerName='AnaPhotons')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (recomputeIsEM=False)
    config.setOptions (recalibratePhyslite=False)
    config.setOptions (decorateTruth=True)
    # Photons.WorkingPoint
    config.addBlock ('Photons.WorkingPoint')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (selectionName='tight')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (qualityWP='Tight')
    config.setOptions (isolationWP='FixedCutTight')
    config.setOptions (recomputeIsEM=False)
    # Photons.PtEtaSelection
    config.addBlock ('Photons.PtEtaSelection')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (minPt=10000.0)

    # Muons
    config.addBlock ('Muons', containerName='AnaMuons')
    config.setOptions (recalibratePhyslite=False)
    config.setOptions (decorateTruth=True)
    # Muons.WorkingPoint
    config.addBlock ('Muons.WorkingPoint')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (selectionName='medium')
    config.setOptions (quality='Medium')
    config.setOptions (isolation='Loose_VarRad')
    config.setOptions (writeTrackD0Z0=True)
    # Muons.IFFClassification
    config.addBlock ('Muons.IFFClassification')
    config.setOptions (containerName='AnaMuons')
    # Muons.MCTCClassification
    config.addBlock ('Muons.MCTCClassification')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (prefix='truth_')

    # TauJets
    config.addBlock ('TauJets', containerName='AnaTauJets')
    config.setOptions (decorateTruth=True)
    # TauJets.WorkingPoint
    config.addBlock ('TauJets.WorkingPoint')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (selectionName='tight')
    config.setOptions (quality='Tight')
    # TauJets.TriggerSF
    tauTriggerChainsSF = {
        2015: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
        2016: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
        2017: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
        2018: ['HLT_tau25_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA', 'HLT_tau35_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA'],
    }
    config.addBlock ('TauJets.TriggerSF')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (tauID='Tight')
    config.setOptions (triggerChainsPerYear=tauTriggerChainsSF)
    # TauJets.MCTCClassification
    config.addBlock ('TauJets.MCTCClassification')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (prefix='truth_')

    config.addBlock ('SystObjectLink')
    config.setOptions (containerName='AnaJets')
    config.addBlock ('SystObjectLink')
    config.setOptions (containerName='AnaLargeRJets')
    config.addBlock ('SystObjectLink')
    config.setOptions (containerName='AnaElectrons')
    config.addBlock ('SystObjectLink')
    config.setOptions (containerName='AnaPhotons')
    config.addBlock ('SystObjectLink')
    config.setOptions (containerName='AnaMuons')
    config.addBlock ('SystObjectLink')
    config.setOptions (containerName='AnaTauJets')

    config.addBlock ('ObjectCutFlow')
    config.setOptions (containerName='AnaJets')
    config.setOptions (selectionName='jvt')
    config.addBlock ('ObjectCutFlow')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (selectionName='loose')
    config.addBlock ('ObjectCutFlow')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (selectionName='tight')
    config.addBlock ('ObjectCutFlow')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (selectionName='medium')
    config.addBlock ('ObjectCutFlow')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (selectionName='tight')

    # GeneratorLevelAnalysis
    config.addBlock( 'GeneratorLevelAnalysis')

    # MissingET
    config.addBlock ('MissingET')
    config.setOptions (containerName='AnaMET')
    config.setOptions (jets='AnaJets')
    config.setOptions (taus='AnaTauJets.tight')
    config.setOptions (electrons='AnaElectrons.loose')
    config.setOptions (photons='AnaPhotons.tight')
    config.setOptions (muons='AnaMuons.medium')

    # OverlapRemoval
    config.addBlock( 'OverlapRemoval' )
    config.setOptions (inputLabel='preselectOR')
    config.setOptions (outputLabel='passesOR' )
    config.setOptions (jets='AnaJets.baselineJvt')
    config.setOptions (taus='AnaTauJets.tight')
    config.setOptions (electrons='AnaElectrons.loose')
    config.setOptions (photons='AnaPhotons.tight')
    config.setOptions (muons='AnaMuons.medium')

    # Particle-level objects
    config.addBlock ('PL_Electrons')
    config.setOptions (containerName='TruthElectrons')
    config.addBlock ('PL_Electrons.PtEtaSelection')
    config.setOptions (containerName='TruthElectrons')
    config.setOptions (skipOnData=True)
    config.setOptions (useDressedProperties=True)
    config.setOptions (minPt=20e3)
    config.addBlock ('PL_Electrons.MCTCClassification')
    config.setOptions (containerName='TruthElectrons')
    config.setOptions (prefix='')

    config.addBlock ('PL_Photons')
    config.setOptions (containerName='TruthPhotons')
    config.addBlock ('PL_Photons.PtEtaSelection')
    config.setOptions (containerName='TruthPhotons')
    config.setOptions (skipOnData=True)
    config.setOptions (minPt=20e3)

    config.addBlock ('PL_Muons')
    config.setOptions (containerName='TruthMuons')
    config.addBlock ('PL_Muons.PtEtaSelection')
    config.setOptions (containerName='TruthMuons')
    config.setOptions (skipOnData=True)
    config.setOptions (useDressedProperties=True)
    config.setOptions (minPt=20e3)
    config.addBlock ('PL_Muons.MCTCClassification')
    config.setOptions (containerName='TruthMuons')
    config.setOptions (prefix='')

    config.addBlock ('PL_Taus')
    config.setOptions (containerName='TruthTaus')
    config.addBlock ('PL_Taus.PtEtaSelection')
    config.setOptions (containerName='TruthTaus')
    config.setOptions (skipOnData=True)
    config.setOptions (minPt=20e3)
    config.addBlock ('PL_Taus.MCTCClassification')
    config.setOptions (containerName='TruthTaus')
    config.setOptions (prefix='')

    config.addBlock ('PL_Jets')
    config.setOptions (containerName='AntiKt4TruthDressedWZJets')
    config.addBlock ('PL_Jets.PtEtaSelection')
    config.setOptions (containerName='AntiKt4TruthDressedWZJets')
    config.setOptions (skipOnData=True)
    config.setOptions (minPt=20e3)

    config.addBlock ('PL_Neutrinos')
    config.setOptions (skipOnData=True)
    config.addBlock ('PL_MissingET')
    config.setOptions (skipOnData=True)

    config.addBlock ('PL_OverlapRemoval')
    config.setOptions (skipOnData=True)
    config.setOptions (electrons='TruthElectrons')
    config.setOptions (muons='TruthMuons')
    config.setOptions (photons='TruthPhotons')
    config.setOptions (jets='AntiKt4TruthDressedWZJets')
    config.setOptions (useRapidityForDeltaR=False)

    # Thinning
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (selectionName='loose')
    config.setOptions (outputName='OutElectrons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (selectionName='tight')
    config.setOptions (outputName='OutPhotons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (selectionName='medium')
    config.setOptions (outputName='OutMuons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (selectionName='tight')
    config.setOptions (outputName='OutTauJets')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaJets')
    config.setOptions (outputName='OutJets')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaLargeRJets')
    config.setOptions (outputName='OutLargeRJets')
    config.addBlock ('Thinning')
    config.setOptions (containerName='TruthElectrons')
    config.setOptions (skipOnData=True)
    config.setOptions (outputName='OutTruthElectrons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='TruthPhotons')
    config.setOptions (skipOnData=True)
    config.setOptions (outputName='OutTruthPhotons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='TruthMuons')
    config.setOptions (skipOnData=True)
    config.setOptions (outputName='OutTruthMuons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='TruthTaus')
    config.setOptions (skipOnData=True)
    config.setOptions (outputName='OutTruthTaus')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AntiKt4TruthDressedWZJets')
    config.setOptions (outputName='OutTruthJets')
    config.setOptions (skipOnData=True)

    # Trigger
    triggerChainsPerYear = {
        2015: ['HLT_e24_lhmedium_L1EM20VH || HLT_e60_lhmedium || HLT_e120_lhloose', 'HLT_mu20_iloose_L1MU15 || HLT_mu40', 'HLT_2g20_tight'],
        2016: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_mu26_ivarmedium || HLT_mu50', 'HLT_g35_loose_g25_loose'],
        2017: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_2g22_tight_L12EM15VHI', 'HLT_mu50'],
        2018: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_g35_medium_g25_medium_L12EM20VH', 'HLT_mu26_ivarmedium', 'HLT_2mu14'],
    }
    config.addBlock ('Trigger')
    config.setOptions (triggerChainsPerYear=triggerChainsPerYear)
    config.setOptions (noFilter=True)
    config.setOptions (electrons='AnaElectrons.loose')
    config.setOptions (photons='AnaPhotons.tight')
    config.setOptions (muons='AnaMuons.medium')
    config.setOptions (taus='AnaTauJets.tight')
    config.setOptions (electronID='Tight')
    config.setOptions (electronIsol='Tight_VarRad')
    config.setOptions (photonIsol='TightCaloOnly')
    config.setOptions (muonID='Tight')
    config.setOptions (triggerMatchingChainsPerYear=triggerChainsPerYear)

    # EventSelection
    # Example cuts used for event selection algorithm test
    exampleSelectionCuts = {
        'SUBcommon': """JET_N_BTAG >= 2
JET_N 25000 >= 4
MET >= 20000
SAVE
""",
        'ejets': """IMPORT SUBcommon
EL_N 5000 == 1
MU_N 3000 == 0
MWT < 170000
MET+MWT > 40000
SAVE
""",
        'mujets': """IMPORT SUBcommon
EL_N 5000 == 0
MU_N medium 25000 > 0
SAVE
"""
    }
    config.addBlock ('EventSelection')
    config.setOptions (electrons='AnaElectrons.loose')
    config.setOptions (muons='AnaMuons.medium')
    config.setOptions (jets='AnaJets.baselineJvt')
    config.setOptions (met='AnaMET')
    config.setOptions (btagDecoration='ftag_select_ftag')
    config.setOptions (noFilter=True)
    config.setOptions (cutFlowHistograms=True)
    config.setOptions (selectionCutsDict=exampleSelectionCuts)

    # Bootstraps
    config.addBlock ('Bootstraps')
    config.setOptions (nReplicas=2000)
    config.setOptions (skipOnMC=False)

    # LeptonSF
    config.addBlock ('LeptonSF')
    config.setOptions (muons='AnaMuons.medium')
    config.setOptions (photons='AnaPhotons.tight')
    config.setOptions (lepton_postfix='nominal')

    # Output
    config.addBlock ('Output')
    config.setOptions (treeName='analysis')
    config.setOptions (vars=[])
    config.setOptions (metVars=[])
    outputContainers = {
        'mu_': 'OutMuons',
        'el_': 'OutElectrons',
        'ph_' : 'OutPhotons',
        'tau_': 'OutTauJets',
        'jet_': 'OutJets',
        'larger_jet_': 'OutLargeRJets',
        'met_': 'AnaMET',
        '': 'EventInfo'}
    config.setOptions (containers=outputContainers)
    outputContainersForMC = {'truth_mu_' : 'OutTruthMuons',
                             'truth_el_' : 'OutTruthElectrons',
                             'truth_ph_' : 'OutTruthPhotons',
                             'truth_tau_': 'OutTruthTaus',
                             'truth_jet_': 'OutTruthJets',
                             'truth_met_': 'TruthMET'}
    config.setOptions (containersOnlyForMC=outputContainersForMC)
    disable_commands = []
    config.setOptions (commands=disable_commands)

    # configure ConfigSequence
    configSeq = config.configure()

    # create text config object to build text configurations
    textConfig = TextConfig(yamlPath)
    textConfigSeq = textConfig.configure()

    # compare - will raise error if False
    compareConfigSeq(configSeq, textConfigSeq, checkOrder=checkOrder)


def compareBlockConfig(yamlPath='', *, checkOrder=False) :
    """
    Create a configSequence using provided YAML file and a
    configSequence using the block configuration and compare.

    Will raise an exception if configSequences differ
    """
    from AthenaConfiguration.Enums import LHCPeriod
    # create configSeq for block configuration
    from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeTestSequenceBlocks
    configSeq = makeTestSequenceBlocks(dataType='fullsim', algSeq=None,
            geometry=LHCPeriod.Run2,
            isPhyslite=False, forceEGammaFullSimConfig=True,
            returnConfigSeq=True)

    # create text config object to build text configurations
    from AnalysisAlgorithmsConfig.ConfigText import TextConfig
    textConfig = TextConfig(yamlPath)
    textConfigSeq = textConfig.configure()

    # compare - will raise error if False
    compareConfigSeq(configSeq, textConfigSeq, checkOrder=checkOrder)


if __name__ == '__main__':
    import os
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('--text-config', dest='text_config',
            default='', action='store',
            help='YAML file used in unit test')
    parser.add_option('--compare-block', dest='compare_block',
            default=False, action='store_true',
            help='Compare config sequence from YAML and block configuration')
    parser.add_option('--compare-builder', dest='compare_builder',
            default=False, action='store_true',
            help='Compare config sequence from YAML and python configuration')
    parser.add_option('--check-order', dest='check_order',
            default=False, action='store_true',
            help='Require blocks to be in the same order')
    (options, args) = parser.parse_args()
    textConfig = options.text_config
    compareBlock = options.compare_block
    compareBuilder = options.compare_builder
    checkOrder = options.check_order

    from PathResolver import PathResolver
    textConfig = PathResolver.FindCalibFile(textConfig)

    if not os.path.isfile(textConfig):
        raise FileNotFoundError(f"{textConfig} is not a file")

    # compare YAML and builder
    if compareBuilder:
        print("Comparing config sequences from the block and text"
            "configuration methods")
        compareTextBuilder(textConfig, checkOrder=checkOrder)
    # compare YAML and block config
    if compareBlock:
        print("Comparing config sequences from the block and block"
            "configuration methods")
        compareBlockConfig(textConfig, checkOrder=checkOrder)
