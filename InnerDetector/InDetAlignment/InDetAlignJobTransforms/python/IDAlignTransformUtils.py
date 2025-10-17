# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignJobTransforms/scripts/IDAlignTransformUtils.py
# Author: David Brunner (david.brunner@cern.ch)

import logging
msg = logging.getLogger('IDAlign')

from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
import PyJobTransforms.trfArgClasses as trfArgClasses

def addIDAlignArguments(parser):
    parser.defineArgGroup('ID alignment', 'ID alignment job flags')
    
    ## Type of running mode
    parser.add_argument('--accumulate', const = trfArgClasses.argBool(True), default = trfArgClasses.argBool(False), nargs='?', type = trfArgClasses.argFactory(trfArgClasses.argBool), group = "ID alignment", help='Run accumulation step')
    parser.add_argument('--solve', const = trfArgClasses.argBool(True), default = trfArgClasses.argBool(False), nargs='?', type = trfArgClasses.argFactory(trfArgClasses.argBool), group = "ID alignment", help='Run solve step')
    parser.add_argument('--dryRun', const = trfArgClasses.argBool(True), default = trfArgClasses.argBool(False), nargs='?', type = trfArgClasses.argFactory(trfArgClasses.argBool), group = "ID alignment", help='Only configure, print and dont execute')
    parser.add_argument('--baseDir', default = trfArgClasses.argString("./"), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Base dir where output is placed')
    parser.add_argument("--iteration", default = trfArgClasses.argInt(0), type = trfArgClasses.argFactory(trfArgClasses.argInt), group = "ID alignment", help = "Current iteration")
    parser.add_argument("--logLevel", default = trfArgClasses.argString("INFO"), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Log level for messages')
    
    ## Input file/track setup for accumulate file
    parser.add_argument("--inputRAWFile", nargs = "+", default = trfArgClasses.argBSFile(defaultTestFiles.RAW_RUN3_DATA24, io = "input", type = "RAW"), type = trfArgClasses.argFactory(trfArgClasses.argBSFile, io = "input", type = "RAW"), group = "ID alignment", help='Input RAW file(s)')
    parser.add_argument("--inputTracksCollection", default = trfArgClasses.argString("CombinedInDetTracks"), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Name of the track collection to use')
    
    ## Output files created in Accumulate/Solve step
    parser.add_argument("--inputTFile", nargs = "+", type = trfArgClasses.argFactory(trfArgClasses.argFile, io = "input", type = "misc"), group = "ID alignment", help='ROOT file produced in MatrixTool in the accumulation step')
    parser.add_argument("--outputTFile", type = trfArgClasses.argFactory(trfArgClasses.argFile, io = "output", type = "misc"), group = "ID alignment", help='ROOT file name outputted in MatrixTool in the accumulation step')
    parser.add_argument("--outputMonitorFile", type = trfArgClasses.argFactory(trfArgClasses.argFile, io = "output", type='hist'), group = "ID alignment", help = 'Name of monitor file to output, if wished')
    parser.add_argument("--outputConditionFile", type = trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io = "output"), group = "ID alignment", help = "Output POOL file with constants created in solve step")
    parser.add_argument("--outputDBFile", type = trfArgClasses.argFactory(trfArgClasses.argFile, io = "output", type = "db"), group = "ID alignment", help = "Output database file with constants created in solve step")
    parser.add_argument("--outputTaredLogFile", type = trfArgClasses.argFactory(trfArgClasses.argFile, io = "output", type = "misc"), group = "ID alignment", help = "Tar zip log files if wished")
    
    ## Local database file to use after initial iteration
    parser.add_argument("--localDatabase", type = trfArgClasses.argFactory(trfArgClasses.argFile, io = "input", type = "db"), group = "ID alignment", help='Local database to use')
    
    ## Tier0 specific tags
    parser.add_argument("--eosT0Dir", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tier0 eos output dir, used to determine solve output while running the CL on Tier0')
    
    ## Alignment setup
    parser.add_argument("--alignLevel", default = trfArgClasses.argInt(11), choices = [trfArgClasses.argInt(11), trfArgClasses.argInt(16), trfArgClasses.argInt(2), trfArgClasses.argInt(3)], type = trfArgClasses.argFactory(trfArgClasses.argInt), group = "ID alignment", help = "Set alignment level")
    parser.add_argument("--excludeIDPart", default = trfArgClasses.argList([]), nargs = "+", choices = ["Pixel", "SCT", "TRT"], type = trfArgClasses.argFactory(trfArgClasses.argList), group = "ID alignment", help = "By default all ID is aligned, exclude some if wished (which may conflict with the set alignment level)")
    
    ## Tags
    parser.add_argument("--globalTag", default = trfArgClasses.argString(defaultConditionsTags.RUN3_DATA), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Global tag')
    parser.add_argument("--beamSpotTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L1IDTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L2PIXTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L2SCTTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L1TRTTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L3SiTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L2TRTTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--L3TRTTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--IBLDistTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--errorScalingTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--lorentzAngleTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--MDNTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--pixelDistortionTag", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--TRTCalibT0TagCos", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--TRTCalibRtTagCos", default = trfArgClasses.argString(""), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Tag to update')
    parser.add_argument("--atlasVersion", default = trfArgClasses.argString(defaultGeometryTags.RUN3), type = trfArgClasses.argFactory(trfArgClasses.argString), group = "ID alignment", help='Global tag')
    
    parser.add_argument("--isBFieldOff", const = trfArgClasses.argBool(True), default = trfArgClasses.argBool(False), nargs='?', type = trfArgClasses.argFactory(trfArgClasses.argBool), group = "ID alignment", help='Check if Bfield is off')
    parser.add_argument("--isCosmics", const = trfArgClasses.argBool(True), default = trfArgClasses.argBool(False), nargs='?', type = trfArgClasses.argFactory(trfArgClasses.argBool), group = "ID alignment", help='Check if cosmics run')
    parser.add_argument("--isHeavyIon", const = trfArgClasses.argBool(True), default = trfArgClasses.argBool(False), nargs='?', type = trfArgClasses.argFactory(trfArgClasses.argBool), group = "ID alignment", help='Check if heavy ion run')
