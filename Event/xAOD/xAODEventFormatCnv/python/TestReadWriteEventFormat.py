# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import sys

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
from xAODEventFormatCnv.EventFormatTestConfig import (
    EventFormatTestFlags,
    EventFormatTestOutputCfg,
)


def main():
    numberOfStreams = 1
    flags = EventFormatTestFlags(
        inputFiles=["Test0.pool.root"],
        eventsPerFile=1,
    )
    streamName = "ReadWriteTest"
    for i in range(numberOfStreams):
        flags.addFlag(
            f"Output.{streamName}{i}FileName",
            f"{streamName}{i}.pool.root",
        )
        flags.addFlag(f"Output.doWrite{streamName}{i}", True)

    flags.fillFromArgs()
    flags.lock()

    itemList = [
        "xAODMakerTest::AVec#TestObject",
        "xAODMakerTest::AAuxContainer#TestObjectAux.",
        "xAODMakerTest::AVec#TestObject2",
        "xAODMakerTest::AAuxContainer#TestObject2Aux.",
        "xAOD::EventInfo#EventInfo",
        "xAOD::EventAuxInfo#EventInfoAux.",
    ]

    acc = EventFormatTestOutputCfg(
        flags,
        streamName="ReadWriteTest",
        itemList=itemList,
        numberOfStreams=numberOfStreams,
    )

    acc.addEventAlgo(
        CompFactory.xAODMakerTest.ACreatorAlg("ACreator", OutputKey="TestObject2")
    )
    acc.merge(EventInfoCnvAlgCfg(flags=flags, inputKey="", disableBeamSpot=True))
    acc.merge(PoolReadCfg(flags))
    acc.merge(
        MetaDataSvcCfg(
            flags,
            tools=[
                CompFactory.xAODMaker.EventFormatMetaDataTool(
                    "EventFormatMetaDataTool",
                    OutputLevel=1,
                    Keys=[
                        "EventFormatStreamTest0",
                        "EventFormatAOD",
                        "SomeNotExistentKey",
                    ],
                ),
                CompFactory.xAODMaker.FileMetaDataTool(
                    "FileMetaDataTool",
                    OutputLevel=3,
                ),
            ],
        )
    )
    acc.addEventAlgo(CompFactory.xAODMakerTest.EventFormatPrinterAlg())
    acc.addEventAlgo(
        CompFactory.xAODMakerTest.ACreatorAlg("ACreator", OutputKey="TestObject2")
    )
    acc.run(flags.Exec.MaxEvents)


if __name__ == "__main__":
    sys.exit(main())
