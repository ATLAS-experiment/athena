#!/usr/bin/env athena.py

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG, INFO
from ActsConfig.ActsPhaseIIRawDataEdmConfig import (
    PhaseIIPixelRawDataContainerCfg,
    PhaseIIStripRawDataContainerCfg,
    )
from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    RDOtoTracccCellConverterAlgCfg,
    PhaseIIRDOtoTracccCellConverterAlgCfg,
    TracccCellValidationAlgCfg,
)
from AthCUDAServices.AthCUDAServicesConfig import (
    HostMemoryResourceToolCfg,
    DeviceMemoryResourceToolCfg,
    CopyToolCfg,
)
from AthDeviceComps.AthDeviceCompsConfig import HostCopyToolCfg

ComponentAccumulator.debugMode = "trackCA trackEventAlgo"

# Enable for development and testing:
# - DEBUG algs
# - config and SG dumps
TESTING = False
# Enable for performance testing:
# - runs input algo twice to make sure data from files are loaded in memory on
#   second execution
# - different input events (400 x ttbar, pu200)
PERFORMANCE_TESTING = False

def RDOtoTracccCellConversionTest(flags, cpu_cell_sorting: bool) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags))

    Ph1Cells = "TracccCellsPh1"
    Ph2Cells = "TracccCellsPh2"

    output_level = DEBUG if TESTING else INFO

    if PERFORMANCE_TESTING:
        # Make sure the data are loaded in memory when the real algo starts
        acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
            name="DummyPh1AlgToPreloadData",
            TracccCells = "DummyCells",
            CPUCellSorting = False,
            ))
    acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
        TracccCells = Ph1Cells,
        CPUCellSorting = cpu_cell_sorting,
        OutputLevel = output_level,
        ))

    # Make the PhaseII RDO containers available
    acc.merge(PhaseIIPixelRawDataContainerCfg(flags))
    acc.merge(PhaseIIStripRawDataContainerCfg(flags))
    if PERFORMANCE_TESTING:
        # Make sure the data are loaded in memory when the real algo starts
        acc.merge(PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
            name="DummyPh2AlgToPreloadData",
            TracccCells = "DummyCells",
            CPUCellSorting = False,
            ))
    acc.merge(PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
        TracccCells = Ph2Cells,
        CPUCellSorting = cpu_cell_sorting,
        OutputLevel = output_level,
        ))
        
    acc.merge(TracccCellValidationAlgCfg(flags,
        name = "ValidatePh2Cells",
        ReferenceCells = Ph1Cells,
        Cells = Ph2Cells,
    ))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()

    if PERFORMANCE_TESTING:
        flags.Input.Files = [
            "/eos/atlas/atlasgroupdisk/trig-daq/dq2/rucio/mc21_14TeV/af/f5/RDO.39626672._000001.pool.root.1",
            "/eos/atlas/atlasgroupdisk/trig-daq/dq2/rucio/mc21_14TeV/2d/fc/RDO.39626672._000003.pool.root.1",
            "/eos/atlas/atlasgroupdisk/trig-daq/dq2/rucio/mc21_14TeV/95/73/RDO.39626672._000004.pool.root.1",
            "/eos/atlas/atlasgroupdisk/trig-daq/dq2/rucio/mc21_14TeV/a9/d8/RDO.39626672._000005.pool.root.1",
        ]
    else:
        flags.Input.Files = defaultTestFiles.RDO_RUN4

    if TESTING:
        flags.Scheduler.ShowDataDeps          = True
        flags.Scheduler.ShowDataFlow          = True
        flags.Scheduler.CheckDependencies     = True

    flags.PerfMon.doFullMonMT = True

    flags.fillFromArgs()

    flags.lock()
    if TESTING:
        flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    # Needed for PixelID and SCT_ID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    acc.merge(RDOtoTracccCellConversionTest(
        flags,
        cpu_cell_sorting=False
        ))
    acc.printConfig(withDetails=True, summariseProps=True)

    if PERFORMANCE_TESTING:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        acc.merge(PerfMonMTSvcCfg(flags))

    if TESTING:
        sg = acc.getService("StoreGateSvc")
        sg.Dump = True

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
