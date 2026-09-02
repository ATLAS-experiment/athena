#!/usr/bin/env python3

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg


def main():
    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.RunNumbers = [1]
    flags.Input.TimeStamps = [1]
    flags.Exec.MaxEvents = 1
    flags.GeoModel.SQLiteDB = True
    flags.GeoModel.SQLiteDBFullPath = "/eos/user/s/smanita/database/ATLAS-LAYOUT.db"
    flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-04-00-00"
    flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN4-05"
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(McEventSelectorCfg(flags))
    acc.merge(MuonGeoModelCfg(flags))
    acc.merge(TGCCablingConfigCfg(flags))

    msgSvc = acc.getService("MessageSvc")
    msgSvc.setDebug = ["TgcIdHelper"]

    status = acc.run()
    return 0 if status.isSuccess() else 1


if __name__ == "__main__":
    raise SystemExit(main())