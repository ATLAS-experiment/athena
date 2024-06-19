# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# "Standalone" test for exercising AthXRT with multiple APIs (OpenCL and XRT),
# and multiple kernels (krnl_VectorAdd and krnl_VectorMult).
#

def PrepareTest(default_xclbins=[]):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import logging

    log = logging.getLogger("AthExXRT.VectorAdditionOCLExample_test")

    flags = initConfigFlags()

    # Add options to command line parser
    parser = flags.getArgumentParser()

    parser.add_argument(
        "-x", "--xclbins", action="append", nargs="*", help="List of xclbins to load"
    )

    # Fill flags from command line
    args = flags.fillFromArgs(parser=parser)

    flags.addFlag("XclbinPathsList", default_xclbins)
    if args is not None and args.xclbins is not None:
        # Flatten the list of lists of xclbins.
        flags.XclbinPathsList = [item for sub_list in args.xclbins for item in sub_list]
    elif flags.XclbinPathsList:
        log.info("Using provided default xclbin list, use -x to override")
    else:
        from pathlib import Path

        # This is the location of default implement of the kernel, if run from hls folder.
        flags.XclbinPathsList = [
            str(Path(__file__).resolve().parent / "../hls/krnl_VectorAdd.xclbin")
        ]
        log.info("Using default xclbin build path, use -x to override")

    # If not specified, run only 2 events.
    if flags.Exec.MaxEvents==-1:
        log.info("Running test for 2 events by default, use --evtMax to override")
        flags.Exec.MaxEvents = 2

    flags.lock()

    log.info("Setup options:")
    log.info("  %s = %s", "Exec.MaxEvents", flags.Exec.MaxEvents)
    log.info("  %s = %s", "XclbinPathsList", flags.XclbinPathsList)

    return flags

if __name__ == "__main__":

    flags = PrepareTest()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg = MainServicesCfg(flags)

    from AthenaConfiguration.ComponentFactory import CompFactory

    XRTSvc = CompFactory.AthXRT.DeviceMgmtSvc(XclbinPathsList=flags.XclbinPathsList)
    cfg.addService(XRTSvc)

    # Add only the VectorAddOCLExampleAlg.
    XRTVectorAddOCLEx = CompFactory.AthExXRT.VectorAddOCLExampleAlg()
    cfg.addEventAlgo(XRTVectorAddOCLEx)

    cfg.run()
