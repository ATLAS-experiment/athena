#!/bin/env python3
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# ReadFromCrestCompare.py
# (Based on ReadFromCoolCompare.py, adapted for CREST)
# Ekaterina Ramakoti 2026-05-21

# Note: this is a low level tool to be used only for tests
# It reads conditions from two CREST servers (or/and JSON files),
# compares values and dumps to the file difference, if it is above
# specified threshould
#
#=== examples of folder/tag names
#folderPath = "/TILE/OFL02/CALIB/CES"
#folderPath = "/TILE/OFL02/TIME/CHANNELOFFSET/PHY"
#folderPath = "/TILE/OFL02/NOISE/SAMPLE"
#tag = "RUN2-HLT-UPD1-01"
#folderPath = "/TILE/ONL01/CALIB/CIS/LIN"
#folderPath = "/TILE/ONL01/MUID"
#folderPath = "/TILE/ONL01/FRAG1"
#tag = ""
#
# if folder tag is full tag like TileOfl02CalibCisLin-RUN2-UPD4-23
# or TILEONL01CALIBCES-HEAD folder path is not needed
#==================================================

from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobObjs.Classes import TileCalibUtils
import os, sys
from builtins import input
os.environ['TERM'] = 'linux'

#------------------------------- parse arguments and change defaults
import argparse

parser = argparse.ArgumentParser(
    prog='ReadFromCrestCompare.py',
    description='Read conditions from CREST server (or/and JSON files), compare values and dump differences above specified thresholds',
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog="""
Notes:
- Conditions are compared using logical AND: both maxdiff AND maxdiffpercent must be present (both have to be TRUE) to print
- If *2 parameters (run2, lumi2, folder2, tag2, schema2) are not given, values from primary parameters are used
- For single-version folders, use empty tag: --tag=""

Usage:
  # Compare values in last IOV in two different tags
  # tag name can be short, it'll be resolved to proper current tag
  %(prog)s \\
           --maxdiffpercent=0.01 \\
           --folder=/TILE/OFL02/CALIB/CIS/LIN \\
           --tag=UPD1 \\
           --tag2=UPD4

  # Compare CREST server vs JSON file
  # for JSON file folder name and tag are not needed
  %(prog)s \\
           --maxdiff=100 \\
           --run=500000 \\
           --folder=/TILE/ONL01/CALIB/CES \\
           --tag='' \\
           --jsonfn2=tileCalib.json

  # Compare two runs from same tag/schema
  # folder name is not needed if tag is full tag
  %(prog)s \\
           --tag=TileOfl02CalibLasLin-RUN2-UPD4-26 \\
           --maxdiffpercent=5.5 \\
           --run=400000 \\
           --run2=999999999
    """
)

# primary parameters
parser.add_argument('--run', '-r', type=int, default=999999999,
                    help='Run number for first comparison (default: 999999999)')
parser.add_argument('--lumi', '-l', type=int, default=0,
                    help='Luminosity block for first comparison (default: 0)')
parser.add_argument('--folder', '-f', dest='folder', default="/TILE/OFL02/CALIB/CES",
                    help='Folder path for first comparison (default: /TILE/OFL02/CALIB/CES)')
parser.add_argument('--tag', '-t', default="RUN2-HLT-UPD1-01",
                    help='Tag name for first comparison (default: RUN2-HLT-UPD1-01)')
parser.add_argument('--schema', '-s', default="CREST",
                    help='CREST URL or JSON file path for first comparison (default: CREST from env)')
parser.add_argument('--jsonfn', '-j', default="none",
                    help='JSON file path for first comparison (overrides schema if file exists)')

# secondary parameters (*2) - default to primary values if not specified
parser.add_argument('--run2', '-r2', type=int, default=0,
                    help='Run number for second comparison (default: same as --run)')
parser.add_argument('--lumi2', '-l2', type=int, default=-1,
                    help='Luminosity block for second comparison (default: same as --lumi)')
parser.add_argument('--folder2', '-f2', default="none",
                    help='Folder path for second comparison (default: same as --folder)')
parser.add_argument('--tag2', '-t2', default="none",
                    help='Tag name for second comparison (default: same as --tag)')
parser.add_argument('--schema2', '-s2', default="none",
                    help='CREST URL or JSON file path for second comparison (default: same as --schema)')
parser.add_argument('--jsonfn2', '-j2', default="none",
                    help='JSON file path for second comparison (use "same" to use same as --jsonfn)')

# threshold parameters
parser.add_argument('--maxdiff', type=float, default=-1.0,
                    help='Maximum absolute difference threshold (default: -1.0 = dump all)')
parser.add_argument('--maxdiffpercent', type=float, default=-1.0,
                    help='Maximum percent difference threshold (default: -1.0 = dump all)')
# other parameters
parser.add_argument('--stdout', '-o', action='store_true',
                    help='Print differences to standard output (in addition to file)')

args = parser.parse_args()

# apply default logic for *2 parameters (can be modified from command line)
run = args.run
run2 = args.run2 if args.run2 != 0 else run
lumi = args.lumi
lumi2 = args.lumi2 if args.lumi2 >= 0 else lumi
folderPath = args.folder
folderPath2 = args.folder2 if args.folder2 != "none" else folderPath
tag = args.tag
tag2 = args.tag2 if args.tag2 != "none" else tag
schema = args.schema
schema2 = args.schema2 if args.schema2 != "none" else schema
jsonfn = args.jsonfn
jsonfn2 = args.jsonfn2 if args.jsonfn2 != "same" else jsonfn
maxdiff = args.maxdiff
maxdiffpercent = args.maxdiffpercent
print_to_stdout = args.stdout

if tag.upper().startswith('TILE'):
    folderPath=''
if tag2.upper().startswith('TILE'):
    folderPath2=''

if (jsonfn != 'none' and os.path.isfile(jsonfn)):
    tag=''
    folderPath=''
    schema=''
elif (schema != 'CREST' and os.path.isfile(schema)):
    tag=''
    folderPath=''
if (jsonfn2 != 'none' and os.path.isfile(jsonfn2)):
    tag2=''
    folderPath2=''
    schema2=''
elif (schema2 != 'CREST' and os.path.isfile(schema2)):
    tag2=''
    folderPath2=''

print("\n" + "="*65)
print("  ReadFromCrestCompare - Configuration")
print("="*65)
print(f"  {'Run/Lumi:':<14} 1st: run={run:<10} lumi={lumi:<6}")
print(f"  {' ':<14} 2nd: run={run2:<10} lumi={lumi2:<6}")
print(f"  {'Thresholds:':<14} maxdiff={maxdiff:<8} maxdiffpercent={maxdiffpercent}")
print(f"  {'Folder:':<14} {folderPath}")
print(f"  {'Folder2:':<14} {folderPath2}")
print(f"  {'Tag:':<14} {tag}")
print(f"  {'Tag2:':<14} {tag2}")
print(f"  {'Schema:':<14} {schema}")
print(f"  {'Schema2:':<14} {schema2}")
print(f"  {'JSON File:':<14} {jsonfn}")
print(f"  {'JSON File2:':<14} {jsonfn2}")
print("="*65 + "\n")
    
#===================================================================
#====================== FILL DB parameters BELOW ===================
#===================================================================
#--- Read from CREST server or from local JSON file:

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("ReadFromCrest")
import logging
log.setLevel(logging.DEBUG)

#==================================================

if jsonfn != 'none' and os.path.isfile(jsonfn):
    db = jsonfn
else:
    db = schema

if jsonfn2 != 'none' and os.path.isfile(jsonfn2):
    db2 = jsonfn2
else:
    db2 = schema2
    
# f=open('output.ascii', 'w')
f=open('output_crest.ascii', 'w')
if run2!=run and tag2==tag and folderPath2==folderPath and ("/TIME" in folderPath or "TIME" in tag.upper()):
    fd=open('from_%d_to_%d.dif'%(run2,run), 'w')
    writedif=True
else:
    writedif=False

try:
    log.info("Connecting to DB1: schema=%s, folder=%s, tag=%s, run=%s, lumi=%s",
             db, folderPath, tag, run, lumi)
    blobReader = TileCalibCrest.TileBlobReaderCrest(db, folderPath, tag, run, lumi)
    log.info("Connecting to DB2: schema=%s, folder=%s, tag=%s, run=%s, lumi=%s",
             db2, folderPath2, tag2, run2, lumi2)
    blobReader2 = TileCalibCrest.TileBlobReaderCrest(db2, folderPath2, tag2, run2, lumi2)
except Exception as e:
    log.error("Initialization failed: %s", e)
    sys.exit(1)

#=== get drawer with status at given run
ros    = 1 # ros 0 sometimes contains obsolete header !
drawer = 0
log.info("Initializing for run1 %d lumi %d run2 %d lumi2 %d maxdiff %f maxdiffpercent %f", run, lumi, run2, lumi2, maxdiff,maxdiffpercent)
log.info("Comment1: %s", blobReader.getComment((run,lumi)))
log.info("Comment2: %s", blobReader2.getComment((run2,lumi2)))

flt=None
r=5
d=0
while not flt:
    d-=1
    if d<0:
        r-=1
        if r<0:
            log.error("No valid drawers in first database")
            sys.exit()
        d=TileCalibUtils.getMaxDrawer(r)-1
    flt = blobReader.getDrawer(r, d, (run,lumi), False, False)
#flt.dump()
ot = flt.getObjType()
ov = flt.getObjVersion()
os = flt.getObjSizeByte()//4
no = flt.getNObjs()
nc = flt.getNChans()
ng = flt.getNGains()

flt2=None
r=5
d=0
while not flt2:
    d-=1
    if d<0:
        r-=1
        if r<0:
            log.error("No valid drawers in second database")
            sys.exit()
        d=TileCalibUtils.getMaxDrawer(r)-1
    flt2 = blobReader2.getDrawer(r, d, (run2,lumi2), False, False)
ot2 = flt2.getObjType()
os2 = flt2.getObjSizeByte()//4

if (os != os2) or (ot != ot2):
    log.error("Object sizes (%s vs %s) or types (%s vs %s) are different", os, os2, ot, ot2)
    answ=input(' continue anyway? (y/n)')
    if (answ != 'y'):
        sys.exit()

v =[]
v2=[]
for ind in range(0,os):
    v.append(0)
    v2.append(0)

f.write("Command line parameters used: \n %s \n" % sys.argv[1:])
f.write("---- Object header for ros=%d drawer=%d \n" % (r,d))
f.write("ObjType        : %d \n" % ot)
f.write("ObjVersion     : %d \n" % ov)
f.write("ObjSize[4bytes]: %d \n" % os)
f.write("NObjs          : %d \n" % no)
f.write("NChannels      : %d \n" % nc)
f.write("NGains         : %d \n" % ng)

answ='n'
#=== get value for a gived channel
for ros in range(0,5):
    for mod in range(0, min(64,TileCalibUtils.getMaxDrawer(ros))):
        modName = TileCalibUtils.getDrawerString(ros,mod)
        #log.info("ros %d, drawer %s at run %d" % (ros, modName, run))
        flt = blobReader.getDrawer(ros, mod,(run,lumi), False, False)
        flt2 = blobReader2.getDrawer(ros, mod,(run2,lumi2), False, False)
        if flt and flt2:
            osc = flt.getObjSizeByte()//4
            os2c = flt2.getObjSizeByte()//4
            oscMax = max(osc,os2c)
            if (((os != osc) or (os2 != os2c)) and answ != 'y'):
                if (ros==0 and osc==os2c and os==os2):
                    log.warning("Object sizes are different for last drawer in DB (%s) and default drawer %s (%s)", os, modName, osc)
                else:
                    log.error("Object sizes are different for last drawer in DB (%s and %s) and drawer %s (%s and %s)", os, os2, modName, osc, os2c)
                    answ=input(' continue anyway? (y/n)')
                    if (answ != 'y'):
                        sys.exit()
                    else:
                        for ind in range(0,oscMax):
                            v.append(0)
                            v2.append(0)


            for chn in range(TileCalibUtils.max_chan()):
                chnName = " %2i" % chn
                for adc in range(ng):
                    for ind in range(0,oscMax):
                        if (ind<osc):
                            v[ind] = flt.getData(chn, adc, ind)
                        if (ind<os2c):
                            v2[ind] = flt2.getData(chn, adc, ind)
                        dv12 = v[ind] - v2[ind]
                        if v2[ind] == 0:
                            if v[ind] == 0:
                                dv12percent=0
                            else:
                                dv12percent=dv12*100./v[ind]
                        else:
                            dv12percent=dv12*100./v2[ind]
                        #print ( modName, ' chann ',  repr(chn),  ' adc ',  repr(adc),  ' ind ',  repr(ind),  ' val1 ',  repr(v[ind]),' val2 ',  repr(v2[ind]), ' diff ',  repr(dv12), 'percent ', repr(dv12percent))
                        if abs(dv12) > maxdiff and abs(dv12percent) > maxdiffpercent:
                            if ot==30: # integers
                                line = '%s chann %2d adc %d ind %d val1 %d val2 %d  diff %d' % (modName,chn,adc,ind,v[ind],v2[ind],dv12)
                            elif ot==20: # bad channels
                                line = '%s chann %2d adc %d ind %d val1 %s val2 %s  diff %f' % (modName,chn,adc,ind,hex(int(v[ind])),hex(int(v2[ind])),dv12)
                            else:       # floats
                                line = '%s chann %2d adc %d ind %d val1 %.4f val2 %.4f  diff %.4f %.2f%%' % (modName,chn,adc,ind,v[ind],v2[ind],dv12,dv12percent)
                                if writedif and adc==0 and ind==0:
                                    fd.write("%s ch %2d %.4f\n" % (modName,chn,dv12))

                            f.write(line + '\n')
                            if print_to_stdout:
                                print(line)

#=== close output file
f.close()
if writedif:
    fd.close()
