# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
## ################################################
"""
JetCalibAlgConfig

This module currently demonstrates a ConfigBlock for a single JetCalibAlg using the new Jet calibration tools.
"""
# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnaAlgorithm.DualUseConfig import isAthena

# Jet config imports
from JetCalibTools.JetCalibStepsConfig import calibToolFromConfigFile

class JetCalibAlgConfig(ConfigBlock):
    
    def __init__(self):
        super().__init__()

        self.addOption('inputJets', '', type=str,
            info="the name of the input jet container, also used to look up the "
            "calibration configuration (unless `CalibFile` is set).")
        self.addOption('calibratedJets', '', type=str, meta={'role':'container'},
            info="the name of the output (calibrated) jet container.")
        self.addOption('context', 'AnalysisLatest', type=str,
            info="the calibration context used to look up the calibration "
            "configuration file (ignored if `CalibFile` is set).")
        self.addOption('CalibFile', '', type=str,
            info="expert override: path to the calibration configuration file, "
            "skipping the lookup based on `context` and `inputJets`.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.calibratedJets

    def makeAlgs(self, config):

        config.setSourceName (self.calibratedJets, self.inputJets, originalName = self.inputJets)

        if config.wantCopy (self.calibratedJets) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'JetShallowCopyAlg' )
            alg.input = config.readName (self.calibratedJets)
            alg.output = config.copyName (self.calibratedJets)

        alg = config.createAlgorithm('CP::JetCalibAlg', 'JetCalibAlg')
        alg.jets = config.readName(self.calibratedJets)

        if self.CalibFile:
            calibFile = self.CalibFile
        else:
            from JetCalibTools.JetCalibToolsCfg import get_calib_cfg_path, get_jet_collection_name
            jetcollection = self.inputJets
            if jetcollection.endswith('Jets'):
                jetcollection = jetcollection[:-4]
            jetcollection = get_jet_collection_name(jetcollection)
            calibFile = get_calib_cfg_path(self.context, jetcollection)

        calibtool = calibToolFromConfigFile(config.flags, calibFile, name=self.inputJets+"Calib")
        
        if isAthena:
            # In this case calibtool is a regular configurable and the usual syntax works:
            alg.calibrationTool = calibtool
        else:
            # AnalysisBase : use the special JetAnalysisCommon syntax to propagate the config of this tool:
            calibtool.toToolInAnaAlg(alg, "calibrationTool")
