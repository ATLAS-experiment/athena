#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def ITkStripRawContByteStreamToolCfg(flags, name="ITkStripRawContByteStreamToolCustom", **kwargs) :
    acc = ComponentAccumulator()
    acc.setPrivateTools( CompFactory.ITkStripsRawContByteStreamTool(name=name,**kwargs))
    return acc

def ITkStripRawContByteStreamToolProviderToolCfg(flags, name="SCTRawContByteStreamToolProviderTool", **kwargs) :
    acc = ComponentAccumulator()
    if "RawContByteStreamTool" not in kwargs :
        kwargs.setdefault("RawContByteStreamTool", acc.popToolsAndMerge(ITkStripRawContByteStreamToolCfg(flags)))
    acc.addPublicTool( CompFactory.ITkStripRawContByteStreamToolProviderTool(name=name,**kwargs))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Detector.GeometryITkStrip = True

    flags.ITk.Geometry.AllLocal = False

    # for debugging
    from AthenaCommon.Constants import INFO
    flags.Exec.OutputLevel=INFO
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # For POOL file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # For ByteStream file writing
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
    writingAcc = ByteStreamWriteCfg(flags, [ "SCT_RDO_Container#ITkStripRDOs" ] )
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamType = "EventStorage"
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamName = "StreamBSFileOutput"
    acc.merge(writingAcc)

    if flags.GeoModel.Run is LHCPeriod.Run4:
        acc.merge(ITkStripRawContByteStreamToolProviderToolCfg(flags))        

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc.merge(GeoModelCfg(flags))

    from ITkStripCabling.ITkStripCablingConfig import ITkStripCablingToolCfg
    acc.popToolsAndMerge(ITkStripCablingToolCfg(flags))

    # For EventInfo necessary for ByteStream file writing
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    acc.merge(EventInfoCnvAlgCfg(flags,
                                 inputKey="McEventInfo",
                                 outputKey="EventInfo"))
    
    acc.run(maxEvents=100)
