# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
import copy, re

class OutputAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self) :
        super (OutputAnalysisConfig, self).__init__ ()
        self.addOption ('postfix', '', type=str)
        self.addOption ('vars', [], type=None)
        self.addOption ('varsOnlyForMC', [], type=None)
        self.addOption ('metVars', [], type=None)
        self.addOption ('containers', {}, type=None)
        self.addOption ('containersOnlyForMC', [], type=None)
        self.addOption ('treeName', 'analysis', type=str)
        self.addOption ('metTermName', 'Final', type=str)
        self.addOption ('commands', [], type=None,
                        info="a list of commands for branch selection/configuration")


    def makeAlgs (self, config) :

        self.vars = set(self.vars)
        self.varsOnlyForMC = set(self.varsOnlyForMC)
        # merge the MC-specific branches and containers into the main list/dictionary only if we are not running on data
        if config.dataType() is not DataType.Data:
            self.vars |= self.varsOnlyForMC
            self.containers.update(self.containersOnlyForMC)

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
