#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## quick wrapper script around dcube.py -c to create a dcube xml config
## based on a reference root file and dynamically adjust the plotopts of each plot
## based on the plot type

import argparse, os, sys
import subprocess
from AthenaCommon.Logging import logging
log = logging.getLogger( "makeDcubeConfig.py" )

# Parsing arguments
parser = argparse.ArgumentParser( description = "makeDcubeConfig.py options:" )
parser.add_argument( "-i", "--inputFile", help="IDPVM input file", required=True )
parser.add_argument( "-c", "--configName", help="Name of the output dcube xml config file", default="dcube_config.xml" )
parser.add_argument( "-d", "--debug", help='set debug level printout', action='store_true', default=False )
MyArgs = parser.parse_args()
if MyArgs.debug : log.setLevel( logging.DEBUG )

## $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py
dcubeExe = os.getenv( "ATLAS_LOCAL_ROOT" )+"/dcube/current/DCubeClient/python/dcube.py"
log.debug( f"Setting Dcube executable to {dcubeExe}" )
if not os.path.exists( dcubeExe ):
    log.error( f"Dcube executable {dcubeExe} not found. Exiting." )
    sys.exit(1)

## First running dcube.py -c to get the (baseline) xml config
dcubeCmd = [ dcubeExe, '-g',
             '-c', 'tmp_'+MyArgs.configName,
             '-r', MyArgs.inputFile, MyArgs.inputFile ]
log.debug( "Running: "+(' '.join( dcubeCmd )) )
subprocess.run( dcubeCmd, check=True )
if not os.path.exists( 'tmp_'+MyArgs.configName ):
    log.error( f"Temporary dcube config tmp_{MyArgs.configName} not found. Exiting." )
    sys.exit(1)

## Now reading and parsing dcube xml config
import xml.etree.ElementTree as ET
treeXML = ET.parse( 'tmp_'+MyArgs.configName )
rootXML = treeXML.getroot()

## Loop plots to adjust each plotopts
## First, loop all TDirectories
for tdir in rootXML.iter("TDirectory"):
    histList = tdir.findall("hist1D")
    histList += tdir.findall("hist2D")
    histList += tdir.findall("graph")
    ## Then, loop all hist1Ds, hist2Ds, and graphs in each TDirectory
    for hist in histList:
        ## General settings, by plot type
        if "TH1" in hist.get("type"):
            hist.set("plotopts", "norm;ratio") # for 1D plots
        if "TH2" in hist.get("type"):
            hist.set("plotopts", "norm;box") # for 1D plots
        if hist.get("type") == "TProfile" or hist.get("type") == "TEfficiency":
            hist.set("plotopts", "ratio") # for 1D TProfiles and TEfficiencies
        if ( "2D" in hist.get("type") or
             ( "TEfficiency" in hist.get("type") and "2D" in hist.get("name") ) ):
            hist.set("plotopts", "box") # for 2D TProfile2Ds and TEff2Ds
        ## Specific cases per catogory, by plot name
        if tdir.get("name") == "Multiplicities":
            if "summary" in hist.get("name"):
                hist.set("plotopts", "logy")
        if tdir.get("name") == "Resolutions":
            if ( "resolution" in hist.get("name") or
                 "resmean" in hist.get("name") or
                 "pullwidth" in hist.get("name") or
                 "pullmean" in hist.get("name") ):
                hist.set("plotopts", "ratio") # just ratio for resol, no need for norm
            if "_vs_" in hist.get("name") and hist.get("name").endswith("pt"):
                hist.set("plotopts", "logx;ratio") # also logx for e.g. resol vs pt plots
log.info( "Done editing Dcube XML config" )

## Writing new XML config
treeXML.write( MyArgs.configName, xml_declaration=True )
log.info( f"New Dcube XML config written {MyArgs.configName}" )
os.remove( 'tmp_'+MyArgs.configName ) # remove tmp config
