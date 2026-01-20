#!/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# WritePulseShapeToCool.py
# Sanya Solodkov 2023-02-23
#

import os, sys, getopt, cppyy, logging
os.environ['TERM'] = 'linux'

def usage():
    print ("How to use: ",sys.argv[0]," [OPTION] ... ")
    print ("Write pulse shapes to COOL")
    print ("")
    print ("-h, --help      shows this help")
    print ("-f, --folder=   specify folder to use f.i. /TILE/OFL02/PULSESHAPE/PHY or only PHY")
    print ("-t, --tag=      specify tag to use, f.i. RUN2-HLT-UPD1-00")
    print ("-r, --run=      specify run  number, default is 0")
    print ("-m, --module=   specify module name for which pulse shape will be written, default is AUX01")
    print ("-L, --lowgain=  specify the text file with new pulse shape for low gain")
    print ("-H, --highgain= specify the text file with new pulse shape for high gain")
    print ("-s, --schema=   specify schema to use, f.i. 'sqlite://;schema=tileSqlite.db;dbname=CONDBR2'")
    print ("-D, --dbname=   specify dbname part of schema if schema only contains file name, default is tileSqlite.db")
    print ("-U, --user=     specify username for comment")
    print ("-C, --comment=  specify comment which should be written to DB")
    print ("-z, --zero      if present, means that zero-sized blob is written for missing drawers")
    print ("-u  --update    set this flag if output sqlite file should be updated, otherwise it'll be recreated")


letters = "L:H:s:D:U:C:f:t:m:r:hzu"
words = ["lowgain=","highgain=","schema=","dbname=","user=","comment=","folder=","tag=","module=","run=","help","zero","update"]

try:
    options,args = getopt.getopt(sys.argv[1:],letters,words)
except getopt.GetoptError as err:
    print ()
    print (str(err))
    print ()
    usage()
    sys.exit(2)


#=== all defaults

#=== input files
pulseLG = "pulselo_physics.dat"
pulseHG = "pulsehi_physics.dat"

#=== output file
dbname = "tileSqlite.db"

#=== folder for pulse in COOL DB
folder = "/TILE/OFL02/PULSESHAPE/PHY"

#=== tag for pulse in COOL DB
tag = "RUN2-HLT-UPD1-00"

#=== module name with new pulse
module = "AUX01"

#=== run number for new IOV
run = 0

#=== create zero-sized records for all other modules in DB
zeros = False

#=== update or recreate output DB
update = False

#=== print help and exit
help = False

comment = None
schema = None

try:
    user=os.getlogin()
except Exception:
    user="UnknownUser"


#=== read command line parameters

for o, a in options:
    a = a.strip()
    if o in ("-h","--help"):
        usage()
        sys.exit(2)
    elif o in ("-L","--lowgain"):
        pulseLG = a
    elif o in ("-H","--highgain"):
        pulseHG = a
    elif o in ("-s","--schema"):
        schema = a
    elif o in ("-D","--dbname"):
        dbname = a
    elif o in ("-f","--folder"):
        folder = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-m","--module"):
        module = a
    elif o in ("-U","--user"):
        user = a
    elif o in ("-C","--comment"):
        comment = a
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-z","--zero"):
        zeros = True
    elif o in ("-u","--update"):
        update = True
    else:
        raise RuntimeError("unhandled option")


part=['AUX','LBA','LBC','EBA','EBC']
ros=part.index(module[:3])
drawer=int(module[3:])-1

if schema is None:
    schema = 'sqlite://;schema=%s;dbname=CONDBR2' % (dbname)

if '/TILE' not in folder:
    folder='/TILE/OFL02/PULSESHAPE/'+folder

if comment is None:
    comment = "Pulses from %s %s for module %s" % (pulseLG,pulseHG,module)


#=== read low gain pulse shape
xlo = []
ylo = []
lines = open(pulseLG,"r").readlines()
for line in lines:
    fields = line.strip().split()
    #=== ignore empty and comment lines
    if not len(fields)          :
        continue
    if fields[0].startswith("#"):
        continue
    if fields[0].startswith("*"):
        continue
    if len(fields) != 2         :
        continue

    xlo.append(float(fields[0]))
    ylo.append(float(fields[1]))

#=== read high gain pulse shape
xhi = []
yhi = []
lines = open(pulseHG,"r").readlines()
for line in lines:
    fields = line.strip().split()
    #=== ignore empty and comment lines
    if not len(fields)          :
        continue
    if fields[0].startswith("#"):
        continue
    if fields[0].startswith("*"):
        continue
    if len(fields) != 2         :
        continue

    xhi.append(float(fields[0]))
    yhi.append(float(fields[1]))

#=== build pulseshape vectors for db
vecLo = cppyy.gbl.std.vector('float')()
for x in xlo:
    vecLo.push_back(x)
for y in ylo:
    vecLo.push_back(y)
vecHi = cppyy.gbl.std.vector('float')()
for x in xhi:
    vecHi.push_back(x)
for y in yhi:
    vecHi.push_back(y)
newPulse = cppyy.gbl.std.vector('std::vector<float>')()
newPulse.push_back(vecLo)
newPulse.push_back(vecHi)


#=== write pulse shapes to COOL DB
from TileCalibBlobPython import TileCalibTools
from TileCalibBlobObjs.Classes import TileCalibUtils
from TileCalibBlobPython.TileCalibTools import MAXRUN, MAXLBK

#=== open the database
db = TileCalibTools.openDbConn(schema,('UPDATE' if update else 'RECREATE'))
blobWriter = TileCalibTools.TileBlobWriter(db,folder,'Flt')
blobWriter.setLogLvl(logging.DEBUG)

#=== create zero-sized blobs for all drawers
if zeros:
    util = cppyy.gbl.TileCalibUtils()
    for r in range(util.max_ros()):
        for d in range(util.getMaxDrawer(r)):
            blobWriter.zeroBlob(r,d)

#=== write pulse shape to one drawer
det = blobWriter.getDrawer(ros,drawer)
det.init(newPulse,1,200)

blobWriter.setComment(user,comment)
folderTag = TileCalibUtils.getFullTag(folder, tag)
blobWriter.register((run,0),(MAXRUN,MAXLBK),folderTag)

#=== close the database connection
db.closeDatabase()
