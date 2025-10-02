#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Main script execution
if __name__ == "__main__":

    import argparse
    import glob
    import re

    # Initialize the configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    flags = initConfigFlags()

    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)

    parser = argparse.ArgumentParser(description='Run HGTD reconstruction with ACTS')
    parser.add_argument('--file_dir', type=str, required=True, help='Path to directory with RDO files')
    parser.add_argument('--Nfiles', type=int, default=-1, help='Number of files to process (-1 for all)')
    parser.add_argument('--output_suffix', type=str, default='', help='Suffix to append to output files')
    args = parser.parse_args()

    # Get the list of all matching files
    file_pattern = f"{args.file_dir}/RDO*.pool.root.1"    
    all_files = sorted(glob.glob(file_pattern))

    matching_files = []
    for f in all_files:
        matching_files.append(f)

    if args.Nfiles > 0 and len(matching_files) > args.Nfiles:
        matching_files = matching_files[:args.Nfiles]
        print(f"Limiting to first {args.Nfiles} files as requested")

    flags.Input.Files = matching_files

    # Check the files matched
    print(f"Matched {len(matching_files)} files:")
    for f in matching_files:
        print(f)

    flags.Exec.MaxEvents = -1 #100

    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = f"ActsHGTD_track_extension_test_AOD_{args.output_suffix}.root"
    flags.Output.HISTFileName = f"ActsHGTD_track_extension_HIST_{args.output_suffix}.root"

    # Set specific tracking flags
    flags.Tracking.doTruth = False

    flags.Detector.EnableCalo = False
    flags.DQ.useTrigger = False
    flags.Acts.doMonitoring = True
    flags.HGTD.doMonitoring = True

    flags.Detector.EnableHGTD = True
    flags.Acts.doITkConversion=False
    flags.Tracking.doITkConversion=False
    
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True
    from TrkConfig.TrkConfigFlags import TrackingComponent
    flags.Tracking.recoChain = [TrackingComponent.ActsLegacyChain] # Use ActsLegacyChain to avoid fast tracking requirement
    flags.Acts.doRotCorrection = False

    flags.lock()
    flags.dump()

    # Main services configuration
    acc = MainServicesCfg(flags)
    acc.getService("MessageSvc").debugLimit = 100000000
    acc.getService("MessageSvc").verboseLimit = 100000000
    acc.merge(PoolReadCfg(flags))

    # HGTD Reconstruction - xAOD EDM
    from ActsConfig.ActsClusterizationConfig import ActsHgtdClusterizationAlgCfg
    acc.merge(ActsHgtdClusterizationAlgCfg(flags))

    # Schedule the complete ITk tracking reconstruction
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
    acc.merge(ITkTrackRecoCfg(flags))

    # Add the HGTD Track Extension Algorithm
    from ActsConfig.ActsHGTDTrackExtensionAlgConfig import ActsHGTDTrackExtensionAlgConfig
    acc.merge(ActsHGTDTrackExtensionAlgConfig(flags))


    # Run the job
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.run()

