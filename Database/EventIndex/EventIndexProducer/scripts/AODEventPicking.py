#!/usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@file AODEventPicking.py
@author Alaettin Serhan Mete
@brief A basic script for picking events from AOD files
'''

from AthenaPython.PyAthena import Alg, py_svc, StatusCode

# A basic filtering algorithm
class PyxAODEvtFilter(Alg):

    # Constructor
    def __init__(self, name = 'PyxAODEvtFilter', **kw):
        kw['name'] = name
        super(PyxAODEvtFilter, self).__init__(**kw)
        self.evtList = kw.get('evtList',   None)
        self.isMC = kw.get('isMC', False)
        return

    # Initialize the algorithm
    def initialize(self):
        self.sg = py_svc('StoreGateSvc')
        return StatusCode.Success

    # Execute the algorithm
    def execute(self):

        # Read the run/event number from xAOD::EventInfo
        if self.sg.contains('xAOD::EventInfo', 'EventInfo'):
            ei = self.sg.retrieve('xAOD::EventInfo', 'EventInfo')
            if not self.isMC:
                runNumber = ei.runNumber()
            else:
                runNumber = ei.mcChannelNumber()
            eventNumber = ei.eventNumber()

            # Check to see if we should accept or reject the event
            if (runNumber, eventNumber) in self.evtList:
                self.setFilterPassed(True)
            else:
                self.setFilterPassed(False)

            # Let's happily move to the next event
            return StatusCode.Success

        # If we made it thus far something went wrong
        return StatusCode.Failure

# Main executable
if '__main__' in __name__:

    # Parse user input
    import argparse
    parser = argparse.ArgumentParser(description='Select events identified by run number (mc channel number'\
                                     ' in case of Monte Carlo), event number from the input AOD file(s)')
    parser.add_argument('--inputAODFiles', required=True,
                        help='Input AOD file(s) separated with commas')
    parser.add_argument('--outputAODFile', required=True,
                        help='Output AOD file')
    parser.add_argument('--eventList', required=True,
                        help='Text file containing <run number> <event number> [<guid>] record(s) (one per line)')
    args, _ = parser.parse_known_args()

    # Setup configuration logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('AODEventPicking')
    log.info('== Picking events from AOD files w/ the CA Configuration')

    # Parse input file that contains run_number, event_number[, guid] combinations
    evtList = []
    with open(args.eventList) as f:
        for line in f:
            run, evt, *guid = line.rstrip().split()
            evtList.append((int(run), int(evt)))

    # Set the configuration flags
    log.info('== Setting ConfigFlags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = args.inputAODFiles.split(',')
    flags.Output.AODFileName = args.outputAODFile
    flags.PerfMon.doFastMonMT = True
    flags.PerfMon.OutputJSON = 'perfmonmt_AODEventPicking.json'

    # Lock and dump the configuration flags
    flags.lock()
    log.info('== ConfigFlags Locked')

    # Setup the main services
    log.info('== Configuring Main Services')
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Setup the input reading
    log.info('== Configuring Input Reading')
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Setup event picking
    cfg.addEventAlgo(PyxAODEvtFilter('EventFilterAlg', evtList = evtList, isMC = flags.Input.isMC), sequenceName = 'AthAlgSeq')

    # Configure the output stream
    log.info('== Configuring Output Stream')
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    cfg.merge(OutputStreamCfg(flags, 'AOD', takeItemsFromInput=True))

    # Configure metadata
    log.info('== Configuring metadata for the output stream')
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, 'AOD'))

    # Setup the output stream algorithm
    StreamAOD = cfg.getEventAlgo(outputStreamName("AOD"))
    StreamAOD.ForceRead = True
    StreamAOD.AcceptAlgs += ['EventFilterAlg']

    # For (un)packing Cell Containers
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    # For being able to read pre Run-3 data w/ Trk objects
    from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
    cfg.merge(TrkEventCnvSuperToolCfg(flags))

    # Setup PerfMon
    log.info('== Configuring PerfMon')
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    cfg.merge(PerfMonMTSvcCfg(flags))

    # Now run the job
    log.info('== Running...')
    sc = cfg.run()

    # Exit accordingly
    import sys
    sys.exit(not sc.isSuccess())
