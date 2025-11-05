#!/usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@file EventPicking.py
@author Alaettin Serhan Mete
@brief A basic script for picking events from POOL files
'''

from AthenaPython.PyAthena import Alg, py_svc, StatusCode

# A basic filtering algorithm
class PyEvtFilter(Alg):

    # Constructor
    def __init__(self, name='PyEvtFilter', **kwargs):
        kwargs['name'] = name
        super().__init__(**kwargs)
        self.evtList = kwargs.get('evtList', None)
        self.isMC = kwargs.get('isMC', False)

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
if __name__ == "__main__":

    # Parse user input
    import argparse
    parser = argparse.ArgumentParser(description='Select events identified by run number (mc channel number'
                                     ' in case of Monte Carlo), event number from the input POOL file(s)')
    parser.add_argument('--inputFiles', required=True,
                        help='Input POOL file(s) separated with commas')
    parser.add_argument('--outputFile', required=True,
                        help='Output POOL file')
    parser.add_argument('--eventList', required=True,
                        help='Text file containing <run number> <event number> [<guid>] record(s) (one per line)')
    args, _ = parser.parse_known_args()

    # Setup configuration logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('EventPicking')
    log.info('== Picking events from POOL files w/ the CA Configuration')

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
    flags.Input.Files = args.inputFiles.split(',')

    try:
        streamToOutput = flags.Input.ProcessingTags[0].removeprefix('Stream')
    except Exception as e:
        raise RuntimeError('Could not determine the stream type') from e

    for name, value in ((f'Output.{streamToOutput}FileName', args.outputFile),
                        (f'Output.doWrite{streamToOutput}', True)):
        if flags.hasFlag(name):
            setattr(flags, name, value)
        else:
            flags.addFlag(name, value)
    if 'DAOD' in streamToOutput:
        flags.Output.doWriteDAOD = True

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
    cfg.addEventAlgo(PyEvtFilter('EventFilterAlg', evtList=evtList, isMC=flags.Input.isMC), sequenceName='AthAlgSeq')

    # Configure the output stream
    log.info(f'== Configuring Output Stream {streamToOutput!r}')
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    cfg.merge(OutputStreamCfg(flags, streamToOutput, takeItemsFromInput=True, extendProvenanceRecord=False))

    # Configure metadata
    log.info('== Configuring metadata for the output stream')
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory

    cfg.merge(SetupMetaDataForStreamCfg(flags, streamToOutput,
                                        createMetadata=[MetadataCategory.IOVMetaData]))

    # Setup the output stream algorithm
    Stream = cfg.getEventAlgo(outputStreamName(streamToOutput))
    Stream.ForceRead = True
    Stream.AcceptAlgs += ['EventFilterAlg']

    log.info(f'== Configured {streamToOutput!r} writing')

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

    # Needed for merging in MT
    if 'ESD' in streamToOutput:
        Stream.ExtraInputs.add(
            ( 'MuonGM::MuonDetectorManager',
                  'ConditionStore+MuonDetectorManager' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::SiDetectorElementCollection',
                  'ConditionStore+PixelDetectorElementCollection' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::SiDetectorElementCollection',
                  'ConditionStore+SCT_DetectorElementCollection' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::TRT_DetElementContainer',
                  'ConditionStore+TRT_DetElementContainer' ) )

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
