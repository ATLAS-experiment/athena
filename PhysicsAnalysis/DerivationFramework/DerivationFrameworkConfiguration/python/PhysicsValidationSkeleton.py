# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logDerivation = logging.getLogger('PhysicsValidation')
    logDerivation.info('****************** STARTING NTUP_PHYSVAL *****************')

    logDerivation.info('**** Transformation run arguments')
    logDerivation.info(str(runArgs))

    logDerivation.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.EventPrintoutInterval = 100
    commonRunArgsToFlags(runArgs, flags)

    # Switch on PerfMon
    flags.PerfMon.doFullMonMT = True

    # Input types
    if not hasattr(runArgs, 'inputDAOD_PHYSVALFile'):
        raise ValueError('Input must be provided using --inputDAOD_PHYSVALFile')
    flags.Input.Files = runArgs.inputDAOD_PHYSVALFile

    # Physics Validation
    if hasattr(runArgs, 'outputNTUP_PHYSVALFile'):
        logDerivation.info('Will produce NTUP_PHYSVAL file')
        flags.PhysVal.OutputFileName = runArgs.outputNTUP_PHYSVALFile

        if not (hasattr(runArgs, 'inputAODFile') or hasattr(runArgs, 'inputDAOD_PHYSVALFile')):
            logDerivation.error('NTUP_PHYSVAL requires AOD or DAOD_PHYSVAL input')
            raise ValueError('Incorrect inputs for NTUP_PHYSVAL')

        if not hasattr(runArgs, 'validationFlags'):
            raise ValueError('--validationFlags is mandatory')

        for flag in runArgs.validationFlags:
            if flag == 'doPFlow_FlowElements':  # backwards compatibility
                flag = 'doPFlow'

            name = f'PhysVal.{flag}'
            if not flags.hasFlag(name):
                raise ValueError(f"Unknown validation flag '{name}'")

            logDerivation.info("Enabling validation flag '%s'", name)
            flags._set(name, True)
    else:
        raise ValueError('Output file name needs to be set using --outputNTUP_PHYSVALFile')

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    # The main services configuration
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    # Run NTUP_PHYSVAL making
    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    cfg.merge(PhysValMonitoringCfg(flags))

    # Enabling PerfMon always for physics validation jobs regardless of PerfMon flags (unlike most other skeletons)
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    cfg.merge(PerfMonMTSvcCfg(flags))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
