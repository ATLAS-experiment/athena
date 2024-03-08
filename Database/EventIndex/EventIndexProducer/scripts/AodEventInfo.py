#!/usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@file AodEventInfo.py
@brief A basic script to output run number, event number of events in the input AOD file(s)
'''

from AthenaPython.PyAthena import Alg, py_svc, StatusCode

# A basic filtering algorithm
class PyxAODEventInfo(Alg):

    # Constructor
    def __init__(self, name = 'PyxAODEventInfo', **kwds):
        kwds['name'] = name
        super().__init__(**kwds)
        self.isMC = kwds.get('isMC', False)
        self.prefix = kwds.get('prefix')

    # Initialize the algorithm
    def initialize(self):
        self.sg = py_svc('StoreGateSvc')
        from collections import defaultdict
        self.info = defaultdict(list)
        return StatusCode.Success

    # Finalize the algorithm
    def finalize(self):
        if not self.isMC:
            key_name = 'run_number'
        else:
            key_name = 'mc_channel_number'
        for run, event in zip(self.info[key_name], self.info['event_number']): #, strict=True): Changed in version 3.10: Added the strict argument.
            print(f"{'' if self.prefix is None else self.prefix}{run:d} {event:d}")
        return StatusCode.Success

    # Execute the algorithm
    def execute(self):

        # Read the run/event number from xAOD::EventInfo
        if self.sg.contains('xAOD::EventInfo', 'EventInfo'):
            ei = self.sg.retrieve('xAOD::EventInfo', 'EventInfo')
            runNumber = ei.runNumber()
            eventNumber = ei.eventNumber()
            mcChannelNumber = ei.mcChannelNumber()

            self.info['run_number'].append(runNumber)
            self.info['mc_channel_number'].append(mcChannelNumber)
            self.info['event_number'].append(eventNumber)

            # Let's happily move to the next event
            return StatusCode.Success

        # If we made it thus far something went wrong
        return StatusCode.Failure

# Main executable
if '__main__' in __name__:

    # Parse user input
    import argparse
    parser = argparse.ArgumentParser(description='Output run number (mc channel number in case of Monte Carlo),'\
                                     ' event number of events in the input AOD file(s)')
    parser.add_argument('--inputAODFiles', required=True,
                        help='Input AOD file(s) separated with commas')
    parser.add_argument('--prefix',
                        help='Prefix to print in front of each line of output')
    args = parser.parse_args()

    # Setup configuration logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('AodEventInfo')
    log.info('== Listing EventInfo for events from AOD files (the CA Configuration)')

    # Set the configuration flags
    log.info('== Setting ConfigFlags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = args.inputAODFiles.split(',')

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
    cfg.addEventAlgo(PyxAODEventInfo('xAodEventInfoAlg', isMC = flags.Input.isMC, prefix = args.prefix),
                     sequenceName = 'AthAlgSeq')

    # For (un)packing Cell Containers
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    # For being able to read pre Run-3 data w/ Trk objects
    from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
    cfg.merge(TrkEventCnvSuperToolCfg(flags))

    # Now run the job
    log.info('== Running...')
    sc = cfg.run()

    # Exit accordingly
    import sys
    sys.exit(not sc.isSuccess())
