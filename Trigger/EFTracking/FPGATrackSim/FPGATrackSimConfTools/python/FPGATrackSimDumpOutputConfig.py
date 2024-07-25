# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Example configuration for an algorithm which can read back the output header files.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackSimReadOutputCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool("FPGATrackSimReadOutput")
    FPGATrackSimWriteOutput.InFileName = flags.Input.Files
    FPGATrackSimWriteOutput.RWstatus = "READ"
    result.addPublicTool(FPGATrackSimWriteOutput, primary=True)
    return result


def FPGATrackSimDumpOutputCfg(flags):
    acc = ComponentAccumulator()
    alg = CompFactory.FPGATrackSimDumpOutputStatAlg(
        InputBranchName = flags.InputHeader,
        OutputBranchName = flags.OutputHeader,
        InputTool = acc.getPrimaryAndMerge(FPGATrackSimReadOutputCfg(flags))
    )

    acc.addEventAlgo(alg)
    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.addFlag("InputHeader", "LogicalEventInputHeader")
    flags.addFlag("OutputHeader", "LogicalEventOutputHeader")
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags.fillFromArgs()
    log.info("Taken input files from Input.Files(set via cmd line --filesInput option) property: %s", str(flags.Input.Files))
    flags.lock()

    acc=MainServicesCfg(flags)
    acc.store(open('FPGATrackSimDumpOutputStatAlg.pkl','wb'))
    acc.merge(FPGATrackSimDumpOutputCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

