# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode
from JetRecConfig.StandardJetContext import createJetContextFlags


def createJetConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag("Jet.doUpstreamDependencies", False)
    flags.addFlag("Jet.WriteToAOD", 
                  lambda prevFlags: prevFlags.Reco.HIMode in [HIMode.HI,HIMode.UPC,HIMode.HIP] )
    flags.addFlag("Jet.useCalibJetThreshold", True)

    flags.addFlagsCategory("Jet.Context",createJetContextFlags)

    # strictMode will cause jobs to crash if inputs are missing. This
    # is disabled by default in production code because inputs
    # sometimes go missing for good reasons in production.
    flags.addFlag("Jet.strictMode", _useStrictMode)
    return flags


# specific flag defaults
#
def _useStrictMode(flags):
    """Strict mode is disabled by default in production jobs, but
    enabled otherwise
    """
    return not any([
        flags.Output.doWriteESD,
        flags.Output.doWriteAOD,
        flags.Output.doWriteDAOD,
        flags.Output.doWriteEVNT,
        flags.Output.doWriteEVNT_TR,
        flags.Output.doWriteBS,
        flags.Output.doWriteRDO,
        flags.Output.doWriteRDO_SGNL,
        flags.Output.HISTFileName,
    ])
