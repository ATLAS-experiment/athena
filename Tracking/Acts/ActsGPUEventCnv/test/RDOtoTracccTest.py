#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the GPU EDM input conversion chain:
#   RDO -> traccc cells

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import ActsDeviceDetectorDescriptionProviderSvcCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg

def RDOtoTracccCellConverterTest(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    hostMR   = CompFactory.AthCUDA.HostMemoryResourceTool("HostMR")
    deviceMR = CompFactory.AthCUDA.DeviceMemoryResourceTool("DeviceMR")
    copyTool = CompFactory.AthCUDA.CopyTool("CopyProviderTool")

    # Service runs first — loads all device detector description data into detStore
    acc.merge(ActsDeviceDetectorDescriptionProviderSvcCfg(flags,
        PopulateFromFile = True,
        HostMR   = hostMR,
        DeviceMR = deviceMR,
        CopyProviderTool = copyTool))

    acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
        HostMR  = hostMR,
        DeviceMR = deviceMR,
        CopyProviderTool = copyTool))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14697/RDO.33629020._000001.pool.root.1"
    ]

    flags.fillFromArgs()

    # ---- Conditions and geometry ----
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    flags.IOVDb.GlobalTag    = defaultConditionsTags.RUN4_MC
    flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-03-00-00"
    flags.GeoModel.Align.Dynamic = False

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True

    # ---- Scheduler ----
    flags.Concurrency.NumThreads          = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Concurrency.NumProcs            = 0
    flags.Scheduler.ShowDataDeps          = True
    flags.Scheduler.ShowDataFlow          = True
    flags.Scheduler.CheckDependencies     = True

    flags.lock()
    flags.dump()

    acc = RDOtoTracccCellConverterTest(flags)
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"