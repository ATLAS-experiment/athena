# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool

def makeFTagAnalysisSequence( seq, dataType, jetCollection,
                              btagWP = "FixedCutBEff_77",
                              btagger = "DL1r",
                              generator = "default",
                              postfix = "",
                              preselection=None,
                              kinematicSelection = False,
                              noEfficiency = False,
                              enableCutflow = False,
                              minPt = None,
                              makeViewContainer = False):
    """Create a ftag analysis algorithm sequence

    for now the sequence is passed in, I'm unsure if I can concatenate
    two sequences at the moment, or if that blows things up horribly

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      jetCollection -- Jet container to run on
      btagWP -- Flavour tagging working point
      btagger -- Flavour tagger
      generator -- Generator for MC/MC scale factors
      kinematicSelection -- Wether to run kinematic selection
      noEfficiency -- Wether to run efficiencies calculation
      enableCutflow -- Whether or not to dump the cutflow
      minPt -- Kinematic selection for jet calibration validity (depending on jet collection)
    """

    # Kinematic selection depending on validity of the calibration
    # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalibrationRecommendationsRelease21
    if minPt is None:
        if "EMPFlow" in jetCollection:
            minPt = 20e3
        elif "EMTopo" in jetCollection:
            minPt = 20e3
        elif "VRTrack" in jetCollection:
            minPt = 10e3

    preselectionList = []
    if preselection is not None and preselection != '' :
        preselectionList.append (preselection)

    if dataType not in ["data", "mc", "afii"] :
        raise ValueError ("invalid data type: " + dataType)

    if generator not in ["default", "Pythia8", "Sherpa221", "Sherpa2210", "Sherpa2212", "Herwig713", "Herwig721", "amcAtNLOPythia", "amcAtNLOHerwig"]:
        raise ValueError ("invalid generator type: " + generator)

    # MC/MC scale factors configuration
    DSID = "default"
    if generator == "Sherpa221":
        DSID = "410250"
    elif generator == "Sherpa2210":
        DSID = "700122"
    elif generator == "Sherpa2212":
        DSID = "700660"
    elif generator == "Herwig713":
        DSID = "411233"
    elif generator == "Herwig721":
        DSID = "600666"
    elif generator == "amcAtNLOPythia":
        DSID = "410464"
    elif generator == "amcAtNLOHerwig":
        DSID = "412116"

    # CDI file
    bTagCalibFile = "xAODBTaggingEfficiency/13TeV/2023-22-13TeV-MC20-CDI-2023-09-13_v1.root"

    # # Create the analysis algorithm sequence object:
    # seq = AnaAlgSequence( "FTagAnalysisSequence" )

    if kinematicSelection:
        # Set up the ftag kinematic selection algorithm(s):
        alg = createAlgorithm( 'CP::AsgSelectionAlg', 'FTagKinSelectionAlg'+postfix )
        addPrivateTool( alg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.minPt = minPt
        alg.selectionTool.maxEta = 2.5
        alg.selectionDecoration = 'ftag_kin_select_' + btagger + '_' + btagWP
        alg.preselection = '&&'.join (preselectionList)
        preselectionList.append (alg.selectionDecoration)
        seq.append( alg, inputPropName = 'particles' )

        if makeViewContainer :
            # Set up an algorithm that makes a view container using the selections
            # performed previously:
            alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg',
                                   'FTagKinViewFromSelectionAlg'+postfix )
            alg.selection = [ 'ftag_kin_select_' + btagger + '_' + btagWP ]
            seq.append( alg, inputPropName = 'input', outputPropName = 'output',
                        stageName = 'selection' )

    # Set up the ftag selection algorithm(s):
    alg = createAlgorithm( 'CP::AsgSelectionAlg', 'FTagSelectionAlg' + btagger + btagWP + postfix )
    addPrivateTool( alg, 'selectionTool', 'BTaggingSelectionTool' )
    alg.selectionTool.TaggerName = btagger
    alg.selectionTool.OperatingPoint = btagWP
    alg.selectionTool.JetAuthor = jetCollection
    alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
    alg.selectionTool.MinPt = minPt
    alg.preselection = '&&'.join (preselectionList)
    alg.selectionDecoration = 'ftag_select_' + btagger + '_' + btagWP + ',as_char'
    seq.append( alg, inputPropName = 'particles',
                stageName = 'selection' )
    
    if btagWP == 'Continuous':
        alg = createAlgorithm( 'CP::BTaggingInformationDecoratorAlg', 'FTagInfoAlg' + btagger + btagWP + postfix )
        addPrivateTool( alg, 'selectionTool', 'BTaggingSelectionTool' )
        alg.selectionTool.TaggerName = btagger
        alg.selectionTool.OperatingPoint = btagWP
        alg.selectionTool.JetAuthor = jetCollection
        alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
        alg.selectionTool.MinPt = minPt
        alg.preselection = '&&'.join (preselectionList)
        alg.quantileDecoration = 'ftag_quantile_' + btagger
        seq.append( alg, inputPropName = 'jets',
                    stageName = 'selection' )

    if not noEfficiency and dataType != 'data':
        # Set up the efficiency calculation algorithm:
        alg = createAlgorithm( 'CP::BTaggingEfficiencyAlg',
                               'FTagEfficiencyScaleFactorAlg' + btagger + btagWP + postfix )
        addPrivateTool( alg, 'efficiencyTool',
                        'BTaggingEfficiencyTool' )
        alg.efficiencyTool.TaggerName = btagger
        alg.efficiencyTool.OperatingPoint = btagWP
        alg.efficiencyTool.JetAuthor = jetCollection
        alg.efficiencyTool.ScaleFactorFileName = bTagCalibFile
        alg.efficiencyTool.SystematicsStrategy = "Envelope"
        alg.efficiencyTool.MinPt = minPt
        if DSID != "default":
            alg.efficiencyTool.EfficiencyBCalibrations = DSID
            alg.efficiencyTool.EfficiencyTCalibrations = DSID
            alg.efficiencyTool.EfficiencyCCalibrations = DSID
            alg.efficiencyTool.EfficiencyLightCalibrations = DSID
        alg.scaleFactorDecoration = 'ftag_effSF_' + btagger + '_' + btagWP + '_%SYS%'
        alg.selectionDecoration = 'ftag_select_' + btagger + '_' + btagWP + ',as_char'
        alg.onlyEfficiency = btagWP == 'Continuous'
        alg.outOfValidity = 2
        alg.outOfValidityDeco = 'no_ftag_' + btagger + '_' + btagWP
        alg.preselection = '&&'.join (preselectionList)
        seq.append( alg, inputPropName = 'jets',
                    stageName = 'efficiency' )
        pass

    # Set up an algorithm used to create f-tag selection cutflow:
    if enableCutflow:
        alg = createAlgorithm( 'CP::ObjectCutFlowHistAlg', 'FTagCutFlowDumperAlg' + btagger + btagWP + postfix )
        alg.histPattern = 'ftag_cflow_' + btagger + '_' + btagWP + '_%SYS%'
        alg.selections = ['ftag_select_' + btagger + '_' + btagWP + ',as_char']
        seq.append( alg, inputPropName = 'input',
                    stageName = 'selection' )

    # Return the sequence:
    return seq
