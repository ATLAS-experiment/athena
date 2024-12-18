# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
tauTriggerChainsSF = {
    2015: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
    2016: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
    2017: ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo'],
    2018: ['HLT_tau25_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA', 'HLT_tau35_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA'],
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
                        bleedingEdge=False) :

    vars = []
    metVars = []

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
        configSeq.setOptionValue ('.triggerMatchingChainsPerYear', triggerChainsPerYear)

    # Include, and then set up the jet analysis algorithm sequence:
    configSeq += config.makeConfig( 'Jets',
        containerName='AnaJets',
        jetCollection='AntiKt4EMPFlowJets')
    configSeq.setOptionValue ('.runJvtUpdate', False )
    configSeq.setOptionValue ('.runNNJvtUpdate', True )
    configSeq.setOptionValue ('.recalibratePhyslite', False)

    configSeq += config.makeConfig( 'Jets.JVT',
        containerName='AnaJets' )

    # disabling flavor tagging for Run 4, as the configuration just
    # refuses to work on that
    if geometry is not LHCPeriod.Run4:

        btagger = "GN2v01"
        btagWP = "FixedCutBEff_65"
        configSeq += config.makeConfig( 'Jets.FlavourTagging',
                                        containerName='AnaJets',
                                        selectionName='ftag' )
        configSeq.setOptionValue ('.noEffSF', True)
        configSeq.setOptionValue ('.btagger', btagger)
        configSeq.setOptionValue ('.btagWP', btagWP)

        btagWP = "Continuous"
        configSeq += config.makeConfig( 'Jets.FlavourTagging',
                                        containerName='AnaJets')
        configSeq.setOptionValue ('.btagger', btagger)
        configSeq.setOptionValue ('.btagWP', btagWP)
        configSeq.setOptionValue ('.saveScores', 'All')

        configSeq += config.makeConfig( 'Jets.FlavourTaggingEventSF',
                                        containerName='AnaJets.baselineJvt')
        configSeq.setOptionValue ('.btagger', btagger)
        configSeq.setOptionValue ('.btagWP', btagWP)

    if largeRJets :
        configSeq += config.makeConfig( 'Jets',
            containerName='AnaLargeRJets',
            jetCollection='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' )
        outputContainers['larger_jet_'] = 'OutLargeRJets'
        configSeq.setOptionValue ('.recalibratePhyslite', False)


    # Include, and then set up the electron analysis algorithm sequence:
    likelihood = True
    configSeq += config.makeConfig ('Electrons',
        containerName='AnaElectrons' )
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Electrons.WorkingPoint',
        containerName='AnaElectrons',
        selectionName='loose')
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.noEffSF', geometry is LHCPeriod.Run2)
    if likelihood:
        configSeq.setOptionValue ('.identificationWP', 'LooseBLayerLH')
    else:
        configSeq.setOptionValue ('.identificationWP', 'LooseDNN')
    configSeq.setOptionValue ('.isolationWP', 'Loose_VarRad')
    configSeq.setOptionValue ('.writeTrackD0Z0', True)

    configSeq += config.makeConfig ('Electrons.PtEtaSelection',
        containerName='AnaElectrons')
    configSeq.setOptionValue ('.minPt', electronMinPt)


    # Include, and then set up the photon analysis algorithm sequence:
    configSeq += config.makeConfig ('Photons',
        containerName='AnaPhotons' )
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.recomputeIsEM', False)
    configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Photons.WorkingPoint',
        containerName='AnaPhotons',
        selectionName='tight')
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.qualityWP', 'Tight')
    configSeq.setOptionValue ('.isolationWP', 'FixedCutTight')
    configSeq.setOptionValue ('.recomputeIsEM', False)

    configSeq += config.makeConfig ('Photons.PtEtaSelection',
        containerName='AnaPhotons')
    configSeq.setOptionValue ('.minPt', photonMinPt)


    # set up the muon analysis algorithm sequence:
    configSeq += config.makeConfig ('Muons',
        containerName='AnaMuons')
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Muons.WorkingPoint',
        containerName='AnaMuons',
        selectionName='medium')
    configSeq.setOptionValue ('.quality', 'Medium')
    configSeq.setOptionValue ('.isolation', 'Loose_VarRad')
    configSeq.setOptionValue ('.writeTrackD0Z0', True)

    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # configSeq += config.makeConfig ('Muons.Selection', 'AnaMuons.tight')
    # configSeq.setOptionValue ('.quality', 'Tight')
    # configSeq.setOptionValue ('.isolation', 'Loose_VarRad')

    # Include, and then set up the tau analysis algorithm sequence:
    configSeq += config.makeConfig ('TauJets',
        containerName='AnaTauJets')
    configSeq.setOptionValue ('.decorateTruth', True)
    configSeq += config.makeConfig ('TauJets.WorkingPoint',
        containerName='AnaTauJets',
        selectionName='tight')
    configSeq.setOptionValue ('.quality', 'Tight')

    configSeq += config.makeConfig('TauJets.TriggerSF')
    configSeq.setOptionValue('.containerName', 'AnaTauJets')
    configSeq.setOptionValue('.tauID', 'Tight')
    configSeq.setOptionValue('.triggerChainsPerYear', tauTriggerChainsSF)


    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaJets')
    if largeRJets:
        configSeq += config.makeConfig('SystObjectLink', containerName='AnaLargeRJets')
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaElectrons')
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaPhotons')
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaMuons')
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaTauJets')


    # Particle-level objects
    configSeq += config.makeConfig ('PL_Electrons',
        containerName='TruthElectrons')
    configSeq += config.makeConfig ('PL_Electrons.PtEtaSelection',
        containerName='TruthElectrons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.useDressedProperties', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Muons',
        containerName='TruthMuons')
    configSeq += config.makeConfig ('PL_Muons.PtEtaSelection',
        containerName='TruthMuons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.useDressedProperties', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Neutrinos')
    configSeq.setOptionValue ('.skipOnData', True)

    configSeq += config.makeConfig ('PL_Jets',
        containerName='AntiKt4TruthDressedWZJets')
    configSeq += config.makeConfig ('PL_Jets.PtEtaSelection',
        containerName='AntiKt4TruthDressedWZJets')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Taus',
        containerName='TruthTaus')
    configSeq += config.makeConfig ('PL_Taus.PtEtaSelection',
        containerName='TruthTaus')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.minPt', 20e3)

    configSeq += config.makeConfig ('PL_Photons',
        containerName='TruthPhotons')
    configSeq += config.makeConfig ('PL_Photons.PtEtaSelection',
        containerName='TruthPhotons')
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
    configSeq += config.makeConfig ('MissingET',
        containerName='AnaMET')
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
    configSeq += config.makeConfig ('ObjectCutFlow',
        containerName='AnaJets',
        selectionName='jvt')
    configSeq += config.makeConfig ('ObjectCutFlow',
        containerName='AnaElectrons',
        selectionName='loose')
    configSeq += config.makeConfig ('ObjectCutFlow',
        containerName='AnaPhotons',
        selectionName='tight')
    configSeq += config.makeConfig ('ObjectCutFlow',
        containerName='AnaMuons',
        selectionName='medium')
    configSeq += config.makeConfig ('ObjectCutFlow',
        containerName='AnaTauJets',
        selectionName='tight')


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
    configSeq += config.makeConfig ('Thinning',
        containerName='AnaElectrons')
    configSeq.setOptionValue ('.selectionName', 'loose')
    configSeq.setOptionValue ('.outputName', 'OutElectrons')
    configSeq += config.makeConfig ('Thinning',
        containerName='AnaPhotons')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.outputName', 'OutPhotons')
    configSeq += config.makeConfig ('Thinning',
        containerName='AnaMuons')
    configSeq.setOptionValue ('.selectionName', 'medium')
    configSeq.setOptionValue ('.outputName', 'OutMuons')
    configSeq += config.makeConfig ('Thinning',
        containerName='AnaTauJets')
    configSeq.setOptionValue ('.selectionName', 'tight')
    configSeq.setOptionValue ('.outputName', 'OutTauJets')
    configSeq += config.makeConfig ('Thinning',
        containerName='AnaJets')
    configSeq.setOptionValue ('.outputName', 'OutJets')
    if largeRJets :
        configSeq += config.makeConfig ('Thinning',
            containerName='AnaLargeRJets')
        configSeq.setOptionValue ('.outputName', 'OutLargeRJets')
    configSeq += config.makeConfig ('Thinning',
        containerName='TruthElectrons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthElectrons')
    configSeq += config.makeConfig ('Thinning',
        containerName='TruthPhotons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthPhotons')
    configSeq += config.makeConfig ('Thinning',
        containerName='TruthMuons')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthMuons')
    configSeq += config.makeConfig ('Thinning',
        containerName='TruthTaus')
    configSeq.setOptionValue ('.skipOnData', True)
    configSeq.setOptionValue ('.outputName', 'OutTruthTaus')
    configSeq += config.makeConfig ('Thinning',
        containerName='AntiKt4TruthDressedWZJets')
    configSeq.setOptionValue ('.outputName', 'OutTruthJets')
    configSeq.setOptionValue ('.skipOnData', True)

    configSeq += config.makeConfig ('Output')
    configSeq.setOptionValue ('.treeName', 'analysis')
    configSeq.setOptionValue ('.vars', vars)
    configSeq.setOptionValue ('.metVars', metVars)
    configSeq.setOptionValue ('.containers', outputContainers)
    configSeq.setOptionValue ('.containersOnlyForMC', outputContainersForMC)

    disable_commands = []
    configSeq.setOptionValue ('.commands', disable_commands)

    # return configSeq for unit test
    if returnConfigSeq:
        return configSeq

    configAccumulator = ConfigAccumulator (algSeq, dataType, isPhyslite, geometry, autoconfigFromFlags=autoconfigFromFlags, noSystematics=noSystematics)
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
        ca = makeSequenceText(yamlPath, dataType, algSeq, geometry=geometry,
                              isPhyslite=isPhyslite,
                              autoconfigFromFlags=autoconfigFromFlags,
                              noSystematics=noSystematics)

    if ca is not None:
        return ca
    else:
        return algSeq
