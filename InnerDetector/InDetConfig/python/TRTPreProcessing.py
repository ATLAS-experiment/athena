# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# ------------------------------------------------------------
#
# ----------- TRT Data-Preparation stage
#
# ------------------------------------------------------------
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format, BeamType

def TRTPreProcessingCfg(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.EnableTRT:
        return acc

    if (flags.Input.Format is Format.BS
       or 'TRT_RDOs' in flags.Input.Collections
       or f'{flags.Overlay.BkgPrefix}TRT_RDOs' in flags.Input.Collections):

        #
        # --- TRT_RIO_Maker Algorithm
        #
        if flags.Beam.Type is BeamType.Cosmics:
            from InDetConfig.InDetPrepRawDataFormationConfig import (
                InDetTRT_NoTime_RIO_MakerCfg)
            acc.merge(InDetTRT_NoTime_RIO_MakerCfg(flags))
        else:
            from InDetConfig.InDetPrepRawDataFormationConfig import (
                InDetTRT_RIO_MakerCfg)
            acc.merge(InDetTRT_RIO_MakerCfg(flags))

        if flags.InDet.doSplitReco:
            from InDetConfig.InDetPrepRawDataFormationConfig import (
                InDetTRT_RIO_MakerPUCfg)
            acc.merge(InDetTRT_RIO_MakerPUCfg(flags))

        #
        #    Include alg to save the local occupancy inside xAOD::EventInfo
        #
        if flags.InDet.doTRTGlobalOccupancy:
            from InDetConfig.TRT_ElectronPidToolsConfig import (
                TRTOccupancyIncludeCfg)
            acc.merge(TRTOccupancyIncludeCfg(flags))

    #
    # --- we need to do truth association if requested (not for uncalibrated hits in cosmics)
    #
    if flags.InDet.doTruth and (
            flags.Beam.Type is not BeamType.Cosmics and
            'PRD_MultiTruthTRT' not in flags.Input.Collections):
        from InDetConfig.InDetTruthAlgsConfig import (
            InDetPRD_MultiTruthMakerTRTCfg)
        acc.merge(InDetPRD_MultiTruthMakerTRTCfg(flags))
        if flags.InDet.doSplitReco:
            from InDetConfig.InDetTruthAlgsConfig import (
                InDetPRD_MultiTruthMakerTRTPUCfg)
            acc.merge(InDetPRD_MultiTruthMakerTRTPUCfg(flags))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2

    # TODO: TRT only?

    numThreads = 1
    flags.Concurrency.NumThreads = numThreads
    # Might change this later, but good enough for the moment.
    flags.Concurrency.NumConcurrentEvents = numThreads

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    top_acc.merge(TRT_ReadoutGeometryCfg(flags))

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    top_acc.merge(PixelReadoutGeometryCfg(flags))
    top_acc.merge(SCT_ReadoutGeometryCfg(flags))

    top_acc.merge(TRTPreProcessingCfg(flags))

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel = 5
    top_acc.run(25)
    top_acc.store(open("test_TRTPrepocessing.pkl", "wb"))
