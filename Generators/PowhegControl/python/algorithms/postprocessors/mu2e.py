# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ... import Logging
from ...decorators import timed
from ...utility import LHE
import shutil

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


@timed("mu2e")
def mu2e(powheg_LHE_output):
    """! Post-process existing events from muons to electrons

    @param powheg_LHE_output  Name of LHE file produced by PowhegBox.

    @author Jan Kretzschmar <jan.kretzschmar@cern.ch>
    """
    logger.warning("Converting LHE events from muon to electron decays.")

    # Get opening and closing strings
    preamble = LHE.preamble(powheg_LHE_output)
    postamble = LHE.postamble(powheg_LHE_output)
    
    n_events = 0
    powheg_LHE_e = "{}.e".format(powheg_LHE_output)
    with open(powheg_LHE_e, "w") as f_output:
        f_output.write("{}\n".format(preamble))
        for input_event in LHE.event_iterator(powheg_LHE_output):
            is_event_changed, output_event = LHE.mu2e(input_event)
            f_output.write(output_event)
            n_events += [0, 1][is_event_changed]
        f_output.write(postamble)
    logger.info("Changed mu->e in {} events!".format(n_events))

    # Make a backup of the original events
    shutil.move(powheg_LHE_output, "{}.mu2e_backup".format(powheg_LHE_output))
    shutil.move(powheg_LHE_e, powheg_LHE_output)
