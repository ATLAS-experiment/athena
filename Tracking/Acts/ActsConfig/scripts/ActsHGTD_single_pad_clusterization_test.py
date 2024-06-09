#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import argparse

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Run reconstruction script with specified configuration.")
    parser.add_argument("--mode", choices=["legacy", "xAOD"], required=True, help="Specify the reconstruction mode.")
    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    flags.Exec.MaxEvents = -1

    output_suffix = "_legacy" if args.mode == "legacy" else "_xAOD"
    flags.Output.HISTFileName = f"ActsMonitoringOutput{output_suffix}.root"
    flags.DQ.useTrigger = False

    flags.Acts.doMonitoring = True
    flags.HGTD.doMonitoring = True

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    if args.mode == "legacy":
        from HGTD_Config.HGTD_PrepRawDataFormationConfig import PadClusterizationCfg
        acc.merge(PadClusterizationCfg(flags))

        # Add edm converter
        from InDetConfig.InDetPrepRawDataFormationConfig import HGTDInDetToXAODClusterConversionCfg
        acc.merge(HGTDInDetToXAODClusterConversionCfg(flags))

    elif args.mode == "xAOD":
        from ActsConfig.ActsClusterizationConfig import ActsHgtdClusterizationAlgCfg
        acc.merge(ActsHgtdClusterizationAlgCfg(flags))

    else :
        raise ValueError("Invalid mode selected. Please select either 'legacy' or 'xAOD'.")

    from ActsConfig.ActsAnalysisConfig import ActsHgtdClusterAnalysisAlgCfg
    acc.merge(ActsHgtdClusterAnalysisAlgCfg(flags))

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.run()
