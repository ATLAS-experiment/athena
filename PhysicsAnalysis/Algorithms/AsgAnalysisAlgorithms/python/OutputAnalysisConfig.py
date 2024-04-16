# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
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
        self.addOption ('containers', {}, type=None,
            info="a dictionary mapping prefixes (key) to container names "
            "(values) to be used when saving to the output tree. Branches "
            "are then of the form prefix_decoration.")
        # TODO: add info string
        self.addOption ('containersOnlyForMC', [], type=None,
            info="")
        self.addOption ('treeName', 'analysis', type=str,
            info="name of the output TTree to save. The default is analysis.")
        self.addOption ('metTermName', 'Final', type=str,
            info="the name (string) of the MET term to save, turning the MET "
            "container into a single object. The default is 'Final'.")
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


    def makeAlgs (self, config) :

        self.vars = set(self.vars)
        self.varsOnlyForMC = set(self.varsOnlyForMC)
        # merge the MC-specific branches and containers into the main list/dictionary only if we are not running on data
        if config.dataType() is not DataType.Data:
            self.vars |= self.varsOnlyForMC
            self.containers.update(self.containersOnlyForMC)

        if self.storeSelectionFlags:
            self.createSelectionFlagBranches(config)

        outputConfigs = {}
        for prefix in self.containers.keys() :
            containerName = self.containers[prefix]
            outputDict = config.getOutputVars (containerName)
            for outputName in outputDict :
                outputConfig = copy.deepcopy (outputDict[outputName])
                if containerName == 'EventInfo' :
                    outputConfig.outputContainerName = outputConfig.origContainerName
                elif outputConfig.outputContainerName != outputConfig.origContainerName :
                    outputConfig.outputContainerName = containerName + '_%SYS%'
                else :
                    outputConfig.outputContainerName = config.readName (containerName)
                outputConfigs[prefix + outputName] = outputConfig

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

        autoVars = []
        autoMetVars = []
        for outputName in outputConfigs :
            outputConfig = outputConfigs[outputName]
            if outputConfig.enabled :
                if config.isMetContainer (outputConfig.origContainerName) :
                    myVars = autoMetVars
                else :
                    myVars = autoVars
                if outputConfig.noSys :
                    outputConfig.outputContainerName = outputConfig.outputContainerName.replace ('%SYS%', 'NOSYS')
                    outputConfig.variableName = outputConfig.variableName.replace ('%SYS%', 'NOSYS')
                else :
                    outputName += '_%SYS%'
                myVars += [outputConfig.outputContainerName + '.' + outputConfig.variableName + ' -> ' + outputName]

        postfix = self.postfix

        # Add an ntuple dumper algorithm:
        treeMaker = config.createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' + postfix )
        treeMaker.TreeName = self.treeName
        # the auto-flush setting still needs to be figured out
        #treeMaker.TreeAutoFlush = 0

        if len (self.vars) + len (autoVars) :
            ntupleMaker = config.createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' + postfix )
            ntupleMaker.TreeName = self.treeName
            ntupleMaker.Branches = list( self.vars | set(autoVars) )
            # ntupleMaker.OutputLevel = 2  # For output validation

        if len (self.metVars) + len (autoMetVars) > 0:
            ntupleMaker = config.createAlgorithm( 'CP::AsgxAODMetNTupleMakerAlg', 'MetNTupleMaker' + postfix )
            ntupleMaker.TreeName = self.treeName
            ntupleMaker.Branches = self.metVars + autoMetVars
            ntupleMaker.termName = self.metTermName
            #ntupleMaker.OutputLevel = 2  # For output validation

        treeFiller = config.createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' + postfix )
        treeFiller.TreeName = self.treeName


    def createSelectionFlagBranches(self, config):
        """
        For each container and for each selection, create a single pass variable in output NTuple,
        which aggregates all the selections flag of the given selection. For example, this can include
        pT, eta selections, some object ID selection, overlap removal, etc.
        The goal is to have only one flag per object and working point in the output NTuple.
        """
        for prefix in self.containers.keys() :
            outputContainerName = self.containers[prefix]
            containerName = config.getOutputContainerOrigin(outputContainerName)

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
