# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import json
from copy import deepcopy

###     Simple script to generate a BIS78 cabling map as used for the
###     Monte Carlo processing. The channel numbers are place holders
###     for the final ones
station_name = 1 #BIS
eta_index = 7  # To represent BIS78 (Station eta, always 7 for BIS78)
doubletR = 1   # Chamber mounted below the Mdts (Always 1 for BIS78)
doubletPhi = 1  # Single phi module (Always 1 for BIS78)
sub_detector = 101 # 0x65 (same as the RPCs)
json_dict = []
for sector in range(1, 9):
    for doubZ in [1, 2]:
        for measPhi in [0,1]:
            for gasGap in [1, 2, 3]:
                cabling_data = {
                    ### Offline part of the identifier
                    "station": station_name,
                    "eta": eta_index,
                    "phi" : sector,
                    "doubletR" : doubletR,
                    "doubletZ" : doubZ,
                    "doubletPhi": doubletPhi,
                    "measPhi": measPhi,
                    "gasGap": gasGap,
                    ### Online part
                    "subDetector": sub_detector,
                    "tdcSector" : sector*2+16, # sector A2 -> 0x0012, sector A4 -> 0x0014, ..., sector A12 -> 0x001c, ...,           
                }
                ### TDC 
                if measPhi == 0:   # Eta
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 1
                        cabling_data["lastStrip"] = 32  # Standard: 32 eta strips for BIS7 -> special case: BIS7 with 40 eta strips added below
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 32  # Standard: 32 channels per TDC
                        cabling_data["tdc"] = gasGap 
                    elif doubZ == 2: ## BIS8
                        cabling_data["firstStrip"] = 1  # MN: Problem: should firstStrip be -15, to start with strip 1 in channel 17?
                        cabling_data["lastStrip"] = 16  # Standard: 16 eta strips for BIS8 cabled on the second half of the TDC (that means the strips 1-16)
                        cabling_data["firstTdcChan"] = 17
                        cabling_data["lastTdcChan"] = 32  # Last 16 TDC channels
                        cabling_data["tdc"] = gasGap + 9
                    json_dict.append(deepcopy(cabling_data))

                if measPhi == 1:   # Phi
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 1
                        cabling_data["lastStrip"] = 32  # Total 64 phi strips for BIS7 -> First half
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 32  # Standard: 32 channels per TDC
                        cabling_data["tdc"] = gasGap + 3
                    elif doubZ == 2: ## BIS8
                        cabling_data["firstStrip"] = 1
                        cabling_data["lastStrip"] = 32  # Total 64 phi strips for BIS7 -> First half
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 32  # Standard: 32 channels per TDC
                        cabling_data["tdc"] = gasGap + 12
                    json_dict.append(deepcopy(cabling_data))
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 33
                        cabling_data["lastStrip"] = 64  # Total 64 phi strips for BIS7 -> Second half
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 32  # Standard: 32 channels per TDC
                        cabling_data["tdc"] = gasGap + 6
                    elif doubZ == 2: ## BIS8
                        cabling_data["firstStrip"] = 33
                        cabling_data["lastStrip"] = 64  # Total 64 phi strips for BIS7 -> Second half
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 32  # Standard: 32 channels per TDC
                        cabling_data["tdc"] = gasGap + 15
                    json_dict.append(deepcopy(cabling_data))

# MN: Should we allow the doubling of TDC when different chambers are cabled in the same TDC?
# MN: need to review finalize() checks in: MuonSpectrometer/MuonCablings/MuonCablingData/src/MuonNRPC_CablingMap.cxx
# MN: For the moment using a different TDC number (WRONG!) to test the code
                if measPhi == 0:   # Eta
                    if doubZ == 1: ## BIS7
                        cabling_data["firstStrip"] = 33
                        cabling_data["lastStrip"] = 40  # Special case: BIS7 with 40 eta strips -> Adding the 8 leftover strips
                        cabling_data["firstTdcChan"] = 1
                        cabling_data["lastTdcChan"] = 8  # First 8 channels of the TDC used for BIS8 eta strips
                        cabling_data["tdc"] = gasGap + 9
                        json_dict.append(deepcopy(cabling_data))
print (len(json_dict))
with open("CablingFile.json", "w") as my_file:
    my_file.write(json.dumps(json_dict))
