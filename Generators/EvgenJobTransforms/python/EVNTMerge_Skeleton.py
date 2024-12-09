# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger("EVNTMerge")
    log.info("****************** STARTING EVNT MERGING *****************")

    log.info("**** Transformation run arguments")
    log.info(str(runArgs))

    log.info("**** Setting-up configuration flags")
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep, MetadataCategory
    flags.Common.ProductionStep = ProductionStep.Generation

    commonRunArgsToFlags(runArgs, flags)

    if hasattr(runArgs, "inputEVNTFile"):
        flags.Input.Files = runArgs.inputEVNTFile
    else:
        raise RuntimeError("No input EVNT file defined")

    if hasattr(runArgs, "outputEVNT_MRGFile"):
        if runArgs.outputEVNT_MRGFile == "None":
            flags.Output.EVNTFileName = ""
            # TODO decide if we need a specific EVNT_MRGFileName flag
        else:
            flags.Output.EVNTFileName  = runArgs.outputEVNT_MRGFile
    else:
        raise RuntimeError("No outputEVNT_MRGFile defined")

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, "EVNT", disableEventTag = True, takeItemsFromInput = True, extendProvenanceRecord = False))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    cfg.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "EVNT",
            disableEventTag=True,
            createMetadata=[MetadataCategory.IOVMetaData],
        )
    )

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    ## Special configuration
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    hepmc_version = GetFileMD(flags.Input.Files).get("hepmc_version", None)
    if hepmc_version is not None:
        if hepmc_version == "2":
            hepmc_version = "HepMC2"
        elif hepmc_version == "3":
            hepmc_version = "HepMC3"
        log.info("Input file was produced with %s", hepmc_version)
        from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
        cfg.merge(TagInfoMgrCfg(flags, tagValuePairs={"hepmc_version": hepmc_version}))

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Ran EVNTMerge_tf in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
