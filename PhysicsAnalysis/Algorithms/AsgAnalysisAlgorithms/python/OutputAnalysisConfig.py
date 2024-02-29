# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
import copy, re

class OutputAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self, configName) :
        super (OutputAnalysisConfig, self).__init__ (configName)
        self.addOption ('postfix', '', type=str)
        self.addOption ('vars', [], type=None)
        self.addOption ('metVars', [], type=None)
        self.addOption ('containers', {}, type=None)
        self.addOption ('treeName', 'analysis', type=str)
        self.addOption ('metTermName', 'Final', type=str)
        self.addOption ('storeSelectionFlags', True, type=bool)
        self.addOption ('selectionFlagPrefix', 'select', type=str)
        self.addOption ('systematicsHistogram', None , type=str)
        self.addOption ('commands', [], type=None,
                        info="a list of commands for branch selection/configuration")


    def makeAlgs (self, config) :

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
            ntupleMaker.Branches = self.vars + autoVars
            # ntupleMaker.OutputLevel = 2  # For output validation

        if len (self.metVars) + len (autoMetVars) > 0:
            ntupleMaker = config.createAlgorithm( 'CP::AsgxAODMetNTupleMakerAlg', 'MetNTupleMaker' + postfix )
            ntupleMaker.TreeName = self.treeName
            ntupleMaker.Branches = self.metVars + autoMetVars
            ntupleMaker.termName = self.metTermName
            #ntupleMaker.OutputLevel = 2  # For output validation

        treeFiller = config.createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' + postfix )
        treeFiller.TreeName = self.treeName

        if self.systematicsHistogram is not None:
            sysDumper = config.createAlgorithm( 'CP::SysListDumperAlg', 'SystematicsPrinter' )
            sysDumper.histogramName = self.systematicsHistogram

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