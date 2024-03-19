#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs() :
    from argparse import ArgumentParser
    IDTPMparser = ArgumentParser( description='Parser for IDTPM configuration' )
    IDTPMparser.add_argument( "--inputFileNames", help="Comma-separated list of input files", required=True)
    IDTPMparser.add_argument( "--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int )
    IDTPMparser.add_argument( "--debug", help="Enable debugging messages", action="store_true", default=False )
    IDTPMparser.add_argument( "--dirName", help="Main directory name for storing plots", default="InDetTrackPerfMonPlots/" )
    IDTPMparser.add_argument( "--outputFileName", help='Name of output file', default="myIDTPM_out" )
    IDTPMparser.add_argument( "--writeAOD_IDTPM", help="Write output file for reprocessing", action="store_true", default=False )
    IDTPMparser.add_argument( "--trkAnaCfgFile", help='File with track analysis setup (.json format)', default='Default' )
    IDTPMparser.add_argument( "--unpackTrigChains", help="Run each configured trigger chain in a separate track analysis", action="store_true", default=False )
    # TODO - to be included in next MRs
    #IDTPMparser.add_argument( "--histoDefFormat", help='Format of the histogram definition file', default="JSON" )
    #IDTPMparser.add_argument( "--histoDefFileList", help='Plain txt file containing the list of .json file names with the histograms definitions', default="InDetTrackPerfMon/HistoDefFileList_default.txt" )
    #IDTPMparser.add_argument( "--plotsCommonValuesFile", help='JSON file listing all the default values to be used in plots', default="InDetTrackPerfMon/IDTPMPlotCommonValues.json" )
    return IDTPMparser.parse_args()

## Parse the arguments
MyArgs = GetCustomAthArgs()

#from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

## Inputs
flags.Input.Files = []
for path in MyArgs.inputFileNames.split( ',' ):
    flags.Input.Files += glob( path )

## Outputs
flags.PhysVal.OutputFileName = MyArgs.outputFileName + '.HIST.root'
if MyArgs.writeAOD_IDTPM:
    flags.addFlag( 'Output.doWriteAOD_IDTPM', True )
    flags.addFlag( 'Output.AOD_IDTPMFileName',
                   MyArgs.outputFileName + '.AOD_IDTPM.pool.root' )

## Set output log level
if MyArgs.debug:
    from AthenaCommon.Constants import DEBUG
    flags.Exec.OutputLevel = DEBUG

## General config flag category for IDTPM tool job configuration
from InDetTrackPerfMon.InDetTrackPerfMonFlags import createIDTPMConfigFlags
flags.addFlagsCategory( "PhysVal.IDTPM", 
                        createIDTPMConfigFlags, 
                        prefix=True )

flags.PhysVal.IDTPM.DirName = MyArgs.dirName
# TODO - to be included in next MRs
#flags.PhysVal.IDTPM.histoDefFormat = MyArgs.histoDefFormat
#flags.PhysVal.IDTPM.HistoDefFileList = MyArgs.histoDefFileList
#flags.PhysVal.IDTPM.plotsCommonValuesFile = MyArgs.plotsCommonValuesFile

## Create flags category and corresponding set of flags
## (read from trkAnaCfgFile.json) for each TrkAnalysis
from InDetTrackPerfMon.InDetTrackPerfMonFlags import createIDTPMTrkAnaConfigFlags

# Default TrackAnalysis configuration flags category
flags.addFlagsCategory( "PhysVal.IDTPM.Default", 
                        createIDTPMTrkAnaConfigFlags, 
                        prefix=True )
 
## Filling TrkAnalyses setup dictionary
from InDetTrackPerfMon.ConfigUtils import getTrkAnaDicts
analysesDict = getTrkAnaDicts( flags, MyArgs.trkAnaCfgFile, MyArgs.unpackTrigChains )
trkAnaNames = []

if analysesDict:
    for trkAnaName, trkAnaDict in analysesDict.items():
        # Append TrkAnalysisName to list
        trkAnaNames.append( trkAnaName )

        # separate flag category for each TrkAnalysis
        flags.addFlagsCategory( "PhysVal.IDTPM."+trkAnaName, 
                                createIDTPMTrkAnaConfigFlags, 
                                prefix=True )

        # set flags from values in trkAnaDict
        for fname, fvalue in trkAnaDict.items():
            setattr( flags.PhysVal.IDTPM, 
                     trkAnaName+"."+fname, fvalue )

if trkAnaNames:
    flags.PhysVal.IDTPM.trkAnaNames = trkAnaNames

flags.lock()

flags.dump()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge( PoolReadCfg(flags) )

from InDetTrackPerfMon.InDetTrackPerfMonConfig import InDetTrackPerfMonCfg
acc.merge( InDetTrackPerfMonCfg(flags) )

acc.printConfig( withDetails=True )

# Execute and finish
sc = acc.run( maxEvents=MyArgs.maxEvents )

# Success should be 0
import sys
sys.exit( not sc.isSuccess() )
