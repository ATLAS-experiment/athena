# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def CreateLumiBlockCollectionFromFileCfg(configFlags):
    result = ComponentAccumulator()
    createlb = CompFactory.CreateLumiBlockCollectionFromFile()

    if configFlags.GeoModel.Run < LHCPeriod.Run3:
        try:
            createlb.streamName = configFlags.Input.TriggerStream.split("_")[1]
        except IndexError:
            pass
        if not configFlags.Input.isMC and not configFlags.Common.isOnline:
            from IOVDbSvc.IOVDbSvcConfig import addFolders

            result.merge(
                addFolders(
                    configFlags,
                    ["/GLOBAL/FILECOUNT/PROMPT"],
                    tag="GlobalFileCountPrompt-Tier0",
                    detDb="GLOBAL_OFL",
                    className="CondAttrListCollection",
                )
            )
    result.addEventAlgo(createlb)
    return result
