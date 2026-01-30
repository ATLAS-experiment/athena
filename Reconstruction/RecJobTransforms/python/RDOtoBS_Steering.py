# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import LHCPeriod

def RDOtoBS_Steering(flags):
    acc = ComponentAccumulator()

    if flags.GeoModel.Run is LHCPeriod.Run4:
        #ITk pixel
        from ITkPixelCabling.ITkPixelCablingAlgConfig import ITkPixelCablingAlgCfg
        acc.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=True))

        from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
        acc.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

        from ITkPixelByteStreamCnv.ITkPixelEncodingAlgConfig import ITkPixelEncodingAlgCfg
        acc.merge( ITkPixelEncodingAlgCfg(flags) )
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
        acc.merge(ByteStreamWriteCfg(flags, ['ITkPixelRDO_Container#ITkPixelRDOs']))

    return acc
