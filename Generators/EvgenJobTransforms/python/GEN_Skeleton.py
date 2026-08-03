#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Functionality core of the Gen_tf transform"""

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True

# Get logger
from AthenaCommon.Logging import logging
evgenLog = logging.getLogger("Gen_tf")

# Common
from GeneratorConfig.Sequences import EvgenSequence
from PyUtils.Helpers import release_metadata

# Functions for pre/post-include/exec
from PyJobTransforms.TransformUtils import (
    processPreExec, 
    processPreInclude, 
    processPostExec, 
    processPostInclude
)

# Helper functions
from EvgenJobTransforms.EvgenHelpers import (
    _count_lhe_events,
    _validate_sample_properties,
    _handle_input_files,
    _is_txt_only_run
)

# Other imports that are needed
import sys, os, re

# Function that reads the jO and returns an instance of Sample(EvgenCAConfig)
def setupSample(flags):
    # Only permit one jobConfig argument for evgen
    job_config = flags.Generator.jobConfig
    if isinstance(job_config, str):
        job_config = [job_config]
    if len(job_config) != 1:
        raise RuntimeError("You must supply one and only one jobConfig file argument")

    evgenLog.info("Using JOBOPTSEARCHPATH (as seen in skeleton) = {}".format(os.environ["JOBOPTSEARCHPATH"]))

    FIRST_DIR = (os.environ["JOBOPTSEARCHPATH"]).split(":")[0]

    # Find jO file
    jofiles = [f for f in os.listdir(FIRST_DIR) if (f.startswith("mc") and f.endswith(".py"))]
    if len(jofiles) !=1:
        raise RuntimeError("You must supply one and only one jobOption file in DSID directory")
    jofile = jofiles[0]

    # Perform consistency checks on the jO
    from GeneratorConfig.GenConfigHelpers import (
        checkNaming, 
        checkNEventsPerJob, 
        checkKeywords, 
        checkCategories
    )
    checkNaming(jofile)

    # Import the jO as a module
    # We cannot do import BLAH directly since
    # 1. the filenames are not python compatible (mc.GEN_blah.py)
    # 2. the filenames are different for every jO
    import importlib.util
    spec = importlib.util.spec_from_file_location(
        name="sample",
        location=os.path.join(FIRST_DIR,jofile),
    )
    jo = importlib.util.module_from_spec(spec)
    
    spec.loader.exec_module(jo)
    evgenLog.info(f"including file {jofile}")

    # Create instance of Sample(EvgenCAConfig)
    sample = jo.Sample(flags)

    # Set up the sample properties
    sample.setupFlags(flags)

    # Set the random number seed
    # Need to use logic in EvgenJobTransforms.Generate_dsid_ranseed

    # Get DSID
    dsid = os.path.basename(job_config[0])
    if dsid.startswith("Test"):
        dsid = dsid.split("Test")[-1]

    # Update the global flags
    if dsid.isdigit():
        flags.Generator.DSID = int(dsid)

    # Set nEventsPerJob
    if not sample.nEventsPerJob:
        evgenLog.info("#############################################################")
        evgenLog.info(" !!!! no sample.nEventsPerJob set !!!")
        evgenLog.info("#############################################################")
        # We don't need to set the global flag because its default is 10000
    else:
        checkNEventsPerJob(sample)
        evgenLog.info(" nEventsPerJob = " + str(sample.nEventsPerJob))
        flags.Generator.nEventsPerJob = sample.nEventsPerJob

    # Validate all required/conditional sample metadata with explicit rules.
    _validate_sample_properties(sample)

    # Propagate optional sample values to global flags.
    flags.Generator.inputFilesPerJob = sample.inputFilesPerJob
    flags.Generator.MEgenerator = sample.MEgenerator or ""

    # Print sample metadata in the log.
    for var, value in vars(sample).items():
        evgenLog.info("MetaData: {} = {}".format(var, value))

    # Keywords check
    if hasattr(sample, "keywords"):
        checkKeywords(sample, evgenLog)

    # L1, L2 categories check
    if hasattr(sample, "categories"):
        checkCategories(sample, evgenLog)

    return sample


# Function to check black-listed releases
def checkBlackList(cache, generatorName, checkType) :
    isError = None
    fileName = "BlackList_caches.txt" if checkType == "black" else "PurpleList_generators.txt"
    with open(f"/cvmfs/atlas.cern.ch/repo/sw/Generators/MC16JobOptions/common/{fileName}") as bfile:
        for line in bfile.readlines():
            if not line.strip():
                continue
            # Bad caches
            badCache=line.split(',')[1].strip()
            # Bad generators
            badGens=line.split(',')[2].strip()

            used_gens = ','.join(generatorName)
            # Match Generator and release cache
            if cache==badCache and re.search(badGens,used_gens) is not None:
                if badGens=="": badGens="all generators"
                isError=f"{cache} is {checkType}-listed for {badGens}"
                return isError
    return isError


# Main function
def fromRunArgs(runArgs):
    # print release information
    d = release_metadata()
    evgenLog.info("using release [%(project name)s-%(release)s] [%(platform)s] [%(nightly name)s/%(nightly release)s] -- built on [%(date)s]", d)
    athenaRel = d["release"]

    evgenLog.info("****************** STARTING EVENT GENERATION *****************")

    evgenLog.info("**** Transformation run arguments")
    evgenLog.info(runArgs)

    evgenLog.info("**** Setting-up configuration flags")

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Generation

    # Convert run arguments to global athena flags
    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)

    # Convert generator-specific run arguments to global athena flags
    from GeneratorConfig.GeneratorConfigFlags import  generatorRunArgsToFlags
    generatorRunArgsToFlags(runArgs, flags)

    # convert arguments to flags
    flags.fillFromArgs()

    # Determine maximum number of events to generate
    requested_max_events = flags.Exec.MaxEvents
    # Event generation is not using standard event counting
    flags.Exec.MaxEvents = -1

    # Create an instance of the Sample(EvgenCAConfig) and update global flags accordingly
    sample = setupSample(flags)

    # Determine output file name and type.
    output_pool_file = (
        flags.Output.EVNTFileName
        or getattr(runArgs, "outputEVNTFile", None)
        or getattr(runArgs, "outputEVNT_PreFile", None)
    )
    flags.Output.EVNTFileName = output_pool_file or ""
    output_txt_file = (
        flags.Output.TXTFileName
        or getattr(runArgs, "outputTXTFile", None)
    )
    flags.Output.TXTFileName = output_txt_file or ""

    # If no EVNT output is specified, we check if it's a TXT-only run (i.e. standalone LHE output production). 
    # In that case, we don't require an EVNT output file.
    txt_only_mode = _is_txt_only_run(flags)
    if not output_pool_file and not (flags.Generator.outputYODAFile or txt_only_mode):
        raise RuntimeError("No output evgen EVNT or EVNT_Pre file provided.")

    # Setup the main flags
    flags.Exec.FirstEvent = flags.Generator.firstEvent

    # We are always doing MC
    flags.Input.isMC = True

    # If no inputEVNT_PreFile was provided clear transform placeholder input files 
    # and set RunNumber/TimeStamp based on DSID. 
    if hasattr(runArgs, "inputEVNT_PreFile") and runArgs.inputEVNT_PreFile:
        flags.Input.Files = runArgs.inputEVNT_PreFile
    else:
        flags.Input.Files = []

    if not flags.Input.Files:
        flags.Input.Files = []
        flags.Input.RunNumbers = [flags.Generator.DSID]
        flags.Input.TimeStamps = [0]

    flags.PerfMon.doFastMonMT = True
    flags.PerfMon.doFullMonMT = True

    # Process pre-include
    processPreInclude(runArgs, flags)

    # Process pre-exec
    processPreExec(runArgs, flags)

    # Lock flags
    flags.lock()

    evgenLog.info("**** Configuration flags")
    if runArgs.VERBOSE:
        flags.dump()
    else:
        flags.dump("Generator.*")

    # Print various stuff
    evgenLog.info(".transform = Gen_tf")
    evgenLog.info(".platform = " + str(os.environ["BINARY_TAG"]))

    # Announce start of job configuration
    evgenLog.info("**** Configuring event generation")

    # Main object
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
    cfg = MainEvgenServicesCfg(flags, withSequences=True)

    # Input file handling (if needed)
    if flags.Input.Files and not txt_only_mode:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    # EventInfoCnvAlg
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True, xAODKey="TMPEvtInfo"),
                                 sequenceName=EvgenSequence.Generator.value)

    # Set up the process
    cfg.merge(sample.setupProcess(flags))

    # Sort the list of generator names into standard form
    from GeneratorConfig.GenConfigHelpers import gen_sortkey
    from GeneratorConfig.Versioning import generatorsGetInitialVersionedDictionary, generatorsVersionedStringList
    if not flags.Input.Files:
        generators = sorted(cfg.getService("GeneratorInfoSvc").Generators, key=gen_sortkey)
        gendict = generatorsGetInitialVersionedDictionary(generators)
        generatorsWithVersion = generatorsVersionedStringList(gendict)
    else:
        # TODO: read from metadata
        generators = []
        generatorsWithVersion = []

    # Check if the setup requires steering
    from GeneratorConfig.GenConfigHelpers import gen_require_steering
    if gen_require_steering(generators):
        if hasattr(runArgs, "outputEVNTFile") and not hasattr(runArgs, "outputEVNT_PreFile"):
            raise RuntimeError("'EvtGen' found in job options name, please set '--steering=afterburn'")

    # LHE input handling
    nEventsLHE = None
    if flags.Generator.inputFilesPerJob > 0:
        if not flags.Generator.inputGeneratorFile:
            raise RuntimeError(f"Sample sets inputFilesPerJob = {flags.Generator.inputFilesPerJob} but Gen_tf run without inputGeneratorFile")
        else:
            nEventsLHE = _handle_input_files(generators, flags)

    # Check black-list and purple-list
    blError = checkBlackList(athenaRel, generators, "black")
    plError = checkBlackList(athenaRel, generators, "purple")
    if blError is not None:
        raise RuntimeError(blError)
    if plError is not None:
        evgenLog.warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        evgenLog.warning(f"!!! WARNING {plError} !!!")
        evgenLog.warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")

    # Fix non-standard event features
    if not txt_only_mode and not flags.Input.Files:
        from EvgenProdTools.EvgenProdToolsConfig import FixHepMCCfg
        from GeneratorConfig.GenConfigHelpers import gens_purgenoendvtx
        generatorsList = generators.copy()
        if "Pythia8" in generatorsList:
            pythia8Alg = cfg.getEventAlgo("Pythia8_i")
            if pythia8Alg.Beam1 != "PROTON" or pythia8Alg.Beam2 != "PROTON":
            # generator name is still "Pythia8", even when colliding nuclei
                generatorsList.append("Pythia8-Angantyr")
        cfg.merge(FixHepMCCfg(flags,
                              PurgeUnstableWithoutEndVtx=gens_purgenoendvtx(generatorsList)))

    # Merge GenWeightDeclarationCfg to declare the number 
    # of generator weights to the CutFlowSvc
    if output_pool_file and not flags.Input.Files:
        from EvgenProdTools.EvgenProdToolsConfig import GenWeightDeclarationCfg
        cfg.merge(GenWeightDeclarationCfg(flags))

    # Sanity check the event record (not appropriate for all generators)
    from GeneratorConfig.GenConfigHelpers import gens_testhepmc
    if not txt_only_mode and gens_testhepmc(generators):
        from EvgenProdTools.EvgenProdToolsConfig import TestHepMCCfg
        cfg.merge(TestHepMCCfg(flags))

    # Copying event-level HepMC decorations is EVNT-oriented and not needed for
    # standalone LHE output production.
    if not txt_only_mode:
        from EvgenProdTools.EvgenProdToolsConfig import CopyEventWeightCfg
        cfg.merge(CopyEventWeightCfg(flags))

        from EvgenProdTools.EvgenProdToolsConfig import FillFilterValuesCfg
        cfg.merge(FillFilterValuesCfg(flags))

    # Configure the event counting (AFTER all filters)
    from EvgenProdTools.EvgenProdToolsConfig import CountHepMCCfg
    requested_output = (
        1 if txt_only_mode else
        (sample.nEventsPerJob if requested_max_events == -1 else requested_max_events)
    )
    count_kwargs = {"RequestedOutput": requested_output}
    if txt_only_mode:
        # In TXT-only mode there is no GEN_EVENT in StoreGate. Disabling
        # HepMC/EventInfo corrections avoids dereferencing missing event data.
        count_kwargs["CorrectHepMC"] = False
        count_kwargs["CorrectEventID"] = False
        count_kwargs["CorrectRunNumber"] = False
        count_kwargs["CopyRunNumber"] = False
        count_kwargs["InputEventInfo"] = ""
        count_kwargs["OutputEventInfo"] = ""
        count_kwargs["mcEventWeightsKey"] = ""
    cfg.merge(CountHepMCCfg(flags, **count_kwargs))
    evgenLog.info(f"Requested output events = {cfg.getEventAlgo('CountHepMC').RequestedOutput}")

    # Print out the contents of the first 5 events (after filtering)
    if not txt_only_mode and flags.Generator.printEvts > 0:
        from TruthIO.TruthIOConfig import PrintMCCfg
        cfg.merge(PrintMCCfg(flags,
                             LastEvent=flags.Generator.printEvts))

    # PerfMon
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    cfg.merge(PerfMonMTSvcCfg(flags), sequenceName=EvgenSequence.Post.value)

    # Estimate time needed for Simulation
    if not txt_only_mode:
        from EvgenProdTools.EvgenProdToolsConfig import SimTimeEstimateCfg
        cfg.merge(SimTimeEstimateCfg(flags))

    # TODO: Rivet

    # Extra metadata
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    from GeneratorConfig.GenConfigHelpers import gen_lhef
    metadata = {
        "project_name": "IS_SIMULATION",
        f"AtlasRelease_{runArgs.trfSubstepName}": flags.Input.Release or "n/a",
        "beam_energy": str(int(flags.Beam.Energy)),
        "beam_type": flags.Beam.Type.value,
        "hepmc_version": f"HepMC{os.environ['HEPMCVER']}",
        "keywords": ", ".join(sample.keywords).lower(),
        "lhefGenerator": '+'.join(filter(gen_lhef, generators)),
        "mc_channel_number": str(flags.Generator.DSID),
    }
    if not flags.Input.Files:
        metadata.update({
            "generators": '+'.join(generatorsWithVersion),
            "tune": cfg.getService("GeneratorInfoSvc").Tune
        })
    if hasattr(sample, "process"): metadata.update({"evgenProcess": sample.process})
    if hasattr(sample, "specialConfig"): metadata.update({"specialConfiguration": sample.specialConfig})
    if hasattr(sample, "hardPDF"): metadata.update({"hardPDF": sample.hardPDF})
    if hasattr(sample, "softPDF"): metadata.update({"softPDF": sample.softPDF})
    if hasattr(sample, "randomSeed"): metadata.update({"randomSeed": str(flags.Random.SeedOffset)})
    cfg.merge(TagInfoMgrCfg(flags, tagValuePairs=metadata))

    # Print metadata in the log
    evgenLog.info(f"HepMC version {os.environ['HEPMCVER']}")
    if not flags.Input.Files:
        evgenLog.info(f"MetaData: generatorTune = {cfg.getService('GeneratorInfoSvc').Tune}")
    evgenLog.info("MetaData: generatorName = {}".format(generatorsWithVersion))
    if nEventsLHE is not None:
        print(f"MetaData: Number of input LHE events = {nEventsLHE}")
    elif txt_only_mode:
        produced_lhe = None
        for candidate in (flags.Output.TXTFileName, "events.lhe"):
            if candidate and os.path.exists(candidate):
                produced_lhe = candidate
                break
        if produced_lhe:
            nEventsTXT = _count_lhe_events(produced_lhe)
            print(f"MetaData: Number of produced LHE events = {nEventsTXT}")

    if output_pool_file:
        # Count all events that are written
        from EventBookkeeperTools.EventBookkeeperToolsConfig import AllWrittenEventsCounterAlgCfg
        cfg.merge(AllWrittenEventsCounterAlgCfg(flags))

        # Configure output stream
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        cfg.merge(OutputStreamCfg(flags, "EVNT", ["McEventCollection#*"],
                                  MetadataItemList=["IOVMetaDataContainer#*"]))

        # Add in-file MetaData
        from AthenaConfiguration.Enums import MetadataCategory
        from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
        cfg.merge(SetupMetaDataForStreamCfg(flags, "EVNT",
                                            createMetadata=[MetadataCategory.CutFlowMetaData,
                                                            MetadataCategory.TruthMetaData]))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file MetaData
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    # Hack the main sequence to not ignore filters
    # TODO: figure out if we can do it in a more elegant way without another nested sequence
    cfg.getSequence("AthAlgSeq").IgnoreFilterPassed = False

    # Print ComponentAccumulator components
    cfg.printConfig(prefix="Gen_tf", printSequenceTreeOnly=not runArgs.VERBOSE)

    # Run final ComponentAccumulator
    sys.exit(not cfg.run().isSuccess())
