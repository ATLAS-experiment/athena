#!/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# ReadCellNoiseFromCrestCompare.py
# (Based on ReadCellNoiseFromCoolCompare.py, adapted for CREST)
# Ekaterina Ramakoti 2026-05-21
#

import sys,os
os.environ['TERM'] = 'linux'

#------------------------------- parse arguments and change defaults
import argparse

parser = argparse.ArgumentParser(
    prog='ReadCellNoiseFromCrestCompare.py',
    description='Dumps noise constants from online or offline folders/tags in CREST and compares two sources',
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog="""
Notes:
- Conditions are compared using logical AND: both --maxdiff AND --maxdiffpercent
  thresholds must be exceeded (both have to be TRUE) to print a difference
- If *2 parameters (run2, lumi2, folder2, tag2, schema2) are not given,
  values from primary parameters are used automatically
- Use --cell, --gain, --index with -1 to process all values

Usage:
  # Compare two tags from same folder
  python %(prog)s -f /TILE/OFL02/NOISE/CELL -t UPD4 -t2 UPD1 -r 497970

  # Compare specific cell/gain with thresholds
  python %(prog)s -f /TILE/OFL02/NOISE/CELL -t UPD4 -c 123 -g 0 --maxdiff 0.001 --maxdiffpercent 5.0

  # Wide format, brief output, all cells
  python %(prog)s -f /TILE/OFL02/NOISE/CELL -t UPD4 -w -b -r 497970
    """
)

# primary source parameters
parser.add_argument('-s', '--schema', default='CREST',
                    help='CREST server path or schema (default: CREST from env)')
parser.add_argument('-f', '--folder', default='/TILE/OFL02/NOISE/CELL',
                    help='Status folder to use, e.g., /TILE/OFL02/NOISE/CELL')
parser.add_argument('-t', '--tag', default='UPD4',
                    help='Tag to use, e.g., UPD4 or tag suffix like 14TeV-N200_dT50-01')
parser.add_argument('-r', '--run', type=int, default=2147483647,
                    help='Run number (default: 2147483647, uses latest IOV)')
parser.add_argument('-l', '--lumi', type=int, default=0,
                    help='Luminosity block number (default: 0)')

# secondary parameters (*2) - default to primary values if not specified
parser.add_argument('-s2', '--schema2', default='none',
                    help='Second schema for comparison (default: same as --schema)')
parser.add_argument('-f2', '--folder2', default='none',
                    help='Second folder for comparison (default: same as --folder)')
parser.add_argument('-t2', '--tag2', default='none',
                    help='Second tag for comparison (default: same as --tag)')
parser.add_argument('-r2', '--run2', type=int, default=0,
                    help='Second run number (default: same as --run)')
parser.add_argument('-l2', '--lumi2', type=int, default=-1,
                    help='Second lumi block (default: same as --lumi)')

# threshold parameters
parser.add_argument('-m', '--maxdiff', type=float, default=-1.0,
                    help='Absolute maximal difference to compare constants (default: -1.0 = dump all)')
parser.add_argument('-m2', '--maxdiffpercent', type=float, default=-1.0,
                    help='Maximal difference in percent to compare constants (default: -1.0 = dump all)')

# others parameters
parser.add_argument('-n', '--channel', type=int, default=48,
                    help='COOL channel to read (default: 48 for Tile)')
parser.add_argument('-c', '--cell', type=int, default=-1,
                    help='Cell hash 0-5183 (default: -1 = all cells)')
parser.add_argument('-g', '--gain', type=int, default=-1,
                    help='Gain 0-3 (default: -1 = all gains)')
parser.add_argument('-i', '--index', type=int, default=-1,
                    help='Parameter index 0-4 (default: -1 = all parameters)')
parser.add_argument('-z', '--zero', type=float, default=5e-7,
                    help='Zero threshold: treat DB values below this as zeros (default: 5e-7)')
parser.add_argument('-w', '--wide', action='store_true', default=False,
                    help='Wide format: print all values per cell in one line')
parser.add_argument('-b', '--brief', action='store_true', default=False,
                    help='Brief output: print only numbers without character names')
parser.add_argument('-d', '--double', action='store_true', default=False,
                    help='Print values with double precision')
parser.add_argument('--stdout', '-o', action='store_true',
                    help='Print differences to standard output (in addition to file)')

args = parser.parse_args()

# apply default logic for *2 parameters (can be modified from command line)
run = args.run
run2 = args.run2 if args.run2 != 0 else run
lumi = args.lumi
lumi2 = args.lumi2 if args.lumi2 >= 0 else lumi
schema = args.schema
schema2 = args.schema2 if args.schema2 != "none" else schema
folderPath = args.folder
folderPath2 = args.folder2 if args.folder2 != "none" else folderPath
tag = args.tag
tag2 = args.tag2 if args.tag2 != "none" else tag
maxdiff = args.maxdiff
maxdiffpercent = args.maxdiffpercent
chan = args.channel
cell = args.cell
gain = args.gain
index = args.index
zthr = args.zero
multi = not args.wide  # --wide sets multi=False
brief = args.brief
doubl = args.double
print_to_stdout = args.stdout

tile=(chan==48)

print("\n" + "="*65)
print("  ReadCellNoiseFromCrestCompare - Configuration")
print("="*65)
print(f"  {'Run/Lumi:':<14} 1st: run={run:<10} lumi={lumi:<6}")
print(f"  {' ':<14} 2nd: run={run2:<10} lumi={lumi2:<6}")
print(f"  {'Folder:':<14} {folderPath}")
print(f"  {'Folder2:':<14} {folderPath2}")
print(f"  {'Tag:':<14} {tag}")
print(f"  {'Tag2:':<14} {tag2}")
print(f"  {'Schema:':<14} {schema}")
print(f"  {'Schema2:':<14} {schema2}")
print(f"  {'Channel:':<14} {chan}")
print(f"  {'Cell/Gain/Idx:':<14} {cell}/{gain}/{index}")
print(f"  {'Thresholds:':<14} maxdiff={maxdiff:<8} maxdiffpercent={maxdiffpercent}")
print(f"  {'Zero thr:':<14} {zthr:.2e}")
print(f"  {'Format:':<14} wide={args.wide}, brief={brief}, double={doubl}")
print("="*65 + "\n")

from CaloCondBlobAlgs import CaloCondLogger
from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobPython import TileCellTools

#=== get a logger
log = CaloCondLogger.getLogger("ReadCellNoise")
import logging
log.setLevel(logging.DEBUG)

if run>=400000:
    cabling = 'RUN3'
elif run>=342550:
    cabling = 'RUN2a'
elif run>=222222:
    cabling = 'RUN2'
else:
    cabling = 'RUN1'
# hashMgr=TileCellTools.TileCellHashMgr(cabling=cabling)
hashMgrDef=TileCellTools.TileCellHashMgr(cabling=cabling)
hashMgrA=TileCellTools.TileCellHashMgr("UpgradeA")
hashMgrBC=TileCellTools.TileCellHashMgr("UpgradeBC")
hashMgrABC=TileCellTools.TileCellHashMgr("UpgradeABC")

#=== Initialize blob reader for sources
folderTag = tag
tag_upper = tag.upper()
tag1 = tag_upper.split('_')[1][:4] if '_' in tag_upper else tag_upper[:4]
if tag1 == "TILE" or tag1 == "CALO" or tag1[:3] == "LAR" or tag.startswith("TILE") or tag.startswith("CALO") or tag.startswith("LAR"):
    folderPath1=""
else:
    folderPath1=folderPath
if not os.path.isfile(schema):
    log.info("Initializing folder %s with tag %s (%s)", folderPath1, folderTag, tag1)

folderTag2 = tag2
tag2_upper = tag2.upper()
tag2_1 = tag2_upper.split('_')[1][:4] if '_' in tag2_upper else tag2_upper[:4]
if tag2_1 == "TILE" or tag2_1 == "CALO" or tag2_1[:3] == "LAR" or tag2.startswith("TILE") or tag2.startswith("CALO") or tag2.startswith("LAR"):
    folderPath2=""
else:
    folderPath2=folderPath2
if not os.path.isfile(schema2):
    log.info("Initializing folder2 %s with tag2 %s (%s)", folderPath2, folderTag2, tag2_1)
    
try:
    blobReader = TileCalibCrest.TileBlobReaderCrest(schema, folderPath1, folderTag, run, lumi)
    log.debug("Connecting to DB: schema=%s, folder=%s, tag=%s, run=%s, lumi=%s", 
            schema, folderPath1, folderTag, run, lumi)
    log.info("Comment1: %s", blobReader.getComment((run,lumi)))
    blobReader2 = TileCalibCrest.TileBlobReaderCrest(schema2, folderPath2, folderTag2, run2, lumi2)
    log.debug("Connecting to DB2: schema=%s, folder=%s, tag=%s, run=%s, lumi=%s", 
            schema2, folderPath2, folderTag2, run2, lumi2)
    log.info("Comment1: %s", blobReader2.getComment((run2,lumi2)))
except Exception as e:
    log.error("Initialization failed: %s", e)
    sys.exit(1)

#=== create CaloCondBlobFlt
blobFlt = blobReader.getDrawer(-1,chan,None,False,False)
if blobFlt is None:
    log.critical("Could not locate a data blob in CREST payload for COOL channel %s", chan)
    sys.exit(1)

blobFlt2 = blobReader2.getDrawer(-1,chan,None,False,False)
if blobFlt2 is None:
    log.critical("Could not locate a data blob in CREST payload for COOL channel %s", chan)
    sys.exit(1)

#=== create the output file
outputfile = 'output_crest.ascii'
f = open(outputfile, 'w')

f.write("Command line parameters used: \n %s \n\n" % sys.argv[1:])
f.write("=== Source 1 ===\n")
f.write("schema   : %s\n" % schema)
f.write("folder   : %s\n" % folderPath1)
f.write("tag      : %s\n" % folderTag)
f.write("run      : %d\n" % run)
f.write("lumi     : %d\n" % lumi)
f.write("channel  : %d\n" % chan)
f.write("comment  : %s\n\n" % blobReader.getComment((run,lumi)))

f.write("=== Source 2 ===\n")
f.write("schema   : %s\n" % schema2)
f.write("folder   : %s\n" % folderPath2)
f.write("tag      : %s\n" % folderTag2)
f.write("run      : %d\n" % run2)
f.write("lumi     : %d\n" % lumi2)
f.write("channel  : %d\n" % chan)
f.write("comment  : %s\n\n" % blobReader2.getComment((run2,lumi2)))

f.write("=== Comparison parameters ===\n")
f.write("maxdiff          : %f\n" % maxdiff)
f.write("maxdiffpercent   : %f\n\n" % maxdiffpercent)
f.write("zero threshold   : %e\n\n" % zthr)


#=== retrieve data from the blob
#cell  = 0 # 0..5183 - Tile hash
#gain  = 0 # 0..3    - four Tile cell gains: -11, -12, -15, -16
#index = 0 # 0..4    - electronic or pile-up noise or 2-G noise parameters
ncell=blobFlt.getNChans()
ngain=blobFlt.getNGains()
nval=blobFlt.getObjSizeUint32()

if cell<0 or cell>=ncell:
    cellmin=0
    cellmax=ncell
else:
    cellmin=cell
    cellmax=cell+1

if gain<0 or gain>=ngain:
    gainmin=0
    gainmax=ngain
else:
    gainmin=gain
    gainmax=gain+1

if index<0 or index>=nval:
    indexmin=0
    indexmax=nval
else:
    indexmin=index
    indexmax=index+1

log.info("From DB:  ncell: %d ngain %d index nval %d", ncell, ngain, nval)

if brief or doubl:
    name1 = ["","","0.0     "]
    names = ["S0 ", "Pl ", "S1 ", "S2 ", "Ra "]
    dm=" "
else:
    name1 = ["Noise cell ", "gain ","0.00    "]
    names = ["   RMS ", "pileup ", "  RMS1 ", "  RMS2 ", " Ratio "]
    for i in range(len(names),indexmax):
        names += ["c"+str(i)+" "]
    dm="\t"

# Use appropriate hashMgr based on cabling
if ncell>hashMgrA.getHashMax():
    hashMgr=hashMgrABC
elif ncell>hashMgrBC.getHashMax():
    hashMgr=hashMgrA
elif ncell>hashMgrDef.getHashMax():
    hashMgr=hashMgrBC
else:
    hashMgr=hashMgrDef
log.info("Using %s CellMgr with hashMax %d", hashMgr.getGeometry(),hashMgr.getHashMax())

for cell in range(cellmin,cellmax):
    if tile and len(name1[0]):
        name1[0] = "%s %6s hash " % hashMgr.getNames(cell)
    for gain in range(gainmin,gainmax):
        msg="%s%4d %s%d\t" % ( name1[0], cell, name1[1], gain)
        l0=len(msg)
        if multi:
            dm="\n"+msg
        for index in range(indexmin,indexmax):
            v=blobFlt.getData(cell, gain, index)
            v2=blobFlt2.getData(cell, gain, index)
            dv12 = v - v2
            if abs(dv12)<zthr:
                dv12 = 0
            if v2 == 0:
                if v==0:
                    dp12=0
                else:
                    dp12=100
            else:
                dp12=dv12*100./v2

            if abs(dv12) > maxdiff and abs(dp12) > maxdiffpercent:
                if doubl:
                    s1 = "{0:<14.9g}".format(v)    if    v<0 else "{0:<15.10g}".format(v)
                    s2 = "{0:<14.9g}".format(v2)   if   v2<0 else "{0:<15.10g}".format(v2)
                    s3 = "{0:<14.9g}".format(dv12) if dv12<0 else "{0:<15.10g}".format(dv12)
                    s4 = "{0:<14.9g}".format(dp12) if dp12<0 else "{0:<15.10g}".format(dp12)
                    msg += "%s v1 %s v2 %s diff %s diffpercent %s%s" % (names[index],s1.ljust(15),s2.ljust(15),s3.ljust(15),s4.ljust(15),dm)
                else:
                    s1 = name1[2] if    abs(v)<zthr else "%8.6f" %    v if    abs(v)<1 else "{0:<8.7g}".format(v).ljust(8)
                    s2 = name1[2] if   abs(v2)<zthr else "%8.6f" %   v2 if   abs(v2)<1 else "{0:<8.7g}".format(v2).ljust(8)
                    s3 = name1[2] if abs(dv12)<zthr else "%8.6f" % dv12 if abs(dv12)<1 else "{0:<8.7g}".format(dv12).ljust(8)
                    s4 = name1[2] if abs(dp12)<zthr else "%8.6f" % dp12 if abs(dp12)<1 else "{0:<8.7g}".format(dp12).ljust(8)
                    msg += "%s v1 %s v2 %s diff %s diffpercent %s%s" % (names[index],s1[:8],s2[:8],s3[:8],s4[:8],dm)

        if len(msg)>l0:
            output_line = msg[:len(msg)-len(dm)]
            f.write(output_line + "\n")
            if print_to_stdout:
                print(output_line)

f.close()
log.info("Output written to %s", outputfile)