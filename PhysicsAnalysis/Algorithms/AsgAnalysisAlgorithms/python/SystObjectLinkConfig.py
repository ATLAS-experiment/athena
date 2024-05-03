# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class SystObjectLinkBlock (ConfigBlock):
    """the ConfigBlock for linking systematic variation and nominal objects"""

    def __init__ (self, containerName='') :
        super (SystObjectLinkBlock, self).__init__ ()
        self.addOption('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")

    def makeAlgs (self, config) :

        alg = config.createAlgorithm('CP::SystObjectLinkerAlg', f'SystObjLinker_{self.containerName}', reentrant=True)
        alg.input = config.readName (self.containerName)




def makeSystObjectLinkConfig( seq, containerName ):
    """Create an alg for linking systematic variations with their nominal object

    Keyword arguments:
      containerName: the container to be decorated with the links
    """

    config = SystObjectLinkBlock (containerName)
    seq.append (config)
