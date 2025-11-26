#!/usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@brief A basic script to output run number, event number of events in POOL file(s)
@author ATLAS Computing Activity
@file EventInfo.py
'''

from AthenaPython.PyAthena import Alg, py_svc, StatusCode

# A basic filtering algorithm
class PyEventInfo(Alg):

    # Constructor
    def __init__(self, name='PyEventInfo', **kwargs):
        kwargs['name'] = name
        super().__init__(**kwargs)
        self.isMC = kwargs.get('isMC', False)
        self.output = kwargs.get('output')
        self.prefix = kwargs.get('prefix')

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
        for run, event in zip(self.info[key_name], self.info['event_number'], strict=True):
            # Changed in version 3.10: Added the strict argument.
            print(f"{'' if self.prefix is None else self.prefix}{run:d} {event:d}",
                  file=self.output)
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
if __name__ == "__main__":

    # Parse user input
    import argparse
    parser = argparse.ArgumentParser(description='Output run number (mc channel number in case of Monte Carlo),'
                                     ' event number of events in POOL file(s)')
    parser.add_argument('--inputFiles', required=True,
                        help='Input POOL file(s) separated with commas')
    parser.add_argument('--outputFile',
                        help='Output text file containing <run number> <event number> record(s) (one per line);'
                        ' if is -, write output on standard output (default: -)')
    parser.add_argument('--prefix',
                        help='Prefix to print in front of each line of output')
    args = parser.parse_args()

    import sys
    if args.outputFile is None or args.outputFile == "-":
        output = sys.stdout
        opened = False
    else:
        output = open(args.outputFile, 'w', encoding="utf-8")
        opened = True

    # Setup configuration logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('EventInfo')
    log.setLevel(logging.ERROR)

    log.info('== Listing EventInfo for events from POOL files (the CA Configuration)')

    # Set the configuration flags
    log.info('== Setting ConfigFlags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = args.inputFiles.split(',')

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

    # Setup event listing
    cfg.addEventAlgo(PyEventInfo('EventInfoAlg', isMC=flags.Input.isMC, output=output,
                                 prefix=args.prefix),
                     sequenceName='AthAlgSeq')

    for item in flags.Input.TypedCollections:
        ctype, cname = item.split('#')
        if ctype.startswith('Trk') or ctype.startswith('InDet'):
            from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
            cfg.merge(TrkEventCnvSuperToolCfg(flags))
        if ctype.startswith('Calo') or ctype.startswith('LAr'):
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            cfg.merge(LArGMCfg(flags))
        if ctype.startswith('Calo') or ctype.startswith('Tile'):
            from TileGeoModel.TileGMConfig import TileGMCfg
            cfg.merge(TileGMCfg(flags))
        if ctype.startswith('Muon'):
            from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
            cfg.merge(MuonGeoModelCfg(flags))

    # Now run the job
    log.info('== Running...')
    sc = cfg.run()

    if opened: output.close()

    # Exit accordingly
    sys.exit(not sc.isSuccess())
