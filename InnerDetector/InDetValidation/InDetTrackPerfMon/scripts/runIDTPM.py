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
    IDTPMparser.add_argument( "--outputFilePrefix", help='Name of output file', default="myIDTPM_out" )
    IDTPMparser.add_argument( "--writeAOD_IDTPM", help="Write output file for reprocessing", action="store_true", default=False )
    IDTPMparser.add_argument( "--trkAnaCfgFile", help='File with track analysis setup (.json format)', default='Default' )
    IDTPMparser.add_argument( "--unpackTrigChains", help="Run each configured trigger chain in a separate track analysis", action="store_true", default=False )
    IDTPMparser.add_argument( "--plotsDefFormat", help='Format of the plots definition file', default="JSON" )
    IDTPMparser.add_argument( "--plotsDefFileList", help='Plain txt file containing the list of .json file names with the plots definitions', default="InDetTrackPerfMon/PlotsDefFileList_default.txt" )
    IDTPMparser.add_argument( "--plotsCommonValuesFile", help='JSON file listing all the default values to be used in plots', default="" )
    IDTPMparser.add_argument( "--sortPlotsByChain", help="Arrange plots first in subdirectories named after the current chain", action="store_true", default=False )
    IDTPMparser.add_argument( "--commonTrkAnaFlags", help="Common flags to be overridden for all track Analises in the format flag_name=value", nargs='*', default=[] )
    return IDTPMparser.parse_args()

## Parse the arguments
MyArgs = GetCustomAthArgs()

#from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

## Inputs
flags.Input.Files = []
for path in MyArgs.inputFileNames.split( ',' ):
    fileList = glob( path )
    if not fileList:
        raise RuntimeError( f"Input file {path} not found" )
    flags.Input.Files += fileList

## Set output log level
if MyArgs.debug:
    from AthenaCommon.Constants import DEBUG
    flags.Exec.OutputLevel = DEBUG

from InDetTrackPerfMon.InDetTrackPerfMonFlags import initializeIDTPMConfigFlags, initializeIDTPMTrkAnaConfigFlags

# initialize general IDTPM job flags
flags = initializeIDTPMConfigFlags( flags )
flags.PhysVal.IDTPM.DirName = MyArgs.dirName
flags.PhysVal.IDTPM.outputFilePrefix = MyArgs.outputFilePrefix
flags.PhysVal.IDTPM.plotsDefFormat = MyArgs.plotsDefFormat
flags.PhysVal.IDTPM.plotsDefFileList = MyArgs.plotsDefFileList
flags.PhysVal.IDTPM.plotsCommonValuesFile = MyArgs.plotsCommonValuesFile
flags.PhysVal.IDTPM.sortPlotsByChain = MyArgs.sortPlotsByChain
flags.PhysVal.IDTPM.trkAnaCfgFile = MyArgs.trkAnaCfgFile
flags.Output.doWriteAOD_IDTPM = MyArgs.writeAOD_IDTPM
flags.PhysVal.IDTPM.unpackTrigChains = MyArgs.unpackTrigChains
flags.PhysVal.IDTPM.commonTrkAnaFlags = MyArgs.commonTrkAnaFlags

# initialize individual TrkAnalises flags (and output file names)
flags = initializeIDTPMTrkAnaConfigFlags( flags )

flags.PhysVal.doExample = False

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
