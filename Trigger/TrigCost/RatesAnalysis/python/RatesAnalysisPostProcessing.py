#!/usr/bin/env python
#
#  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#

'''
@file RatesPostProcessing.py
@author T. Martin
@date 2020-02-04
@brief Script to consume merged rates histograms from the RatesAnalysis package and produce structured CSV, JSON output.
'''

import ROOT
from RatesAnalysis.Util import getTableName, getMetadata, populateTriggers, getGlobalGroup, toJson, toCSV, toROOT, populateScanTriggers, slice_dictionary
from AthenaCommon.Logging import logging



def main():
  from argparse import ArgumentParser
  parser = ArgumentParser()
  parser.add_argument('--file', default='RatesHistograms.root', 
                      help='Input ROOT file to generate output from, run hadd first if you have more than one')
  parser.add_argument('--outputTag', default='LOCAL', 
                      help='Tag identifying this processing to be used in the output folder name (any underscores will be removed)')
  parser.add_argument('--outputJSONFile', default='rates.json', 
                      help='JSON file of rates for use with the RuleBook')
  parser.add_argument('--outputROOTFile', default='scanTriggers.root', 
                      help='ROOT file of normalised scan-triggers')
  parser.add_argument('--userDetails',
                      help='User supplied metadata string giving any extra details about this run.')    
  parser.add_argument('--jira', 
                      help='Related jira ticket number')
  parser.add_argument('--amiTag', 
                      help='AMI tag used for data reprocessing')   
  parser.add_argument('--doBinomialCorrection', action='store_true',
                      help='apply binomial correction to trigger rates when using Monte Carlo JZ slices.')  
       

  args = parser.parse_args()
  log = logging.getLogger('RatesPostProcessing')

  inputFile = ROOT.TFile(args.file, 'READ')

  metadata = getMetadata(inputFile)

  normdict = slice_dictionary(inputFile, "normalisation")

  if len(normdict)==0 or metadata is None:
    log.error('Cannot locate normHist, or metadata in top level of ntuple.')
    exit()

  for suffix, normHist in normdict.items():
    metadata['normalisation'+suffix] = normHist.GetBinContent(1)
    metadata['n_evts'+suffix] = normHist.GetBinContent(2)
    metadata['n_evts_weighted'+suffix] = normHist.GetBinContent(3)

  metadata['details'] = args.userDetails
  metadata['JIRA'] = args.jira
  metadata['amiTag'] = args.amiTag
  metadata['doBinomialCorrection'] = args.doBinomialCorrection 

  HLTGlobalGroup = getGlobalGroup(inputFile, 'RATE_GLOBAL_HLT')
  L1GlobalGroup = getGlobalGroup(inputFile, 'RATE_GLOBAL_L1')

  L1Triggers = populateTriggers(inputFile, metadata, L1GlobalGroup, 'ChainL1')
  HLTTriggers = populateTriggers(inputFile, metadata, HLTGlobalGroup, 'ChainHLT')
  AllGlobalGroups = populateTriggers(inputFile, metadata, HLTGlobalGroup, 'Group')

  scanTriggers = populateScanTriggers(inputFile, metadata)


  if not scanTriggers and (not L1Triggers or not HLTTriggers or not AllGlobalGroups):
    log.error("Failed to populate triggers")
    return

  L1Table = getTableName("L1")
  HLTTable = getTableName("HLT")
  GroupTable = getTableName("Group")

  log.info("Exporting " + args.outputJSONFile)
  toJson(args.outputJSONFile, metadata, L1Triggers, HLTTriggers)
  log.info("Exporting " + HLTTable)
  toCSV(HLTTable, metadata, HLTTriggers)
  log.info("Exporting " + L1Table)
  toCSV(L1Table, metadata, L1Triggers)
  log.info("Exporting " + GroupTable)
  toCSV(GroupTable, metadata, AllGlobalGroups)
  log.info("Exporting scan triggers")
  toROOT(args.outputROOTFile, scanTriggers)
  
if __name__== "__main__":
  main()
