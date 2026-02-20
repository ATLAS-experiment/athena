# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

class ReclusteredJetCalibrationBlock(ConfigBlock):
    """ConfigBlock for the jet reclustering calibration algorithm:
    bootstrap reclustered Large-R jet calibration by manually
    setting 4-momentum from calibrated constituent small-R jets"""

    def __init__(self):
        super(ReclusteredJetCalibrationBlock, self).__init__()
        self.addOption ('containerName', '', type=str,
            info='the name of the output container after calibration.')
        self.addOption ('jetCollection', '', type=str,
            info="the reclustered Large-R jet container to run on.")
        self.addOption ('jetInput', '', type=str,
            info='the input calibrated small-R jet collection to use.')

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):

        config.setSourceName (self.containerName, self.jetCollection, originalName = self.jetCollection)

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'ReclusteredJetShallowCopyAlg')
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        alg = config.createAlgorithm('CP::ReclusteredJetCalibrationAlg', 'ReclusteredJetCalibrationAlg')

        alg.reclusteredJets = config.readName(self.containerName)
        alg.reclusteredJetsOut = config.copyName(self.containerName)
        alg.smallRJets = config.readName(self.jetInput)

        config.addOutputVar(self.containerName, 'pt', 'pt')
        config.addOutputVar(self.containerName, 'eta', 'eta')
        config.addOutputVar(self.containerName, 'phi', 'phi')
        config.addOutputVar(self.containerName, 'm', 'm')

