# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
## ################################################
"""
JetCalibAlgConfig

This module is currently demonstrates a ConfigBlock for a single JetCalibAlg using the new Jet calibration tools.
It is loosely inspired from 

"""
# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnaAlgorithm.DualUseConfig import isAthena

# Jet config imports
from JetCalibTools.JetCalibStepsConfig import calibToolFromConfigFile

class JetCalibAlgConfig(ConfigBlock):
    
    def __init__(self):
        super(JetCalibAlgConfig, self).__init__()

        self.addOption('inputJets', '', type=str)
        self.addOption('calibratedJets', '', type=str)

        
        self.addOption('CalibFile', '', type=str)

    def makeAlgs(self, config):

        config.setSourceName (self.calibratedJets, self.inputJets, originalName = self.inputJets)

        # Perform a shallow copy of the input : 
        if config.wantCopy (self.calibratedJets) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'JetShallowCopyAlg' )
            alg.input = config.readName (self.calibratedJets)
            alg.output = config.copyName (self.calibratedJets)
            
        alg = config.createAlgorithm('CP::JetCalibAlg', 'JetCalibAlg')
        alg.jets = config.readName(self.calibratedJets )
        alg.OutputLevel = 3

        # Call calibToolFromConfigFile
        calibtool = calibToolFromConfigFile(config.flags, self.CalibFile, name=self.inputJets+"Calib")
        calibtool.OutputLevel=3
        
        if isAthena:
            # In this case calibtool is a regular configurable and the usual syntax works:
            alg.calibrationTool = calibtool
        else:
            # AnalysisBase : use the special JetAnalysisCommon syntax to propagate the config of this tool:
            calibtool.toToolInAnaAlg(alg, "calibrationTool")
        
        
        


    
