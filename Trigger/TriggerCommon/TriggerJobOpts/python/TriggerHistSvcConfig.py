#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerHistSvcConfig')

def setTHistSvcOutput(outputList):
    """Build the Output list of the THistSvc ensuring minimum set of output files"""

    if not any('EXPERT' in o for o in outputList):
        outputList += ["EXPERT DATAFILE='expert-monitoring.root' OPT='RECREATE'"]

    return


def TriggerHistSvcConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    acc = ComponentAccumulator()

    if flags.Trigger.Online.useOnlineTHistSvc:
        log.info("Configuring online TrigMonTHistSvc")
        histSvc = CompFactory.TrigMonTHistSvc("THistSvc")  # no outputs in online
    elif flags.Trigger.Online.useOnlineWebdaqHistSvc:
        log.info("Configuring online Webdaq HistSvc")
        histSvc = CompFactory.WebdaqHistSvc("THistSvc") 
    else:
        log.info("Configuring offline THistSvc")
        output = []
        if flags.Trigger.L1MuonSim.WriteNSWDebugNtuple:
            output += ["NSWL1Simulation DATAFILE='NSWL1Simulation.root' OPT='RECREATE'"]

        # add default outputs
        setTHistSvcOutput(output)
        histSvc = CompFactory.THistSvc("THistSvc", Output = output)

    acc.addService( histSvc )
    return acc
