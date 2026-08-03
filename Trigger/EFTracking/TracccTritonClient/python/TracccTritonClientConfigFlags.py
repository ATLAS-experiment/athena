#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createTracccTritonConfigFlags():
    """Flags configuring the Traccc Triton client: the model name and the
    Triton server connection (url/port)."""
    icf = AthConfigFlags()
    icf.addFlag("Tracking.Traccc.Triton.url", "localhost")
    icf.addFlag("Tracking.Traccc.Triton.model", "traccc-gpu")
    icf.addFlag("Tracking.Traccc.Triton.port", 8001)
    return icf


def tracccTritonFlagsPreInclude(flags):
    """preInclude that registers the Traccc Triton flags on the job's flags.

    Kept in this package (rather than centrally in TrkConfigFlags) so the
    package is self-contained. preIncludes run before preExec, so
    flags.Tracking.Traccc.Triton.* can be overridden on the command line."""
    flags.join(createTracccTritonConfigFlags())
