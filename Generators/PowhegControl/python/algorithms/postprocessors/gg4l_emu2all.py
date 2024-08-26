# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ...decorators import timed
from ...utility import LHE
import shutil

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


@timed("gg4l_emu2all")
def gg4l_emu2all(powheg_LHE_output):
    """! Post-process existing events from electrons to muons

    @param powheg_LHE_output  Name of LHE file produced by PowhegBox.

    @author Guglielmo Frattari <guglielmo.frattari@cern.ch>
    """
    logger.warning("Converting LHE events from 2e2mu to 2l2l' or 4l final states.")

    # Get opening and closing strings
    preamble = LHE.preamble(powheg_LHE_output)
    postamble = LHE.postamble(powheg_LHE_output)
    
    n_events = 0
    powheg_LHE_2e2mu = "{}.mod".format(powheg_LHE_output)
    with open(powheg_LHE_2e2mu, "w") as f_output:
        f_output.write("{}\n".format(preamble))
        for input_event in LHE.event_iterator(powheg_LHE_output):
            is_event_changed, output_event = LHE.gg4l_emu2all(input_event)
            f_output.write(output_event)
            n_events += [0, 1][is_event_changed]
        f_output.write(postamble)
    logger.info("Changed 2e2mu final state to 2l2'/4l in {} events!".format(n_events))

    # Make a backup of the original events
    shutil.move(powheg_LHE_output, "{}.mod_backup".format(powheg_LHE_output))
    shutil.move(powheg_LHE_2e2mu, powheg_LHE_output)
