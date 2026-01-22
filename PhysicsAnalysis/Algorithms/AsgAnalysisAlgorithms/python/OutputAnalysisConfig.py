# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AnalysisAlgorithmsConfig.ConfigBlock import filter_dsids
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
            "decorations to output branches.")
        self.addOption ('varsOnlyForMC', [], type=None,
            info="same as `vars`, but for MC-only variables so as to avoid a "
            "crash when running on data.")
        self.addOption ('metVars', [], type=None,
            info="a list of mappings (list of strings) between containers "
            "and decorations to output branches. Specficially for MET "
            "variables, where only the final MET term is retained.")
        self.addOption ('truthMetVars', [], type=None,
            info="a list of mappings (list of strings) between containers "
            "and decorations to output branches for truth MET.")
        self.addOption ('containers', {}, type=None,
            info="a dictionary mapping prefixes (key) to container names "
            "(values) to be used when saving to the output tree. Branches "
            "are then of the form `prefix_decoration`.")
        self.addOption ('containersFullMET', {}, type=None,
            info="same as `containers`, but for MET containers that should be "
            "saved with all terms (as opposed to just the final term). This "
            "is useful for special studies. A container can appear both here and "
            "in containers (with different prefixes).")
        self.addOption ('containersOnlyForMC', {}, type=None,
            info="same as `containers`, but for MC-only containers so as to avoid "
            "a crash when running on data.")
        self.addOption ('containersOnlyForDSIDs', {}, type=None,
            info="specify which DSIDs are allowed to produce a given container. "
            "This works like `onlyForDSIDs`: pass a list of DSIDs or regexps.")
        self.addOption ('nonContainers', ['EventInfo'], type=None,
            info="a list of container names that are not actual containers but should be treated as non-containers.")
        self.addOption ('treeName', 'analysis', type=str,
            info="name of the output TTree to save.")
        self.addOption ('streamName', 'ANALYSIS', type=str,
            info="name of the output stream to save the tree in.")
        self.addOption ('metTermName', 'Final', type=str,
            info="the name of the MET term to save, turning the MET "
            "container into a single object.")
        self.addOption ('truthMetTermName', 'NonInt', type=str,
            info="the name of the truth MET term to save, turning the MET "
            "container into a single object.")
        self.addOption ('storeSelectionFlags', True, type=bool,
            info="whether to store one branch for each object selection.")
        self.addOption ('selectionFlagPrefix', 'select', type=str,
            info="the prefix used when naming selection branches")
        self.addOption ('commands', [], type=None,
            info="a list of strings containing commands (regexp strings "
            "prefaced by the keywords `enable` or `disable`) to turn on/off the "
            "writing of branches to the output ntuple. If left empty, do not modify "
            "the scheduled output branches.")
        self.addOption ('commandsOnlyForDSIDs', {}, type=None,
            info="a dictionary with individual DSIDs as keys, and a list of strings "
            "like for the `commands` option as items. These `commands` will only be run "
            "for the corresponding DSID.")
        self.addOption ('alwaysAddNosys', False, type=bool,
            info="If set to `True`, all branches will be given a systematics suffix, "
            "even if they have no systematics (beyond the nominal).")
        self.addOption ('skipRedundantSelectionFlags', True, type=bool,
            info="remove the redundant `outputSelect` branches created by the `Thinning` step. "
            "These could however be used to simplify downstream workflows, as in Easyjet.")
        # helper to protect for second pass
        self.validated = False

    def instanceName (self) :
        """Return the instance name for this block"""
        if self.postfix is not None and self.postfix != '':
            return self.postfix
        return self.treeName

    @staticmethod
    def branchSortOrder (rule):
        return rule.split('->')[1].strip()

    def createOutputAlgs (self, config, name, vars):
        """A helper function to create output algorithm"""
        alg = config.createAlgorithm('CP::AsgxAODNTupleMakerAlg', name)
        alg.TreeName = self.treeName
        alg.RootStreamName = self.streamName
        alg.NonContainers = list(self.nonContainers)
        branchList = list(vars)
        branchList.sort(key=self.branchSortOrder)
        branchList_nosys = [branch for branch in branchList if "%SYS%" not in branch]
        branchList_sys = [branch for branch in branchList if "%SYS%" in branch]
        alg.Branches = branchList_nosys + branchList_sys
        return alg

    def makeAlgs (self, config) :

        log = logging.getLogger('OutputAnalysisConfig')

        # do some transformations of the options we should only do once
        if not self.validated:

            self.containers = dict(self.containers)
            self.vars = set(self.vars)
            self.varsOnlyForMC = set(self.varsOnlyForMC)
            self.metVars = set(self.metVars)
            self.truthMetVars = set(self.truthMetVars)

            # check for overlaps between containers and containersFullMET
            overlapping_keys = set(self.containers.keys()).intersection(self.containersFullMET.keys())
            if overlapping_keys:
                # convert the set of overlapping keys to a list of strings for the message (represents the empty string too!)
                keys_message = [repr(key) for key in overlapping_keys]
                raise KeyError(f"containersFullMET would overwrite the following container keys: {', '.join(keys_message)}")
            # move items in self.containersFullMET to containers
            self.containers.update(self.containersFullMET)

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
                if self.containersOnlyForDSIDs:
                    for container, dsid_filters in self.containersOnlyForDSIDs.items():
                        if container not in self.containers:
                            log.warning("Skipping unrecognised container prefix '%s' for DSID-filtering in OutputAnalysisConfig...", container)
                            continue
                        if not filter_dsids (dsid_filters, config):
                            # if current DSID is not allowed for this container, remove it
                            log.info("Skipping container prefix '%s' due to DSID filtering...", container)
                            # filter branches for validated containers
                            for var in set(self.vars):  # make a copy of the list to avoid modifying it while iterating
                                var_container = var.split('.')[0].replace('_NOSYS', '').replace('_%SYS%', '')
                                if var_container == self.containers[container]:
                                    self.vars.remove(var)
                                    log.info("Skipping branch definition '%s' for excluded container %s...", var, var_container)
                            # remove the container from the list at the end
                            self.containers.pop (container)
                    # clear the dictionary to avoid warnings during the second pass
                    self.containersOnlyForDSIDs.clear()

            # at this point we are OK
            self.validated = True

        if self.storeSelectionFlags:
            self.createSelectionFlagBranches(config)

        outputConfigs = {}
        for prefix in self.containers.keys() :
            containerName = self.containers[prefix]
            outputDict = config.getOutputVars (containerName)
            for outputName in outputDict :
                outputConfig = copy.deepcopy (outputDict[outputName])
                if containerName != outputConfig.origContainerName or config.checkOutputContainer(containerName):
                    outputConfig.outputContainerName = containerName + '_%SYS%'
                else:
                    outputConfig.outputContainerName = config.readName(containerName)
                outputConfig.prefix = prefix
                # if the container is a MET container with all terms, we
                # also need to write out the name of each MET term
                if prefix in self.containersFullMET and outputConfig.variableName == 'name':
                    outputConfig.enabled = True
                outputConfigs[prefix + outputName] = outputConfig

        # check for DSID-specific commands
        for dsid, dsid_commands in self.commandsOnlyForDSIDs.items():
            if filter_dsids([dsid], config):
                self.commands += dsid_commands

        outputConfigsRename = {}
        for command in self.commands :
            words = command.split (' ')
            if len (words) == 0 :
                raise ValueError ('received empty command for "commands" option')
            optional = words[0] == 'optional'
            if optional :
                words = words[1:]  # remove the 'optional' keyword
            if words[0] == 'enable' :
                if len (words) != 2 :
                    raise ValueError ('enable takes exactly one argument: ' + command)
                used = False
                for name in outputConfigs :
                    if re.match (words[1], name) :
                        outputConfigs[name].enabled = True
                        used = True
                if not used and not optional and config.dataType() is not DataType.Data:
                    raise KeyError ('unknown branch pattern for enable: ' + words[1])
            elif words[0] == 'disable' :
                if len (words) != 2 :
                    raise ValueError ('disable takes exactly one argument: ' + command)
                used = False
                for name in outputConfigs :
                    if re.match (words[1], name) :
                        outputConfigs[name].enabled = False
                        used = True
                if not used and not optional and config.dataType() is not DataType.Data:
                    raise KeyError ('unknown branch pattern for disable: ' + words[1])
            elif words[0] == 'rename' :
                if len (words) != 3 :
                    raise ValueError ('rename takes exactly two arguments: ' + command)
                used = False
                for name in outputConfigs :
                    if re.match (words[1], name) :
                        new_name = re.sub (words[1], words[2], name)
                        outputConfigsRename[new_name] = copy.deepcopy(outputConfigs[name])
                        outputConfigs[name].enabled = False
                        used = True
                if not used and not optional and config.dataType() is not DataType.Data:
                    raise KeyError ('unknown branch pattern for rename: ' + words[1])
            else :
                raise KeyError ('unknown command for "commands" option: ' + words[0])

        # update the outputConfigs with renamed branches
        outputConfigs.update(outputConfigsRename)

        autoVars = set()
        autoMetVars = set()
        autoTruthMetVars = set()
        for outputName, outputConfig in outputConfigs.items():
            if outputConfig.enabled :
                if config.isMetContainer (outputConfig.origContainerName) and outputConfig.prefix not in self.containersFullMET:
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
                branchDecl = f"{outputConfig.outputContainerName}.{outputConfig.variableName} -> {outputName}"
                if outputConfig.auxType is not None :
                    branchDecl += f" type={outputConfig.auxType}"
                if config.isMetContainer (outputConfig.origContainerName) and outputConfig.prefix not in self.containersFullMET:
                    if "Truth" in outputConfig.origContainerName:
                        branchDecl += f" metTerm={self.truthMetTermName}"
                    else:
                        branchDecl += f" metTerm={self.metTermName}"
                myVars.add(branchDecl)

        # Add an ntuple dumper algorithm:
        treeMaker = config.createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
        treeMaker.TreeName = self.treeName
        treeMaker.RootStreamName = self.streamName
        # the auto-flush setting still needs to be figured out
        #treeMaker.TreeAutoFlush = 0

        if self.vars or autoVars:
            self.createOutputAlgs(config, 'NTupleMaker', self.vars | autoVars)

        if self.metVars or autoMetVars:
            userMetVars = set ()
            if self.metVars :
                for var in self.metVars:
                    userMetVars.add(var + " metTerm=" + self.metTermName)
            self.createOutputAlgs(config, 'MetNTupleMaker', userMetVars | autoMetVars)

        if config.dataType() is not DataType.Data and (self.truthMetVars or autoTruthMetVars):
            userTruthMetVars = set ()
            if self.truthMetVars :
                for var in self.truthMetVars:
                    userTruthMetVars.add(var + " metTerm=" + self.truthMetTermName)
            self.createOutputAlgs(config, 'TruthMetNTupleMaker', userTruthMetVars | autoTruthMetVars)

        treeFiller = config.createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
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
                # skip selection coming from the Thinning block
                if self.skipRedundantSelectionFlags and "outputSelect" in selectionName:
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
