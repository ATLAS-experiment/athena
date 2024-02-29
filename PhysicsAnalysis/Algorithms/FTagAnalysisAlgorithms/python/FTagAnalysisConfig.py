# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaConfiguration.Enums import LHCPeriod
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from PathResolver import PathResolver

def parseTDPdatabase(tdpFile, dsid):
    """function to parse the TopDataPreparation database
    'tdpFile' and extract the FTAG showering algorithm
    corresponding to 'dsid'
    """
    result = None
    with open(PathResolver.FindCalibFile(tdpFile), 'r') as _f:
        for line in _f:
            if not line.strip() or line.startswith('#'):
                continue
            columns = line.split()
            if columns[0].isdigit() and int(columns[0]) == dsid:
                result = columns[3].strip()
                break
    return result


class FTagConfig (ConfigBlock):
    """the ConfigBlock for the flavor tagging config"""

    def __init__ (self, containerName, selectionName) :
        super (FTagConfig, self).__init__ (containerName + '.' + selectionName)
        self.containerName = containerName
        self.postfix = selectionName
        self.addOption ('btagWP', "FixedCutBEff_77", type=str)
        self.addOption ('btagger', "DL1r", type=str)
        self.addOption ('generator', "autoconfig", type=str)
        self.addOption ('kinematicSelection', True, type=bool)
        self.addOption ('noEffSF', False, type=bool)
        self.addOption ('globalSF', True, type=bool)
        self.addOption ('minPt', None, type=float)
        self.addOption ('bTagCalibFile', None, type=str,
                        info='calibration file for CDI')
        self.addOption ('systematicsStrategy', 'SFEigen', type=str,
                        info="name of systematics model; presently choose between 'SFEigen' and 'Envelope'")
        self.addOption ('eigenvectorReductionB', 'Loose', type=str,
                        info="b-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'")
        self.addOption ('eigenvectorReductionC', 'Loose', type=str,
                        info="b-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'")
        self.addOption ('eigenvectorReductionLight', 'Loose', type=str,
                        info="b-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'")
        self.addOption ('excludeFromEigenVectorTreatment', '', type=str,
                        info="(semicolon-separated) names of uncertainties to be excluded from all eigenvector decompositions (if used)")
        self.addOption ('excludeFromEigenVectorBTreatment', '', type=str,
                        info="(semicolon-separated) names of uncertainties to be excluded from b-jet eigenvector decompositions (if used)")
        self.addOption ('excludeFromEigenVectorCTreatment', '', type=str,
                        info="(semicolon-separated) names of uncertainties to be excluded from c-jet eigenvector decompositions (if used)")
        self.addOption ('excludeFromEigenVectorLightTreatment', '', type=str,
                        info="(semicolon-separated) names of uncertainties to be excluded from light-flavour-jet eigenvector decompositions (if used)")
        self.addOption ('excludeRecommendedFromEigenVectorTreatment', False, type=str,
                        info="whether or not to add recommended lists to the user specified eigenvector decomposition exclusion lists")

    def resolveMCMCgenerator(self, config, generatorDict):
        """use either the metadata (generatorDict) or TopDataPreparation
        to figure out the appropriate generator settings for MC-MC corrections"""
        result = None

        if config.dataType() is DataType.Data:
            return "default"

        # First, try using the Metadata
        if generatorDict is not None:
            generators = list(generatorDict.keys())
            if "Pythia8" in generators:
                if "aMcAtNlo" in generators:
                    result = "amcAtNLOPythia"
                else:
                    result = "default"
            elif "Herwig7" in generators:
                if "aMcAtNlo" in generators:
                    result = "amcAtNLOHerwig"
                else:
                    version = generatorDict["Herwig7"]
                    if version is not None:
                        if "7.1.3" in version:
                            result = "Herwig713"
                        elif "7.2.1" in version:
                            result = "Herwig721"
            elif "Sherpa" in generators:
                version = generatorDict["Sherpa"]
                if version is not None:
                    if "2.2.10" in version:
                        result = "Sherpa2210"
                    elif "2.2.12" in version:
                        result = "Sherpa2212"
                    elif "2.2.1" in version:
                        result = "Sherpa221"

        # If 'result' is still None, the above didn't succeed:
        # now try with TopDataPreparation
        if result is None:
            dsid = config.dsid()
            # we need to loop up the DSID
            if dsid is None or dsid == 0:
                raise ValueError(
                    f"The value of the DSID for this sample is {dsid}! "
                    "Your metadata may be broken, or something else"
                    " has gone wrong!")
            else:
                if config.geometry() is LHCPeriod.Run2:
                    tdpFile = 'dev/AnalysisTop/TopDataPreparation/XSection-MC16-13TeV_JESinfo.data'
                elif config.geometry() is LHCPeriod.Run3:
                    tdpFile = 'dev/AnalysisTop/TopDataPreparation/XSection-MC21-13p6TeV.data'
                else:
                    # only support Run2 and Run3 so far
                    raise ValueError("Unrecognised geometry "+str(config.geometry())+" for FTAG MC-MC corrections, aborting.")
                result = parseTDPdatabase(tdpFile, dsid)
                # need to translate from TopDataPreparation notation to FTAG notation
                tdpTranslation = {
                    'herwig': 'Herwig7',
                    'herwigpp': 'Herwig7',
                    'pythia': 'default',
                    'pythia8': 'default',
                    'sherpa': 'Sherpa221',
                    'sherpa21': 'Sherpa221',
                    'amcatnlopythia8': 'amcAtNLOPythia',
                    'herwigpp713': 'Herwig713',
                    'sherpa228': 'Sherpa228',
                    'sherpa2210': 'Sherpa2210',
                    'sherpa2212': 'Sherpa2212',
                    'herwigpp721': 'Herwig721',
                }
                try:
                    result = tdpTranslation[result]
                except KeyError:
                    raise Exception(f"Unrecognised FTAG MC-to-MC generator setup {result}, aborting.")

        # At this point we either have a valid string for 'result' or we've already crashed
        return result

    def makeAlgs (self, config) :

        jetCollection = config.originalName (self.containerName)

        selectionName = self.postfix
        if selectionName is None or selectionName == '' :
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_' :
            postfix = '_' + postfix

        # Kinematic selection depending on validity of the calibration
        # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagRecommendationsRelease22
        minPt = self.minPt
        if minPt is None:
            if "EMPFlow" in jetCollection:
                minPt = 20e3
            elif "EMTopo" in jetCollection:
                minPt = 20e3
            elif "VR" in jetCollection:
                minPt = 10e3

        # MC/MC scale factors defined only for Run 2 and Run 3
        if config.geometry() in [LHCPeriod.Run2, LHCPeriod.Run3]:
            # special setting: allow for on-the-fly look up of the generator information
            if self.generator == "autoconfig":
                self.generator = self.resolveMCMCgenerator(config, generatorDict=config.generatorInfo())

            if config.geometry() == LHCPeriod.Run2:
                if self.generator not in ["default", "Pythia8", "Sherpa221", "Sherpa2210", "Sherpa2212", "Herwig713", "Herwig721", "amcAtNLOPythia", "amcAtNLOHerwig"]:
                    raise ValueError ("invalid generator type: " + self.generator)
            elif config.geometry() == LHCPeriod.Run3:
                if self.generator not in ["default", "Pythia8", "Sherpa2212", "Herwig713"]:
                    raise ValueError ("invalid generator type: " + self.generator)

        # MC/MC scale factors configuration
        DSID = "default"
        if self.generator == "Sherpa221":
            DSID = "410250"
        elif self.generator == "Sherpa2210":
            DSID = "700122"
        elif self.generator == "Sherpa2212":
            DSID = "700660"
        elif self.generator == "Herwig713":
            DSID = "411233"
        elif self.generator == "Herwig721":
            DSID = "600666"
        elif self.generator == "amcAtNLOPythia":
            DSID = "410464"
        elif self.generator == "amcAtNLOHerwig":
            DSID = "412116"

        # CDI file
        if self.bTagCalibFile is not None :
            bTagCalibFile = self.bTagCalibFile
        else:
            if config.geometry() == LHCPeriod.Run2:
                bTagCalibFile = "xAODBTaggingEfficiency/13TeV/2023-22-13TeV-MC20-CDI-2023-09-13_v1.root"
            elif config.geometry() >= LHCPeriod.Run3:
                bTagCalibFile = "xAODBTaggingEfficiency/13p6TeV/2023-22-13TeV-MC21-CDI-2023-09-13_v1.root"

        if self.kinematicSelection:
            # Set up the ftag kinematic selection algorithm(s):
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'FTagKinSelectionAlg'+postfix )
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = minPt
            alg.selectionTool.maxEta = 2.5
            alg.selectionDecoration = 'ftag_kin_select_' + selectionName + ',as_char'
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            config.addSelection (self.containerName, selectionName,
                                 alg.selectionDecoration,
                                 preselection=True,
                                 writeToOutput=False)
            alg.particles = config.readName (self.containerName)

        # Set up the ftag selection algorithm(s):
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'FTagSelectionAlg' + postfix )
        config.addPrivateTool( 'selectionTool', 'BTaggingSelectionTool' )
        alg.selectionTool.TaggerName = self.btagger
        alg.selectionTool.OperatingPoint = self.btagWP
        alg.selectionTool.JetAuthor = jetCollection
        alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
        alg.selectionTool.MinPt = minPt
        alg.preselection = config.getPreselection (self.containerName, selectionName)
        alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
        alg.particles = config.readName (self.containerName)
        if self.btagWP != 'Continuous':
            config.addOutputVar (self.containerName, 'ftag_select_' + selectionName, selectionName + '_select', noSys=True)

        if self.btagWP == 'Continuous':
            alg = config.createAlgorithm( 'CP::BTaggingInformationDecoratorAlg', 'FTagInfoAlg' + postfix )
            config.addPrivateTool( 'selectionTool', 'BTaggingSelectionTool' )
            alg.selectionTool.TaggerName = self.btagger
            alg.selectionTool.OperatingPoint = self.btagWP
            alg.selectionTool.JetAuthor = jetCollection
            alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
            alg.selectionTool.MinPt = minPt
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            alg.quantileDecoration = 'ftag_quantile_' + selectionName
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, 'ftag_quantile_' + selectionName, selectionName + '_quantile', noSys=True)

        if not self.noEffSF and config.dataType() is not DataType.Data:
            # Set up the efficiency calculation algorithm:
            alg = config.createAlgorithm( 'CP::BTaggingEfficiencyAlg',
                                          'FTagEfficiencyScaleFactorAlg' + postfix )
            config.addPrivateTool( 'efficiencyTool', 'BTaggingEfficiencyTool' )
            alg.efficiencyTool.TaggerName = self.btagger
            alg.efficiencyTool.OperatingPoint = self.btagWP
            alg.efficiencyTool.JetAuthor = jetCollection
            alg.efficiencyTool.MinPt = minPt
            alg.efficiencyTool.EfficiencyFileName = bTagCalibFile
            alg.efficiencyTool.ScaleFactorFileName = bTagCalibFile
            alg.efficiencyTool.SystematicsStrategy = self.systematicsStrategy
            if self.systematicsStrategy == "SFEigen":
                alg.efficiencyTool.EigenvectorReductionB = self.eigenvectorReductionB
                alg.efficiencyTool.EigenvectorReductionC = self.eigenvectorReductionC
                alg.efficiencyTool.EigenvectorReductionLight = self.eigenvectorReductionLight
                alg.efficiencyTool.ExcludeFromEigenVectorTreatment = self.excludeFromEigenVectorTreatment
                alg.efficiencyTool.ExcludeFromEigenVectorBTreatment = self.excludeFromEigenVectorBTreatment
                alg.efficiencyTool.ExcludeFromEigenVectorCTreatment = self.excludeFromEigenVectorCTreatment
                alg.efficiencyTool.ExcludeFromEigenVectorLightTreatment = self.excludeFromEigenVectorLightTreatment
                alg.efficiencyTool.ExcludeRecommendedFromEigenVectorTreatment = self.excludeRecommendedFromEigenVectorTreatment
            if DSID != "default":
                alg.efficiencyTool.EfficiencyBCalibrations = DSID
                alg.efficiencyTool.EfficiencyTCalibrations = DSID
                alg.efficiencyTool.EfficiencyCCalibrations = DSID
                alg.efficiencyTool.EfficiencyLightCalibrations = DSID
            alg.scaleFactorDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
            alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
            alg.onlyEfficiency = self.btagWP == 'Continuous'
            alg.outOfValidity = 2
            alg.outOfValidityDeco = 'no_ftag_' + selectionName
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, selectionName + '_eff')

            if self.globalSF:
                alg = config.createAlgorithm('CP::AsgEventScaleFactorAlg',
                                             'FTagEventScaleFactorAlg' + postfix)
                preselection = config.getFullSelection(self.containerName, '')
                alg.preselection = ((preselection + '&&' if preselection else '')
                                    + 'ftag_kin_select_' + selectionName + ',as_char')
                alg.scaleFactorInputDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
                alg.scaleFactorOutputDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
                alg.particles = config.readName(self.containerName)

                config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration,
                                    'weight_ftag_effSF_' + selectionName)


def makeFTagAnalysisConfig( seq, containerName,
                            selectionName,
                            btagWP = None,
                            btagger = None,
                            generator = None,
                            kinematicSelection = None,
                            noEffSF = None,
                            globalSF = None,
                            minPt = None ):
    """Create a ftag analysis algorithm config

    Keyword arguments:
      btagWP -- Flavour tagging working point
      btagger -- Flavour tagger
      generator -- Generator for MC/MC scale factors
      kinematicSelection -- Wether to run kinematic selection
      noEffSF -- Disables efficiency and scale factor calculations
      globalSF -- Compute event level FTAG scale factor
      minPt -- Kinematic selection for jet calibration validity (depending on jet collection)
    """

    config = FTagConfig (containerName, selectionName)
    if btagWP is not None :
        config.setOptionValue ('btagWP', btagWP)
    if btagger is not None :
        config.setOptionValue ('btagger', btagger)
    if generator is not None :
        config.setOptionValue ('generator', generator)
    if kinematicSelection is not None :
        config.setOptionValue ('kinematicSelection', kinematicSelection)
    if noEffSF is not None :
        config.setOptionValue ('noEffSF', noEffSF)
    if globalSF is not None :
        config.setOptionValue ('globalSF', globalSF)
    if minPt is not None :
        config.setOptionValue ('minPt', minPt)
    seq.append (config)
