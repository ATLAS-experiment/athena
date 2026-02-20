#!/usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@brief A basic script to output run number, event number of events in POOL/BS file(s)
@author ATLAS Computing Activity
@file EventInfo.py
'''

from AthenaPython.PyAthena import Alg, py_svc, StatusCode

class PyEventInfo(Alg):

    # Constructor
    def __init__(self, name='PyEventInfo', **kwargs):
        kwargs['name'] = name
        super().__init__(**kwargs)
        # location of EventInfo
        self.evt_info = kwargs.get('evt_info', None)
        self.is_mc  = kwargs.get('is_mc', False) # default value
        self.output = kwargs.get('output')
        self.prefix = kwargs.get('prefix', '')

    # Initialize the algorithm
    def initialize(self):
        _info  = self.msg.info
        _error = self.msg.error
        _info('==> initialize...')

        _info ('EventInfo name:  %s',
               self.evt_info
               if self.evt_info else '<any>')

        self.sg = py_svc('StoreGateSvc')
        # if self.sg is None:
        if not self.sg:
            _error ('could not retrieve event store')
            return StatusCode.Failure
        _info (f'retrieved {self.sg=} service')

        from collections import defaultdict
        self.info = defaultdict(list)
        return StatusCode.Success

    # Execute the algorithm
    def execute(self):
        _info = self.msg.info
        _error= self.msg.error

        for evtinfocls in ('xAOD::EventInfo', 'EventInfo'):
            try:
                 evtinfo = self.sg.retrieve (evtinfocls, self.evt_info)
            except Exception as e:
                _info ('could not retrieve %r at [%s]\n    caught exception:\n%r', evtinfocls, self.evt_info, e)
            else:
                if evtinfo is None:
                    _info ('retrieved \'None\' for %r at [%s]', evtinfocls, self.evt_info)
                    continue
                break
        else:
            _error ("could not retrieve 'EventInfo' or 'xAOD::EventInfo' at [%s]", self.evt_info)
            return StatusCode.Failure

        if evtinfocls == 'EventInfo':
            evtid = evtinfo.event_ID()
            runnbr = evtid.run_number()
            evtnbr = evtid.event_number()
        elif evtinfocls == 'xAOD::EventInfo':
            if not self.is_mc:
                runnbr = evtinfo.runNumber()
            else:
                runnbr = evtinfo.mcChannelNumber()
            evtnbr = evtinfo.eventNumber()

        self.info['run_number'].append(runnbr)
        self.info['event_number'].append(evtnbr)

        return StatusCode.Success

    # Finalize the algorithm
    def finalize(self):
        for run, event in zip(self.info['run_number'], self.info['event_number'], strict=True):
            # Changed in version 3.10: Added the strict argument.
            print(f"{self.prefix}{run:d} {event:d}",
                  file=self.output)
        return StatusCode.Success

def main(args=None):
    # Parse user input
    import sys
    import argparse
    parser = argparse.ArgumentParser(description='Output run number (mc channel number in case of Monte Carlo),'
                                     ' event number of events in input (POOL/BS) file(s).',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--inputFiles', required=True,
                        help='input (POOL/BS) file(s) separated with commas')
    parser.add_argument('--outputFile', default='-',
                        help='output text file containing <run number> <event number> record(s) (one per line);'
                        ' if is -, write output on standard output')
    parser.add_argument('--prefix', default='',
                        help='prefix to print in front of each line of output')
    args = parser.parse_args(args=args)

    if args.outputFile == "-":
        output = sys.stdout
        opened = False
    else:
        output = open(args.outputFile, 'w', encoding="utf-8")
        opened = True

    # Setup configuration logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('EventInfo')

    from os.path import expandvars, expanduser
    args.inputFiles = [expandvars(expanduser(fn)) for fn in args.inputFiles.split(',')]

    log.info(f"input files: {",".join(repr(fn) for fn in args.inputFiles)}")
    log.info(f"output file: {args.outputFile!r}")
    log.info(f"prefix: {args.prefix!r}")

    from PyUtils.MetaReader import read_metadata
    logging.getLogger('MetaReader').setLevel(logging.WARNING)
    metadata = read_metadata(args.inputFiles[0], mode='tiny')[args.inputFiles[0]]

    if metadata['file_type'] == 'BS':
        # run directly AtlListBSEvents
        import subprocess
        cmd = ["AtlListBSEvents", "-l"]
        cmd.extend(args.inputFiles)
        try:
            log.info("running")
            log.debug(f"... {cmd=}")
            log.info("   ...")
            proc = subprocess.run(cmd, capture_output=True, text=True, check=True)
        except OSError as err:
            log.error(err)
            return err.errno
        except subprocess.CalledProcessError as err:
            log.error(f"{err}\nstderr={err.stderr!r}\nstdout={err.stdout!r}")
            return err.returncode
        else:
            from collections import defaultdict
            info = defaultdict(list)

            indexprefix = "Index="
            runprefix = "Run="
            runoff = len(runprefix)
            eventprefix = "Event="
            eventoff = len(eventprefix)
            for line in proc.stdout.splitlines():
                match line.split():
                    case [Index, Run, Event, _, _, *_] if Index.startswith(indexprefix):
                        if Run.startswith(runprefix) and Event.startswith(eventprefix):
                            try:
                                run = int(Run[runoff:])
                                event = int(Event[eventoff:])
                            except ValueError:
                                log.warning(f"could not parse {line=}")
                            else:
                                log.debug(f"parsed {run=} {event=}")
                                info['run_number'].append(run)
                                info['event_number'].append(event)
                        else:
                            log.warning(f"could not parse {line=}")

            for run, event in zip(info['run_number'], info['event_number'], strict=True):
                # Changed in version 3.10: Added the strict argument.
                print(f"{args.prefix}{run:d} {event:d}",
                      file=output)
        finally:
            if opened: output.close()

        return 0

    log.info('== Listing EventInfo for events from POOL files (the CA Configuration)')

    # Set the configuration flags
    log.info('== Setting ConfigFlags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = args.inputFiles

    import AthenaCommon.Constants as Lvl
    flags.Exec.OutputLevel=Lvl.WARNING

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

    # add event listing algorithm
    cfg.addEventAlgo(PyEventInfo('EventInfoAlg',
                                 # the store-gate key
                                 evt_info='EventInfo',
                                 is_mc=flags.Input.isMC,
                                 output=output,
                                 prefix=args.prefix,
                                 OutputLevel=Lvl.WARNING),
                     sequenceName='AthAlgSeq')

    for item in flags.Input.TypedCollections:
        ctype, cname = item.split('#')
        if ctype.startswith(('Trk', 'InDet')):
            from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
            cfg.merge(TrkEventCnvSuperToolCfg(flags))
        if ctype.startswith(('Calo', 'LAr')):
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            cfg.merge(LArGMCfg(flags))
        if ctype.startswith(('Calo', 'Tile')):
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
    return sc.isFailure()

# Main executable
if __name__ == "__main__":
    import sys
    sys.exit(main())
