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

    parser = argparse.ArgumentParser(description='Run HGTD reconstruction with legacy Athena')
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

    flags.Exec.MaxEvents = -1  # Process all events

    # Output configuration
    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = f"LegacyHGTD_track_extension_test_AOD_{args.output_suffix}.root"
    flags.Output.HISTFileName = f"LegacyHGTD_track_extension_HIST_{args.output_suffix}.root"

    # Set specific tracking flags for legacy reconstruction
    flags.Tracking.doTruth = False
    flags.Detector.EnableCalo = False
    flags.DQ.useTrigger = False
    
    # Important: Configure to use legacy Athena tracking
    flags.Detector.EnableHGTD = True
    flags.HGTD.doActs = False  # Use legacy clustering
    flags.Reco.EnableHGTDExtension = True  # Enable HGTD track extension

    # Disable ACTS
    flags.Acts.doMonitoring = False
    flags.Acts.doITkConversion = False
    flags.Tracking.doITkConversion = False


    # Set tracking chain to use legacy tracking (not ACTS)
    from TrkConfig.TrkConfigFlags import TrackingComponent
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain]

    flags.lock()
    flags.dump()

    # Main services configuration
    acc = MainServicesCfg(flags)
    acc.getService("MessageSvc").debugLimit = 1000000
    acc.merge(PoolReadCfg(flags))

    # Schedule legacy ITk reconstruction
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
    acc.merge(ITkTrackRecoCfg(flags))

    # Configure HGTD pad clusterization (legacy method)
    from HGTD_Config.HGTD_PrepRawDataFormationConfig import PadClusterizationCfg
    acc.merge(PadClusterizationCfg(flags))

    # # Configure legacy HGTD track extension
    # from HGTD_Config.HGTD_TrackTimeExtensionConfig import TrackTimeExtensionCfg
    # acc.merge(TrackTimeExtensionCfg(flags))

    from HGTD_Config.HGTD_TrackTimeExtensionConfig import HGTD_IterativeExtensionToolCfg, TrackTimeExtensionCfg

    # Create your HGTD iterative extension tool with Chi2Cut set to 10.0
    myIterativeExtensionTool = acc.popToolsAndMerge(HGTD_IterativeExtensionToolCfg(flags, Chi2Cut=10000.0))

    # Pass the preconfigured tool to the TrackTimeExtension configuration
    acc.merge(TrackTimeExtensionCfg(flags, TimeExtensionTool=myIterativeExtensionTool))



    # Configure track time definitions and quality evaluation
    from HGTD_Config.HGTD_TrackTimeExtensionConfig import TrackTimeDefAndQualityAlgCfg
    acc.merge(TrackTimeDefAndQualityAlgCfg(flags))

    # Configure vertex time algorithm (if needed)
    from HGTD_Config.HGTD_VertexTimeConfig import VertexTimeAlgCfg
    acc.merge(VertexTimeAlgCfg(flags))

    # Run the job
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.run()
