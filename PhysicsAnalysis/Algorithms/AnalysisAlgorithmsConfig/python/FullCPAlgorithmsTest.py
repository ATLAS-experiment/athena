# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool
from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import pileupConfigFiles
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator, DataType

# Config:
triggerChainsPerYear = {
    '2015': ['HLT_e24_lhmedium_L1EM20VH || HLT_e60_lhmedium || HLT_e120_lhloose', 'HLT_mu20_iloose_L1MU15 || HLT_mu50', 'HLT_2g20_tight'],
    '2016': ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_mu26_ivarmedium || HLT_mu50', 'HLT_g35_loose_g25_loose'],
    '2017': ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_2g22_tight_L12EM15VHI', 'HLT_mu50'],
    '2018': ['HLT_e26_lhtight_nod0_ivarloose || HLT_e60_lhmedium_nod0 || HLT_e140_lhloose_nod0', 'HLT_g35_medium_g25_medium_L12EM20VH', 'HLT_mu26_ivarmedium', 'HLT_2mu14'],
    '2022': ['HLT_e26_lhtight_ivarloose_L1EM22VHI || HLT_e60_lhmedium_L1EM22VHI || HLT_e140_lhloose_L1EM22VHI'],
}
triggerChains = [
    'HLT_2mu14',
    'HLT_mu20_mu8noL1',
    'HLT_2e17_lhvloose_nod0'
]

# Example cuts used for event selection algorithm test
exampleSelectionCuts = {
  'SUBcommon': """
JET_N_BTAG >= 2
JET_N 25000 >= 4
MET >= 20000
SAVE
""",
  'ejets': """
IMPORT SUBcommon
EL_N 25000 >= 1
EL_N loose 25000 == 1
MU_N 5000 == 0
MWT < 170000
MET+MWT > 40000
SAVE
""",
  'mujets': """
IMPORT SUBcommon
EL_N 5000 == 0
MU_N medium 25000 > 0
SAVE
"""
}

electronMinPt = 10e3
electronMaxEta = None
photonMinPt = 10e3
photonMaxEta = None
muonMinPt = None
muonMaxEta = None
tauMinPt = None
tauMaxEta = None
jetMinPt = None
jetMaxEta = None

def addOutputCopyAlgorithms (algSeq, ca, postfix, inputContainer, outputContainer, selection) :
    """add a uniformly filtered set of deep copies based on the
    systematics dependent selection"""

    if postfix[0] != '_' :
        postfix = '_' + postfix

    if selection != '' :
        unionalg = createAlgorithm( 'CP::AsgUnionSelectionAlg', 'UnionSelectionAlg' + postfix)
        unionalg.preselection = selection
        unionalg.particles = inputContainer
        unionalg.selectionDecoration = 'outputSelect'
        if ca:
            ca.addEventAlgo(unionalg, algSeq.name if algSeq else None)
        else:
            algSeq += unionalg

    copyalg = createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'DeepCopyAlg' + postfix )
    copyalg.input = inputContainer
    copyalg.output = outputContainer
    if selection != '' :
        copyalg.selection = ['outputSelect']
    else :
        copyalg.selection = []
    copyalg.deepCopy = False
    if ca:
        ca.addEventAlgo(copyalg, algSeq.name if algSeq else None)
    else:
        algSeq += copyalg


def makeSequenceOld (dataType, algSeq, forCompare, isPhyslite, noSystematics, forceEGammaFullSimConfig=False) :

    vars = []
    metVars = []

    from AnaAlgorithm.DualUseConfig import isAthena, useComponentAccumulator
    if isAthena and useComponentAccumulator:
        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
        ca = ComponentAccumulator()
        caSeq = None
        if algSeq:
            ca.addSequence(algSeq)
            caSeq = algSeq.name
    else:
        ca = None

    from AsgAnalysisAlgorithms.CommonServiceSequence import makeCommonServiceSequence
    makeCommonServiceSequence (algSeq, runSystematics = not noSystematics, ca=ca)

    # Include, and then set up the pileup analysis sequence:
    if not isPhyslite :
        campaign, files, prwfiles, lumicalcfiles = None, None, None, None
        useDefaultConfig = False

        # need to convert dataType from string to enum for the call to pileupConfigFiles
        prwfiles, lumicalcfiles = pileupConfigFiles( {'data': DataType.Data, 'mc': DataType.FullSim, 'afii': DataType.FastSim}.get(dataType, None) )

        from AsgAnalysisAlgorithms.PileupAnalysisSequence import \
            makePileupAnalysisSequence
        pileupSequence = makePileupAnalysisSequence(
            dataType,
            campaign=campaign,
            files=files,
            useDefaultConfig=useDefaultConfig,
            userPileupConfigs=prwfiles,
            userLumicalcFiles=lumicalcfiles,
        )
        pileupSequence.configure( inputName = {}, outputName = {} )

        # Add the pileup sequence to the job:
        if ca:
            for element in pileupSequence.getGaudiConfig2Algorithms():
                ca.addEventAlgo(element, caSeq)
        else:
            algSeq += pileupSequence

    # Add the pileup sequence to the job:
    vars += [ 'EventInfo.runNumber     -> runNumber',
             'EventInfo.eventNumber   -> eventNumber', ]
    if dataType != 'data':
        vars += [ 'EventInfo.mcChannelNumber -> mcChannelNumber' ]
    if not isPhyslite and dataType in ['mc']:
        vars += [ 'EventInfo.PileupWeight_%SYS% -> weight_pileup_%SYS%' ]
    if not isPhyslite and dataType in ['mc', 'afii']:
        vars += [ 'EventInfo.beamSpotWeight -> weight_beamspot' ]


    # Skip events with no primary vertex:
    from AsgAnalysisAlgorithms.EventSelectionAnalysisSequence import makeEventSelectionAnalysisSequence
    eventCleaningSequence = makeEventSelectionAnalysisSequence(
        dataType,
        runPrimaryVertexSelection=True,
        runEventCleaning=True,
    )
    eventCleaningSequence.configure( inputName = {}, outputName = {} )
    if ca:
        for element in eventCleaningSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += eventCleaningSequence


    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetContainer = 'AntiKt4EMPFlowJets'
    if isPhyslite :
        input = 'AnalysisJets'
    else :
        input = jetContainer
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer,
                                           runJvtUpdate = False, runNNJvtUpdate = True,
                                           enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False,
                                           runGhostMuonAssociation = not isPhyslite)

    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
    btagger = "DL1dv01"
    btagWP = "FixedCutBEff_60"
    makeFTagAnalysisSequence( jetSequence, dataType, jetContainer, noEfficiency = False,
                              enableCutflow=True, btagger = btagger, btagWP = btagWP, kinematicSelection = False )
    vars += [
        'OutJets_NOSYS.ftag_select_' + btagger + '_' + btagWP + ' -> jet_ftag_select',
    ]
    if dataType != 'data' and not forCompare:
        vars += [
            'OutJets_%SYS%.ftag_effSF_' + btagger + '_' + btagWP + '_%SYS% -> jet_ftag_eff_%SYS%'
        ]

    jetSequence.configure( inputName = input, outputName = 'AnaJets_%SYS%' )

    # Add the sequences to the job:
    if ca:
        for element in jetSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += jetSequence
    vars += ['OutJets_%SYS%.pt  -> jet_pt_%SYS%',
             'OutJets_NOSYS.phi -> jet_phi',
             'OutJets_NOSYS.eta -> jet_eta'
            ]
    if not forCompare:
        vars += ['OutJets_%SYS%.jvt_selection -> jet_select_jvt_%SYS%']

    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    if isPhyslite :
        input = 'AnalysisMuons'
    else :
        input = 'Muons'

    muonSequenceMedium = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
                                                   workingPoint = 'Medium.Loose_VarRad', postfix = 'medium',
                                                   enableCutflow=True, enableKinematicHistograms=True, ptSelectionOutput = True )
    muonSequenceMedium.configure( inputName = input,
                                  outputName = 'AnaMuons_%SYS%' )
    if ca:
        for element in muonSequenceMedium.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += muonSequenceMedium

    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # muonSequenceTight = makeMuonAnalysisSequence( dataType, deepCopyOutput = False, shallowViewOutput = False,
    #                                               workingPoint = 'Tight.Loose_VarRad', postfix = 'tight',
    #                                               enableCutflow=True, enableKinematicHistograms=True, ptSelectionOutput = True )
    # muonSequenceTight.removeStage ("calibration")
    # # FIX ME: the current version of the `MuonSelectionTool` doesn't work
    # # on the current version of PHYSLITE, and needs a new PHYSLITE production
    # # campaign
    # muonSequenceTight.configure( inputName = 'AnaMuonsMedium_%SYS%',
    #                              outputName = 'AnaMuons_%SYS%')
    # if ca:
    #     for element in muonSequenceTight.getGaudiConfig2Algorithms():
    #         ca.addEventAlgo(element, caSeq)
    # else:
    #     algSeq += muonSequenceTight

    vars += [ 'OutMuons_NOSYS.eta -> mu_eta',
              'OutMuons_NOSYS.phi -> mu_phi',
              'OutMuons_%SYS%.pt  -> mu_pt_%SYS%',
              'OutMuons_NOSYS.charge -> mu_charge',]
    if not forCompare:
        vars += [ 'OutMuons_%SYS%.baselineSelection_medium -> mu_select_medium_%SYS%', ]
                #'OutMuons_%SYS%.baselineSelection_tight  -> mu_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutMuons_%SYS%.muon_effSF_medium_%SYS% -> mu_reco_effSF_medium_%SYS%', ]
                  #'OutMuons_%SYS%.muon_effSF_tight_%SYS% -> mu_reco_effSF_tight_%SYS%', ]

    # Include, and then set up the electron analysis sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import \
        makeElectronAnalysisSequence
    if isPhyslite :
        input = 'AnalysisElectrons'
    else :
        input = 'Electrons'
    likelihood = True
    recomputeLikelihood=False
    if likelihood:
        workingpoint = 'LooseBLayerLHElectron.Loose_VarRad'
    else:
        workingpoint = 'LooseDNNElectron.Loose_VarRad'
    electronSequence = makeElectronAnalysisSequence(
        dataType, workingpoint, postfix = 'loose',
        recomputeLikelihood=recomputeLikelihood, enableCutflow=True,
        enableKinematicHistograms=True, shallowViewOutput = False,
        forceFullSimConfig=forceEGammaFullSimConfig)
    electronSequence.configure( inputName = input,
                                outputName = 'AnaElectrons_%SYS%' )
    if ca:
        for element in electronSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += electronSequence

    if not(forceEGammaFullSimConfig and dataType=='afii' and forCompare):
        vars += [ 'OutElectrons_%SYS%.pt  -> el_pt_%SYS%',
                  'OutElectrons_NOSYS.phi -> el_phi',
                  'OutElectrons_NOSYS.eta -> el_eta',
                  'OutElectrons_NOSYS.charge -> el_charge', ]
        if not forCompare:
            vars += [ 'OutElectrons_%SYS%.baselineSelection_loose -> el_select_loose_%SYS%', ]
        if dataType != 'data' and not forCompare:
            vars += [ 'OutElectrons_%SYS%.effSF_loose_%SYS% -> el_effSF_loose_%SYS%',]


    # Include, and then set up the photon analysis sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import \
        makePhotonAnalysisSequence
    if isPhyslite :
        input = 'AnalysisPhotons'
    else :
        input = 'Photons'
    photonSequence = makePhotonAnalysisSequence(
        dataType, 'Tight.FixedCutTight', postfix = 'tight',
        recomputeIsEM=False, enableCutflow=True,
        enableKinematicHistograms=True, shallowViewOutput = False,
        forceFullSimConfig=forceEGammaFullSimConfig)
    photonSequence.configure( inputName = input,
                              outputName = 'AnaPhotons_%SYS%' )
    if ca:
        for element in photonSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += photonSequence
    vars += [ 'OutPhotons_%SYS%.pt  -> ph_pt_%SYS%',
              'OutPhotons_NOSYS.phi -> ph_phi',
              'OutPhotons_NOSYS.eta -> ph_eta', ]
    if not forCompare:
        vars += [ 'OutPhotons_%SYS%.baselineSelection_tight -> ph_select_tight_%SYS%', ]
    if dataType != 'data' and not forCompare:
        vars += [ 'OutPhotons_%SYS%.ph_effSF_tight_%SYS% -> ph_effSF_tight_%SYS%', ]


    # Include, and then set up the tau analysis algorithm sequence:
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence
    if isPhyslite :
        input = 'AnalysisTauJets'
    else :
        input = 'TauJets'
    tauSequence = makeTauAnalysisSequence( dataType, 'Tight', postfix = 'tight',
                                           enableCutflow=True, enableKinematicHistograms=True, shallowViewOutput = False )
    tauSequence.configure( inputName = input, outputName = 'AnaTauJets_%SYS%' )

    # Add the sequence to the job:
    if ca:
        for element in tauSequence.getGaudiConfig2PublicTools():
            ca.addPublicTool(element)
        for element in tauSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += tauSequence
    vars += [ 'OutTauJets_%SYS%.pt  -> tau_pt_%SYS%',
              'OutTauJets_NOSYS.phi -> tau_phi',
              'OutTauJets_NOSYS.eta -> tau_eta',
              'OutTauJets_NOSYS.charge -> tau_charge',
              'OutTauJets_NOSYS.NNDecayMode -> tau_NNDecayMode',
            ]
    if not forCompare:
        vars += [ 'OutTauJets_%SYS%.baselineSelection_tight -> tau_select_tight_%SYS%', ]
    if dataType != 'data':
        vars += [ 'OutTauJets_%SYS%.tau_effSF_tight_%SYS% -> tau_effSF_tight_%SYS%', ]



    # FIX ME: temporarily disabled until di-taus are supported in R22
    # # Include, and then set up the tau analysis algorithm sequence:
    # from TauAnalysisAlgorithms.DiTauAnalysisSequence import makeDiTauAnalysisSequence
    # diTauSequence = makeDiTauAnalysisSequence( dataType, 'Tight', postfix = 'tight' )
    # diTauSequence.configure( inputName = 'DiTauJets', outputName = 'AnaDiTauJets_%SYS%' )

    # Add the sequence to the job:
    # disabling this, the standard test files don't have DiTauJets
    # algSeq += diTauSequence


    # set up pt-eta selection for all the object types
    # currently disabling most cuts, but leaving these as placeholders
    # the cuts I have are mostly to silence MET building warnings

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserElectronsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if electronMinPt is not None :
        selalg.selectionTool.minPt = electronMinPt
    if electronMaxEta is not None :
        selalg.selectionTool.maxEta = electronMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaElectrons_%SYS%'
    if ca:
        ca.addEventAlgo(selalg, caSeq)
    else:
        algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserPhotonsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if photonMinPt is not None :
        selalg.selectionTool.minPt = photonMinPt
    if photonMaxEta is not None :
        selalg.selectionTool.maxEta = photonMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaPhotons_%SYS%'
    if ca:
        ca.addEventAlgo(selalg, caSeq)
    else:
        algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserMuonsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if muonMinPt is not None :
        selalg.selectionTool.minPt = muonMinPt
    if muonMaxEta is not None :
        selalg.selectionTool.maxEta = muonMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaMuons_%SYS%'
    if ca:
        ca.addEventAlgo(selalg, caSeq)
    else:
        algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserTauJetsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if tauMinPt is not None :
        selalg.selectionTool.minPt = tauMinPt
    if tauMaxEta is not None :
        selalg.selectionTool.maxEta = tauMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaTauJets_%SYS%'
    if ca:
        ca.addEventAlgo(selalg, caSeq)
    else:
        algSeq += selalg

    selalg = createAlgorithm( 'CP::AsgSelectionAlg', 'UserJetsSelectionAlg' )
    addPrivateTool( selalg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    if jetMinPt is not None :
        selalg.selectionTool.minPt = jetMinPt
    if jetMaxEta is not None :
        selalg.selectionTool.maxEta = jetMaxEta
    selalg.selectionDecoration = 'selectPtEta'
    selalg.particles = 'AnaJets_%SYS%'
    if ca:
        ca.addEventAlgo(selalg, caSeq)
    else:
        algSeq += selalg



    # Now make view containers for the inputs to the met calculation
    metInputs = { 'jets'      : 'AnaJets_%SYS%',
                  'taus'      : 'AnaTauJets_%SYS%',
                  'muons'     : 'AnaMuons_%SYS%',
                  'electrons' : 'AnaElectrons_%SYS%',
                  'photons'   : 'AnaPhotons_%SYS%' }
    # Include, and then set up the met analysis algorithm sequence:
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence
    if isPhyslite :
        metSuffix = 'AnalysisMET'
    else :
        metSuffix = jetContainer[:-4]
    metSequence = makeMetAnalysisSequence(
            dataType,
            metSuffix=metSuffix,
            electronsSelection = "selectPtEta && baselineSelection_loose,as_char",
            photonsSelection = "selectPtEta && baselineSelection_tight,as_char",
            muonsSelection = "selectPtEta && baselineSelection_medium,as_char",
            tausSelection = "selectPtEta && baselineSelection_tight,as_char" )
    metSequence.configure( inputName = metInputs,
                           outputName = 'AnaMET_%SYS%' )

    # Add the sequence to the job:
    if ca:
        for element in metSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += metSequence
    metVars += [
        'AnaMET_%SYS%.met   -> met_met_%SYS%',
        'AnaMET_%SYS%.phi   -> met_phi_%SYS%',
        'AnaMET_%SYS%.sumet -> met_sumet_%SYS%',
    ]


    # Make view containers holding as inputs for OR
    orInputs = {
            'photons'   : 'AnaPhotons_%SYS%',
            'muons'     : 'AnaMuons_%SYS%',
            'jets'      : 'AnaJets_%SYS%',
            'taus'      : 'AnaTauJets_%SYS%'
        }

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORElectronsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_loose,as_char'
    selectalg.particles = 'AnaElectrons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    if ca:
        ca.addEventAlgo(selectalg, caSeq)
    else:
        algSeq += selectalg
    orInputs['electrons'] = 'AnaElectrons_%SYS%'

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORPhotonsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_tight,as_char'
    selectalg.particles = 'AnaPhotons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    if ca:
        ca.addEventAlgo(selectalg, caSeq)
    else:
        algSeq += selectalg

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORMuonsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_medium,as_char'
    selectalg.particles = 'AnaMuons_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    if ca:
        ca.addEventAlgo(selectalg, caSeq)
    else:
        algSeq += selectalg

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORTauJetsSelectAlg' )
    selectalg.preselection = 'selectPtEta&&baselineSelection_tight,as_char'
    selectalg.particles = 'AnaTauJets_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    if ca:
        ca.addEventAlgo(selectalg, caSeq)
    else:
        algSeq += selectalg

    selectalg = createAlgorithm( 'CP::AsgSelectionAlg','ORJetsSelectAlg' )
    selectalg.preselection = 'selectPtEta'
    selectalg.particles = 'AnaJets_%SYS%'
    selectalg.selectionDecoration = 'preselectOR,as_char'
    if ca:
        ca.addEventAlgo(selectalg, caSeq)
    else:
        algSeq += selectalg


    # Include, and then set up the overlap analysis algorithm sequence:
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import \
        makeOverlapAnalysisSequence
    overlapSequence = makeOverlapAnalysisSequence( dataType, doTaus=True, enableCutflow=True, shallowViewOutput = False, inputLabel = 'preselectOR', outputLabel = 'passesOR' )
    overlapSequence.configure(
        inputName = {
            'electrons' : 'AnaElectrons_%SYS%',
            'photons'   : 'AnaPhotons_%SYS%',
            'muons'     : 'AnaMuons_%SYS%',
            'jets'      : 'AnaJets_%SYS%',
            'taus'      : 'AnaTauJets_%SYS%'
        },
        outputName = { } )

    if ca:
        for element in overlapSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += overlapSequence
    vars += [
        'OutJets_%SYS%.passesOR_%SYS% -> jet_select_or_%SYS%',
        'OutPhotons_%SYS%.passesOR_%SYS% -> ph_select_or_%SYS%',
        'OutMuons_%SYS%.passesOR_%SYS% -> mu_select_or_%SYS%',
        'OutTauJets_%SYS%.passesOR_%SYS% -> tau_select_or_%SYS%',
    ]
    if not(forceEGammaFullSimConfig and dataType=='afii' and forCompare):
        vars += ['OutElectrons_%SYS%.passesOR_%SYS% -> el_select_or_%SYS%']

    if dataType != 'data' :
        # Include, and then set up the generator analysis sequence:
        from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import \
            makeGeneratorAnalysisSequence
        generatorSequence = makeGeneratorAnalysisSequence( dataType, saveCutBookkeepers=True, runNumber=284500, cutBookkeepersSystematics=True )
        if ca:
            for element in generatorSequence.getGaudiConfig2Algorithms():
                ca.addEventAlgo(element, caSeq)
        else:
            algSeq += generatorSequence
        vars += [ 'EventInfo.generatorWeight_%SYS% -> weight_mc_%SYS%', ]


    # disabling comparisons for triggers, because the config blocks do a lot
    # more than the sequences
    if not forCompare :
        # Include, and then set up the trigger analysis sequence:
        from TriggerAnalysisAlgorithms.TriggerAnalysisSequence import \
            makeTriggerAnalysisSequence
        triggerSequence = makeTriggerAnalysisSequence( dataType, triggerChains=triggerChains, noFilter=True )
        if ca:
            for element in triggerSequence.getGaudiConfig2Algorithms():
                ca.addEventAlgo(element, caSeq)
        else:
            algSeq += triggerSequence
        vars += ['EventInfo.trigPassed_' + t + ' -> trigPassed_' + t for t in triggerChains]



    # make filtered output containers

    addOutputCopyAlgorithms (algSeq, ca, 'Electrons', 'AnaElectrons_%SYS%', 'OutElectrons_%SYS%',
                             'selectPtEta&&baselineSelection_loose,as_char&&passesOR_%SYS%,as_char')
    addOutputCopyAlgorithms (algSeq, ca, 'Photons', 'AnaPhotons_%SYS%', 'OutPhotons_%SYS%',
                             'selectPtEta&&baselineSelection_tight,as_char&&passesOR_%SYS%,as_char')
    addOutputCopyAlgorithms (algSeq, ca, 'Muons', 'AnaMuons_%SYS%', 'OutMuons_%SYS%',
                             'selectPtEta&&baselineSelection_medium,as_char&&passesOR_%SYS%,as_char')
    addOutputCopyAlgorithms (algSeq, ca, 'TauJets', 'AnaTauJets_%SYS%', 'OutTauJets_%SYS%',
                             'selectPtEta&&baselineSelection_tight,as_char&&passesOR_%SYS%,as_char')
    addOutputCopyAlgorithms (algSeq, ca, 'Jets', 'AnaJets_%SYS%', 'OutJets_%SYS%',
                             'selectPtEta&&passesOR_%SYS%,as_char')

    # Include, and then set up the jet analysis algorithm sequence:
    from JetAnalysisAlgorithms.JetJvtAnalysisSequence import makeJetJvtAnalysisSequence
    jvtSequence = makeJetJvtAnalysisSequence( dataType, jetContainer, enableCutflow=True, shallowViewOutput = False,
                                              preselection='selectPtEta&&passesOR_%SYS%,as_char' )
    jvtSequence.configure( inputName = { 'jets'      : 'OutJets_%SYS%' },
                           outputName = {  } )

    # Add the sequences to the job:
    if ca:
        for element in jvtSequence.getGaudiConfig2Algorithms():
            ca.addEventAlgo(element, caSeq)
    else:
        algSeq += jvtSequence

    if dataType != 'data' :
        vars += [ 'OutJets_%SYS%.jvt_effSF_%SYS% -> jet_jvtEfficiency_%SYS%', ]
        vars += [
            'EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%',
            # 'EventInfo.fjvt_effSF_%SYS% -> weight_fjvt_effSF_%SYS%',
            # 'OutJets_%SYS%.fjvt_effSF_NOSYS -> jet_fjvtEfficiency_%SYS%',
        ]

    # Add an ntuple dumper algorithm:
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'analysis'
    # the auto-flush setting still needs to be figured out
    #treeMaker.TreeAutoFlush = 0
    if ca:
        ca.addEventAlgo(treeMaker, caSeq)
    else:
        algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
    ntupleMaker.TreeName = 'analysis'
    ntupleMaker.Branches = vars
    # ntupleMaker.OutputLevel = 2  # For output validation
    if ca:
        ca.addEventAlgo(ntupleMaker, caSeq)
    else:
        algSeq += ntupleMaker
    if len (metVars) > 0:
        ntupleMaker = createAlgorithm( 'CP::AsgxAODMetNTupleMakerAlg', 'MetNTupleMaker' )
        ntupleMaker.TreeName = 'analysis'
        ntupleMaker.Branches = metVars
        #ntupleMaker.OutputLevel = 2  # For output validation
        if ca:
            ca.addEventAlgo(ntupleMaker, caSeq)
        else:
            algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'analysis'
    if ca:
        ca.addEventAlgo(treeFiller, caSeq)
    else:
        algSeq += treeFiller

    return ca




def makeSequenceBlocks (dataType, algSeq, forCompare, isPhyslite,
                        geometry=None, autoconfigFromFlags=None, noSystematics=None, onlyNominalOR=False,  forceEGammaFullSimConfig=False) :

    vars = []
    metVars = []

    # it seems the right containers are in the test input files so far
    largeRJets = False
    # there are no track jets in PHYSLITE, or in the sequence configuration
    trackJets = not isPhyslite and not forCompare

    if autoconfigFromFlags is not None:
        if geometry is None:
            geometry = autoconfigFromFlags.GeoModel.Run

    configSeq = ConfigSequence ()

    outputContainers = {'mu_' : 'OutMuons',
                        'ph_' : 'OutPhotons',
                        'tau_': 'OutTauJets',
                        'jet_': 'OutJets',
                        'met_': 'AnaMET',
                        ''    : 'EventInfo'}
    if not(forceEGammaFullSimConfig and dataType=='afii' and forCompare):
        outputContainers['el_'] = 'OutElectrons'

    # create factory object to build block configurations
    from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
    config = ConfigFactory()

    # noSystematics is passed in block from config accumulator
    configSeq += config.makeConfig('CommonServices')
    configSeq.setOptionValue('.systematicsHistogram', 'systematicsList')
    if forCompare:
        configSeq.setOptionValue('.filterSystematics', "^(?:(?!PseudoData).)*$")

    # FIXME: this should probably be run on PHYSLITE, but the test fails with:
    #   overran integrated luminosity for RunNumber=363262 (0.000000 vs 0.000000)
    if not isPhyslite:
        configSeq += config.makeConfig('PileupReweighting')
        if dataType == 'afii' and forCompare:
            prwfiles, lumicalcfiles = pileupConfigFiles( {'data': DataType.Data, 'mc': DataType.FullSim, 'afii': DataType.FastSim}.get(dataType, None) )
            configSeq.setOptionValue('.userPileupConfigs', prwfiles)
            configSeq.setOptionValue('.userLumicalcFiles', lumicalcfiles)
            configSeq.setOptionValue('.useDefaultConfig', False)
    else :
        # ideally the above block would work both for PHYS and PHYSLITE, but
        # since we disabled it we need to add these variables manually.
        vars += [ 'EventInfo.runNumber     -> runNumber',
                  'EventInfo.eventNumber   -> eventNumber',
                  'EventInfo.mcChannelNumber -> mcChannelNumber']


    # Skip events with no primary vertex,
    # and perform loose jet cleaning
    configSeq += config.makeConfig ('EventCleaning')
    configSeq.setOptionValue ('.runEventCleaning', True)

    # Include, and then set up the jet analysis algorithm sequence:
    configSeq += config.makeConfig( 'Jets',
        containerName='AnaJets',
        jetCollection='AntiKt4EMPFlowJets')
    configSeq.setOptionValue ('.runJvtUpdate', False )
    configSeq.setOptionValue ('.runNNJvtUpdate', True )
    if not forCompare :
        configSeq.setOptionValue ('.recalibratePhyslite', False)

    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaJets')

    btagger = "DL1dv01"
    btagWP = "FixedCutBEff_60"
    configSeq += config.makeConfig( 'Jets.FlavourTagging',
        containerName='AnaJets',
        selectionName='ftag' )
    configSeq.setOptionValue ('.noEffSF', forCompare)
    configSeq.setOptionValue ('.btagger', btagger)
    configSeq.setOptionValue ('.btagWP', btagWP)
    
    configSeq += config.makeConfig( 'Jets.JVT',
        containerName='AnaJets' )

    if not forCompare:
        configSeq += config.makeConfig( 'Jets.FlavourTaggingEventSF',
            containerName='AnaJets.baselineJvt',
            selectionName='ftag')
        configSeq.setOptionValue ('.btagger', btagger)
        configSeq.setOptionValue ('.btagWP', btagWP)

    if largeRJets :
        configSeq += config.makeConfig( 'Jets',
            containerName='AnaLargeRJets',
            jetCollection='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' )
        configSeq.setOptionValue ('.postfix', 'largeR_jets' )
        outputContainers['larger_jet_'] = 'OutLargeRJets'

        # Add systematic object links
        configSeq += config.makeConfig('SystObjectLink', containerName='AnaLargeRJets')
        if not forCompare :
            configSeq.setOptionValue ('.recalibratePhyslite', False)

    if trackJets :
        configSeq += config.makeConfig( 'Jets',
            containerName='AnaTrackJets',
            jetCollection='AntiKtVR30Rmax4Rmin02PV0TrackJets' )
        configSeq.setOptionValue ('.postfix', 'track_jets' )
        outputContainers['track_jet_'] = 'OutTrackJets'


    # Include, and then set up the electron analysis algorithm sequence:

    likelihood = True
    recomputeLikelihood=False
    configSeq += config.makeConfig ('Electrons',
        containerName='AnaElectrons' )
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    if not forCompare :
        configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Electrons.WorkingPoint',
        containerName='AnaElectrons',
        selectionName='loose')
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    if forCompare :
        configSeq.setOptionValue ('.noEffSF', True)
    if likelihood:
        configSeq.setOptionValue ('.likelihoodWP', 'LooseBLayerLH')
    else:
        configSeq.setOptionValue ('.likelihoodWP', 'LooseDNN')
    configSeq.setOptionValue ('.isolationWP', 'Loose_VarRad')
    configSeq.setOptionValue ('.recomputeLikelihood', recomputeLikelihood)

    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaElectrons')

    # Include, and then set up the photon analysis algorithm sequence:
    configSeq += config.makeConfig ('Photons',
        containerName='AnaPhotons' )
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    configSeq.setOptionValue ('.recomputeIsEM', False)
    if not forCompare :
        configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Photons.WorkingPoint',
        containerName='AnaPhotons',
        selectionName='tight')
    configSeq.setOptionValue ('.forceFullSimConfig', forceEGammaFullSimConfig)
    if forCompare :
        configSeq.setOptionValue ('.noEffSF', True)
    configSeq.setOptionValue ('.qualityWP', 'Tight')
    configSeq.setOptionValue ('.isolationWP', 'FixedCutTight')
    configSeq.setOptionValue ('.recomputeIsEM', False)

    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaPhotons')


    # set up the muon analysis algorithm sequence:
    configSeq += config.makeConfig ('Muons',
        containerName='AnaMuons')
    if not forCompare :
        configSeq.setOptionValue ('.recalibratePhyslite', False)
    configSeq += config.makeConfig ('Muons.WorkingPoint',
        containerName='AnaMuons',
        selectionName='medium')
    configSeq.setOptionValue ('.quality', 'Medium')
    configSeq.setOptionValue ('.isolation', 'Loose_VarRad')
    if forCompare :
        configSeq.setOptionValue ('.onlyRecoEffSF', True)
    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # configSeq += config.makeConfig ('Muons.Selection', 'AnaMuons.tight')
    # configSeq.setOptionValue ('.quality', 'Tight')
    # configSeq.setOptionValue ('.isolation', 'Loose_VarRad')

    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaMuons')


    # Include, and then set up the tau analysis algorithm sequence:
    configSeq += config.makeConfig ('TauJets',
        containerName='AnaTauJets')
    configSeq += config.makeConfig ('TauJets.WorkingPoint',
        containerName='AnaTauJets',
        selectionName='tight')
    configSeq.setOptionValue ('.quality', 'Tight')

    # Add systematic object links
    configSeq += config.makeConfig('SystObjectLink', containerName='AnaTauJets')


    if dataType is not DataType.Data :
        # Include, and then set up the generator analysis sequence:
        configSeq += config.makeConfig( 'GeneratorLevelAnalysis')
        configSeq.setOptionValue ('.saveCutBookkeepers', True)
        configSeq.setOptionValue ('.runNumber', 284500)
        configSeq.setOptionValue ('.cutBookkeepersSystematics', True)


    configSeq += config.makeConfig ('Electrons.PtEtaSelection',
        containerName='AnaElectrons')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', electronMinPt)
    configSeq.setOptionValue ('.maxEta', electronMaxEta)
    configSeq += config.makeConfig ('Photons.PtEtaSelection',
        containerName='AnaPhotons')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', photonMinPt)
    configSeq.setOptionValue ('.maxEta', photonMaxEta)
    configSeq += config.makeConfig ('Muons.PtEtaSelection',
        containerName='AnaMuons')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', muonMinPt)
    configSeq.setOptionValue ('.maxEta', muonMaxEta)
    configSeq += config.makeConfig ('TauJets.PtEtaSelection',
        containerName='AnaTauJets')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', tauMinPt)
    configSeq.setOptionValue ('.maxEta', tauMaxEta)
    configSeq += config.makeConfig ('Jets.PtEtaSelection',
        containerName='AnaJets')
    configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue ('.minPt', jetMinPt)
    configSeq.setOptionValue ('.maxEta', jetMaxEta)
    if largeRJets :
        configSeq += config.makeConfig ('Jets.PtEtaSelection',
            containerName='AnaLargeRJets')
        configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
        configSeq.setOptionValue ('.minPt', jetMinPt)
        configSeq.setOptionValue ('.maxEta', jetMaxEta)
    if trackJets :
        configSeq += config.makeConfig ('Jets.PtEtaSelection',
            containerName='AnaTrackJets')
        configSeq.setOptionValue ('.selectionDecoration', 'selectPtEta')
        configSeq.setOptionValue ('.minPt', jetMinPt)
        configSeq.setOptionValue ('.maxEta', jetMaxEta)

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
    configSeq += config.makeConfig ('ObjectCutFlow',
        containerName='AnaJets',
        selectionName='jvt')

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
    configSeq.setOptionValue ('.jets',        'AnaJets')
    configSeq.setOptionValue ('.taus',        'AnaTauJets.tight')
    configSeq.setOptionValue ('.inputLabel',  'preselectOR')
    configSeq.setOptionValue ('.outputLabel', 'passesOR' )
    configSeq.setOptionValue ('.nominalOnly', onlyNominalOR )
    if not forCompare :
        # ask to be added to the baseline selection for all objects, and to
        # provide a preselection for the objects in subsequent algorithms
        configSeq.setOptionValue ('.addToAllSelections', True)
        configSeq.setOptionValue ('.addPreselection', True)

    # Include and set up a basic run of the event selection algorithm config:
    if not forCompare:
        # configSeq += config.makeConfig( 'EventSelection', None )
        # configSeq.setOptionValue ('.electrons',   'AnaElectrons.loose')
        # configSeq.setOptionValue ('.muons',       'AnaMuons.medium')
        # configSeq.setOptionValue ('.jets',        'AnaJets')
        # configSeq.setOptionValue ('.met',         'AnaMET')
        # configSeq.setOptionValue ('.selectionCutsDict', exampleSelectionCuts)
        from EventSelectionAlgorithms.EventSelectionConfig import makeMultipleEventSelectionConfigs
        makeMultipleEventSelectionConfigs(configSeq, electrons = 'AnaElectrons.loose', muons = 'AnaMuons.medium', jets = 'AnaJets',
                                          met = 'AnaMET', btagDecoration = 'ftag_select_ftag',
                                          selectionCutsDict = exampleSelectionCuts, noFilter = True)


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
    if trackJets :
        configSeq += config.makeConfig ('Thinning',
            containerName='AnaTrackJets')
        configSeq.setOptionValue ('.outputName', 'OutTrackJets')
        
    # disabling comparisons for triggers, because the config blocks do a lot
    # more than the sequences
    if not forCompare :
        # Include, and then set up the trigger analysis sequence:
        configSeq += config.makeConfig( 'Trigger' )
        configSeq.setOptionValue ('.triggerChainsPerYear', triggerChainsPerYear )
        configSeq.setOptionValue ('.noFilter', True )
        configSeq.setOptionValue ('.electronID', 'Tight' )
        configSeq.setOptionValue ('.electronIsol', 'Tight_VarRad')
        configSeq.setOptionValue ('.photonIsol', 'TightCaloOnly')
        configSeq.setOptionValue ('.muonID', 'Tight')
        configSeq.setOptionValue ('.electrons', 'AnaElectrons' )
        configSeq.setOptionValue ('.photons', 'AnaPhotons' )
        configSeq.setOptionValue ('.muons', 'AnaMuons' )

    if not forCompare:
        configSeq += config.makeConfig ('Bootstraps')
        configSeq.setOptionValue ('.nReplicas', 2000 )
        configSeq.setOptionValue ('.runOnMC', True )

    configSeq += config.makeConfig ('Output')
    configSeq.setOptionValue ('.treeName', 'analysis')
    configSeq.setOptionValue ('.vars', vars)
    configSeq.setOptionValue ('.metVars', metVars)
    configSeq.setOptionValue ('.containers', outputContainers)
    disable_commands = []
    if forCompare:
        disable_commands += [
            'disable jet_select_baselineJvt.*',
            'disable mu_select_medium.*',
            'disable ph_select_tight.*',
            'disable tau_select_tight.*',
            ]
        if not (dataType == 'afii' and forceEGammaFullSimConfig):
            disable_commands.append('disable el_select_loose.*')
    configSeq.setOptionValue ('.commands', disable_commands)

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


def makeSequence (dataType, useBlocks, yamlPath, forCompare, noSystematics, hardCuts = False,
                  isPhyslite = False, geometry = None, autoconfigFromFlags = None, onlyNominalOR = False,
                  forceEGammaFullSimConfig = False) :

    # do some harder cuts on all object types, this is mostly used for
    # benchmarking
    if hardCuts :
        global electronMinPt
        electronMinPt = 27e3
        global photonMinPt
        photonMinPt = 27e3
        global muonMinPt
        muonMinPt = 27e3
        global tauMinPt
        tauMinPt = 27e3
        global jetMinPt
        jetMinPt = 45e3

    algSeq = AlgSequence('AnalysisSequence')

    ca = None
    if useBlocks :
        ca = makeSequenceBlocks (dataType, algSeq, forCompare=forCompare,
                                 isPhyslite=isPhyslite,
                                 geometry=geometry, onlyNominalOR=onlyNominalOR,
                                 autoconfigFromFlags=autoconfigFromFlags, noSystematics=noSystematics,
                                 forceEGammaFullSimConfig=forceEGammaFullSimConfig)
    elif yamlPath :
        from AnalysisAlgorithmsConfig.ConfigText import makeSequence as makeSequenceText
        ca = makeSequenceText(yamlPath, dataType, algSeq, geometry=geometry,
                              isPhyslite=isPhyslite,
                              autoconfigFromFlags=autoconfigFromFlags, noSystematics=noSystematics)
    else :
        ca = makeSequenceOld (dataType, algSeq, forCompare=forCompare,
                              isPhyslite=isPhyslite,
                              noSystematics=noSystematics,
                              forceEGammaFullSimConfig=forceEGammaFullSimConfig)

    if ca is not None:
        return ca
    else:
        return algSeq
