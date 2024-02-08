# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaConfiguration.Enums import LHCPeriod
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class CommonServicesConfig (ConfigBlock) :
    """the ConfigBlock for common services

    The idea here is that all algorithms need some common services, and I should
    provide configuration blocks for those.  For now there is just a single
    block, but in the future I might break out e.g. the systematics service.
    """

    def __init__ (self) :
        super (CommonServicesConfig, self).__init__ ('CommonServices')
        self.addOption ('runSystematics', None, type=bool)
        self.addOption ('filterSystematics', None, type=str)

    def makeAlgs (self, config) :

        sysService = config.createService( 'CP::SystematicsSvc', 'SystematicsSvc' )

        if config.dataType() is not DataType.Data:
            if self.runSystematics is not None :
                runSystematics = self.runSystematics
            elif config.noSystematics() is not None :
                # if option not set:
                # check to see if set in config accumulator
                self.runSystematics = not config.noSystematics()
                runSystematics = self.runSystematics
            else :
                runSystematics = True
        else:
            runSystematics = False
        if runSystematics :
            sysService.sigmaRecommended = 1
            if self.filterSystematics is not None:
                sysService.systematicsRegex = self.filterSystematics
        config.createService( 'CP::SelectionNameSvc', 'SelectionNameSvc')



class PileupReweightingBlock (ConfigBlock):
    """the ConfigBlock for pileup reweighting"""

    def __init__ (self) :
        super (PileupReweightingBlock, self).__init__ ('Event')
        self.addOption ('campaign', None, type=None)
        self.addOption ('files', None, type=None)
        self.addOption ('useDefaultConfig', True, type=bool)
        self.addOption ('userLumicalcFiles', None, type=None)
        self.addOption ('userLumicalcFilesPerCampaign', None, type=None)
        self.addOption ('userPileupConfigs', None, type=None)
        self.addOption ('userPileupConfigsPerCampaign', None, type=None)


    def makeAlgs (self, config) :

        from Campaigns.Utils import Campaign

        try:
            from AthenaCommon.Logging import logging
        except ImportError:
            import logging
        log = logging.getLogger('makePileupAnalysisSequence')

        if config.isPhyslite():
            log.info(f'Physlite does not need pileup reweighting. {config.isPhyslite}')
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
        if config.dataType() is not DataType.Data:
            # check if user provides per-campaign pileup config list
            if self.userPileupConfigs and self.userPileupConfigsPerCampaign:
                raise ValueError('Both userPileupConfigs and userPileupConfigsPerCampaign specified, '
                                 'use only one of the options!')
            if self.userPileupConfigsPerCampaign:
                if not campaign:
                    raise Exception('userPileupConfigsPerCampaign requires campaign to be configured!')
                if campaign is Campaign.Unknown:
                    raise Exception('userPileupConfigsPerCampaign used, but campaign = Unknown!')
                try:
                    toolConfigFiles = self.userPileupConfigsPerCampaign[campaign.value][:]
                    log.info('Using user provided per-campaign PRW configuration')
                except KeyError as e:
                    raise KeyError(f'Unconfigured campaign {e} for userPileupConfigsPerCampaign!')

            elif self.userPileupConfigs:
                toolConfigFiles = self.userPileupConfigs[:]
                log.info('Using user provided PRW configuration')

            if not toolConfigFiles:
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
            if self.userLumicalcFilesPerCampaign and self.userLumicalcFiles:
                raise ValueError('Both userLumicalcFiles and userLumicalcFilesYear specified, '
                                'use only one of the options!')
            if self.userLumicalcFilesPerCampaign:
                try:
                    toolLumicalcFiles = self.userLumicalcFilesPerCampaign[campaign.value][:]
                    log.info('Using user-provided per-campaign lumicalc files')
                except KeyError as e:
                    raise KeyError(f'Unconfigured campaign {e} for userLumicalcFilesPerCampaign!')
            elif self.userLumicalcFiles:
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
        alg = config.createAlgorithm( 'CP::PileupReweightingAlg', 'PileupReweightingAlg' )
        config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
        alg.pileupReweightingTool.ConfigFiles = toolConfigFiles
        if not toolConfigFiles and config.dataType() is not DataType.Data:
            log.info("No PRW config files provided. Disabling reweighting")
            # Setting the weight decoration to the empty string disables the reweighting
            alg.pileupWeightDecoration = ""
        alg.pileupReweightingTool.LumiCalcFiles = toolLumicalcFiles
        config.addOutputVar ('EventInfo', 'runNumber', 'runNumber', noSys=True)
        config.addOutputVar ('EventInfo', 'eventNumber', 'eventNumber', noSys=True)

        if config.dataType() is not DataType.Data:
            config.addOutputVar ('EventInfo', 'mcChannelNumber', 'mcChannelNumber', noSys=True)
            if toolConfigFiles:
                config.addOutputVar ('EventInfo', 'PileupWeight_%SYS%', 'weight_pileup')
            if config.geometry() is LHCPeriod.Run2:
                config.addOutputVar ('EventInfo', 'beamSpotWeight', 'weight_beamspot', noSys=True)


class GeneratorAnalysisBlock (ConfigBlock):
    """the ConfigBlock for generator algorithms"""

    def __init__ (self) :
        super (GeneratorAnalysisBlock, self).__init__ ('Generator')
        self.addOption ('saveCutBookkeepers', True, type=bool)
        self.addOption ('runNumber', None, type=int)
        self.addOption ('cutBookkeepersSystematics', None, type=bool)

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
          alg = config.createAlgorithm('CP::AsgCutBookkeeperAlg', 'CutBookkeeperAlg')
          alg.runNumber = self.runNumber
          if self.cutBookkeepersSystematics:
              alg.enableSystematics = self.cutBookkeepersSystematics
          else:
              alg.enableSystematics = not config.noSystematics()
          config.addPrivateTool( 'truthWeightTool', 'PMGTools::PMGTruthWeightTool' )

        # Set up the weights algorithm:
        alg = config.createAlgorithm( 'CP::PMGTruthWeightAlg', 'PMGTruthWeightAlg' )
        config.addPrivateTool( 'truthWeightTool', 'PMGTools::PMGTruthWeightTool' )
        alg.decoration = 'generatorWeight_%SYS%'
        config.addOutputVar ('EventInfo', 'generatorWeight_%SYS%', 'weight_mc')


class PtEtaSelectionBlock (ConfigBlock):
    """the ConfigBlock for a pt-eta selection"""

    def __init__ (self, containerName, selectionName) :
        groupName = containerName
        if selectionName != '' :
            groupName += '.' + selectionName
        super (PtEtaSelectionBlock, self).__init__ (groupName)
        self.containerName = containerName
        self.selectionName = selectionName
        self.addOption ('postfix', '', type=str)
        self.addOption ('minPt', None, type=float,
                        duplicateAction='skip')
        self.addOption ('maxEta', None, type=float,
                        duplicateAction='skip')
        self.addOption ('selectionDecoration', 'selectPtEta', type=str)
        self.addOption ('useClusterEta', False, type=bool)

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PtEtaSelectionAlg' + self.containerName + postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        if self.minPt is not None :
            alg.selectionTool.minPt = self.minPt
        if self.maxEta is not None :
            alg.selectionTool.maxEta = self.maxEta
        alg.selectionTool.useClusterEta = self.useClusterEta
        alg.selectionDecoration = self.selectionDecoration
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)



class ObjectCutFlowBlock (ConfigBlock):
    """the ConfigBlock for an object cutflow"""

    def __init__ (self, containerName, selectionName) :
        groupName = containerName
        if selectionName != '' :
            groupName += '.' + selectionName
        super (ObjectCutFlowBlock, self).__init__ (groupName)
        self.containerName = containerName
        self.selectionName = selectionName
        self.addOption ('postfix', '', type=str)

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        alg = config.createAlgorithm( 'CP::ObjectCutFlowHistAlg', 'CutFlowDumperAlg_' + self.containerName + '_' + self.selectionName + postfix )
        alg.histPattern = 'cflow_' + self.containerName + "_" + self.selectionName + postfix + '_%SYS%'
        alg.selections = config.getSelectionCutFlow (self.containerName, self.selectionName)
        alg.input = config.readName (self.containerName)
        alg.histTitle = "Object Cutflow: " + self.containerName + "." + self.selectionName


class EventCutFlowBlock (ConfigBlock):
    """the ConfigBlock for an event-level cutflow"""

    def __init__ (self, containerName, selectionName) :
        groupName = containerName
        if selectionName != '' :
            groupName += '.' + selectionName
        super (EventCutFlowBlock, self).__init__ (groupName)
        self.containerName = containerName
        self.selectionName = selectionName
        self.addOption ('customSelections', [], type=None)
        self.addOption ('postfix', '', type=str)

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
        if self.selectionName:
            alg.preselection = self.selectionName + '_%SYS%'
        alg.eventInfo = config.readName (self.containerName)
        alg.histTitle = "Event Cutflow: " + self.containerName + "." + self.selectionName


class OutputThinningBlock (ConfigBlock):
    """the ConfigBlock for output thinning"""

    def __init__ (self, containerName, configName) :
        super (OutputThinningBlock, self).__init__ (containerName + '.' + configName)
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('selection', '', type=str)
        self.addOption ('selectionName', '', type=str)
        self.addOption ('outputName', None, type=str)
        self.addOption ('deepCopy', False, type=bool)
        self.addOption ('noUniformSelection', False, type=bool)

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


class IFFLeptonDecorationBlock (ConfigBlock):
    """the ConfigBlock for the IFF classification of leptons"""

    def __init__ (self, containerName='') :
        super (IFFLeptonDecorationBlock, self).__init__( 'IFFLeptonDecoration' )
        self.containerName = containerName
        self.addOption ('separateChargeFlipElectrons', True, type=bool)
        self.addOption ('decoration', 'IFFClass_%SYS%', type=str)

    def makeAlgs (self, config) :
        # the classification is only for MC
        if config.dataType() is DataType.Data: return

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


class PerEventSFBlock (ConfigBlock):
    """the ConfigBlock for the AsgEventScaleFactorAlg"""

    def __init__ (self, name):
        super(PerEventSFBlock, self).__init__()
        self.algName = name
        self.addOption('particles', '', type=str)
        self.addOption('objectSF', '', type=str)
        self.addOption('eventSF', '', type=str)

    def makeAlgs(self, config):
        if config.dataType() is DataType.Data:
            return
        particles, selection = config.readNameAndSelection(self.particles)
        alg = config.createAlgorithm('CP::AsgEventScaleFactorAlg', self.algName)
        alg.particles = particles
        alg.preselection = selection
        alg.scaleFactorInputDecoration = self.objectSF
        alg.scaleFactorOutputDecoration = self.eventSF

        config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration,
                            alg.scaleFactorOutputDecoration.split("_%SYS%")[0])


def makeCommonServicesConfig( seq ):
    """Create the common services config"""

    seq.append (CommonServicesConfig ())



def makePileupReweightingConfig( seq, campaign=None, files=None, useDefaultConfig=None, userLumicalcFiles=None, userPileupConfigs=None ):
    """Create a PRW analysis config

    Keyword arguments:
    """
    # TO DO: add explanation of the keyword arguments, left to experts

    config = PileupReweightingBlock ()
    config.setOptionValue ('campaign', campaign, noneAction='ignore')
    config.setOptionValue ('files', files, noneAction='ignore')
    config.setOptionValue ('useDefaultConfig', useDefaultConfig, noneAction='ignore')
    config.setOptionValue ('userLumicalcFiles', userLumicalcFiles, noneAction='ignore')
    config.setOptionValue ('userPileupConfigs', userPileupConfigs, noneAction='ignore')
    seq.append (config)



def makeGeneratorAnalysisConfig( seq,
                                 saveCutBookkeepers=None,
                                 runNumber=None,
                                 cutBookkeepersSystematics=None ):
    """Create a generator analysis algorithm sequence

    Keyword arguments:
      saveCutBookkeepers -- save cut bokkeepers information into output file
      runNumber -- MC run number
      cutBookkeepersSystematics -- store CutBookkeepers systematics
    """

    config = GeneratorAnalysisBlock ()
    config.setOptionValue ('saveCutBookkeepers', saveCutBookkeepers, noneAction='ignore')
    config.setOptionValue ('runNumber', runNumber, noneAction='ignore')
    config.setOptionValue ('cutBookkeepersSystematics', cutBookkeepersSystematics, noneAction='ignore')
    seq.append (config)



def makePtEtaSelectionConfig( seq, containerName,
                              *, postfix = None, minPt = None, maxEta = None,
                              useClusterEta = None,
                              selectionDecoration = None, selectionName = ''):
    """Create a pt-eta kinematic selection config

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      minPt -- minimum pt value
      maxEta -- maximum eta value
      useClusterEta -- use cluster eta (for electrons/photons) instead of track eta
      selectionDecoration -- the name of the decoration to set
      selectionName -- the name of the selection to append this to
    """

    config = PtEtaSelectionBlock (containerName, selectionName)
    config.setOptionValue ('postfix',postfix, noneAction='ignore')
    config.setOptionValue ('minPt',minPt, noneAction='ignore')
    config.setOptionValue ('maxEta',maxEta, noneAction='ignore')
    config.setOptionValue ('selectionDecoration',selectionDecoration, noneAction='ignore')
    config.setOptionValue ('useClusterEta',useClusterEta, noneAction='ignore')
    seq.append (config)



def makeObjectCutFlowConfig( seq, containerName,
                              *, postfix = None, selectionName):
    """Create a pt-eta kinematic selection config

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      selectionName -- the name of the selection to do the cutflow for
    """

    config = ObjectCutFlowBlock (containerName, selectionName)
    config.setOptionValue ('postfix',postfix, noneAction='ignore')
    seq.append (config)


def makeEventCutFlowConfig( seq, containerName,
                              *, postfix = None, selectionName, customSelections = None):
    """Create an event-level cutflow config

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm names.
      selectionName -- the name of the selection to do the cutflow for
      customSelections -- a list of decorations to use in the cutflow, to override the retrieval of all decorations
    """

    config = EventCutFlowBlock (containerName, selectionName)
    config.setOptionValue ('postfix', postfix, noneAction='ignore')
    config.setOptionValue ('customSelections', customSelections, noneAction='ignore')
    seq.append (config)


def makeOutputThinningConfig( seq, containerName,
                              *, postfix = None, selection = None, selectionName = None, outputName = None, configName='Thinning'):
    """Create an output thinning config

    This will do a consistent selection of output containers (if there
    is a preselection or a selection specified) and then creates a set
    of view containers (or deep copies) based on that selection.

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      selection -- the name of an optional selection decoration to use
      outputName -- an optional name for the output container

    """

    config = OutputThinningBlock (containerName, configName)
    config.setOptionValue ('postfix', postfix, noneAction='ignore')
    config.setOptionValue ('selection', selection, noneAction='ignore')
    config.setOptionValue ('selectionName', selectionName, noneAction='ignore')
    config.setOptionValue ('outputName', outputName, noneAction='ignore')
    seq.append (config)
