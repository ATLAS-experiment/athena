# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import sys
from AthenaConfiguration.ComponentFactory import CompFactory
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
from xAODEventFormatCnv.EventFormatTestConfig import (
    EventFormatTestFlags,
    EventFormatTestOutputCfg,
)
from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg


def main():
    numberOfStreams = 5
    streamName = "Test"
    flags = EventFormatTestFlags(eventsPerFile=5)
    for i in range(numberOfStreams):
        flags.addFlag(
            f"Output.{streamName}{i}FileName",
            f"{streamName}{i}.pool.root",
        )
        flags.addFlag(f"Output.doWrite{streamName}{i}", True)

    flags.lock()

    acc = EventFormatTestOutputCfg(
        flags,
        itemList=[
            "xAODMakerTest::AVec#TestObject",
            "xAODMakerTest::AAuxContainer#TestObjectAux.",
            "xAOD::EventInfo#EventInfo",
            "xAOD::EventAuxInfo#EventInfoAux.",
        ],
    )
    acc.addEventAlgo(
        CompFactory.xAODMakerTest.ACreatorAlg("ACreator", OutputKey="TestObject"),
        sequenceName="AthAlgSeq",
    )
    acc.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True))
    acc.merge(
        MetaDataSvcCfg(
            flags,
            toolNames=[
                "xAODMaker::EventFormatMetaDataTool",
                "xAODMaker::FileMetaDataTool",
            ],
        )
    )
    acc.run(flags.Exec.MaxEvents)


if __name__ == "__main__":
    sys.exit(main())
