#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def SCTRawContByteStreamToolCfg(flags, name="SCTRawContByteStreamToolCustom", **kwargs) :
    acc = ComponentAccumulator()
    acc.setPrivateTools( CompFactory.SCTRawContByteStreamTool(name=name,**kwargs))
    return acc

def SCTRawContByteStreamToolProviderToolCfg(flags, name="SCTRawContByteStreamToolProviderTool", **kwargs) :
    acc = ComponentAccumulator()
    if "RawContByteStreamTool" not in kwargs :
        kwargs.setdefault("RawContByteStreamTool", acc.popToolsAndMerge(SCTRawContByteStreamToolCfg(flags)))
    acc.addPublicTool( CompFactory.SCTRawContByteStreamToolProviderTool(name=name,**kwargs))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TriggerTest/valid1.110401.PowhegPythia_P2012_ttbar_nonallhad.recon.RDO.e3099_s2578_r7572_tid07644622_00/RDO.07644622._000001.pool.root.1"]
    flags.IOVDb.GlobalTag = "OFLCOND-RUN12-SDR-31"
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # For POOL file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # For ByteStream file writing
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
    writingAcc = ByteStreamWriteCfg(flags, [ "SCT_RDO_Container#SCT_RDOs" ] )
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamType = "EventStorage"
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamName = "StreamBSFileOutput"
    acc.merge(writingAcc)

    if flags.GeoModel.Run is not LHCPeriod.Run4:
        acc.merge(SCTRawContByteStreamToolProviderToolCfg(flags))    

    # For SCT geometry and cabling
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))
    from SCT_Cabling.SCT_CablingConfig import SCT_CablingToolCfg
    acc.popToolsAndMerge(SCT_CablingToolCfg(flags))

    # For EventInfo necessary for ByteStream file writing
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    acc.merge(EventInfoCnvAlgCfg(flags,
                                 inputKey="McEventInfo",
                                 outputKey="EventInfo"))

    acc.run(maxEvents=10)
