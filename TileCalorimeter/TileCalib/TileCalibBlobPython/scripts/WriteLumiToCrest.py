#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    WriteLumiToCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-12-12
#
# Purpose: Prepare JSON file with new lumi values
# specified at command line with --value= and --value2= options
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Prepare sqlite file with Lumi values for CALO database")
    print ("")
    print ("-h, --help      shows this help")
    print ("-i, --inschema=   specify name of input JSON file or CREST_SERVER_PATH")
    print ("-o, --outschema=  specify name of output JSON file, default is CaloNoise.json")
    print ("-t, --tag=      specify the tag")
    print ("-f, --folder=   specify folder to use e.g. /CALO/Ofl/Noise/PileUpNoiseLumi ")
    print ("-x, --txtfile=  specify the text file with the new noise constants")
    print ("-v, --value=    specify new lumi value")
    print ("-V, --value2=   specify new valid flag")
    print ("-c, --channel=  specify COOL channel, by default COOL channels 0 and 1 are used")
    print ("-r, --run=      specify run number for start of IOV")
    print ("-l, --lumi=     specify lumiblock number for start of IOV, default is 0")

letters = "hi:o:t:f:x:v:V:c:r:l:u"
keywords = ["help","inschema=","outschema=","tag=","folder=","txtfile=","value=","value2=","channel=","run=","lumi=","update","infile=","outfile="]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
inSchema    = 'CREST'
outSchema   = 'PileUp.json'
folderPath  = '/CALO/Ofl/Noise/PileUpNoiseLumi'
tag         = 'UPD4'
txtFile     = ''
value       = None
value2      = None
run         = -1
lumi        = 0
update      = False
channels = [0,1]

for o, a in opts:
    a = a.strip()
    if o in ("-i","--inschema","--infile"):
        inSchema = a
    elif o in ("-o","--outschema","--outfile"):
        outSchema = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-f","--folder"):
        folderPath = a
    elif o in ("-c","--channel"):
        channels = [int(a)]
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-u","--update"):
        update = True
    elif o in ("-x","--txtfile"):
        txtFile = a
    elif o in ("-v","--value"):
        value = float(a)
    elif o in ("-V","--value2"):
        value2 = int(a)
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        print (o, a)
        usage()
        sys.exit(2)

#=== check presence of all parameters
print ("")
inputIsFile = os.path.isfile(inSchema)
tagIsFullTag = tag.upper().startswith('CALO')
if len(inSchema)<1:
    print("Please, provide inschema (e.g. --inschema=PileUp.json or --inschema=CREST)")
    sys.exit(2)
if len(outSchema)<1:
    print("Please, provide outschema (e.g. --outschema=PileUp.json)")
    sys.exit(2)
if not inputIsFile:
    if len(folderPath)<1 and not tagIsFullTag:
        print("Please, provide folder (e.g. --folder=/CALO/Ofl/Noise/PileUpNoiseLumi)")
        sys.exit(2)
    if len(tag)<1:
        print("Please, provide tag (e.g. --tag=RUN2-UPD4-05 or --tag=CALOOflNoisePileUpNoiseLumi-RUN2-UPD4-05)")
        sys.exit(2)
else:
    if not tagIsFullTag:
        print("Please, provide full tag (e.g. --tag=CALOOflNoisePileUpNoiseLumi-RUN2-UPD4-05)")
        sys.exit(2)

if run<0:
    print("Please, provide run number (e.g. --run=123456)")
    sys.exit(2)

from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobPython.TileCalibLogger import getLogger

#=== get a logger
log = getLogger("WriteLumi")
import logging
log.setLevel(logging.DEBUG)

#=== Initialize blob reader
folder = folderPath
if tagIsFullTag:
    folder=""
if not inputIsFile:
    log.info("Initializing folder %s with tag %s", folder, tag)
reader = TileCalibCrest.TileBlobReaderCrest(inSchema,folder,tag,run,lumi,channels[0],channels[-1],True)
obj = reader.getPayload(None, False)
if obj is None:
    log.critical("Could not read payload from CREST DB")
    sys.exit(1)
if not tagIsFullTag:
    tag = reader.getTag()

for chan in channels:
    try:
        if value is not None:
            obj[str(chan)][0] = value
        if value2 is not None:
            obj[str(chan)][1] = value2
    except Exception:
        log.critical(f"Can not set values for channel {chan}")
        log.critical("Full payload is: "+str(obj))
        sys.exit(1)

if len(txtFile):
    try:
        with open(txtFile,"r") as f:
            allData = f.readlines()
    except Exception:
        print("\nCan not read input file %s" % (txtFile))
        sys.exit(2)

    for line in allData:
        fields = line.strip().split()
        #=== ignore empty and comment lines
        if not len(fields)          :
            continue
        if fields[0].startswith("#"):
            continue
        try:
            ch = int(fields[0])
            for chan in (channels if ch<0 else [ch]):
                if len(fields)>1 and fields[1].lower() != "keep":
                    obj[str(chan)][0] = float(fields[1])
                if len(fields)>2 and fields[2].lower() != "keep":
                    obj[str(chan)][1] = int(fields[2])
        except Exception:
            log.error(f"Can not process line {line} - skipping")

log.info( "Writing payload "+str(obj) )
writer = TileCalibCrest.TileBlobWriterCrest(outSchema,folder,None,obj)
writer.register((run,lumi), tag)

