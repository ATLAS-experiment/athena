# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""Test multi-threaded xAOD::FileMetaData writing

Read default test AOD and configure to write output stream with
xAOD::FileMetaData object. Set up to process 4 concurrent events. Inspect the
produced file for xAOD::FileMetaData metadata items.

    Typical usage example
        python -m xAODMetaDataCnv.TestWriteFileMetaDataAOD |tee log
"""
import sys
from AthenaConfiguration import AllConfigFlags, Enums, TestDefaults, MainServicesConfig
from AthenaCommon import Constants, Logging
from OutputStreamAthenaPool import OutputStreamConfig
from PyUtils import MetaReader
from xAODEventInfoCnv import xAODEventInfoCnvConfig
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
from AthenaConfiguration.Enums import Project


def writeFileMetaData(flags):
    """set up an output stream and xAOD::EventInfo"""
    accumulator = OutputStreamConfig.OutputStreamCfg(flags, streamName="AOD")
    accumulator.merge(SetupMetaDataForStreamCfg(flags, streamName="AOD"))
    # AthAnalysis doesn't have the dictionary for ByteStreamMetadata(Container)
    # Therefore, do not attempt to write it - although this is more of a hack
    if flags.Common.Project == Project.AthAnalysis:
        originalList = accumulator.getEventAlgo('StreamAOD').MetadataItemList
        modifiedList = [s for s in originalList if s != "ByteStreamMetadataContainer#*"]
        accumulator.getEventAlgo('StreamAOD').MetadataItemList = modifiedList

    accumulator.merge(
        xAODEventInfoCnvConfig.EventInfoCnvAlgCfg(
            flags=flags, inputKey="", disableBeamSpot=flags.Common.Project is not Enums.Project.Athena
        )
    )

    return accumulator


def testMetaData(file_name):
    """Check that file metadata is in output"""
    meta = MetaReader.read_metadata([file_name])[file_name]["metadata_items"]
    file_info_items = [
        v for _, v in meta.items() if "FileMetaData" in v
    ]
    print("found file metadata objects:", file_info_items)
    return bool(file_info_items)


def main():
    """Run a job writing a file with FileMetaData"""
    msg = Logging.logging.getLogger("TestFileMetaData")

    flags = AllConfigFlags.initConfigFlags()
    flags.Exec.OutputLevel = Constants.DEBUG
    flags.Input.Files = TestDefaults.defaultTestFiles.AOD_RUN2_DATA
    flags.Output.AODFileName = "test.pool.root"
    flags.Output.doWriteAOD = True
    flags.Concurrency.NumThreads = 4
    flags.Concurrency.NumConcurrentEvents = 4
    flags.lock()

    write = MainServicesConfig.MainServicesCfg(flags)
    write.merge(PoolReadCfg(flags))
    write.merge(writeFileMetaData(flags))
    write.run(100)

    try:
        if testMetaData(flags.Output.AODFileName):
            msg.info("File contains xAOD::FileMetaData")
            return 0
        msg.error("File does not contain xAOD::FileMetaData")
    except ReferenceError:
        msg.error("Failed to produce output file")
    except KeyError:
        msg.error("Failed to get metadata item list from file")
    return 1


if __name__ == "__main__":
    sys.exit(main())
