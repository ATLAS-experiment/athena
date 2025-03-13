# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AnalysisAlgorithmsConfig.ConfigSequence import filter_dsids
from AthenaCommon.Logging import logging
import copy, re

class OutputAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self) :
        super (OutputAnalysisConfig, self).__init__ ()
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here.")
        self.addOption ('vars', [], type=None,
            info="a list of mappings (list of strings) between containers and "
            "decorations to output branches. The default is [] (empty list).")
        self.addOption ('varsOnlyForMC', [], type=None,
            info="same as vars, but for MC-only variables so as to avoid a "
            "crash when running on data. The default is [] (empty list).")
        self.addOption ('metVars', [], type=None,
            info="a list of mappings (list of strings) between containers "
            "and decorations to output branches. Specficially for MET "
            "variables, where only the final MET term is retained. "
            "The default is [] (empty list).")
        self.addOption ('truthMetVars', [], type=None,
            info="a list of mappings (list of strings) between containers "
            "and decorations to output branches for truth MET. "
            "The default is [] (empty list).")
        self.addOption ('containers', {}, type=None,
            info="a dictionary mapping prefixes (key) to container names "
            "(values) to be used when saving to the output tree. Branches "
            "are then of the form prefix_decoration.")
        self.addOption ('containersOnlyForMC', {}, type=None,
            info="same as containers, but for MC-only containers so as to avoid "
            "a crash when running on data.")
        self.addOption ('containersOnlyForDSIDs', {}, type=None,
            info="specify which DSIDs are allowed to produce a given container. "
            "This works like 'onlyForDSIDs': pass a list of DSIDs or regexps.")
        self.addOption ('treeName', 'analysis', type=str,
            info="name of the output TTree to save. The default is analysis.")
        self.addOption ('streamName', 'ANALYSIS', type=str,
            info="name of the output stream to save the tree in. "
            "The default is ANALYSIS.")
        self.addOption ('metTermName', 'Final', type=str,
            info="the name (string) of the MET term to save, turning the MET "
            "container into a single object. The default is 'Final'.")
        self.addOption ('truthMetTermName', 'NonInt', type=str,
            info="the name (string) of the truth MET term to save, turning the MET "
            "container into a single object. The default is 'NonInt'.")
        # TODO: add info strng
        self.addOption ('storeSelectionFlags', True, type=bool,
            info="")
        # TODO: add info strng
        self.addOption ('selectionFlagPrefix', 'select', type=str,
            info="")
        self.addOption ('commands', [], type=None,
            info="a list of strings containing commands (regexp strings "
            "prefaced by the keywords enable or disable) to turn on/off the "
            "writing of branches to the output ntuple. The default is None "
            "(no modification to the scheduled output branches).")
        self.addOption ('commandsOnlyForDSIDs', {}, type=None,
            info="a dictionary with individual DSIDs as keys, and a list of strings "
            "like for the 'commands' option as items. These 'commands' will only be run "
            "for the corresponding DSID.")
        self.addOption ('alwaysAddNosys', False, type=bool,
            info="If set to True, all branches will be given a systematics suffix, "
            "even if they have no systematics (beyond the nominal).")

    @staticmethod
    def branchSortOrder (rule):
        return rule.split('->')[1].strip()

    def createOutputAlgs (self, config, name, vars, isMet=False):
        """A helper function to create output algorithm"""
        alg = config.createAlgorithm('CP::AsgxAODMetNTupleMakerAlg' if isMet else 'CP::AsgxAODNTupleMakerAlg', name)
        alg.TreeName = self.treeName
        alg.RootStreamName = self.streamName
        branchList = list(vars)
        branchList.sort(key=self.branchSortOrder)
        branchList_nosys = [branch for branch in branchList if "%SYS%" not in branch]
        branchList_sys = [branch for branch in branchList if "%SYS%" in branch]
        alg.Branches = branchList_nosys + branchList_sys
        return alg

    def makeAlgs (self, config) :

        log = logging.getLogger('OutputAnalysisConfig')

        self.vars = set(self.vars)
        self.varsOnlyForMC = set(self.varsOnlyForMC)
        self.metVars = set(self.metVars)
        self.truthMetVars = set(self.truthMetVars)

        # merge the MC-specific branches and containers into the main list/dictionary only if we are not running on data
        if config.dataType() is not DataType.Data:
            self.vars |= self.varsOnlyForMC

            # protect 'containers' against being overwritten
            # find overlapping keys
            overlapping_keys = set(self.containers.keys()).intersection(self.containersOnlyForMC.keys())
            if overlapping_keys:
                # convert the set of overlapping keys to a list of strings for the message (represents the empty string too!)
                keys_message = [repr(key) for key in overlapping_keys]
                raise KeyError(f"containersOnlyForMC would overwrite the following container keys: {', '.join(keys_message)}")

            # move items in self.containersOnlyForMC to self.containers
            self.containers.update(self.containersOnlyForMC)
            # clear the dictionary to avoid overlapping key error during the second pass
            self.containersOnlyForMC.clear()

        # now filter the containers depending on DSIDs
        for container,dsid_filters in self.containersOnlyForDSIDs.items():
            if container not in self.containers:
                log.warning(f"Skipping unrecognised container {container} for DSID-filtering in OutputAnalysisConfig...")
                continue
            if not filter_dsids (dsid_filters, config):
                # if current DSID is not allowed for this container, remove it
                self.containers.pop (container)

        if self.storeSelectionFlags:
            self.createSelectionFlagBranches(config)

        outputConfigs = {}
        for prefix in self.containers.keys() :
            containerName = self.containers[prefix]
            outputDict = config.getOutputVars (containerName)
            for outputName in outputDict :
                outputConfig = copy.deepcopy (outputDict[outputName])
                if containerName != outputConfig.origContainerName :
                    outputConfig.outputContainerName = containerName + '_%SYS%'
                else :
                    outputConfig.outputContainerName = config.readName (containerName)
                outputConfigs[prefix + outputName] = outputConfig

        # check for DSID-specific commands
        for dsid, dsid_commands in self.commandsOnlyForDSIDs.items():
            if filter_dsids([dsid], config):
                self.commands += dsid_commands

        for command in self.commands :
            words = command.split (' ')
            if len (words) == 0 :
                raise ValueError ('received empty command for "commands" option')
            if words[0] == 'enable' :
                if len (words) != 2 :
                    raise ValueError ('enable takes exactly one argument: ' + command)
                used = False
                for name in outputConfigs :
                    if re.match (words[1], name) :
                        outputConfigs[name].enabled = True
                        used = True
                if not used and config.dataType() is not DataType.Data:
                    raise KeyError ('unknown branch pattern for enable: ' + words[1])
            elif words[0] == 'disable' :
                if len (words) != 2 :
                    raise ValueError ('disable takes exactly one argument: ' + command)
                used = False
                for name in outputConfigs :
                    if re.match (words[1], name) :
                        outputConfigs[name].enabled = False
                        used = True
                if not used and config.dataType() is not DataType.Data:
                    raise KeyError ('unknown branch pattern for disable: ' + words[1])
            else :
                raise KeyError ('unknown command for "commands" option: ' + words[0])

        autoVars = set()
        autoMetVars = set()
        autoTruthMetVars = set()
        for outputName in outputConfigs :
            outputConfig = outputConfigs[outputName]
            if outputConfig.enabled :
                if config.isMetContainer (outputConfig.origContainerName):
                    if "Truth" in outputConfig.origContainerName:
                        myVars = autoTruthMetVars
                    else:
                        myVars = autoMetVars
                else :
                    myVars = autoVars
                if outputConfig.noSys :
                    outputConfig.outputContainerName = outputConfig.outputContainerName.replace ('%SYS%', 'NOSYS')
                    outputConfig.variableName = outputConfig.variableName.replace ('%SYS%', 'NOSYS')
                    if self.alwaysAddNosys :
                        outputName += "_NOSYS"
                else :
                    outputName += '_%SYS%'
                myVars.add(f"{outputConfig.outputContainerName}.{outputConfig.variableName} -> {outputName}")

        if self.postfix:
            postfix = self.postfix
        else:
            postfix = self.treeName

        # Add an ntuple dumper algorithm:
        treeMaker = config.createAlgorithm( 'CP::TreeMakerAlg', f'TreeMaker{postfix}' )
        treeMaker.TreeName = self.treeName
        treeMaker.RootStreamName = self.streamName
        # the auto-flush setting still needs to be figured out
        #treeMaker.TreeAutoFlush = 0

        if self.vars or autoVars:
            ntupleMaker = self.createOutputAlgs(config, f'NTupleMaker{postfix}', self.vars | autoVars)

        if self.metVars or autoMetVars:
            ntupleMaker = self.createOutputAlgs(config, f'MetNTupleMaker{postfix}', self.metVars | autoMetVars, isMet=True)
            ntupleMaker.termName = self.metTermName

        if config.dataType() is not DataType.Data and (self.truthMetVars or autoTruthMetVars):
            ntupleMaker = self.createOutputAlgs(config, f'TruthMetNTupleMaker{postfix}', self.truthMetVars | autoTruthMetVars, isMet=True)
            ntupleMaker.termName = self.truthMetTermName

        treeFiller = config.createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' + postfix )
        treeFiller.TreeName = self.treeName
        treeFiller.RootStreamName = self.streamName



    def createSelectionFlagBranches(self, config):
        """
        For each container and for each selection, create a single pass variable in output NTuple,
        which aggregates all the selections flag of the given selection. For example, this can include
        pT, eta selections, some object ID selection, overlap removal, etc.
        The goal is to have only one flag per object and working point in the output NTuple.
        """
        originalContainersSeen = []
        for prefix in self.containers.keys() :
            outputContainerName = self.containers[prefix]
            containerName = config.getOutputContainerOrigin(outputContainerName)
            if containerName in originalContainersSeen:
                continue
            else:
                originalContainersSeen.append(containerName)

            # EventInfo is one obvious example of a container that has no object selections
            if containerName == 'EventInfo':
                continue

            selectionNames = config.getSelectionNames(containerName)
            for selectionName in selectionNames:
                # skip default selection
                if selectionName == '':
                    continue
                self.makeSelectionSummaryAlg(config, containerName, selectionName)

    def makeSelectionSummaryAlg(self, config, containerName, selectionName):
        """
        Schedule an algorithm to pick up all cut flags for a given selectionName.
        The summary selection flag is written to output as selectionFlagPrefix_selectionName.
        """
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      f'ObjectSelectionSummary_{containerName}_{selectionName}')
        selectionDecoration = f'baselineSelection_{selectionName}_%SYS%'
        alg.selectionDecoration =  f'{selectionDecoration},as_char'
        alg.particles = config.readName (containerName)
        alg.preselection = config.getFullSelection (containerName, selectionName)
        config.addOutputVar (containerName, selectionDecoration, self.selectionFlagPrefix + '_' + selectionName)
