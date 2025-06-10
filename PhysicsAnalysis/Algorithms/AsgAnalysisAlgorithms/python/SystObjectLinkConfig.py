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

    def makeAlgs (self, config) :

        alg = config.createAlgorithm('CP::SystObjectLinkerAlg', f'SystObjLinker_{self.containerName}', reentrant=True)
        alg.input = config.readName (self.containerName)



