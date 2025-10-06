#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
"""Functionality core of the Gen_tf transform"""

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True

# Get logger
from AthenaCommon.Logging import logging
evgenLog = logging.getLogger("Gen_tf")

# Common
from AthenaCommon.SystemOfUnits import GeV
from GeneratorConfig.Sequences import EvgenSequence
from PyUtils.Helpers import release_metadata

# Functions for pre/post-include/exec
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# Other imports that are needed
import sys, os, re

# Function that reads the jO and returns an instance of Sample(EvgenCAConfig)
def setupSample(runArgs, flags):
    # Only permit one jobConfig argument for evgen
    if len(runArgs.jobConfig) != 1:
        raise RuntimeError("You must supply one and only one jobConfig file argument")

    evgenLog.info("Using JOBOPTSEARCHPATH (as seen in skeleton) = {}".format(os.environ["JOBOPTSEARCHPATH"]))

    FIRST_DIR = (os.environ["JOBOPTSEARCHPATH"]).split(":")[0]

    # Find jO file
    jofiles = [f for f in os.listdir(FIRST_DIR) if (f.startswith("mc") and f.endswith(".py"))]
    if len(jofiles) !=1:
        raise RuntimeError("You must supply one and only one jobOption file in DSID directory")
    jofile = jofiles[0]

    # Perform consistency checks on the jO
    from GeneratorConfig.GenConfigHelpers import checkJOConsistency, checkNEventsPerJob, checkKeywords, checkCategories
    officialJO = checkJOConsistency(jofile)

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
    evgenLog.info("including file %s", jofile)

    # Create instance of Sample(EvgenCAConfig)
    sample = jo.Sample(flags)

    # Set up the sample properties
    sample.setupFlags(flags)

    # Set the random number seed
    # Need to use logic in EvgenJobTransforms.Generate_dsid_ranseed

    # Get DSID
    dsid = os.path.basename(runArgs.jobConfig[0])
    if dsid.startswith("Test"):
        dsid = dsid.split("Test")[-1]

    # Update the global flags
    if dsid.isdigit():
        flags.Generator.DSID = int(dsid)

    # Set nEventsPerJob
    if not sample.nEventsPerJob:
        evgenLog.info("#############################################################")
        evgenLog.info(" !!!! no sample.nEventsPerJob set !!!  The default 10000 used. !!! ")
        evgenLog.info("#############################################################")
    else:
        checkNEventsPerJob(sample)
        evgenLog.info(" nEventsPerJob = " + str(sample.nEventsPerJob))
        flags.Generator.nEventsPerJob = sample.nEventsPerJob

    # Check if sample attributes have been properly set
    for var, value in vars(sample).items():
       if not value:
           raise RuntimeError("self.{} should be set in Sample(EvgenConfig)".format(var))
       else:
           if var == "generators":
               from GeneratorConfig.Versioning import generatorsGetInitialVersionedDictionary, generatorsVersionedStringList
               from GeneratorConfig.GenConfigHelpers import gen_sortkey
               gennames = sorted(sample.generators, key=gen_sortkey)
               gendict = generatorsGetInitialVersionedDictionary(gennames)
               gennamesvers = generatorsVersionedStringList(gendict)
               evgenLog.info("MetaData: generatorName = {}".format(gennamesvers))
           else:
               evgenLog.info("MetaData: {} = {}".format(var, value))

    # Check for other inconsistencies in jO
    if len(sample.generators) > len(set(sample.generators)):
        raise RuntimeError("Duplicate entries in generators: invalid configuration, please check your JO")
    from GeneratorConfig.GenConfigHelpers import gen_require_steering
    if gen_require_steering(sample.generators):
        if hasattr(runArgs, "outputEVNTFile") and not hasattr(runArgs, "outputEVNT_PreFile"):
            raise RuntimeError("'EvtGen' found in job options name, please set '--steering=afterburn'")

    # Keywords check
    if hasattr(sample, "keywords"):
        checkKeywords(sample, evgenLog, officialJO)

    # L1, L2 categories check
    if hasattr(sample, "categories"):
        checkCategories(sample, evgenLog, officialJO)

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

    # Create an instance of the Sample(EvgenCAConfig) and update global flags accordingly
    sample = setupSample(runArgs, flags)

    # Sort the list of generator names into standard form
    from GeneratorConfig.GenConfigHelpers import gen_sortkey, gen_lhef
    generatorNames = sorted(sample.generators, key=gen_sortkey)

    # Check black-list and purple-list
    blError = checkBlackList(athenaRel,generatorNames, "black")
    plError = checkBlackList(athenaRel,generatorNames, "purple")
    if blError is not None:
        raise RuntimeError(blError)
    if plError is not None:
        evgenLog.warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        evgenLog.warning(f"!!! WARNING {plError} !!!")
        evgenLog.warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")

    # Setup the main flags
    flags.Exec.FirstEvent = runArgs.firstEvent
    # Max events should be not set, job stopping is handled by CountHepMC
    # using the RequestedOutput property
    flags.Exec.MaxEvents = -1

    if hasattr(runArgs, "inputEVNT_PreFile"):
        flags.Input.Files = runArgs.inputEVNT_PreFile
    else:
        flags.Input.Files = []
        flags.Input.RunNumbers = [flags.Generator.DSID]
        flags.Input.TimeStamps = [0]

    flags.Output.EVNTFileName = runArgs.outputEVNTFile

    flags.Beam.Energy = runArgs.ecmEnergy / 2 * GeV

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
    evgenLog.info(".transform =                  Gen_tf")
    evgenLog.info(".platform = " + str(os.environ["BINARY_TAG"]))

    # Announce start of job configuration
    evgenLog.info("**** Configuring event generation")

    # Main object
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
    cfg = MainEvgenServicesCfg(flags, withSequences=True)

    # Input file handling (if needed)
    if flags.Input.Files:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    # EventInfoCnvAlg
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True, xAODKey="TMPEvtInfo"),
                                 sequenceName=EvgenSequence.Generator.value)

    # Set up the process
    cfg.merge(sample.setupProcess(flags))

    # Fix non-standard event features
    if not flags.Input.Files:
        from EvgenProdTools.EvgenProdToolsConfig import FixHepMCCfg
        from GeneratorConfig.GenConfigHelpers import gens_purgenoendvtx
        generatorsList = sample.generators
        if "Pythia8" in generatorsList:
            pythia8Alg = cfg.getEventAlgo("Pythia8_i")
            if pythia8Alg.Beam1 != "PROTON" or pythia8Alg.Beam2 != "PROTON":
            # generator name is still "Pythia8", even when colliding nuclei
                generatorsList.append("Pythia8-Angantyr")
        cfg.merge(FixHepMCCfg(flags,
                              PurgeUnstableWithoutEndVtx=gens_purgenoendvtx(generatorsList)))

    ## Sanity check the event record (not appropriate for all generators)
    from GeneratorConfig.GenConfigHelpers import gens_testhepmc
    if gens_testhepmc(sample.generators):
        from EvgenProdTools.EvgenProdToolsConfig import TestHepMCCfg
        cfg.merge(TestHepMCCfg(flags))

    # Copy the event weight from HepMC to the Athena EventInfo class
    from EvgenProdTools.EvgenProdToolsConfig import CopyEventWeightCfg
    cfg.merge(CopyEventWeightCfg(flags))

    from EvgenProdTools.EvgenProdToolsConfig import FillFilterValuesCfg
    cfg.merge(FillFilterValuesCfg(flags))

    # Configure the event counting (AFTER all filters)
    from EvgenProdTools.EvgenProdToolsConfig import CountHepMCCfg
    cfg.merge(CountHepMCCfg(flags,
                            RequestedOutput=sample.nEventsPerJob if runArgs.maxEvents == -1
                                            else runArgs.maxEvents))
    evgenLog.info("Requested output events = %d", cfg.getEventAlgo("CountHepMC").RequestedOutput)

    # Print out the contents of the first 5 events (after filtering)
    if hasattr(runArgs, "printEvts") and runArgs.printEvts > 0:
        from TruthIO.TruthIOConfig import PrintMCCfg
        cfg.merge(PrintMCCfg(flags,
                             LastEvent=runArgs.printEvts))

    # PerfMon
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    cfg.merge(PerfMonMTSvcCfg(flags), sequenceName=EvgenSequence.Post.value)

    # Estimate time needed for Simulation
    from EvgenProdTools.EvgenProdToolsConfig import SimTimeEstimateCfg
    cfg.merge(SimTimeEstimateCfg(flags))

    # TODO: Rivet

    # Include information about generators in metadata
    from GeneratorConfig.Versioning import generatorsGetInitialVersionedDictionary, generatorsVersionedStringList
    generatorDictionary = generatorsGetInitialVersionedDictionary(generatorNames)
    generatorList = generatorsVersionedStringList(generatorDictionary)

    # Extra metadata
    # TODO: to be optimised
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    metadata = {
        "project_name": "IS_SIMULATION",
        f"AtlasRelease_{runArgs.trfSubstepName}": flags.Input.Release or "n/a",
        "beam_energy": str(int(flags.Beam.Energy)),
        "beam_type": flags.Beam.Type.value,
        "generators": '+'.join(generatorList),
        "hepmc_version": f"HepMC{os.environ['HEPMCVER']}",
        "keywords": ", ".join(sample.keywords).lower(),
        "lhefGenerator": '+'.join(filter(gen_lhef, generatorNames)),
        "mc_channel_number": str(flags.Generator.DSID),
    }
    if hasattr(sample, "process"): metadata.update({"evgenProcess": sample.process})
    if hasattr(sample, "tune"): metadata.update({"evgenTune": sample.tune})
    if hasattr(sample, "specialConfig"): metadata.update({"specialConfiguration": sample.specialConfig})
    if hasattr(sample, "hardPDF"): metadata.update({"hardPDF": sample.hardPDF})
    if hasattr(sample, "softPDF"): metadata.update({"softPDF": sample.softPDF})
    if hasattr(sample, "randomSeed"): metadata.update({"randomSeed": str(runArgs.randomSeed)})
    cfg.merge(TagInfoMgrCfg(flags, tagValuePairs=metadata))

    # Print version of HepMC to the log
    evgenLog.info("HepMC version %s", os.environ["HEPMCVER"])

    # Configure output stream
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, "EVNT", ["McEventCollection#*"]))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, "EVNT"))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file MetaData
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    # Print ComponentAccumulator components
    cfg.printConfig(prefix="Gen_tf", printSequenceTreeOnly=not runArgs.VERBOSE)

    # Run final ComponentAccumulator
    sys.exit(not cfg.run().isSuccess())
