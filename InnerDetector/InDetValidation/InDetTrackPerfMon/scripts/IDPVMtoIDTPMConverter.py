#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## quick script to change the directory structure of a histogram file to that of another
## this is used to make an IDPVM output with the same format of IDTPM for comparison

import argparse, ROOT, os, sys
from InDetTrackPerfMon.ConfigUtils import custom_find_datafile
from AthenaCommon.Logging import logging
log = logging.getLogger( "IDPVMtoIDTPMConverter.py" )

# Parsing arguments
parser = argparse.ArgumentParser( description = "IDPVMtoIDTPMConverter.py options:" )
parser.add_argument( "-i", "--inputFile", help="IDPVM input file", required=True )
parser.add_argument( "-c", "--config", help="config file", default="InDetTrackPerfMon/IDPVM_to_IDTPM_map_skeleton.txt" )
parser.add_argument( "-o", "--outputFile", help="Name of the output IDPVM file converted in IDTPM format", default="IDPVMtoIDTPM.root" )
parser.add_argument( "-d", "--debug", help='set debug level printout', action='store_true', default=False )
parser.add_argument( "--doTightPrimary", help='also include tight-primary plots in map', action='store_true', default=False )
# TODO - add here other flags for specific use cases
MyArgs = parser.parse_args()

if MyArgs.debug : log.setLevel( logging.DEBUG )

## Check if input histogram file exists
inputPath = custom_find_datafile( MyArgs.inputFile )
if not inputPath :
    log.error( f"Input file {MyArgs.inputFile} not found" )
    sys.exit(1)

## Check if config file exists
configFileName = custom_find_datafile( MyArgs.config )
if not configFileName :
    log.error( f"Config txt {MyArgs.config} not found" )
    sys.exit(1)

## First, read config file to fill bas map 
namingMap_base = {}
with open( configFileName, 'r' ) as configFile :
    for line in configFile.readlines() :
        # skip empty lines or those starting with an empty space or #
        if not line.strip() : continue
        if line.startswith(" ") or line.startswith("#") : continue
        # parsing line
        parsed  = line.strip().split()
        htype   = parsed[0]
        hIDPVM  = parsed[1]
        hIDTPM  = parsed[2]
        if hIDPVM=="NONE" or hIDTPM=="NONE" : continue
        # filling map
        namingMap_base.update( { hIDPVM : { "IDTPMname" : hIDTPM, "type" : htype } } )

## Expand map to account for other track analyses
expandMap = []
if MyArgs.doTightPrimary : expandMap.append( { "IDPVMrepl" : "SquirrelPlots/TightPrimary/",
                                               "IDTPMrepl" : "TrkAnaOffl_TightPrimary" } )
# TODO - add here corresponding lines for other flags for specific use cases

namingMap = namingMap_base.copy()
for expitem in expandMap :
    namingMap_add = {}
    for hname, hdict in namingMap_base.items() :
        newhname = hname.replace( "SquirrelPlots/", expitem["IDPVMrepl"] )
        newhdict = hdict.copy()
        newhdict["IDTPMname"] = newhdict["IDTPMname"].replace( "TrkAnaOffl", expitem["IDTPMrepl"] )
        namingMap_add.update( { newhname : newhdict } )
    namingMap.update( namingMap_add )
log.info( "Naming map successfully created" )

## now opening and looping input IDPVM file
def getall(d, basepath=""):
    "Generator function to recurse into a ROOT file/dir and yield (path, obj) pairs"
    for key in d.GetListOfKeys():
        kname = key.GetName()
        ## Skip a tree
        if key.GetClassName() == "TTree":
          continue
        if key.IsFolder():
            for i in getall(d.Get(kname), basepath+kname+"/"):
                yield i
        else:
            yield basepath+kname, d.Get(kname)

inFile = ROOT.TFile.Open( inputPath, "READ" )
outFile = ROOT.TFile.Open( MyArgs.outputFile, "RECREATE" )

for keyname, keyobj in getall( inFile ) :
    if not keyobj : continue
    keytype = keyobj.ClassName()

    ## checking if histogram is in map
    if keyname not in namingMap :
        log.debug( f"WARNING: plot {keyname} not found. Skipping" )
        continue

    ## Grabbing info from naming map
    mapHtype = namingMap[ keyname ]["type"]
    newHname = namingMap[ keyname ]["IDTPMname"]

    ## Checking consistency of types
    if keytype != mapHtype :
        log.warning( f"histogram {keyname} is {keytype}, but {mapHtype} requested. Skipping." )
        continue

    ## getting input histo
    inFile.cd()
    keyobj.SetDirectory(0)

    ## writing histo
    outFile.cd()
    newHname_path = os.path.dirname( newHname )
    newHname_obj = os.path.basename( newHname )
    if( not outFile.GetDirectory( newHname_path ) ):
        outFile.mkdir( newHname_path, newHname_path )
    outFile.cd( newHname_path )
    keyobj.SetName( newHname_obj )
    keyobj.Write()

log.info( f"Output file {MyArgs.outputFile} successfully written. All done!" )
inFile.Close()
outFile.Close()
