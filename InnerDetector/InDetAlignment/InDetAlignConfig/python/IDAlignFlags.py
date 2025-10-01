# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/IDAlignFlags.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaCommon.Logging import logging

def createInDetAlignFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("baseDir", "./")
    icf.addFlag("accumulate", True)
    icf.addFlag("doMonitoring", False)
    icf.addFlag("alignPixel", True)
    icf.addFlag("alignSCT", True)
    icf.addFlag("alignTRT", True)
    icf.addFlag("writeAlignNtuple", False)
    icf.addFlag("inputTracksCollection", "CombinedInDetTracks")
    icf.addFlag("pixelAlignmentLevel", -1)
    icf.addFlag("pixelAlignmentLevelBarrel", -1)
    icf.addFlag("pixelAlignmentLevelEndcaps", -1)
    icf.addFlag("SCTAlignmentLevel", -1)
    icf.addFlag("SCTAlignmentLevelBarrel", -1)
    icf.addFlag("SCTAlignmentLevelEndcaps", -1)
    icf.addFlag("TRTAlignmentLevel", -1)
    icf.addFlag("TRTAlignmentLevelBarrel", -1)
    icf.addFlag("TRTAlignmentLevelEndcaps", -1)
    icf.addFlag("beamSpotTag", "")
    icf.addFlag("IBLDistTag", "")
    icf.addFlag("L1IDTag", "")
    icf.addFlag("L2PIXTag", "")
    icf.addFlag("L2SCTTag", "")
    icf.addFlag("L1TRTTag", "")
    icf.addFlag("L3SiTag", "")
    icf.addFlag("L2TRTTag", "")
    icf.addFlag("L3TRTTag", "")
    icf.addFlag("errorScalingTag", "")
    icf.addFlag("lorentzAngleTag", "")
    icf.addFlag("MDNTag", "")
    icf.addFlag("pixelDistortionTag", "")
    icf.addFlag("TRTCalibT0TagCos", "")
    icf.addFlag("TRTCalibRtTagCos", "")
    icf.addFlag("inputTFiles", "AlignmentTFile.root")
    icf.addFlag("outputConditionFile", "alignment_output.pool.root")
    
    return icf
    
def setL11AlignmentFlags(flags, InputLocalDatabase = ""):
    flags.InDet.Align.pixelAlignmentLevel = 11
    flags.InDet.Align.pixelAlignmentLevelBarrel = -1
    flags.InDet.Align.pixelAlignmentLevelEndcaps = -1
    
    flags.InDet.Align.SCTAlignmentLevel = 1
    flags.InDet.Align.SCTAlignmentLevelBarrel = -1
    flags.InDet.Align.SCTAlignmentLevelEndcaps = -1
    
    flags.InDet.Align.TRTAlignmentLevel = 1
    flags.InDet.Align.TRTAlignmentLevelBarrel = -1
    flags.InDet.Align.TRTAlignmentLevelEndcaps = -1

    if InputLocalDatabase:
        msg = logging.getLogger('setL16AlignmentFlags')
        msg.info(f"Change IBLDist tag from '{flags.InDet.Align.IBLDistTag}' to 'InDetAlignIBLDIST-T0-Alignment'")
        msg.info(f"Change L1IDTag tag from '{flags.InDet.Align.L1IDTag}' to 'InDetAlignL1-T0-Alignment'")
            
        flags.InDet.Align.IBLDistTag = "InDetAlignIBLDIST-T0-Alignment"
        flags.InDet.Align.L1IDTag = "InDetAlignL1-T0-Alignment"

def setL16AlignmentFlags(flags, InputLocalDatabase = ""):
    if not flags.InDet.Align.alignPixel:
        raise Exception("With alignment level '16' the flag 'flags.InDet.Align.alignPixel' must be true'")

    flags.InDet.Align.pixelAlignmentLevel = 16
    flags.InDet.Align.pixelAlignmentLevelBarrel = -1
    flags.InDet.Align.pixelAlignmentLevelEndcaps = -1
    
    flags.InDet.Align.SCTAlignmentLevel = 1
    flags.InDet.Align.SCTAlignmentLevelBarrel = -1
    flags.InDet.Align.SCTAlignmentLevelEndcaps = -1
    
    flags.InDet.Align.TRTAlignmentLevel = 1
    flags.InDet.Align.TRTAlignmentLevelBarrel = -1
    flags.InDet.Align.TRTAlignmentLevelEndcaps = -1

    flags.InDet.Align.alignSCT = False
    flags.InDet.Align.alignTRT = False
    
    if InputLocalDatabase:
        msg = logging.getLogger('setL16AlignmentFlags')
        msg.info(f"Change IBLDist tag from '{flags.InDet.Align.IBLDistTag}' to 'InDetAlignIBLDIST-T0-Alignment'")
        msg.info(f"Change L1IDTag tag from '{flags.InDet.Align.L1IDTag}' to 'InDetAlignL1-T0-Alignment'")
    
        flags.InDet.Align.IBLDistTag = "InDetAlignIBLDIST-T0-Alignment"
        flags.InDet.Align.L1IDTag = "InDetAlignL1-T0-Alignment"

## TODO Fill L2 and L3 from current T0 setup
def setL2AlignmentFlags(flags):
    pass  
    
def setL3AlignmentFlags(flags):
    pass  
