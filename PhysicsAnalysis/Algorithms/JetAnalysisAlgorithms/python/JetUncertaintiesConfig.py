# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration



# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.Logging import logging
import re


class JetUncertaintiesConfig (ConfigBlock) :
    """the ConfigBlock for the common preprocessing of jet uncertainties"""

    def __init__ (self) :
        super (JetUncertaintiesConfig, self).__init__ ()
        self.setBlockName('Uncertainties')
        self.addDependency('OverlapRemoval', required=False)
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('jetInput', '', type=str,
            noneAction='error',
            info="")
        self.addOption('analysisJetSelection', '', type=str,
            info="the jet selection to use to calculate N jets for an analysis specific "
            "jet flavor composition uncertainty. Of the form jvt_selection,as_char&&passesOR,as_char...")
        self.addOption('analysisFile', '', type=str,
            info="the file containing gluon fraction histograms needed to calculate an analysis specific "
            "jet flavor composition uncertainty.")
        self.addOption ('largeRMass', "Comb", type=str,
            info="")
        self.addOption ('systematicsModelJES', "Category", type=str,
            info="the NP reduction scheme to use for JES: All, Global, Category, "
            "Scenario. The default is Category.")
        self.addOption ('systematicsModelJER', "Full", type=str,
            info="the NP reduction scheme to use for JER: All, Full, Simple. The "
            "default is Full.")
        self.addOption ('systematicsModelJMS', "Full", type=str)
        self.addOption ('systematicsModelJMR', "Full", type=str,
            info="the NP reduction scheme to use for JMR: Full, Simple. The default is Full.")
        self.addOption ('runJERsystematicsOnData', False, type=bool,
            info="whether to run the All/Full JER model variations also on data samples. Expert option!")
        # Uncertainties tool options
        self.addOption ('uncertToolConfigPath', None, type=str,
            info="name (str) of the config file to use for the jet uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.")
        self.addOption ('uncertToolCalibArea', None, type=str,
            info="name (str) of the CVMFS area to use for the jet uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.")
        self.addOption ('uncertToolMCType', None, type=str,
            info="data type (str) to use for the jet uncertainty tool (e.g. "
            "'AF3' or 'MC16'). Expert option to override JetETmiss "
            "recommendations. The default is None.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def getUncertaintyToolSettingsSmallRJets(self, config):

        # Retrieve appropriate JES/JER recommendations for the JetUncertaintiesTool.
        # We do this separately from the tool declaration, as we may need to set uo
        # two such tools, but they have to be private.

        # Config file:
        config_file = None
        if self.systematicsModelJES == "All" and self.systematicsModelJER == "All":
            config_file = "R4_AllNuisanceParameters_AllJERNP.config"
        elif "Scenario" in self.systematicsModelJES:
            if self.systematicsModelJER != "Simple":
                raise ValueError(
                    "Invalid uncertainty configuration - Scenario* systematicsModelJESs can "
                    "only be used together with the Simple systematicsModelJER")
            config_file = "R4_{0}_SimpleJER.config".format(self.systematicsModelJES)
        elif self.systematicsModelJES in ["Global", "Category"] and self.systematicsModelJER in ["Simple", "Full"]:
            config_file = "R4_{0}Reduction_{1}JER.config".format(self.systematicsModelJES, self.systematicsModelJER)
        else:
            raise ValueError(
                "Invalid combination of systematicsModelJES and systematicsModelJER settings: "
                "systematicsModelJES: {0}, systematicsModelJER: {1}".format(self.systematicsModelJES, self.systematicsModelJER) )

        # Calibration area:
        calib_area = None
        if self.uncertToolCalibArea is not None:
            calib_area = self.uncertToolCalibArea

        # Expert override for config path:
        if self.uncertToolConfigPath is not None:
            config_file = self.uncertToolConfigPath
        else:
            if config.geometry() is LHCPeriod.Run2:
                if config.dataType() is DataType.FastSim:
                    config_file = "rel22/Fall2024_PreRec/" + config_file
                else:
                    if self.jetInput == "HI":
                        config_file = "HIJetUncertainties/Spring2023/HI" + config_file
                    else:
                        config_file = "rel22/Summer2023_PreRec/" + config_file
            else:
                if config.dataType() is DataType.FastSim:
                    config_file = "rel22/Winter2025_AF3_PreRec/" + config_file
                else:
                    if self.jetInput == "HI":
                        config_file = "HIJetUncertainties/Spring2023/HI" + config_file
                    else:
                        config_file = "rel22/Winter2025_PreRec/" + config_file

        # MC type:
        mc_type = None
        if self.uncertToolMCType is not None:
            mc_type = self.uncertToolMCType
        else:
            if config.geometry() is LHCPeriod.Run2:
                if config.dataType() is DataType.FastSim:
                    mc_type = "AF3"
                else:
                    mc_type = "MC20"
            else:
                if config.dataType() is DataType.FastSim:
                    mc_type = "MC23AF3"
                else:
                    if self.jetInput == "HI":
                        mc_type = "MC16"
                    else:
                        mc_type = "MC23"

        return config_file, calib_area, mc_type


    def createUncertaintyToolSmallRJets(self, jetUncertaintiesAlg, config, jetCollectionName, doPseudoData=False):

        # Create an instance of JetUncertaintiesTool, following JetETmiss recommendations.
        # To run Jet Energy Resolution (JER) uncertainties in the "Full" or "All" schemes,
        # we need two sets of tools: one configured as normal (MC), the other with the
        # exact same settings but pretending to run on data (pseudo-data).
        # This is achieved by passing "isPseudoData=True" to the arguments.

        # Retrieve the common configuration settings
        configFile, calibArea, mcType = self.getUncertaintyToolSettingsSmallRJets(config)

        # The main tool for all JES+JER combinations
        config.addPrivateTool( 'uncertaintiesTool', 'JetUncertaintiesTool' )
        jetUncertaintiesAlg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
        jetUncertaintiesAlg.uncertaintiesTool.ConfigFile = configFile
        if calibArea is not None:
            jetUncertaintiesAlg.uncertaintiesTool.CalibArea = calibArea
        jetUncertaintiesAlg.uncertaintiesTool.MCType = mcType
        jetUncertaintiesAlg.uncertaintiesTool.IsData = (config.dataType() is DataType.Data)
        jetUncertaintiesAlg.uncertaintiesTool.PseudoDataJERsmearingMode = False

        if config.dataType() is DataType.Data and not (doPseudoData and self.runJERsystematicsOnData):
            # we don't want any systematics on data if we're not using the right JER model!
            jetUncertaintiesAlg.affectingSystematicsFilter = '.*'
        if config.dataType() is not DataType.Data and doPseudoData and not self.runJERsystematicsOnData:
            # The secondary tool for pseudo-data JER smearing
            config.addPrivateTool( 'uncertaintiesToolPD', 'JetUncertaintiesTool' )
            jetUncertaintiesAlg.uncertaintiesToolPD.JetDefinition = jetCollectionName[:-4]
            jetUncertaintiesAlg.uncertaintiesToolPD.ConfigFile = configFile
            if calibArea is not None:
                jetUncertaintiesAlg.uncertaintiesToolPD.CalibArea = calibArea
            jetUncertaintiesAlg.uncertaintiesToolPD.MCType = mcType

            # This is the part that is different!
            jetUncertaintiesAlg.uncertaintiesToolPD.IsData = True
            jetUncertaintiesAlg.uncertaintiesToolPD.PseudoDataJERsmearingMode = True

    def getUncertaintyToolSettingsLargeRJets(self, config):
        # Retrieve appropriate JES/JER recommendations for the JetUncertaintiesTool.
        # We do this separately from the tool declaration, as we may need to set uo
        # two such tools, but they have to be private.

        log = logging.getLogger('LargeRJetAnalysisConfig')

        # Config file:
        config_file = None
        if self.systematicsModelJER in ["Simple", "Full"] and self.systematicsModelJMS in ["Simple", "Full"]:
            config_file = "R10_CategoryJES_{0}JER_{1}JMS.config".format(self.systematicsModelJER, self.systematicsModelJMS)
        else:
            raise ValueError(
                "Invalid request for systematicsModelJER/JMS settings: "
                "systematicsModelJER = '{0}', "
                "systematicsModelJMS = '{1}'".format(self.systematicsModelJER, self.systematicsModelJMS) )
        if self.uncertToolConfigPath is not None:
            # Expert override
            config_file = self.uncertToolConfigPath
        else:
            if config.geometry() in [LHCPeriod.Run2, LHCPeriod.Run3]:
                config_file = "rel22/Summer2025_PreRec/" + config_file
            else:
                log.warning("Uncertainties for UFO jets are not for Run 4!")

        # Calibration area:
        calib_area = None
        if self.uncertToolCalibArea is not None:
            calib_area = self.uncertToolCalibArea
                
        # MC type:
        if self.uncertToolMCType is not None:
            mc_type = self.uncertToolMCType
        else:
            if config.dataType() is DataType.FastSim:
                if config.geometry() is LHCPeriod.Run2:
                    mc_type = "MC20AF3"
                else:
                    mc_type = "MC23AF3"
            else:
                if config.geometry() is LHCPeriod.Run2:
                    mc_type = "MC20"
                else:
                    mc_type = "MC23"

        return config_file, calib_area, mc_type

    def createUncertaintyToolLargeRJets(self, jetUncertaintiesAlg, config, jetCollectionName, doPseudoData=False):
        '''
        Create instance(s) of JetUncertaintiesTool following JetETmiss recommendations.

        JER uncertainties under the "Full" scheme must be run on MC samples twice:
        1. Normal (MC) mode,
        2. Pseudodata (PD) mode, as if the events are Data.
        '''

        # Retrieve the common configuration settings
        configFile, calibArea, mcType = self.getUncertaintyToolSettingsLargeRJets(config)

        # The main tool for all JER combinations
        config.addPrivateTool( 'uncertaintiesTool', 'JetUncertaintiesTool' )
        jetUncertaintiesAlg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
        jetUncertaintiesAlg.uncertaintiesTool.ConfigFile = configFile
        from PathResolver import PathResolver
        if self.analysisFile is not None:
          jetUncertaintiesAlg.uncertaintiesTool.AnalysisFile = PathResolver.FindCalibFile(self.analysisFile)
        if calibArea is not None:
            jetUncertaintiesAlg.uncertaintiesTool.CalibArea = calibArea
        jetUncertaintiesAlg.uncertaintiesTool.MCType = mcType
        jetUncertaintiesAlg.uncertaintiesTool.IsData = (config.dataType() is DataType.Data)
        jetUncertaintiesAlg.uncertaintiesTool.PseudoDataJERsmearingMode = False
        jetUncertaintiesAlg.uncertaintiesTool.NJetAccessorName = "Njet_NOSYS"

        # JER smearing on data 
        if config.dataType() is DataType.Data and not (config.isPhyslite() and doPseudoData and self.runJERsystematicsOnData):
            # we don't want any systematics on data if we're not using the right JER model!
            jetUncertaintiesAlg.affectingSystematicsFilter = '.*'

        if config.dataType() is not (DataType.Data and config.isPhyslite()) and doPseudoData and not self.runJERsystematicsOnData:
            # The secondary tool for pseudo-data JER smearing
            config.addPrivateTool( 'uncertaintiesToolPD', 'JetUncertaintiesTool' )
            jetUncertaintiesAlg.uncertaintiesToolPD.JetDefinition = jetCollectionName[:-4]
            jetUncertaintiesAlg.uncertaintiesToolPD.ConfigFile = configFile
            if calibArea is not None:
                jetUncertaintiesAlg.uncertaintiesToolPD.CalibArea = calibArea
            jetUncertaintiesAlg.uncertaintiesToolPD.MCType = mcType
            jetUncertaintiesAlg.uncertaintiesToolPD.IsData = True
            jetUncertaintiesAlg.uncertaintiesToolPD.PseudoDataJERsmearingMode = True

    def makeAlgs (self, config) :

        jetCollectionName=config.originalName(self.containerName)
        if(config.originalName(self.containerName)=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(config.originalName(self.containerName)=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

        # interpret the jet collection
        collection_pattern = re.compile(
            r"AntiKt(\d+)(EMTopo|EMPFlow|LCTopo|TrackCaloCluster|UFO|Track|HI)(TrimmedPtFrac5SmallR20|CSSKSoftDropBeta100Zcut10)?Jets")
        match = collection_pattern.match(jetCollectionName)
        if not match:
            raise ValueError(
                "Jet collection {0} does not match expected pattern!".format(jetCollectionName) )
        radius = int(match.group(1) )
        if radius not in [2, 4, 6, 10]:
            raise ValueError("Jet collection has an unsupported radius '{0}'!".format(radius) )

        if (self.analysisJetSelection!= ''):
            alg = config.createAlgorithm( 'CP::NJetDecoratorAlg', 'NJetDecoratorAlg' )
            alg.jets = config.readName(self.containerName)
            alg.jetSelection = self.analysisJetSelection
            config.addOutputVar('EventInfo', 'Njet_%SYS%', 'Njet')

        # Jet uncertainties
        if (radius == 4):
            alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg' )
            self.createUncertaintyToolSmallRJets(alg, config, jetCollectionName, doPseudoData=( self.systematicsModelJER in ["Full","All"] ))
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        elif (radius == 10):
            if self.jetInput == "UFO" and config.dataType() in [DataType.FullSim, DataType.FastSim]:
                alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg' )
                self.createUncertaintyToolLargeRJets(alg, config, jetCollectionName, doPseudoData=( self.systematicsModelJER in ["Full","All"] ))

                alg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]

                # R=1.0 jets have a validity range
                alg.outOfValidity = 2 # SILENT
                alg.outOfValidityDeco = 'outOfValidity'

                alg.jets = config.readName (self.containerName)
                alg.jetsOut = config.copyName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')

            if self.jetInput != "UFO":
                alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg' )

                # R=1.0 jets have a validity range
                alg.outOfValidity = 2 # SILENT
                alg.outOfValidityDeco = 'outOfValidity'
                config.addPrivateTool( 'uncertaintiesTool', 'JetUncertaintiesTool' )

                alg.uncertaintiesTool.JetDefinition = jetCollectionName[:-4]
                alg.uncertaintiesTool.ConfigFile = \
                    "rel21/Moriond2018/R10_{0}Mass_all.config".format(self.largeRMass)
                alg.uncertaintiesTool.MCType = "MC16a"
                alg.uncertaintiesTool.IsData = (config.dataType() is DataType.Data)

                alg.jets = config.readName (self.containerName)
                alg.jetsOut = config.copyName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')
                config.addSelection (self.containerName, '', 'outOfValidity')
