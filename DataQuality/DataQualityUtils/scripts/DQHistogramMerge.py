#!/usr/bin/env python

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration


import DataQualityUtils.DQHistogramMergeMod as mod
import os
import argparse

## Get usable signal handlers
os.environ['TDAQ_ERS_NO_SIGNAL_HANDLERS'] = '1'

def fakebool(x):
  return bool(eval(x))

########################################

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('input_list_file_name', help='Text file containing input file list (one file per line)')
  parser.add_argument('merged_file_name', help='Name of output merged ROOT file')
  parser.add_argument('run_post_processing', nargs='?', type=fakebool, default=False, help='False/True/0/1 default=0')
  parser.add_argument('is_incremental_merge', nargs='?', type=fakebool, default=False, help='False/True/0/1 default=0')
  parser.add_argument('output_file_compression_level', nargs='?', type=int, default=1, help='see ROOT TFile doc. default=1')
  parser.add_argument('debugLevel', nargs='?', type=int, default=0, help='integer default=0')
  parser.add_argument('--excludeDir', help='Regex pattern for directories to exclude from merge')
  parser.add_argument('--excludeHist', help='Regex pattern for histogram names to exclude from merge\n'
                                            'Note that this is just the name - paths cannot be specified')
  parser.add_argument('--doTiming',action = 'store_true', help="Print CPU timing per histogram")

  args = parser.parse_args()
  print(args)

  runPostProcessing = args.run_post_processing

  isIncremental = args.is_incremental_merge

  compressionLevel = args.output_file_compression_level

  debugLevel = args.debugLevel

  if args.excludeDir:
    directoryRegularExpression = f'^((?!{args.excludeDir}).)*$'
  else:
    directoryRegularExpression = '.*'

  if args.excludeHist:
    histogramRegularExpression = f'^((?!{args.excludeHist}).)*$'
  else:
    histogramRegularExpression = '.*'

  doTiming=False
  if args.doTiming:
    doTiming=True
    
  mod.DQHistogramMerge(args.input_list_file_name, args.merged_file_name, 
                       runPostProcessing, isIncremental=isIncremental, 
                       compressionLevel=compressionLevel, debugLevel=debugLevel,
                       doTiming=doTiming,
                       directoryRegularExpression=directoryRegularExpression,
                       histogramRegularExpression=histogramRegularExpression)
