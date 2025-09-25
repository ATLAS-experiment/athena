# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys
from OutputStreamAthenaPool.OutputStreamConfig import (
    OutputStreamCfg,
    addToMetaData,
    outputStreamName,
)
from AthExHiveAthenaPool.WriteHiveDataObjConfig import WriteHiveDataObjCfg
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory

if __name__ == "__main__":
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.RunNumbers = [284600]
    flags.Input.TimeStamps = [1]  # dummy value
    # workaround for building xAOD::EventInfo without input files
    flags.Input.TypedCollections = []
    flags.Exec.MaxEvents = 20

    streamName = "TestStream"
    flags.addFlag(
        f"Output.{streamName}FileName",
        f"{streamName}.pool.root",
    )
    flags.addFlag(f"Output.doWrite{streamName}", True)

    flags.fillFromArgs()
    flags.lock()

    # The example runs with no input file. We configure it with the McEventSelector
    cfg = MainEvgenServicesCfg(flags, withSequences=True)
    cfg.merge(WriteHiveDataObjCfg(flags))

    # Output stream
    cfg.merge(
        OutputStreamCfg(
            flags,
            streamName=streamName,
            ItemList=[
                "xAOD::EventInfo#EventInfo",
                "xAOD::EventAuxInfo#EventInfoAux.",
                "HiveDataObj#*",
            ],
        )
    )
    cfg.merge(
        addToMetaData(
            flags,
            streamName=streamName,
            itemOrList=[
                f"xAOD::EventFormat#EventFormat{outputStreamName(streamName)}",
                "xAOD::FileMetaData#FileMetaData",
                "xAOD::FileMetaDataAuxInfo#FileMetaDataAux.",
            ],
            HelperTools=[
                CompFactory.xAODMaker.EventFormatStreamHelperTool(
                    f"{outputStreamName(streamName)}_EventFormatStreamHelperTool",
                    Key=f"EventFormat{outputStreamName(streamName)}",
                    DataHeaderKey=f"{outputStreamName(streamName)}",
                    TypeNames=["HiveDataObj#*"],
                ),
                CompFactory.xAODMaker.FileMetaDataCreatorTool(
                    f"{outputStreamName(streamName)}_FileMetaDataCreatorTool",
                    OutputKey="FileMetaData",
                    StreamName=f"{outputStreamName(streamName)}",
                ),
            ],
        )
    )

    # Execute
    sys.exit(cfg.run().isFailure())
