#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
"""Functionality core of the Gen_tf transform"""

# temporarily force no global config flags
from AthenaConfiguration import AllConfigFlags
del AllConfigFlags.ConfigFlags

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True

# Get logger
from AthenaCommon.Logging import logging
evgenLog = logging.getLogger('Gen_tf')
    
# Functions for pre/post-include/exec
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude

# For creating CA instances of algorithms
from AthenaConfiguration.ComponentFactory import CompFactory

# Other imports that are needed
import sys, os


# Function that reads the jO and returns an instance of Sample(EvgenCAConfig)
def setupSample(runArgs, flags):
    # Only permit one jobConfig argument for evgen
    if len(runArgs.jobConfig) != 1:
        evgenLog.info("runArgs.jobConfig = %s", runArgs.jobConfig)
        evgenLog.error("You must supply one and only one jobConfig file argument")
        sys.exit(1)
    
    evgenLog.info("Using JOBOPTSEARCHPATH (as seen in skeleton) = {}".format(os.environ["JOBOPTSEARCHPATH"]))
    
    FIRST_DIR = (os.environ['JOBOPTSEARCHPATH']).split(":")[0]
    
    # Find jO file
    jofiles = [f for f in os.listdir(FIRST_DIR) if (f.startswith('mc') and f.endswith('.py'))]
    if len(jofiles) !=1:
        evgenLog.error("You must supply one and only one jobOption file in DSID directory")
        sys.exit(1)
    jofile = jofiles[0]
    
    # Perform consistency checks on the jO
    from GeneratorConfig.GenConfigHelpers import checkJOConsistency
    checkJOConsistency(jofile)
    
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
    
    # Create instance of Sample(EvgenCAConfig)
    sample = jo.Sample(flags)
    
    # Set up the sample properties
    sample.setupFlags(flags)
            
    # Set the random number seed
    # Need to use logic in EvgenJobTransforms.Generate_dsid_ranseed 
    
    # Get DSID
    dsid = os.path.basename(runArgs.jobConfig[0])
        
    # Update the global flags
    if dsid.isdigit(): 
        flags.Generator.DSID
    flags.Input.Files = []
    flags.Input.RunNumbers = [flags.Generator.DSID]
    flags.Input.TimeStamps = [0]
    flags.Generator.nEventsPerJob = sample.nEventsPerJob
    flags.Exec.MaxEvents = runArgs.maxEvents if runArgs.maxEvents != -1 else sample.nEventsPerJob
    flags.Output.EVNTFileName = runArgs.outputEVNTFile

    # Check if sample attributes have been properly set
    sample.checkAttributes()

    return sample


# Function to set up event generation services
def setupEvgenServices(flags, cfg):   
    
    # EventInfoCnvAlg
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True, xAODKey="TMPEvtInfo"))


# Function to set up event generation services
def setupOutputStream(flags, runArgs, cfg, sample):   
    
    # Items to save
    ItemList = ["McEventCollection#*"]
    
    # Configure output stream
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(
        OutputStreamCfg(
            flags, 
            "EVNT", 
            ItemList = ItemList
        )
    )
        
    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, "EVNT"))  
    
    # Write AMI tag into in-file MetaData
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))    


# Main function
def fromRunArgs(runArgs):
    # Set logging level
    evgenLog.setLevel(logging.DEBUG if runArgs.VERBOSE else logging.INFO)
    
    # Announce arg checking
    if runArgs.VERBOSE:
        evgenLog.debug("****************** CHECKING EVENT GENERATION ARGS *****************")
        evgenLog.debug(runArgs)
        evgenLog.debug('****************** Setting-up configuration flags *****************')
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
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
        
    # Process pre-include
    processPreInclude(runArgs, flags)

    # Process pre-exec
    processPreExec(runArgs, flags)

    # Lock flags
    flags.lock()
    
    if runArgs.VERBOSE:
        evgenLog.debug('****************** Dump configuration flags *****************')
        evgenLog.debug(flags.dump())
        
        # Announce start of job configuration
        evgenLog.debug("****************** CONFIGURING EVENT GENERATION *****************")
    
    # Main object
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
    cfg = MainEvgenServicesCfg(flags)  
    
    # Set up evgen job with all sequences
    setupEvgenServices(flags, cfg)
    
    # Set up the process
    AthSequencer = CompFactory.AthSequencer
    evgenGenSeq = AthSequencer('EvgenGenSeq')
    cfg.addSequence(evgenGenSeq, parentName='AthAllAlgSeq')
    cfg.merge(sample.setupProcess(flags), sequenceName="EvgenGenSeq")
    
    # Set up the output stream
    setupOutputStream(flags, runArgs, cfg, sample)
    
    # Print ComponentAccumulator components
    if runArgs.VERBOSE:
        evgenLog.debug("****************** DUMPING CA CONFIG *****************")
        evgenLog.debug(cfg.printConfig())

    # Run final ComponentAccumulator
    sys.exit(not cfg.run().isSuccess())
