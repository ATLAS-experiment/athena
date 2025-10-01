# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Instantiate the things needed upstream the main egamma Reconstruction,
    EGamma Reconstruction,
    Output Item Lists,
    xAOD related Thinning
"""

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def EGammaSteeringCfg(flags,
                      name="EGammaSteering"):

    mlog = logging.getLogger(name)
    mlog.info('Starting EGamma Steering')

    acc = ComponentAccumulator()

    # e/gamma main Reconstruction
    from egammaConfig.egammaReconstructionConfig import (
        egammaReconstructionCfg)
    acc.merge(egammaReconstructionCfg(flags))

    # Add e/gamma related containers to the output stream
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        from egammaConfig.egammaOutputConfig import (
            egammaOutputCfg)
        acc.merge(egammaOutputCfg(flags))

    # LRT Reconstruction
    if flags.Tracking.doLargeD0:
        from egammaConfig.egammaLRTReconstructionConfig import (
            egammaLRTReconstructionCfg)
        acc.merge(egammaLRTReconstructionCfg(flags))

        # LRT output
        if flags.Output.doWriteESD or flags.Output.doWriteAOD:
            from egammaConfig.egammaLRTOutputConfig import (
                egammaLRTOutputCfg)
            acc.merge(egammaLRTOutputCfg(flags))

    # Add e/gamma xAOD thinning
    if flags.Output.doWriteAOD:
        from egammaConfig.egammaxAODThinningConfig import (
            egammaxAODThinningCfg)
        acc.merge(egammaxAODThinningCfg(flags))

    mlog.info("EGamma Steering done")
    return acc

# Run with python -m egammaConfig.egammaSteeringConfig
def egammaSteeringConfigTest(flags=None):

    if flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.addFlag('egamma.configOnly', False, help='custom option for egammaSteeringConfig to not run, only output config file')

        from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
        flags.Input.Files = defaultTestFiles.RDO_RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
        flags.Exec.MaxEvents = 1

        flags.Output.doWriteESD = True  # To test the ESD parts
        flags.Output.doWriteAOD = True  # To test the AOD parts
        flags.Output.ESDFileName = "myESD.pool.root"
        flags.Output.AODFileName = "myAOD.pool.root"
        
        from egammaConfig.ConfigurationHelpers import egammaOnlyFromRaw
        egammaOnlyFromRaw(flags)
        flags.fillFromArgs()

        flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    # Special message service configuration
    from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg
    acc.merge(DigitizationMessageSvcCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    with open("egammasteeringconfig.pkl", "wb") as f:
        acc.store(f)

    if hasattr(flags, 'egamma') and flags.egamma.configOnly:
        return None  # returns statusCode of None
    else:
        statusCode = acc.run()
        return statusCode

if __name__ == "__main__":
    egammaSteeringConfigTest()
