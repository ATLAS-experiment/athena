# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class SystObjectLinkBlock (ConfigBlock):
    """the ConfigBlock for linking systematic variation and nominal objects"""

    def __init__ (self) :
        super (SystObjectLinkBlock, self).__init__ ()
        self.addOption('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        alg = config.createAlgorithm('CP::SystObjectLinkerAlg', 'SystObjLinker', reentrant=True)
        alg.input = config.readName (self.containerName)



