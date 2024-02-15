#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
"""Functionality core of the Gen_tf transform"""

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


# Main function
def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    evgenLog = logging.getLogger('Gen_tf')
    evgenLog.setLevel(logging.DEBUG if runArgs.VERBOSE else logging.INFO)    
    
    # Set up sequences
        
    # Announce arg checking
    evgenLog.debug("****************** CHECKING EVENT GENERATION ARGS *****************")
    evgenLog.debug(runArgs)

    evgenLog.debug('****************** Setting-up configuration flags *****************')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    # Convert run arguments to athena flags
    from GeneratorConfig.GeneratorConfigFlags import  generatorRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)
    generatorRunArgsToFlags(runArgs, flags)
    
    # Lock flags
    flags.lock()
        
    # Announce start of job configuration
    evgenLog.debug("****************** CONFIGURING EVENT GENERATION *****************")
    
    evgenLog.info("Hello from GENtoEVGEN_Skeleton")
