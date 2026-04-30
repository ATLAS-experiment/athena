#!/usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
Select events for a given stream name from an input file and write them to an output file.
The output file obeys the conventions used by the SFO at P1.

Multiple files can be processed but all events need to be from the same run.
"""

import sys
import os
import argparse
import eformat
import logging
from libpyevent_storage import CompressionType
from libpyeformat_helper import SourceIdentifier, SubDetector


def peb_writer():
  """Runs the splitting routines"""

  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

  parser.add_argument("files", metavar="FILE", nargs='+',
                      help="RAW file to inspect")

  parser.add_argument('-s', '--stream-name', metavar='NAME', type=str, required=True,
                      help='Name(s) of stream(s) which should be written out, e.g. "stream1,stream2,stream3"')

  parser.add_argument('-o', '--output-name', metavar='NAME', type=str,
                      help='Core output file name (by default derived from input)')

  parser.add_argument('-d', '--output-dir', metavar='DIR', type=str, default='.',
                      help='Directory in which the output file should be written')

  parser.add_argument('-a', '--start-event', metavar='N', type=int, default=0,
                      help='Number of events which should be skipped from the begin')

  parser.add_argument('-n', '--max-events', metavar='N', type=int,
                      help='Maximum number of events in the output file')

  parser.add_argument('-u', '--uncompressed', action='store_true',
                      help='Write out uncompressed data')

  parser.add_argument('-p', '--project-tag', metavar='TAG', type=str,
                      help='Project tag which should be used for the output file')

  parser.add_argument('-l', '--lumi-block', metavar='N', type=int,
                      help='Lumiblock number used for the output file. Use 0 if multiple LBs in file.')

  parser.add_argument('-m', '--hlt-only', metavar='ID', type=int,
                      help='Drop all detector data and write out only HLT data for the given module ID. '
                           'Module ID <0 is a wildcard for all HLT module IDs.')

  parser.add_argument('-v', '--verbosity', metavar='N', type=int, default=logging.INFO,
                      help='Log verbosity')

  parser.add_argument('-P', '--progress-bar', action='store_true',
                      help='Show progress bar when running interactively')

  args = parser.parse_args()

  # global defaults
  logging.getLogger('').name = 'trigbs_extractStream'
  logging.getLogger('').setLevel(args.verbosity)

  # input data stream
  stream = eformat.istream(args.files)

  # get metadata from inputfile
  dr = eformat.EventStorage.pickDataReader(args.files[0])

  # interpret input file name
  df = eformat.EventStorage.RawFileName(args.files[0])

  # extract some parameters from meta-data 
  projectTag      = args.project_tag or dr.projectTag()
  lumiBlockNumber = args.lumi_block if args.lumi_block is not None else dr.lumiblockNumber()
  applicationName = dr.appName()
  streamType      = 'unknown' # the real stream type will be extracted from the matching stream tag
  if df.hasValidCore() :
    productionStep  = df.productionStep()
  else:
    productionStep  = 'unknown'

  # input parameters for building the output file name
  runNumber       = dr.runNumber() 
  outputDirectory = args.output_dir
  streamName      = args.stream_name

  # check if multiple streams should be written to the same output file (used in debug recovery) 
  streamNames_out = streamName.split(',')
  if len(streamNames_out) > 1:
    streamName = 'accepted'

  if lumiBlockNumber==0:
    productionStep  = 'merge'

  # check the output directory if it exists
  if (not os.path.exists(outputDirectory)) or (not os.path.isdir(outputDirectory)): 
    logging.fatal(' Output directory %s does not exist ' % outputDirectory)
    sys.exit(1)

  # event counters
  totalEvents_in = 0
  totalEvents_out = 0
  totalEvents_skipped = 0

  # Loop over events
  for e in stream:

    if args.max_events and totalEvents_in >= args.max_events:
      logging.info(' Maximum number of events reached : %d', args.max_events)
      break

    totalEvents_in += 1

    # select events
    if args.start_event > 0:
      args.start_event -= 1
      totalEvents_skipped += 1
      continue

    # find StreamTags and see if there is a match
    streamTags = e.stream_tag()
    logging.debug(' === New Event nr = %s (Run,Global ID) = (%d,%d) === ', totalEvents_in,e.run_no(),e.global_id())
    #count accepted streams for each event 
    streamAccepted = 0
    for tag in streamTags:
      if tag.name in streamNames_out:
        #avoid duplication of events that have > 1 streamNames_out
        if streamAccepted : continue
        streamAccepted += 1

        # the event should be written out        
        logging.debug(' Matching event found for stream tag = %s', tag)
        logging.debug('      Stream Tag:Robs = %s', [hex(r) for r in tag.robs])
        logging.debug('      Stream Tag:Dets = %s', [hex(d) for d in tag.dets])

        # check the lumi block number from the event against the lumi block number defined for the file
        # this check is only done if the lumi block number for the file is different from 0
        if lumiBlockNumber > 0:
          if e.lumi_block() != lumiBlockNumber:
            logging.error(' Event (Run,Global ID) = (%d,%d) has a lumi block number %d,'
                          ' which is different from LB = %d for the output file. Event skipped.',
                          e.run_no(),e.global_id(),e.lumi_block(),lumiBlockNumber)
            continue

        # check that all events have the same run number as the output file indicates otherwise skip event
        if e.run_no() != runNumber:
            logging.error(' Event (Run,Global ID) = (%d,%d) has a run number,'
                          ' which is different from the run number = %d for the output file. Event skipped.',
                          e.run_no(),e.global_id(),runNumber)
            continue

        # set the overall tag type for the first match
        if streamType != tag.type:
          streamType = tag.type
          logging.debug(' streamType set to = %s', streamType)

          # create the RAW output file name
          outRawFile = args.output_name or \
            eformat.EventStorage.RawFileName(projectTag,
                                             runNumber,
                                             streamType,
                                             streamName,
                                             lumiBlockNumber,
                                             applicationName,
                                             productionStep).fileNameCore()
          logging.debug(' set output file name = %s', outRawFile)

          # Note: EventStorage and eformat compression enums have different values
          compressionTypeES = CompressionType.NONE if args.uncompressed else CompressionType.ZLIB
          compressionType = eformat.helper.Compression.UNCOMPRESSED if args.uncompressed \
                            else eformat.helper.Compression.ZLIB
          compressionLevel = 0 if args.uncompressed else 1

          # create the output stream
          ostream = eformat.ostream(directory=outputDirectory,
                                    core_name=outRawFile,
                                    run_number=dr.runNumber(), 
                                    trigger_type=dr.triggerType(),
                                    detector_mask=dr.detectorMask(), 
                                    beam_type=dr.beamType(),
                                    beam_energy=dr.beamEnergy(),
                                    compression=compressionTypeES,
                                    complevel=compressionLevel)
        
        # decide what to write out
        is_feb_tag = (len(tag.robs)==0 and len(tag.dets)==0)
        if is_feb_tag and not args.hlt_only:
          # write out the full event fragment
          pbev = eformat.write.FullEventFragment(e)  
          logging.debug(' Write full event fragment ')
        else:
          # filter stream tag robs and dets for the hlt-only option
          dets = []
          robs = []
          if args.hlt_only:
            if args.hlt_only < 0:
              dets = [SubDetector.TDAQ_HLT] if SubDetector.TDAQ_HLT in tag.dets or is_feb_tag else []
              robs = [robid for robid in tag.robs if SourceIdentifier(robid).subdetector_id()==SubDetector.TDAQ_HLT]
            else:
              requested_rob_id = int(SourceIdentifier(SubDetector.TDAQ_HLT, args.hlt_only))
              if SubDetector.TDAQ_HLT in tag.dets or requested_rob_id in tag.robs or is_feb_tag:
                robs = [requested_rob_id]
          else:
            dets = list(tag.dets)
            robs = list(tag.robs)

          # select ROBs to write out
          rob_output_list = []
          logging.debug(' Write partial event fragment ')
          for rob in e:
            if rob.source_id().code() in robs:
              rob_output_list.append(rob)
            if rob.source_id().subdetector_id() in dets:
              rob_output_list.append(rob)
          # write out the partial event fragment
          pbev = eformat.write.FullEventFragment()
          pbev.copy_header(e)
          for out_rob in rob_output_list: 
            pbev.append_unchecked(out_rob)
        
        # put the event onto the output stream
        pbev.compression_type(compressionType)
        pbev.compression_level(compressionLevel)
        ostream.write(pbev)
        if (logging.getLogger('').getEffectiveLevel() > logging.DEBUG) and args.progress_bar:
          sys.stdout.write('.')
          sys.stdout.flush()

        # increase output event counter
        totalEvents_out += 1

  # print final statistics
  logging.info('Input file(s)                             = %s ', args.files)
  logging.info('Total number of events processed          = %d ', totalEvents_in)
  logging.info('Number of events skipped at the beginning = %d ', totalEvents_skipped)
  logging.info('Number of events written to output file   = %d ', totalEvents_out)
  if totalEvents_out > 0:
    logging.info('Output file                               = %s ', ostream.last_filename())
  else:
    logging.error('No events selected so no output file created')
    sys.exit(1)

  sys.exit(0)


if __name__ == "__main__":
  peb_writer()
