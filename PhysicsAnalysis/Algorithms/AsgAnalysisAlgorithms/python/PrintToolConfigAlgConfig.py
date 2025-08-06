# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnaAlgorithm.DualUseConfig import isAthena, useComponentAccumulator

from pathlib import Path

class PrintToolConfigAlgBlock(ConfigBlock):
    """ConfigBlock for tool properties printing.

    This class handles configuration for printing tool properties to a specified
    output file.
    """

    def __init__(self):
        super (PrintToolConfigAlgBlock, self).__init__ ()

        self.addOption('OutputFile', 'tool_config.txt', type=str,
                       info="Name of the file where the tool configuration will be written.")
        self.addOption('OutputDir', None, type=str,
                       info="Directory where the output file will be written. If 'None',"
                       " the current directory of the job.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return '' # no instance name needed for singleton block

    def get_output_path(self) -> Path:
        """Get the complete output file path.

        Returns:
            Path object representing the full output file path.
        """
        output_dir = self.OutputDir if self.OutputDir is not None else Path.cwd()
        return Path(output_dir) / self.OutputFile

    def makeAlgs(self, config) -> None:
        """Create and configure the PrintToolConfigAlg algorithm.

        Args:
            config: Configuration object used to create the algorithm.
        """
        if isAthena and useComponentAccumulator:
            # we leave the implementation for Athena/AthAnalysis to a future MR
            # this will be based on https://gitlab.cern.ch/atlas/athena/-/merge_requests/77616
            return

        alg = config.createAlgorithm('CP::PrintToolConfigAlg', 'PrintToolConfigAlg')
        alg.OutputFile = str(self.get_output_path())
