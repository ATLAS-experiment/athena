# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

# AnaAlgorithm import(s):
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool
import re

# These algorithms set up the jet recommendations as-of 04/02/2019.
# Jet calibration recommendations
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR21
# Jet uncertainties recommendations
# Small-R
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2018SmallR
# Large-R
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetUncertaintiesRel21Moriond2018LargeR
# JVT recommendations
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JVTCalibrationRel21

def makeJetAnalysisSequence( dataType, jetCollection, postfix = '',
                             deepCopyOutput = False,
                             shallowViewOutput = True,
                             runGhostMuonAssociation = True,
                             enableCutflow = False,
                             enableKinematicHistograms = False,
                             defineSystObjectLinks=False,
                             **kwargs):
    """Create a jet analysis algorithm sequence
      The jet collection is interpreted and selects the correct function to call,
      makeSmallRJetAnalysisSequence, makeRScanJetAnalysisSequence or
      makeLargeRJetAnalysisSequence

      Keyword arguments
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        postfix -- String to be added to the end of all public names.
        deepCopyOutput -- Whether or not to deep copy the output
        shallowViewOutput -- Whether or not to output a shallow view as the output
        enableCutflow -- Whether or not to dump the cutflow
        enableKinematicHistograms -- Whether or not to dump the kinematic histograms
        Other keyword arguments are forwarded to the other functions.
    """

    if dataType not in ["data", "mc", "afii"]:
        raise ValueError ("invalid data type: " + dataType )

    # Setup the postfix
    if postfix != '':
        postfix = "_" + postfix

    # Make sure selection options make sense
    if deepCopyOutput and shallowViewOutput:
        raise ValueError ("deepCopyOutput and shallowViewOutput can't both be true!")

    # Remove b-tagging calibration from the container name
    btIndex = jetCollection.find('_BTagging')
    if btIndex != -1:
        jetCollection = jetCollection[:btIndex]

    jetCollectionName=jetCollection
    if(jetCollection=="AnalysisJets") :
        jetCollectionName="AntiKt4EMPFlowJets"
    if(jetCollection=="AnalysisLargeRJets") :
        jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    #AntiKt10UFO CSSKSoftDropBeta100Zcut10Jets

    # interpret the jet collection
    collection_pattern = re.compile(
        r"AntiKt(\d+)(EMTopo|EMPFlow|LCTopo|TrackCaloCluster|UFO)(TrimmedPtFrac5SmallR20|CSSKSoftDropBeta100Zcut10)?Jets")
    match = collection_pattern.match(jetCollectionName)
    if not match:
        raise ValueError(
            "Jet collection {0} does not match expected pattern!".format(jetCollectionName) )
    radius = int(match.group(1) )
    if radius not in [2, 4, 6, 10]:
        raise ValueError("Jet collection has an unsupported radius '{0}'!".format(radius) )
    jetInput = match.group(2)

    # Create the analysis algorithm sequence object.
    seq = AnaAlgSequence( "JetAnalysisSequence"+postfix )
    # Relink original jets in case of b-tagging calibration
    if btIndex != -1:
        alg = createAlgorithm( 'CP::AsgOriginalObjectLinkAlg',
            'JetOriginalObjectLinkAlg'+postfix )
        alg.baseContainerName = jetCollection
        seq.append( alg, inputPropName = 'particles', outputPropName = 'particlesOut', stageName = 'calibration' )

    # Set up the jet ghost muon association algorithm:
    if runGhostMuonAssociation:
        alg = createAlgorithm( 'CP::JetGhostMuonAssociationAlg',
            'JetGhostMuonAssociationAlg'+postfix )
        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration' )

    # record all the selections each subfunction makes
    seq.addMetaConfigDefault ("selectionDecorNames", [])
    seq.addMetaConfigDefault ("selectionDecorCount", [])

    if radius == 4:
        makeSmallRJetAnalysisSequence(seq,
            dataType, jetCollection, jetInput=jetInput, postfix=postfix, **kwargs)
    elif radius in [2, 6]:
        makeRScanJetAnalysisSequence(seq,
            dataType, jetCollection, jetInput=jetInput, radius=radius,
            postfix=postfix, **kwargs)
    else:
        trim = match.group(3)
        if trim == "":
            raise ValueError("Untrimmed large-R jets are not supported!")
        makeLargeRJetAnalysisSequence(seq,
            dataType, jetCollection, jetInput=jetInput, postfix=postfix, **kwargs)

    # Build links between nominal and systematic objects
    # This is useful downstream for constructing the union of all selected objects
    if defineSystObjectLinks:
        if deepCopyOutput:
            raise ValueError ("Systematic object links may not work correctly on deep copy output")
        alg = createAlgorithm('CP::SystObjectLinkerAlg', 'SystObjLinker'+postfix)
        #alg.OutputLevel=1
        seq.append( alg, inputPropName = 'input', stageName = 'selection' )

    # Set up an algorithm used to create jet selection cutflow:
    if enableCutflow:
        alg = createAlgorithm( 'CP::ObjectCutFlowHistAlg', 'JetCutFlowDumperAlg'+postfix )
        alg.histPattern = 'jet_cflow_%SYS%'+postfix
        seq.append( alg, inputPropName = 'input', stageName = 'selection',
                    dynConfig = {'selections' : lambda meta : meta["selectionDecorNames"][:]} )

    # Set up an algorithm dumping the kinematic properties of the jets:
    if enableKinematicHistograms:
        alg = createAlgorithm( 'CP::KinematicHistAlg', 'JetKinematicDumperAlg'+postfix )
        alg.histPattern = 'jet_%VAR%_%SYS%'+postfix
        seq.append( alg, inputPropName = 'input', stageName = 'selection',
                    dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNames"])} )

    if shallowViewOutput:
      # Set up an algorithm that makes a view container using the selections
      # performed previously:
      alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'JetViewFromSelectionAlg'+postfix )
      seq.append( alg, inputPropName = 'input', outputPropName = 'output',
                  stageName = 'selection',
                  dynConfig = {'selection' : lambda meta : meta["selectionDecorNames"][:]} )

    # Set up a final deep copy making algorithm if requested:
    if deepCopyOutput:
        alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'JetDeepCopyMaker'+postfix )
        alg.deepCopy = True
        seq.append( alg, inputPropName = 'input', outputPropName = 'output',
                    stageName = 'selection' )

    return seq

def makeSmallRJetAnalysisSequence( seq, dataType, jetCollection,
                                   jetInput, postfix = '',
                                   runJvtUpdate = False, runNNJvtUpdate = False, runFJvtUpdate = False,
                                   runJvtSelection = True, runFJvtSelection = False,
                                   runJvtEfficiency = True, runFJvtEfficiency = False,
                                   reduction = "Category", JEROption = "Full"):
    """Add algorithms for the R=0.4 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        postfix -- String to be added to the end of all public names.
        runJvtUpdate -- Determines whether or not to update JVT on the jets
        runNNJvtUpdate -- Determines whether or not to update NN JVT on the jets
        runFJvtUpdate -- Determines whether or not to update forward JVT on the jets
        runJvtSelection -- Determines whether or not to run JVT selection on the jets
        runFJvtSelection -- Determines whether or not to run forward JVT selection on the jets
        runJvtEfficiency -- Determines whether or not to calculate the JVT efficiency
        runFJvtEfficiency -- Determines whether or not to calculate the forward JVT efficiency
        reduction -- Which NP reduction scheme should be used (All, Global, Category, Scenario)
        JEROption -- Which variant of the reduction should be used (All, Full, Simple). Note that not all combinations of reduction and JEROption are valid!
    """

    jetCollectionName=jetCollection
    if(jetCollection=="AnalysisJets") :
        jetCollectionName="AntiKt4EMPFlowJets"
    if(jetCollection=="AnalysisLargeRJets") :
        jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    if jetInput not in ["EMTopo", "EMPFlow"]:
        raise ValueError(
            "Unsupported input type '{0}' for R=0.4 jets!".format(jetInput) )

    if runNNJvtUpdate:
        assert jetInput=="EMPFlow", "NN JVT only defined for PFlow jets"

    if (runJvtEfficiency or runFJvtEfficiency) and dataType != "data":
        # Decorate jets with isHS labels (required to retrieve Jvt SFs)
        alg = createAlgorithm( 'CP::JetDecoratorAlg', 'JetPileupLabelAlg'+postfix )
        addPrivateTool( alg, 'decorator', 'JetPileupLabelingTool' )
        alg.decorator.RecoJetContainer = jetCollection
        alg.decorator.SuppressOutputDependence=True

        seq.append(alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'selection')
        seq.addDecorAwareTool(tool=alg.decorator,parent=alg,outputPropName='RecoJetContainer')

    # Prepare the jet calibration algorithm
    alg = createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
    addPrivateTool( alg, 'calibrationTool', 'JetCalibrationTool' )
    alg.calibrationTool.JetCollection = jetCollectionName[:-4]
    # Get the correct string to use in the config file name
    if jetInput == "EMPFlow":
        configFile = "PreRec_R22_PFlow_ResPU_EtaJES_GSC_February23_230215.config"
    else:
        if dataType == 'afii':
            configFile = "JES_MC16Recommendation_AFII_{0}_Apr2019_Rel21.config"
        else:
            configFile = "JES_MC16Recommendation_Consolidated_{0}_Apr2019_Rel21.config"
        configFile = configFile.format(jetInput)
    alg.calibrationTool.ConfigFile = configFile
    if dataType == 'data':
        alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Insitu'
    else:
        if jetInput == "EMPFlow":
            alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC'
        else:
            alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Smear'
    alg.calibrationTool.IsData = (dataType == 'data')
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration')

    # Jet uncertainties
    # Prepare the config file
    if reduction == "All" and JEROption == "All":
        alg.uncertaintiesTool.ConfigFile = "R4_AllNuisanceParameters_AllJERNP.config"
    elif "Scenario" in reduction:
        if JEROption != "Simple":
            raise ValueError(
                "Invalid uncertainty configuration - Scenario* reductions can "
                "only be used together with the Simple JEROption")
        configFile = "R4_{0}_SimpleJER.config".format(reduction)
    elif reduction in ["Global", "Category"] and JEROption in ["Simple", "Full"]:
        configFile = "R4_{0}Reduction_{1}JER.config".format(reduction, JEROption)
    else:
        raise ValueError(
            "Invalid combination of reduction and JEROption settings: "
            "reduction: {0}, JEROption: {1}".format(reduction, JEROption) )

    alg = createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg'+postfix )
    addPrivateTool( alg, 'uncertaintiesTool', 'JetUncertaintiesTool' )
    alg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
    # Add the correct directory on the front
    alg.uncertaintiesTool.ConfigFile = "rel22/Summer2023_PreRec/"+configFile
    alg.uncertaintiesTool.MCType = "MC20"
    alg.uncertaintiesTool.IsData = (dataType == 'data')
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut',
                stageName = 'calibration' )

    # Set up the JVT update algorithm:
    if runJvtUpdate :
        alg = createAlgorithm( 'CP::JvtUpdateAlg', 'JvtUpdateAlg'+postfix )
        addPrivateTool( alg, 'jvtTool', 'JetVertexTaggerTool' )
        alg.jvtTool.JetContainer = jetCollection
        alg.jvtTool.SuppressInputDependence=True

        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'selection' )

    if runNNJvtUpdate :
        alg = createAlgorithm( 'CP::JetDecoratorAlg', 'NNJvtUpdateAlg'+postfix )
        addPrivateTool( alg, 'decorator', 'JetPileupTag::JetVertexNNTagger' )
        # Set this actually to the *output* collection
        alg.decorator.JetContainer = jetCollection
        alg.decorator.SuppressInputDependence=True
        alg.decorator.SuppressOutputDependence=True

        seq.append(alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'selection')
        seq.addDecorAwareTool(tool=alg.decorator,parent=alg,outputPropName='JetContainer')

    if runFJvtUpdate :
        alg = createAlgorithm( 'CP::JetModifierAlg', 'JetModifierAlg'+postfix )
        addPrivateTool( alg, 'modifierTool', 'JetForwardJvtTool')
        alg.modifierTool.OutputDec = "passFJVT_internal" #Output decoration
        alg.modifierTool.FJVTName = "fJVT"
        # fJVT WPs depend on the MET WP
        # see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EtmissRecommendationsRel21p2#fJVT_and_MET
        alg.modifierTool.EtaThresh = 2.5 # Eta dividing central from forward jets
        alg.modifierTool.ForwardMaxPt = 120.0e3 #Max Pt to define fwdJets for JVT
        alg.modifierTool.RenounceOutputs = True
        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'selection' )
        pass

    if runJvtSelection:
        alg = createAlgorithm("CP::AsgSelectionAlg", "JvtSelectionAlg"+postfix)
        addPrivateTool(alg, "selectionTool", "CP::NNJvtSelectionTool")
        alg.selectionTool.MaxPtForJvt = 60e3 if jetInput == "EMPFlow" else 120e3
        alg.selectionTool.WorkingPoint = "FixedEffPt"
        alg.selectionDecoration = 'jvt_selection'
        seq.append( alg, inputPropName = 'particles', stageName = 'selection' )
        # TODO: How to set the JetContainer in this version?

        if runJvtEfficiency and dataType != "data":
            alg = createAlgorithm( 'CP::JvtEfficiencyAlg', 'JvtEfficiencyAlg'+postfix )
            addPrivateTool( alg, 'efficiencyTool', 'CP::NNJvtEfficiencyTool' )
            alg.efficiencyTool.MaxPtForJvt = 60e3 if jetInput == "EMPFlow" else 120e3
            alg.efficiencyTool.WorkingPoint = "FixedEffPt"
            alg.selection = 'jvt_selection'
            alg.scaleFactorDecoration = 'jvt_effSF_%SYS%'
            alg.outOfValidity = 2
            alg.outOfValidityDeco = 'no_jvt'
            alg.skipBadEfficiency = 0
            seq.append( alg, inputPropName = 'jets', stageName = 'efficiency' )

    if runFJvtSelection:
        alg = createAlgorithm("CP::AsgSelectionAlg", "FJvtSelectionAlg"+postfix)
        addPrivateTool(alg, "selectionTool", "CP::FJvtSelectionTool")
        alg.selectionTool.JvtMomentName = "fJVT" if runFJvtUpdate else "DFCommonJets_fJvt"
        alg.selectionTool.WorkingPoint = "Loose"
        alg.selectionDecoration = 'fjvt_selection'
        seq.append( alg, inputPropName = 'particles', stageName = 'selection' )
        # TODO: How to set the JetContainer in this version?

        if runFJvtEfficiency and dataType != "data":
            alg = createAlgorithm( 'CP::JvtEfficiencyAlg', 'JvtEfficiencyAlg'+postfix )
            addPrivateTool( alg, 'efficiencyTool', 'CP::FJvtEfficiencyTool' )
            alg.efficiencyTool.WorkingPoint = "Loose"
            alg.efficiencyTool.SFFile = f"JetJvtEfficiency/May2020/fJvtSFFile.{jetInput}.root"
            alg.selection = 'fjvt_selection'
            alg.scaleFactorDecoration = 'fjvt_effSF_%SYS%'
            alg.outOfValidity = 2
            alg.outOfValidityDeco = 'no_fjvt'
            alg.skipBadEfficiency = 0
            seq.append( alg, inputPropName = 'jets', stageName = 'efficiency' )


    # Return the sequence:
    return seq

def makeRScanJetAnalysisSequence( seq, dataType, jetCollection,
                                  jetInput, radius, postfix = '' ):
    """Add algorithms for the R-scan jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        radius -- The radius of the r-scan jets.
        postfix -- String to be added to the end of all public names.
    """

    jetCollectionName=jetCollection
    if(jetCollection=="AnalysisJets") :
        jetCollectionName="AntiKt4EMPFlowJets"
    if(jetCollection=="AnalysisLargeRJets") :
        jetCollectionName="AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"


    if jetInput != "LCTopo":
        raise ValueError(
            "Unsupported input type '{0}' for R-scan jets!".format(jetInput) )
    # Prepare the jet calibration algorithm
    alg = createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
    addPrivateTool( alg, 'calibrationTool', 'JetCalibrationTool' )
    alg.calibrationTool.JetCollection = jetCollectionName[:-4]
    alg.calibrationTool.ConfigFile = \
        "JES_MC16Recommendation_Rscan{0}LC_Feb2022_R21.config".format(radius)
    if dataType == 'data':
        alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
    else:
        alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
    alg.calibrationTool.IsData = (dataType == 'data')
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration' )
    # Logging would be good
    print("WARNING: uncertainties for R-Scan jets are not yet released!")

def makeLargeRJetAnalysisSequence( seq, dataType, jetCollection,
                                   jetInput, postfix = '', largeRMass = "Comb"):
    """Add algorithms for the R=1.0 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        dataType -- The data type to run on ("data", "mc" or "afii")
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        postfix -- String to be added to the end of all public names.
        largeRMass -- Which large-R mass definition to use. Ignored if not running on large-R jets ("Comb", "Calo", "TA")
    """

    jetCollectionName=jetCollection
    if(jetCollection=="AnalysisJets") :
        jetCollectionName="AntiKt4EMPFlowJets"
    if(jetCollection=="AnalysisLargeRJets") :
        jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    if largeRMass not in ["Comb", "Calo", "TA"]:
        raise ValueError ("Invalid large-R mass defintion {0}!".format(largeRMass) )

    if jetInput not in ["LCTopo", "TrackCaloCluster", "UFO"]:
        raise ValueError (
            "Unsupported input type '{0}' for large-R jets!".format(jetInput) )

    if jetInput == "TrackCaloCluster":
        # Only one mass defintion supported
        if largeRMass != "Calo":
            raise ValueError(
                "Unsupported large-R TCC jet mass '{0}'!".format(largeRMass) )
        configFile = "JES_MC16recommendation_FatJet_TCC_JMS_calo_30Oct2018.config"

    if jetInput == "LCTopo":
        if largeRMass == "Comb":
            if dataType == "data":
                configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
            if dataType == "mc":
                configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_17Oct2018.config"
        elif largeRMass == "Calo":
            if dataType == "data":
                configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
            if dataType == "mc":
                configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_calo_12Oct2018.config "
        elif largeRMass == "TA":
            if dataType == "data":
                configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
            if dataType == "mc":
                configFile = "JES_MC16recommendation_FatJet_Trimmed_JMS_TA_12Oct2018.config"

    if jetInput == "UFO":
        # R22 pre-recs
        # More info: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ApplyJetCalibrationR22#R_1_0_unified_flow_object_UFO_je
        configFile = "JES_MC20PreRecommendation_R10_UFO_CSSK_SoftDrop_JMS_R21Insitu_10Mar2023.config"

    # Prepare the jet calibration algorithm
    alg = createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg'+postfix )
    addPrivateTool( alg, 'calibrationTool', 'JetCalibrationTool' )
    alg.calibrationTool.JetCollection = jetCollectionName[:-4]
    alg.calibrationTool.ConfigFile = configFile
    if jetInput == "TrackCaloCluster" or jetInput == "UFO" or dataType == "mc":
        alg.calibrationTool.CalibSequence = "EtaJES_JMS"
    elif dataType == "data":
        alg.calibrationTool.CalibSequence = "EtaJES_JMS_Insitu_InsituCombinedMass"
    alg.calibrationTool.IsData = (dataType == "data")
    seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut', stageName = 'calibration' )

    # Jet uncertainties

    if jetInput == "UFO":
        print("WARNING: uncertainties for UFO jets are not yet released!")

    if jetInput != "UFO":
        alg = createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg'+postfix )
        # R=1.0 jets have a validity range
        alg.outOfValidity = 2 # SILENT
        alg.outOfValidityDeco = 'outOfValidity'
        addPrivateTool( alg, 'uncertaintiesTool', 'JetUncertaintiesTool' )

        alg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
        alg.uncertaintiesTool.ConfigFile = \
            "rel21/Moriond2018/R10_{0}Mass_all.config".format(largeRMass)
        alg.uncertaintiesTool.MCType = "MC16a"
        alg.uncertaintiesTool.IsData = (dataType == "data")

        seq.append( alg, inputPropName = 'jets', outputPropName = 'jetsOut',
                    stageName = 'calibration',
                    metaConfig = {'selectionDecorNames' : ['outOfValidity'], 'selectionDecorCount' : [1]} )
