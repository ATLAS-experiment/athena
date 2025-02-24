# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags


def HitMapBuilderCfg(flags):
    result=ComponentAccumulator()
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import PixelRawDataProviderAlgCfg
    result.merge(PixelRawDataProviderAlgCfg(flags))

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    result.merge(ByteStreamReadCfg(flags)) #Arguably, this should be added to PixelRawDataProviderAlgCfg
    
    from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
    result.merge(PixelReadoutManagerCfg(flags))

    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg
    result.merge(PixelCablingCondAlgCfg(flags))
    
    HitMapBuilder=CompFactory.HitMapBuilder()
    HitMapBuilder.LBMin = 0
    HitMapBuilder.LBMax = -1
    result.addEventAlgo(HitMapBuilder)

    result.addService(CompFactory.THistSvc(Output = [ "histfile DATAFILE='HitMap.root' OPT='RECREATE'"]))
    result.setAppProperty("HistogramPersistency","ROOT")

    return result



if __name__=="__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files =  defaultTestFiles.RAW_RUN3
    flags.Concurrency.NumThreads=1
    flags.Detector.GeometryMuon=False
    flags.Detector.GeometryTRT=False
    flags.IOVDb.GlobalTag="CONDBR2-BLKPA-2023-03"
    flags.GeoModel.AtlasVersion="ATLAS-R3S-2021-03-02-00"
    
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    acc.merge(HitMapBuilderCfg(flags))
    acc.getService("AvalancheSchedulerSvc").ShowDataDependencies=True
    
    acc.run()
