# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator, DataType
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.SystemOfUnits	import GeV

# Config:
triggerChainsPerYear = {
    2015: ['HLT_e24_lhmedium_L1EM20VH || HLT_e60_lhmedium || HLT_e120_lhloose', 'HLT_mu20_iloose_L1MU15 || HLT_mu40', 'HLT_2g20_tight'],
    2016: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_mu26_ivarmedium || HLT_mu50', 'HLT_g35_loose_g25_loose'],
    2017: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_2g22_tight_L12EM15VHI', 'HLT_mu50'],
    2018: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_g35_medium_g25_medium_L12EM20VH', 'HLT_mu26_ivarmedium', 'HLT_2mu14'],
    # '2022': ['HLT_e26_lhtight_ivarloose_L1EM22VHI || HLT_e60_lhmedium_L1EM22VHI || HLT_e140_lhloose_L1EM22VHI'],
    # '2023': ['HLT_e26_lhtight_ivarloose_L1EM22VHI || HLT_e60_lhmedium_L1EM22VHI || HLT_e140_lhloose_L1EM22VHI'],
}
triggerMatchingChainsPerYear = {
    2015: ['HLT_e24_lhmedium_L1EM20VH || HLT_e60_lhmedium || HLT_e120_lhloose', 'HLT_mu20_iloose_L1MU15 || HLT_mu40'],
    2016: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_mu26_ivarmedium || HLT_mu50'],
    2017: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_mu50'],
    2018: ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_mu26_ivarmedium'],
}
tauTriggerChainsSF = {
    2015: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
    2016: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
    2017: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
    2018: ['HLT_tau25_medium1_tracktwoEF', 'HLT_tau35_medium1_tracktwoEF'],
    2022: ['HLT_tau25_mediumRNN_tracktwoMVA', 'HLT_tau35_mediumRNN_tracktwoMVA'],
    2023: ['HLT_tau25_mediumRNN_tracktwoMVA', 'HLT_tau35_mediumRNN_tracktwoMVA'],
}
bjetTriggerChainsPerYear = {
    2022: ['HLT_2j45_0eta290_020jvt_bdl1d60_2j45_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25'],
    2023: ['HLT_2j45_0eta290_020jvt_bgn160_2j45_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25'],
    2024: ['HLT_2j45_0eta290_020jvt_bgn260_2j45_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25'],
}

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

electronMinPt = 10*GeV
photonMinPt = 10*GeV


def makeTestSequenceBlocks (dataType, algSeq, isPhyslite,
                        geometry=None, autoconfigFromFlags=None, noSystematics=None,
                        onlyNominalOR=False,  forceEGammaFullSimConfig=False,
                        returnConfigSeq=False,
                        bleedingEdge=False # Enabled for CI tests running on new derivations from derivation CI output
                        ) :

    largeRJets = True

    if autoconfigFromFlags is not None:
        if geometry is None:
            geometry = autoconfigFromFlags.GeoModel.Run

    configSeq = ConfigSequence ()

    outputContainers = {'mu_' : 'OutMuons',
                        'el_' : 'OutElectrons',
                        'ph_' : 'OutPhotons',
                        'tau_': 'OutTauJets',
                        'jet_': 'OutJets',
                        'met_': 'AnaMET',
                        ''    : 'EventInfo'}
    outputContainersForMC = {'truth_mu_' : 'OutTruthMuons',
                             'truth_el_' : 'OutTruthElectrons',
                             'truth_ph_' : 'OutTruthPhotons',
                             'truth_tau_': 'OutTruthTaus',
                             'truth_jet_': 'OutTruthJets',
                             'truth_met_': 'TruthMET'}

    # create factory object to build block configurations
    from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
    config = ConfigFactory()

    configSeq += config.makeConfig('CommonServices')
    configSeq.setOptionValue('.systematicsHistogram', 'systematicsList')

    configSeq += config.makeConfig('PileupReweighting')

    # Skip events with no primary vertex,
    # and perform loose jet cleaning
    configSeq += config.makeConfig ('EventCleaning')
    configSeq.setOptionValue ('.runEventCleaning', True)

    # disabling comparisons for triggers, because the config blocks do a
    # lot more than the sequences. Also disabling for Run 3+4, as there is no SF yet
    if geometry is LHCPeriod.Run2:
        # Include, and then set up the trigger analysis sequence:
        configSeq += config.makeConfig( 'Trigger' )
        configSeq.setOptionValue ('.triggerChainsPerYear', triggerChainsPerYear )
        configSeq.setOptionValue ('.noFilter', True )
        configSeq.setOptionValue ('.electrons', 'AnaElectrons.loose' )
        configSeq.setOptionValue ('.photons', 'AnaPhotons.tight' )
        configSeq.setOptionValue ('.muons', 'AnaMuons.medium' )
        configSeq.setOptionValue ('.taus', 'AnaTauJets.tight' )
        configSeq.setOptionValue ('.electronID', 'Tight' )
        configSeq.setOptionValue ('.electronIsol', 'Tight_VarRad')
        configSeq.setOptionValue ('.photonIsol', 'TightCaloOnly')
        configSeq.setOptionValue ('.muonID', 'Tight')
        configSeq.setOptionValue ('.triggerMatchingChainsPerYear', triggerMatchingChainsPerYear)

    # Include, and then set up the jet analysis algorithm sequence:
    configSeq += config.makeConfig( 'Jets',
        containerName='AnaJets',
        jetCollection='AntiKt4EMPFlowJets')
    configSeq.setOptionValue ('.runJvtUpdate', False )
    configSeq.setOptionValue ('.runNNJvtUpdate', True )
    configSeq.setOptionValue ('.recalibratePhyslite', False)

    configSeq += config.makeConfig( 'Jets.JVT' )
    configSeq.setOptionValue ('.containerName', 'AnaJets')

    configSeq += config.makeConfig( 'Jets.FTagTriggerMatching' )
    configSeq.setOptionValue('.containerName', 'AnaJets')
    configSeq.setOptionValue('.triggerChainsPerYear', bjetTriggerChainsPerYear)
    
    # disabling flavor tagging for Run 4, as the configuration just
    # refuses to work on that
    if geometry is not LHCPeriod.Run4:

        btagger = "GN2v01"
        btagWP = "FixedCutBEff_65"
        configSeq += config.makeConfig( 'Jets.FlavourTagging' )
        configSeq.setOptionValue ('.containerName', 'AnaJets')
        configSeq.setOptionValue ('.selectionName', 'ftag')
        configSeq.setOptionValue ('.btagger', btagger)
        configSeq.setOptionValue ('.btagWP', btagWP)
        configSeq.setOptionValue ('.saveScores', 'All')

        configSeq += config.makeConfig( 'Jets.FlavourTaggingEventSF' )
        configSeq.setOptionValue ('.containerName', 'AnaJets.baselineJvt')
        configSeq.setOptionValue ('.btagger', btagger)

    if largeRJets :
        configSeq += config.makeConfig( 'Jets',
            containerName='AnaLargeRJets',
            jetCollection='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' )
        outputContainers['larger_jet_'] = 'OutLargeRJets'
        configSeq.setOptionValue ('.recalibratePhyslite', False)


    # Include, and then set up the electron analysis algorithm sequence:
    likelihood = True
    configSeq += config.makeConfig ('Electrons')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq.setOptionValue ('.decorateCaloClusterEta', True)
    configSeq.setOptionValue ('.writeTrackD0Z0', True)
    configSeq.setOptionValue ('.forceFullSimConfigForIso', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq.setOptionValue ('.minPt', electronMinPt)
    configSeq += config.makeConfig ('Electrons.WorkingPoint')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq.setOptionValue ('.selectionName', 'loose')
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    if likelihood:
        configSeq.setOptionValue ('.identificationWP', 'LooseBLayerLH')
    else:
        configSeq.setOptionValue ('.identificationWP', 'LooseDNN')
    configSeq.setOptionValue ('.isolationWP', 'Tight_VarRad')
    configSeq.setOptionValue ('.chargeIDSelectionRun2', True)
    configSeq.setOptionValue ('.addChargeMisIDSF', geometry is LHCPeriod.Run2)

    configSeq += config.makeConfig ('Electrons.IFFClassification')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq += config.makeConfig ('Electrons.MCTCClassification')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq.setOptionValue ('.prefix', 'truth_')

    configSeq += config.makeConfig ('Electrons.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq.setOptionValue ('.minPt', electronMinPt)


    # Include, and then set up the photon analysis algorithm sequence:
    configSeq += config.makeConfig ('Photons')
    configSeq.setOptionValue ('.containerName', 'AnaPhotons')
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq.setOptionValue ('.forceFullSimConfigForIso', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.recomputeIsEM', False)
    configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Photons.WorkingPoint')
    configSeq.setOptionValue ('.containerName', 'AnaPhotons')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.forceFullSimConfigForID', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.forceFullSimConfigForIso', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.qualityWP', 'Tight')
    configSeq.setOptionValue ('.isolationWP', 'FixedCutTight')
    configSeq.setOptionValue ('.recomputeIsEM', False)

    configSeq += config.makeConfig ('Photons.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'AnaPhotons')
    configSeq.setOptionValue ('.minPt', photonMinPt)


    # set up the muon analysis algorithm sequence:
    configSeq += config.makeConfig ('Muons')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq.setOptionValue ('.writeTrackD0Z0', True)
    configSeq.setOptionValue ('.recalibratePhyslite', False)

    configSeq += config.makeConfig ('Muons.WorkingPoint')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq.setOptionValue ('.selectionName', 'medium')
    configSeq.setOptionValue ('.quality', 'Medium')
    configSeq.setOptionValue ('.isolation', 'Loose_VarRad')

    configSeq += config.makeConfig ('Muons.IFFClassification')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq += config.makeConfig ('Muons.MCTCClassification')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq.setOptionValue ('.prefix', 'truth_')


    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # configSeq += config.makeConfig ('Muons.Selection', 'AnaMuons.tight')
    # configSeq.setOptionValue ('.quality', 'Tight')
    # configSeq.setOptionValue ('.isolation', 'Loose_VarRad')

    # Include, and then set up the tau analysis algorithm sequence:
    configSeq += config.makeConfig ('TauJets')
    configSeq.setOptionValue ('.containerName', 'AnaTauJets')
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq += config.makeConfig ('TauJets.WorkingPoint')
    configSeq.setOptionValue ('.containerName', 'AnaTauJets')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.quality', 'Tight')

    configSeq += config.makeConfig('TauJets.TriggerSF')
    configSeq.setOptionValue('.containerName', 'AnaTauJets')
    configSeq.setOptionValue('.tauID', 'Tight')
    configSeq.setOptionValue('.triggerChainsPerYear', tauTriggerChainsSF)

    configSeq += config.makeConfig ('TauJets.MCTCClassification')
    configSeq.setOptionValue ('.containerName', 'AnaTauJets')
    configSeq.setOptionValue ('.prefix', 'truth_')


    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink')
    configSeq.setOptionValue ('.containerName', 'AnaJets')
    if largeRJets:
        configSeq += config.makeConfig('SystObjectLink')
        configSeq.setOptionValue ('.containerName', 'AnaLargeRJets')
    configSeq += config.makeConfig('SystObjectLink')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq += config.makeConfig('SystObjectLink')
    configSeq.setOptionValue ('.containerName', 'AnaPhotons')
    configSeq += config.makeConfig('SystObjectLink')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq += config.makeConfig('SystObjectLink')
    configSeq.setOptionValue ('.containerName', 'AnaTauJets')


    # Particle-level objects
    configSeq += config.makeConfig ('PL_Electrons')
    configSeq.setOptionValue ('.containerName', 'TruthElectrons')
    configSeq += config.makeConfig ('PL_Electrons.MCTCClassification')
    configSeq.setOptionValue ('.containerName', 'TruthElectrons')
    configSeq.setOptionValue ('.prefix', '')
    configSeq += config.makeConfig ('PL_Electrons.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'TruthElectrons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.useDressedProperties', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Muons')
    configSeq.setOptionValue ('.containerName', 'TruthMuons')
    configSeq += config.makeConfig ('PL_Muons.MCTCClassification')
    configSeq.setOptionValue ('.containerName', 'TruthMuons')
    configSeq.setOptionValue ('.prefix', '')
    configSeq += config.makeConfig ('PL_Muons.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'TruthMuons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.useDressedProperties', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Neutrinos')
    configSeq.setOptionValue ('.skipOnData', True)

    configSeq += config.makeConfig ('PL_Jets')
    configSeq.setOptionValue ('.containerName', 'AntiKt4TruthDressedWZJets')
    configSeq += config.makeConfig ('PL_Jets.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'AntiKt4TruthDressedWZJets')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Taus')
    configSeq.setOptionValue ('.containerName', 'TruthTaus')
    configSeq += config.makeConfig ('PL_Taus.MCTCClassification')
    configSeq.setOptionValue ('.containerName', 'TruthTaus')
    configSeq.setOptionValue ('.prefix', '')
    configSeq += config.makeConfig ('PL_Taus.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'TruthTaus')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Photons')
    configSeq.setOptionValue ('.containerName', 'TruthPhotons')
    configSeq += config.makeConfig ('PL_Photons.PtEtaSelection')
    configSeq.setOptionValue ('.containerName', 'TruthPhotons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_MissingET')
    configSeq.setOptionValue ('.skipOnData', True)

    configSeq += config.makeConfig ('PL_OverlapRemoval')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.electrons', 'TruthElectrons')
    configSeq.setOptionValue ('.muons', 'TruthMuons')
    configSeq.setOptionValue ('.photons', 'TruthPhotons')
    configSeq.setOptionValue ('.jets', 'AntiKt4TruthDressedWZJets')
    configSeq.setOptionValue ('.useRapidityForDeltaR', False)


    if dataType is not DataType.Data :
        # Include, and then set up the generator analysis sequence:
        configSeq += config.makeConfig( 'GeneratorLevelAnalysis')


    # Include, and then set up the met analysis algorithm config:
    configSeq += config.makeConfig ('MissingET')
    configSeq.setOptionValue ('.containerName', 'AnaMET')
    configSeq.setOptionValue ('.jets', 'AnaJets')
    configSeq.setOptionValue ('.taus', 'AnaTauJets.tight')
    configSeq.setOptionValue ('.electrons', 'AnaElectrons.loose')
    configSeq.setOptionValue ('.photons', 'AnaPhotons.tight')
    # Note that the configuration for the muons is not what you'd
    # normally do.  This is specifically here because this is a unit
    # test and I wanted to make sure that selection expressions work.
    # For an actual analysis that would just be `AnaMuons.medium`, but
    # since `tight` is a strict subset of `medium` it doesn't matter
    # if we do an "or" of the two.
    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # configSeq.setOptionValue ('.muons', 'AnaMuons.medium||tight')
    configSeq.setOptionValue ('.muons', 'AnaMuons.medium')


    # Include, and then set up the overlap analysis algorithm config:
    configSeq += config.makeConfig( 'OverlapRemoval' )
    configSeq.setOptionValue ('.electrons',   'AnaElectrons.loose')
    configSeq.setOptionValue ('.photons',     'AnaPhotons.tight')
    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # configSeq.setOptionValue ('.muons',       'AnaMuons.medium||tight')
    configSeq.setOptionValue ('.muons',       'AnaMuons.medium')
    configSeq.setOptionValue ('.jets',        'AnaJets.baselineJvt')
    configSeq.setOptionValue ('.taus',        'AnaTauJets.tight')
    configSeq.setOptionValue ('.inputLabel',  'preselectOR')
    configSeq.setOptionValue ('.outputLabel', 'passesOR' )
    configSeq.setOptionValue ('.nominalOnly', onlyNominalOR )


    # ObjectCutFlow blocks
    configSeq += config.makeConfig ('ObjectCutFlow')
    configSeq.setOptionValue ('.containerName', 'AnaJets')
    configSeq.setOptionValue ('.selectionName', 'jvt')
    configSeq += config.makeConfig ('ObjectCutFlow')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq.setOptionValue ('.selectionName', 'loose')
    configSeq += config.makeConfig ('ObjectCutFlow')
    configSeq.setOptionValue ('.containerName', 'AnaPhotons')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq += config.makeConfig ('ObjectCutFlow')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq.setOptionValue ('.selectionName', 'medium')
    configSeq += config.makeConfig ('ObjectCutFlow')
    configSeq.setOptionValue ('.containerName', 'AnaTauJets')
    configSeq.setOptionValue ('.selectionName', 'tight')

    # Include and set up a basic run of the event selection algorithm config:
    if geometry is not LHCPeriod.Run4:
        # configSeq += config.makeConfig( 'EventSelection', None )
        # configSeq.setOptionValue ('.electrons',   'AnaElectrons.loose')
        # configSeq.setOptionValue ('.muons',       'AnaMuons.medium')
        # configSeq.setOptionValue ('.jets',        'AnaJets')
        # configSeq.setOptionValue ('.met',         'AnaMET')
        # configSeq.setOptionValue ('.selectionCutsDict', exampleSelectionCuts)
        from EventSelectionAlgorithms.EventSelectionConfig import makeMultipleEventSelectionConfigs
        makeMultipleEventSelectionConfigs(configSeq, electrons = 'AnaElectrons.loose', muons = 'AnaMuons.medium', jets = 'AnaJets.baselineJvt',
                                          met = 'AnaMET', btagDecoration = 'ftag_select_ftag',
                                          selectionCutsDict = exampleSelectionCuts, noFilter = True,
                                          cutFlowHistograms = True)

    configSeq += config.makeConfig ('Bootstraps')
    configSeq.setOptionValue ('.nReplicas', 2000 )
    configSeq.setOptionValue ('.skipOnMC', False)

    # per-event lepton SF
    configSeq += config.makeConfig ('LeptonSF')
    if geometry is not LHCPeriod.Run2:
        configSeq.setOptionValue ('.electrons', 'AnaElectrons.loose')
    configSeq.setOptionValue ('.muons', 'AnaMuons.medium')
    configSeq.setOptionValue ('.photons', 'AnaPhotons.tight')
    configSeq.setOptionValue ('.lepton_postfix', 'nominal')

    # Thinning blocks
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'AnaElectrons')
    configSeq.setOptionValue ('.selectionName', 'loose')
    configSeq.setOptionValue ('.outputName', 'OutElectrons')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'AnaPhotons')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.outputName', 'OutPhotons')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'AnaMuons')
    configSeq.setOptionValue ('.selectionName', 'medium')
    configSeq.setOptionValue ('.outputName', 'OutMuons')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'AnaTauJets')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.outputName', 'OutTauJets')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'AnaJets')
    configSeq.setOptionValue ('.outputName', 'OutJets')
    if largeRJets :
        configSeq += config.makeConfig ('Thinning')
        configSeq.setOptionValue ('.containerName', 'AnaLargeRJets')
        configSeq.setOptionValue ('.outputName', 'OutLargeRJets')

    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'TruthElectrons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthElectrons')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'TruthPhotons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthPhotons')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'TruthMuons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthMuons')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'TruthTaus')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthTaus')
    configSeq += config.makeConfig ('Thinning')
    configSeq.setOptionValue ('.containerName', 'AntiKt4TruthDressedWZJets')
    configSeq.setOptionValue ('.outputName', 'OutTruthJets')
    configSeq.setOptionValue ('.skipOnData', True)

    configSeq += config.makeConfig ('Output')
    configSeq.setOptionValue ('.treeName', 'analysis')
    configSeq.setOptionValue ('.vars', [
        'EventInfo.actualInteractionsPerCrossing -> actualMuScaled',
    ])
    configSeq.setOptionValue ('.metVars', [
        'AnaMET_%SYS%.met -> met_%SYS%',
    ])
    configSeq.setOptionValue ('.truthMetVars', [
        'TruthMET_NOSYS.met -> truth_met',
    ])
    configSeq.setOptionValue ('.containers', outputContainers)
    configSeq.setOptionValue ('.containersOnlyForMC', outputContainersForMC)
    configSeq.setOptionValue ('.commands', [
        'disable actualInteractionsPerCrossing',
    ])

    # save the tool configuration to a txt file
    configSeq += config.makeConfig ('PrintConfiguration')

    # return configSeq for unit test
    if returnConfigSeq:
        return configSeq

    configAccumulator = ConfigAccumulator (algSeq=algSeq, flags=autoconfigFromFlags, noSystematics=noSystematics)
    configSeq.fullConfigure (configAccumulator)

    # order can change during fullConfigure
    configSeq.printOptions()

    from AnaAlgorithm.DualUseConfig import isAthena, useComponentAccumulator
    if isAthena and useComponentAccumulator:
        return configAccumulator.CA
    else:
        return None



def printSequenceAlgs (sequence) :
    """print the algorithms in the sequence without the sequence structure

    This is mostly meant for easy comparison of different sequences
    during configuration, particularly the sequences resulting from
    the old sequence configuration and the new block configuration.
    Those have different sequence structures in the output, but the
    algorithms should essentially be configured the same way."""
    if isinstance (sequence, AlgSequence) :
        for alg in sequence :
            printSequenceAlgs (alg)
    else :
        # assume this is an algorithm then
        print (sequence)


def makeSequence (dataType, noSystematics,
        yamlPath=None,
        isPhyslite = False, geometry = None,
        autoconfigFromFlags = None, onlyNominalOR = False,
        forceEGammaFullSimConfig = False,
        bleedingEdge = False) :

    algSeq = AlgSequence('AnalysisSequence')

    ca = None
    if not yamlPath:
        ca = makeTestSequenceBlocks (dataType, algSeq,
                                 isPhyslite=isPhyslite,
                                 geometry=geometry, onlyNominalOR=onlyNominalOR,
                                 autoconfigFromFlags=autoconfigFromFlags,
                                 noSystematics=noSystematics,
                                 forceEGammaFullSimConfig=forceEGammaFullSimConfig,
                                 bleedingEdge=bleedingEdge)
    else:
        from AnalysisAlgorithmsConfig.ConfigText import makeSequence as makeSequenceText
        ca = makeSequenceText(yamlPath, algSeq=algSeq, flags=autoconfigFromFlags,
                              noSystematics=noSystematics)

    if ca is not None:
        return ca
    else:
        return algSeq
