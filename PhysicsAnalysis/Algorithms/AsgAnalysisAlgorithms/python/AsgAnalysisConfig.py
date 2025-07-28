# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaConfiguration.Enums import LHCPeriod
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from enum import Enum

class SystematicsCategories(Enum):
    JETS = ['JET_']
    JER = ['JET_JER']
    ELECTRONS = ['EG_', 'EL_']
    MUONS = ['MUON_']
    PHOTONS = ['EG_', 'PH_']
    TAUS = ['TAUS_']
    MET = ['MET_']
    EVENT = ['GEN_', 'PRW_']
    FTAG = ['FT_']

class CommonServicesConfig (ConfigBlock) :
    """the ConfigBlock for common services

    The idea here is that all algorithms need some common services, and I should
    provide configuration blocks for those.  For now there is just a single
    block, but in the future I might break out e.g. the systematics service.
    """

    def __init__ (self) :
        super (CommonServicesConfig, self).__init__ ()
        self.addOption ('runSystematics', None, type=bool,
            info="whether to turn on the computation of systematic variations. "
            "The default is to run them on MC.")
        self.addOption ('filterSystematics', None, type=str,
            info="a regexp string against which the systematics names will be "
            "matched. Only positive matches are retained and used in the evaluation "
            "of the various algorithms.")
        self.addOption ('onlySystematicsCategories', None, type=list,
            info="a list of strings defining categories of systematics to enable "
            "(only recommended for studies / partial ntuple productions). Choose amongst: "
            "jets, electrons, muons, photons, taus, met, ftag, event. This option is overridden "
            "by 'filterSystematics'.")
        self.addOption ('systematicsHistogram', None , type=str,
            info="the name (string) of the histogram to which a list of executed "
            "systematics will be printed. The default is None (don't write out "
            "the histogram).")

    def makeAlgs (self, config) :

        sysService = config.createService( 'CP::SystematicsSvc', 'SystematicsSvc' )

        if self.runSystematics is False :
            runSystematics = self.runSystematics
        elif config.noSystematics() is not None :
            # if option not set:
            # check to see if set in config accumulator
            self.runSystematics = not config.noSystematics()
            runSystematics = self.runSystematics
        else :
            runSystematics = True

        if runSystematics :
            sysService.sigmaRecommended = 1
            if config.dataType() is DataType.Data:
                # Only one type of allowed systematics on data: the JER variations!
                self.onlySystematicsCategories = ['JER']
            if self.onlySystematicsCategories is not None:
                # Convert strings to enums and validate
                requested_categories = []
                for category_str in self.onlySystematicsCategories:
                    try:
                        category_enum = SystematicsCategories[category_str.upper()]
                        requested_categories += category_enum.value
                    except KeyError:
                        raise ValueError(f"Invalid systematics category passed to option 'onlySystematicsCategories': {category_str}. Must be one of {', '.join(category.name for category in SystematicsCategories)}")
                # Construct regex pattern as logical-OR of category names
                if len(requested_categories):
                    sysService.systematicsRegex = "^(?=.*(" + "|".join(requested_categories) + ")|$).*"
            if self.filterSystematics is not None:
                sysService.systematicsRegex = self.filterSystematics
        config.createService( 'CP::SelectionNameSvc', 'SelectionNameSvc')

        if self.systematicsHistogram is not None:
            sysDumper = config.createAlgorithm( 'CP::SysListDumperAlg', 'SystematicsPrinter' )
            sysDumper.histogramName = self.systematicsHistogram


class IOStatsBlock(ConfigBlock):
    """Print what branches are used in analysis"""

    def __init__(self):
        super(IOStatsBlock, self).__init__()
        self.addOption("printOption", "Summary", type=str,
                       info='option to pass the standard ROOT printing function. Can be "Summary", "ByEntries" or "ByBytes".')

    def makeAlgs(self, config):
        alg = config.createAlgorithm('CP::IOStatsAlg', 'IOStatsAlg')
        alg.printOption = self.printOption


class PileupReweightingBlock (ConfigBlock):
    """the ConfigBlock for pileup reweighting"""

    def __init__ (self) :
        super (PileupReweightingBlock, self).__init__ ()
        self.addOption ('campaign', None, type=None,
            info="the MC campaign for the PRW auto-configuration.")
        self.addOption ('files', None, type=None,
            info="the input files being processed (list of strings). "
            "Alternative to auto-configuration.")
        self.addOption ('useDefaultConfig', True, type=bool,
            info="whether to use the central PRW files. The default is True.")
        self.addOption ('userLumicalcFiles', None, type=None,
            info="user-provided lumicalc files (list of strings). Alternative "
            "to auto-configuration.")
        self.addOption ('userLumicalcFilesPerCampaign', None, type=None,
            info="user-provided lumicalc files (dictionary of list of strings, "
            "with MC campaigns as the keys). Alternative to auto-configuration.")
        self.addOption ('userPileupConfigs', None, type=None,
            info="user-provided PRW files (list of strings). Alternative to "
            "auto-configuration. Alternative to auto-configuration.")
        self.addOption ('userPileupConfigsPerCampaign', None, type=None,
            info="user-provided PRW files (dictionary of list of strings, with "
            "MC campaigns as the keys)")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed unless several instances of PileupReweighting are scheduled.")
        self.addOption ('alternativeConfig', False, type=bool,
            info="whether this is used as an additional alternative config for PileupReweighting. "
            "Will only store the alternative pile up weight in that case.")
        self.addOption ('writeColumnarToolVariables', False, type=bool,
            info="whether to add EventInfo variables needed for running the columnar tool(s) on the output n-tuple. (EXPERIMENTAL)")


    def makeAlgs (self, config) :

        from Campaigns.Utils import Campaign

        try:
            from AthenaCommon.Logging import logging
        except ImportError:
            import logging
        log = logging.getLogger('makePileupAnalysisSequence')

        eventInfoVar = ['runNumber', 'eventNumber', 'actualInteractionsPerCrossing', 'averageInteractionsPerCrossing']
        if config.dataType() is not DataType.Data:
            eventInfoVar += ['mcChannelNumber']
        if self.writeColumnarToolVariables:
            # This is not strictly necessary, as the columnar users
            # could recreate this, but it is also a single constant int,
            # that should compress exceedingly well.
            eventInfoVar += ['eventTypeBitmask']

        if config.isPhyslite() and not self.alternativeConfig:
            # PHYSLITE already has these variables defined, just need to copy them to the output
            log.info(f'Physlite does not need pileup reweighting. Variables will be copied from input instead. {config.isPhyslite}')
            for var in eventInfoVar:
                config.addOutputVar ('EventInfo', var, var, noSys=True)

            if config.dataType() is not DataType.Data:
                config.addOutputVar ('EventInfo', 'PileupWeight_%SYS%', 'weight_pileup')
                if config.geometry() is LHCPeriod.Run2:
                    config.addOutputVar ('EventInfo', 'beamSpotWeight', 'weight_beamspot', noSys=True)
            return

        # check files from autoconfig flags
        if self.files is None and config.autoconfigFlags() is not None:
            self.files = config.autoconfigFlags().Input.Files

        campaign = self.campaign
        # if user didn't explicitly configure campaign, let's try setting it from metadata
        # only needed on MC
        if config.dataType() is not DataType.Data and self.campaign is None:
            # if we used autoconfigflags, campaign is auto-determined
            if config.campaign() is not None and config.campaign() is not Campaign.Unknown:
                campaign = config.campaign()
                log.info(f'Auto-configuring campaign for PRW from flags: {campaign.value}')
            else:
                # we try to determine campaign from files if above failed
                if self.files is not None:
                    from Campaigns.Utils import getMCCampaign
                    campaign = getMCCampaign(self.files)
                    if campaign and campaign is not Campaign.Unknown:
                        log.info(f'Auto-configuring campaign for PRW from files: {campaign.value}')
                    else:
                        log.info('Campaign could not be determined.')


        toolConfigFiles = []
        toolLumicalcFiles = []

        # PRW config files should only be configured if we run on MC
        # Run 4 not supported yet
        if (config.dataType() is not DataType.Data and
            config.geometry() is not LHCPeriod.Run4):
            # check if user provides per-campaign pileup config list
            if self.userPileupConfigs is not None and self.userPileupConfigsPerCampaign is not None:
                raise ValueError('Both userPileupConfigs and userPileupConfigsPerCampaign specified, '
                                 'use only one of the options!')
            if self.userPileupConfigsPerCampaign is not None:
                if not campaign:
                    raise Exception('userPileupConfigsPerCampaign requires campaign to be configured!')
                if campaign is Campaign.Unknown:
                    raise Exception('userPileupConfigsPerCampaign used, but campaign = Unknown!')
                try:
                    toolConfigFiles = self.userPileupConfigsPerCampaign[campaign.value][:]
                    log.info('Using user provided per-campaign PRW configuration')
                except KeyError as e:
                    raise KeyError(f'Unconfigured campaign {e} for userPileupConfigsPerCampaign!')

            elif self.userPileupConfigs is not None:
                toolConfigFiles = self.userPileupConfigs[:]
                log.info('Using user provided PRW configuration')

            else:
                if self.useDefaultConfig and self.files is None:
                    raise ValueError('useDefaultConfig requires files to be configured! '
                                    'Either pass them as an option or use flags.')

                from PileupReweighting.AutoconfigurePRW import getConfigurationFiles
                if campaign and campaign is not Campaign.Unknown:
                    toolConfigFiles = getConfigurationFiles(campaign=campaign,
                                                            files=self.files,
                                                            useDefaultConfig=self.useDefaultConfig,
                                                            data_type=config.dataType())
                    if self.useDefaultConfig:
                        log.info('Auto-configuring universal/default PRW config')
                    else:
                        log.info('Auto-configuring per-sample PRW config files based on input files')
                else:
                    log.info('No campaign specified, no PRW config files configured')

            # check if user provides per-campaign lumical config list
            if self.userLumicalcFilesPerCampaign is not None and self.userLumicalcFiles is not None:
                raise ValueError('Both userLumicalcFiles and userLumicalcFilesYear specified, '
                                'use only one of the options!')
            if self.userLumicalcFilesPerCampaign is not None:
                try:
                    toolLumicalcFiles = self.userLumicalcFilesPerCampaign[campaign.value][:]
                    log.info('Using user-provided per-campaign lumicalc files')
                except KeyError as e:
                    raise KeyError(f'Unconfigured campaign {e} for userLumicalcFilesPerCampaign!')
            elif self.userLumicalcFiles is not None:
                toolLumicalcFiles = self.userLumicalcFiles[:]
                log.info('Using user-provided lumicalc files')
            else:
                if campaign and campaign is not Campaign.Unknown:
                    from PileupReweighting.AutoconfigurePRW import getLumicalcFiles
                    toolLumicalcFiles = getLumicalcFiles(campaign)
                    log.info('Using auto-configured lumicalc files')
                else:
                    log.info('No campaign specified, no lumicalc files configured for PRW')
        else:
            log.info('Data needs no lumicalc and PRW configuration files')

        # Set up the only algorithm of the sequence:
        if config.geometry() is LHCPeriod.Run4:
            log.warning ('Pileup reweighting is not yet supported for Run 4 geometry')
            alg = config.createAlgorithm( 'CP::EventDecoratorAlg', 'EventDecoratorAlg'+self.postfix )
            alg.uint32Decorations = { 'RandomRunNumber' :
                                      config.autoconfigFlags().Input.RunNumbers[0] }

        else:
            alg = config.createAlgorithm( 'CP::PileupReweightingAlg',
                                        'PileupReweightingAlg'+self.postfix )
            config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
            alg.pileupReweightingTool.ConfigFiles = toolConfigFiles
            if not toolConfigFiles and config.dataType() is not DataType.Data:
                log.info("No PRW config files provided. Disabling reweighting")
                # Setting the weight decoration to the empty string disables the reweighting
                alg.pileupWeightDecoration = ""
            else:
                alg.pileupWeightDecoration = "PileupWeight" + self.postfix + "_%SYS%"
            alg.pileupReweightingTool.LumiCalcFiles = toolLumicalcFiles

        if not self.alternativeConfig:
            for var in eventInfoVar:
                config.addOutputVar ('EventInfo', var, var, noSys=True)

            if config.dataType() is not DataType.Data and config.geometry() is LHCPeriod.Run2:
                config.addOutputVar ('EventInfo', 'beamSpotWeight', 'weight_beamspot', noSys=True)

        if config.dataType() is not DataType.Data and toolConfigFiles:
            config.addOutputVar ('EventInfo', 'PileupWeight' + self.postfix + '_%SYS%',
                                 'weight_pileup'+self.postfix)


class GeneratorAnalysisBlock (ConfigBlock):
    """the ConfigBlock for generator algorithms"""

    def __init__ (self) :
        super (GeneratorAnalysisBlock, self).__init__ ()
        self.addOption ('saveCutBookkeepers', True, type=bool,
            info="whether to save the cut bookkeepers information into the "
            "output file. The default is True.")
        self.addOption ('runNumber', None, type=int,
            info="the MC runNumber (int). The default is None (autoconfigure "
            "from metadata).")
        self.addOption ('cutBookkeepersSystematics', None, type=bool,
            info="whether to also save the cut bookkeepers systematics. The "
            "default is None (follows the global systematics flag). Set to "
            "False or True to override.")
        self.addOption ('histPattern', None, type=str,
            info="the histogram name pattern for the cut-bookkeeper histogram names")
        self.addOption ('streamName', 'ANALYSIS', type=str,
            info="name of the output stream to save the cut bookkeeper in. "
            "The default is ANALYSIS.")
        self.addOption ('detailedPDFinfo', False, type=bool,
            info="save the necessary information to run the LHAPDF tool offline. "
                 "The default is False.")

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data:
            # there are no generator weights in data!
            return

        if self.runNumber is None:
            self.runNumber = config.runNumber()

        if self.saveCutBookkeepers and not self.runNumber:
            raise ValueError ("invalid run number: " + str(self.runNumber))

        # Set up the CutBookkeepers algorithm:
        if self.saveCutBookkeepers:
            alg = config.createAlgorithm('CP::AsgCutBookkeeperAlg', 'CutBookkeeperAlg' + self.streamName)
            alg.RootStreamName = self.streamName
            alg.runNumber = self.runNumber
            if self.cutBookkeepersSystematics is None:
                alg.enableSystematics = not config.noSystematics()
            else:
                alg.enableSystematics = self.cutBookkeepersSystematics
            if self.histPattern:
                alg.histPattern = self.histPattern
            config.addPrivateTool( 'truthWeightTool', 'PMGTools::PMGTruthWeightTool' )

        # Set up the weights algorithm:
        alg = config.createAlgorithm( 'CP::PMGTruthWeightAlg', 'PMGTruthWeightAlg' + self.streamName )
        config.addPrivateTool( 'truthWeightTool', 'PMGTools::PMGTruthWeightTool' )
        alg.decoration = 'generatorWeight_%SYS%'
        config.addOutputVar ('EventInfo', 'generatorWeight_%SYS%', 'weight_mc')

        if self.detailedPDFinfo:
            alg = config.createAlgorithm( 'CP::PDFinfoAlg', 'PDFinfoAlg' + self.streamName, reentrant=True )
            for var in ["PDFID1","PDFID2","PDGID1","PDGID2","Q","X1","X2","XF1","XF2"]:
                config.addOutputVar ('EventInfo', var, 'PDFinfo_' + var, noSys=True)


class PtEtaSelectionBlock (ConfigBlock):
    """the ConfigBlock for a pt-eta selection"""

    def __init__ (self) :
        super (PtEtaSelectionBlock, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the selection to append this to. The default is "
            "'' (empty string), meaning that the cuts are applied to every "
            "object within the container. Specifying a name (e.g. loose) "
            "applies the cut only to those object who also pass that selection.")
        self.addOption ('minPt', None, type=float,
            info="minimum pT value to cut on, in MeV. No default value.")
        self.addOption ('maxPt', None, type=float,
            info="maximum pT value to cut on, in MeV. No default value.")
        self.addOption ('minEta', None, type=float,
            info="minimum |eta| value to cut on. No default value.")
        self.addOption ('maxEta', None, type=float,
            info="maximum |eta| value to cut on. No default value.")
        self.addOption ('etaGapLow', None, type=float,
            info="low end of the |eta| gap. No default value.")
        self.addOption ('etaGapHigh', None, type=float,
            info="high end of the |eta| gap. No default value.")
        self.addOption ('selectionDecoration', None, type=str,
            info="the name of the decoration to set. If 'None', will be set "
            "to 'selectPtEta' followed by the selection name.")
        self.addOption ('useClusterEta', False, type=bool,
            info="whether to use the cluster eta (etaBE(2)) instead of the object "
            "eta (for electrons and photons). The default is False.")
        self.addOption ('useDressedProperties', False, type=bool,
            info="whether to use the dressed kinematic properties "
            "(for truth particles only). The default is False.")


    def makeAlgs (self, config) :

        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PtEtaSelectionAlg' + self.containerName + self.selectionName )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        if self.minPt is not None :
            alg.selectionTool.minPt = self.minPt
        if self.maxPt is not None:
            alg.selectionTool.maxPt = self.maxPt
        if self.minEta is not None:
            alg.selectionTool.minEta = self.minEta
        if self.maxEta is not None :
            alg.selectionTool.maxEta = self.maxEta
        if self.etaGapLow is not None:
            alg.selectionTool.etaGapLow = self.etaGapLow
        if self.etaGapHigh is not None:
            alg.selectionTool.etaGapHigh = self.etaGapHigh
        if self.selectionDecoration is None:
            self.selectionDecoration = 'selectPtEta' + (f'_{self.selectionName}' if self.selectionName else '')
        alg.selectionTool.useClusterEta = self.useClusterEta
        alg.selectionTool.useDressedProperties = self.useDressedProperties
        alg.selectionDecoration = self.selectionDecoration
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)



class ObjectCutFlowBlock (ConfigBlock):
    """the ConfigBlock for an object cutflow"""

    def __init__ (self) :
        super (ObjectCutFlowBlock, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the selection to perform the cutflow for. The "
            "default is '' (empty string), meaning that the cutflow is "
            "performed for every object within the container. Specifying a "
            "name (e.g. loose) generates the cutflow only for those object "
            "that also pass that selection.")
        self.addOption ('forceCutSequence', False, type=bool,
            info="whether to force the cut sequence and not accept objects "
            "if previous cuts failed. The default is False.")

    def makeAlgs (self, config) :

        alg = config.createAlgorithm( 'CP::ObjectCutFlowHistAlg', 'CutFlowDumperAlg_' + self.containerName + '_' + self.selectionName )
        alg.histPattern = 'cflow_' + self.containerName + "_" + self.selectionName + '_%SYS%'
        alg.selections = config.getSelectionCutFlow (self.containerName, self.selectionName)
        alg.input = config.readName (self.containerName)
        alg.histTitle = "Object Cutflow: " + self.containerName + "." + self.selectionName
        alg.forceCutSequence = self.forceCutSequence


class EventCutFlowBlock (ConfigBlock):
    """the ConfigBlock for an event-level cutflow"""

    def __init__ (self) :
        super (EventCutFlowBlock, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container, typically EventInfo.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of an optional selection decoration to use.")
        self.addOption ('customSelections', [], type=None,
            info="the selections for which to generate cutflow histograms. If "
            "a single string, corresponding to a particular event selection, "
            "the event cutflow for that selection will be looked up. If a list "
            "of strings, will use explicitly those selections. If left blank, "
            "all selections attached to the container will be looked up.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply in the naming of cutflow histograms. Set "
            "it when defining multiple cutflows.")

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        alg = config.createAlgorithm( 'CP::EventCutFlowHistAlg', 'CutFlowDumperAlg_' + self.containerName + '_' + self.selectionName + postfix )
        alg.histPattern = 'cflow_' + self.containerName + "_" + self.selectionName + postfix + '_%SYS%'
        # find out which selection decorations to use
        if isinstance(self.customSelections, str):
            # user provides a dynamic reference to selections, corresponding to an EventSelection alg
            alg.selections = config.getEventCutFlow(self.customSelections)
        elif len(self.customSelections) > 0:
            # user provides a list of hardcoded selections
            alg.selections = self.customSelections
        else:
            # user provides nothing: get all available selections from EventInfo directly
            alg.selections = config.getSelectionCutFlow (self.containerName, self.selectionName)
        alg.selections = [sel+',as_char' for sel in alg.selections]
        if self.selectionName:
            alg.preselection = self.selectionName + '_%SYS%'
        alg.eventInfo = config.readName (self.containerName)
        alg.histTitle = "Event Cutflow: " + self.containerName + "." + self.selectionName


class OutputThinningBlock (ConfigBlock):
    """the ConfigBlock for output thinning"""

    def __init__ (self) :
        super (OutputThinningBlock, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here.")
        self.addOption ('selection', '', type=str,
            info="the name of an optional selection decoration to use.")
        self.addOption ('selectionName', '', type=str,
            info="the name of the selection to append this to. The default is "
            "'' (empty string), meaning that the cuts are applied to every "
            "object within the container. Specifying a name (e.g. loose) "
            "applies the cut only to those object who also pass that selection.")
        self.addOption ('outputName', None, type=str,
            info="an optional name for the output container.")
        # TODO: add info string
        self.addOption ('deepCopy', False, type=bool,
            info="")
        self.addOption ('sortPt', False, type=bool,
            info="whether to sort objects in pt")
        # TODO: add info string
        self.addOption ('noUniformSelection', False, type=bool,
            info="")

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        selection = config.getFullSelection (self.containerName, self.selectionName)
        if selection == '' :
            selection = self.selection
        elif self.selection != '' :
            selection = selection + '&&' + self.selection

        if selection != '' and not self.noUniformSelection :
            alg = config.createAlgorithm( 'CP::AsgUnionSelectionAlg', 'UnionSelectionAlg' + self.containerName + postfix)
            alg.preselection = selection
            alg.particles = config.readName (self.containerName)
            alg.selectionDecoration = 'outputSelect' + postfix
            config.addSelection (self.containerName, alg.selectionDecoration, selection)
            selection = 'outputSelect' + postfix

        alg = config.createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'DeepCopyAlg' + self.containerName + postfix )
        alg.input = config.readName (self.containerName)
        if self.outputName is not None :
            alg.output = self.outputName + '_%SYS%'
            config.addOutputContainer (self.containerName, self.outputName)
        else :
            alg.output = config.copyName (self.containerName)
        if selection != '' :
            alg.selection = [selection]
        else :
            alg.selection = []
        alg.deepCopy = self.deepCopy
        if self.sortPt and not config.noSystematics() :
            raise ValueError ("Sorting by pt is not supported with systematics")
        alg.sortPt = self.sortPt


class IFFLeptonDecorationBlock (ConfigBlock):
    """the ConfigBlock for the IFF classification of leptons"""

    def __init__ (self) :
        super (IFFLeptonDecorationBlock, self).__init__()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input electron or muon container.")
        self.addOption ('separateChargeFlipElectrons', True, type=bool,
            info="whether to consider charged-flip electrons as a separate class. "
            "The default is True (recommended).")
        self.addOption ('decoration', 'IFFClass_%SYS%', type=str,
            info="the name (str) of the decoration set by the IFF "
            "TruthClassificationTool. The default is 'IFFClass_%SYS%'.")
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def makeAlgs (self, config) :
        particles = config.readName(self.containerName)

        alg = config.createAlgorithm( 'CP::AsgClassificationDecorationAlg', 'IFFClassifierAlg' + self.containerName )
        # the IFF classification tool
        config.addPrivateTool( 'tool', 'TruthClassificationTool')
        # label charge-flipped electrons as such
        alg.tool.separateChargeFlipElectrons = self.separateChargeFlipElectrons
        alg.decoration = self.decoration
        alg.particles = particles

        # write the decoration only once to the output
        config.addOutputVar(self.containerName, alg.decoration, alg.decoration.split("_%SYS%")[0], noSys=True)


class MCTCLeptonDecorationBlock (ConfigBlock):

    def __init__ (self) :
        super (MCTCLeptonDecorationBlock, self).__init__ ()

        self.addOption ("containerName", '', type=str,
                        noneAction='error',
                        info="the input lepton container, with a possible selection, "
                        "in the format container or container.selection.")
        self.addOption ("prefix", 'MCTC_', type=str,
                        info="the prefix (str) of the decorations based on the MCTC "
                        "classification. The default is 'MCTC_'.")
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def makeAlgs (self, config) :
        particles, selection = config.readNameAndSelection(self.containerName)

        alg = config.createAlgorithm ("CP::MCTCDecorationAlg", f"MCTCDecorationAlg{self.containerName}")
        alg.particles = particles
        alg.preselection = selection
        alg.affectingSystematicsFilter = '.*'
        config.addOutputVar (self.containerName, "MCTC_isPrompt", f"{self.prefix}isPrompt", noSys=True)
        config.addOutputVar (self.containerName, "MCTC_fromHadron", f"{self.prefix}fromHadron", noSys=True)
        config.addOutputVar (self.containerName, "MCTC_fromBSM", f"{self.prefix}fromBSM", noSys=True)
        config.addOutputVar (self.containerName, "MCTC_fromTau", f"{self.prefix}fromTau", noSys=True)


class PerEventSFBlock (ConfigBlock):
    """the ConfigBlock for the AsgEventScaleFactorAlg"""

    def __init__ (self):
        super(PerEventSFBlock, self).__init__()
        self.addOption('algoName', '', type=str,
            noneAction='error',
            info="unique name given to the underlying algorithm computing the "
            "per-event scale factors")
        self.addOption('particles', '', type=str,
            info="the input object container, with a possible selection, in the "
            "format container or container.selection.")
        self.addOption('objectSF', '', type=str,
            info="the name of the per-object SF decoration to be used.")
        self.addOption('eventSF', '', type=str,
            info="the name of the per-event SF decoration.")

    def makeAlgs(self, config):
        if config.dataType() is DataType.Data:
            return
        particles, selection = config.readNameAndSelection(self.particles)
        alg = config.createAlgorithm('CP::AsgEventScaleFactorAlg', self.algoName)
        alg.particles = particles
        alg.preselection = selection
        alg.scaleFactorInputDecoration = self.objectSF
        alg.scaleFactorOutputDecoration = self.eventSF

        config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration,
                            alg.scaleFactorOutputDecoration.split("_%SYS%")[0])


class SelectionDecorationBlock (ConfigBlock):
    """the ConfigBlock to add selection decoration to a container"""

    def __init__ (self) :
        super (SelectionDecorationBlock, self).__init__ ()
        # TODO: add info string
        self.addOption('containers', [], type=list,
            noneAction='error',
            info="")

    def makeAlgs(self, config):
        for container in self.containers:
            originContainerName = config.getOutputContainerOrigin(container)
            selectionNames = config.getSelectionNames(originContainerName)
            for selectionName in selectionNames:
                # skip default selection
                if selectionName == '':
                    continue
                alg = config.createAlgorithm(
                    'CP::AsgSelectionAlg',
                    f'SelectionDecoration_{originContainerName}_{selectionName}')
                selectionDecoration = f'baselineSelection_{selectionName}_%SYS%'
                alg.selectionDecoration =  f'{selectionDecoration},as_char'
                alg.particles = config.readName (originContainerName)
                alg.preselection = config.getFullSelection (originContainerName,
                                                            selectionName)
                config.addOutputVar(
                    originContainerName, selectionDecoration, selectionName)

def makeEventCutFlowConfig(seq, containerName,
                            *, postfix=None, selectionName, customSelections=None):
    """Create an event-level cutflow config

    Keyword arguments:
    containerName -- name of the container
    postfix -- a postfix to apply to decorations and algorithm names.
    selectionName -- the name of the selection to do the cutflow for
    customSelections -- a list of decorations to use in the cutflow, to override the retrieval of all decorations
    """

    config = EventCutFlowBlock()
    config.setOptionValue('containerName', containerName)
    config.setOptionValue('selectionName', selectionName)
    config.setOptionValue('postfix', postfix)
    config.setOptionValue('customSelections', customSelections)
    seq.append(config)
