# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import sys

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration import MainServicesConfig
from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
from xAODEventFormatCnv.EventFormatTestConfig import EventFormatTestFlags


def main():
    flags = EventFormatTestFlags(
        inputFiles=["Test0.pool.root"],
        eventsPerFile=1,
    )

    flags.lock()

    acc = MainServicesConfig.MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))
    acc.addEventAlgo(CompFactory.xAODMakerTest.EventFormatPrinterAlg())

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

    acc.run(flags.Exec.MaxEvents)


if __name__ == "__main__":
    sys.exit(main())
