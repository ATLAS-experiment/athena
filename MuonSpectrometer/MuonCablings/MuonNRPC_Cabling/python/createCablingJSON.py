# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import json
from copy import deepcopy

###     Simple script to generate a BIS78 cabling map as used for the
###     Monte Carlo processing. The channel numbers are place holders
###     for the final ones
###     This cabling file describes the conenction between a set of channels of the flat cable and a set of detector strips, which is done sequentially.
###     The nominal connection between TDC channelas anc Flat-Fable channels is specified in the following mapping (Flat-Cable-channel:TDC-channel)
###     dictionary = {0x0000:0, 0x0004:1, 0x0008:2, 0x000c:3, 0x0010:4, 0x0014:5, 0x0018:6, 0x001c:7, 0x0001:8, 0x0005:9, 0x0009:10, 0x000d:11, 0x0011:12, 0x0015:13, 0x0019:14, 0x001d:15, 0x0002:16, 0x0006:17, 0x000a:18, 0x000e:19, 0x0012:20, 0x0016:21, 0x001a:22, 0x001e:23, 0x0003:24, 0x0007:25, 0x000b:26, 0x000f:27, 0x0013:28, 0x0017:29, 0x001b:30, 0x001f:31}
###     By default this scrip associates the nominal TDC-FlatCable cabling ("tdcToFlatCableChan") to all TDCs, but this can be modified if needed.
###     In order to have the mapping code working correctly, it is important that the full TDC-FlatCable mapping information is associated to each TDC, even if only a sub-set of TDC channels are connected to a set of strips
station_name = "BIS"
eta_index = 7  # To represent BIS78 (Station eta, always 7 for BIS78)
doubletR = 1   # Chamber mounted below the Mdts (Always 1 for BIS78)
doubletPhi = 1  # Single phi module (Always 1 for BIS78)
sub_detector = 101 # 0x65 (same as the RPCs)
idTranslateDict = []


### Now write the flat cabling
flatCableBIS78 = [0, 4, 8, 12, 16, 20, 24, 28, 1, 5, 9, 13, 17, 21, 25, 29, 2, 6, 10, 14, 18, 22, 26, 30, 3, 7, 11, 15, 19, 23, 27, 31] 
readoutDict =[
{
    "flatCableId" : 0,
    "pinAssignment" :  [(p+1,t) for t, p  in enumerate(flatCableBIS78)]
},
{
    "flatCableId" : 2,
    "pinAssignment" : [(s,t) for s, t in enumerate([ 0, 8, 16, 24,  1,  9, 17, 25],1)]
},
{
    "flatCableId" : 1,
    "pinAssignment" : [(s,t) for s,t in enumerate([4,12,20,28,5,13,  21,  29,   6,  14,  22,  30,   7,  15,  23,  31],1)]
},



]


for sector in range(1, 9):
    for doubZ in [1, 2]:
        for measPhi in [0,1]:
            for gasGap in [0, 1, 2]:
                cabling_data = {
                    ### Offline part of the identifier
                    "station": station_name,
                    "eta": eta_index,
                    "phi" : sector,
                    "doubletR" : doubletR,
                    "doubletZ" : doubZ,
                    "doubletPhi": doubletPhi,
                    "measPhi": measPhi,
                    "gasGap": gasGap + 1,
                    ### Online part
                    "subDetector": sub_detector,
                    "boardSector" : sector*2+16, # sector A2 -> 0x0012, sector A4 -> 0x0014, ..., sector A12 -> 0x001c, ...,           
                    "flatCableId" : 0,
                }
                ### TDC 
                if measPhi == 0:   # Eta
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 1
                        """
                        cabling_data["lastStrip"] = 32  # Standard: 32 eta strips for BIS7 -> special case: BIS7 with 40 eta strips added below
                        cabling_data["firstTdcChan"] = 0
                        cabling_data["lastTdcChan"] = 31  # Standard: 32 channels per TDC
                        """
                        cabling_data["board"] = gasGap
                    elif doubZ == 2: ## BIS8
                        cabling_data["firstStrip"] = 1  # MN: Problem: should firstStrip be -15, to start with strip 1 in channel 17?
                        """                        
                        cabling_data["lastStrip"] = 16  # Standard: 16 eta strips for BIS8 cabled on the second half of the TDC (that means the strips 1-16)
                        cabling_data["firstTdcChan"] = 16
                        cabling_data["lastTdcChan"] = 31  # Last 16 TDC channels
                        """
                        cabling_data["board"] = gasGap + 9
                        cabling_data["flatCableId"] = 1
                    idTranslateDict.append(deepcopy(cabling_data))

                if measPhi == 1:   # Phi
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 1
                        """
                        cabling_data["lastStrip"] = 32  # Total 64 phi strips for BIS7 -> First half
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 32  # Standard: 32 channels per TDC
                        """
                        cabling_data["board"] = gasGap + 3
                    elif doubZ == 2: ## BIS8
                        cabling_data["firstStrip"] = 1
                        """
                        cabling_data["lastStrip"] = 32  # Total 64 phi strips for BIS7 -> First half
                        cabling_data["firstTdcChan"] = 0
                        cabling_data["lastTdcChan"] = 31  # Standard: 32 channels per TDC
                        """
                        cabling_data["board"] = gasGap + 12
                    idTranslateDict.append(deepcopy(cabling_data))
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 33
                        """
                        cabling_data["lastStrip"] = 64  # Total 64 phi strips for BIS7 -> Second half
                        cabling_data["firstTdcChan"] = 0
                        cabling_data["lastTdcChan"] = 31  # Standard: 32 channels per TDC
                        """
                        cabling_data["board"] = gasGap + 6
                    elif doubZ == 2: ## BIS8
                        cabling_data["firstStrip"] = 33
                        """
                        cabling_data["lastStrip"] = 64  # Total 64 phi strips for BIS7 -> Second half
                        cabling_data["firstTdcChan"] = 0
                        cabling_data["lastTdcChan"] = 31  # Standard: 32 channels per TDC
                        """
                        cabling_data["board"] = gasGap + 15

                    idTranslateDict.append(deepcopy(cabling_data))

# MN: Should we allow the doubling of TDC when different chambers are cabled in the same TDC?
# MN: need to review finalize() checks in: MuonSpectrometer/MuonCablings/MuonCablingData/src/MuonNRPC_CablingMap.cxx
# MN: For the moment using a different TDC number (WRONG!) to test the code
                if measPhi == 0:   # Eta
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 33
                        """
                        cabling_data["lastStrip"] = 40  # Special case: BIS7 with 40 eta strips -> Adding the 8 leftover strips
                        cabling_data["firstTdcChan"] = 0
                        cabling_data["lastTdcChan"] = 7  # First 8 channels of the TDC used for BIS8 eta strips
                        """
                        cabling_data["board"] = gasGap + 9
                        cabling_data["flatCableId"] = 2
                        idTranslateDict.append(deepcopy(cabling_data))


print (len(idTranslateDict))

json_dict= {"chamberMap" : idTranslateDict,
            "readoutCards":  readoutDict }
with open("CablingFile.json", "w") as my_file:
    my_file.write(json.dumps(json_dict,indent=4))
