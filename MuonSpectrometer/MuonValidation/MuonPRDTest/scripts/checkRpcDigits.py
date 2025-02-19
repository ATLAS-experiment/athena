# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from HitIdentifier import RpcIdentifier
from ROOT import TVector3, TVector2, TFile
from argparse import ArgumentParser

### Representation of a single RPC digit
class RpcDigit(object):
    def __init__(self,
                 stationIndex=-1, stationEta=-1, stationPhi=-1, ###  Station fields of the identifier
                 doubletR = -1, doubletPhi = -1, doubletZ = -1, ###  Fields of the Rpc doublet in station
                 gasGap =-1, measuresPhi = False, strip = -1,   ###  Channel fields
                 globPosX= -1, globPosY = -1., globPosZ = -1,   ### Global position of the associated channel
                 locPosX = -1, locPosY= -1,                     ### Local position of the associated channel in the gasGap frame
                 time =  -1.                                    ### Time of the digit signal     
                 ) -> None:
        self.__id = RpcIdentifier(stationIndex=stationIndex, stationEta = stationEta, stationPhi = stationPhi,
                                  doubletR= doubletR, doubletPhi = doubletPhi, doubletZ = doubletZ,
                                  gasGap = gasGap, measuresPhi = measuresPhi, strip = strip)
        
        self.__globPos = TVector3(globPosX, globPosY, globPosZ)
        self.__locPos = TVector2(locPosX, locPosY)
        self.__time = time
    
    def identify(self): 
        return self.__id

    def globalPosition(self):
        return self.__globPos

    def localPosition(self):
        return self.__locPos

    def time(self):
        return self.__time
    
    def __str__(self) -> str:
        return "Rpc digit {id}, time {time:.2f}, localPos ({locX:.2f}, {locY:.2f}) globalPos ({globX:.2f}, {globY:.2f}, {globZ:.2f})".format(id = str(self.identify()),
                                                                                    time = self.__time,
                                                                                    locX = self.localPosition().X(),
                                                                                    locY = self.localPosition().Y(),
                                                                                    globX = self.globalPosition().X(),
                                                                                    globY = self.globalPosition().Y(),
                                                                                    globZ = self.globalPosition().Z())
    def __eq__(self, other) -> bool:
        return self.identify() == other.identify() and \
               (self.localPosition() - other.localPosition()).Mod() < 0.01 

#### Representation of the Rpc SDO
class RpcSDO(RpcDigit):
    def __init__(self, stationIndex=-1, stationEta=-1, stationPhi=-1, 
                       doubletR=-1, doubletPhi=-1, doubletZ=-1, gasGap=-1, 
                       measuresPhi=False, strip=-1, 
                       globPosX=-1, globPosY=-1, globPosZ=-1, 
                       locPosX=-1, locPosY=-1, 
                       time=-1, barcode = -1) -> None:
        super().__init__(stationIndex, stationEta, stationPhi, 
                         doubletR, doubletPhi, doubletZ, 
                         gasGap, measuresPhi, strip, 
                         globPosX, globPosY, globPosZ, 
                         locPosX, locPosY, time)
        
        self.__barcode = barcode
    def barcode (self):
        return self.__barcode
    
    def __str__(self) -> str:
        return "Rpc SDO {id}, time {time:.2f}, localPos ({locX:.2f}, {locY:.2f}), globalPos ({globX:.2f}, {globY:.2f}, {globZ:.2f}), barcode {barcode}".format(id = str(self.identify()),
                                                                                    time = self.time(),
                                                                                    locX = self.localPosition().X(),
                                                                                    locY = self.localPosition().Y(),
                                                                                    globX = self.globalPosition().X(),
                                                                                    globY = self.globalPosition().Y(),
                                                                                    globZ = self.globalPosition().Z(),
                                                                                    barcode = self.barcode())
   
def readDigitTree(in_file, treeName = "HitValidTree"):

    t_file = TFile.Open(in_file, "READ")
    if not t_file:
        print(f"Failed to open {in_file}")
        exit(1)
    tree = t_file.Get(treeName) 
    if not tree:
        print("Failed to read tree {treeName} from {file}".format(treeName=treeName, file=t_file.GetName()))
        exit(1)
    allDigits = {}
    for entry in range(tree.GetEntries()):
        tree.GetEntry(entry)
        evt= tree.eventNumber
        digits = []
        for digit in range(len(tree.Digits_RPC_stationIndex)):
            digits.append(RpcDigit(stationIndex = ord(tree.Digits_RPC_stationIndex[digit]),
                                   stationEta =  ord(tree.Digits_RPC_stationEta[digit]),
                                   stationPhi = ord(tree.Digits_RPC_stationPhi[digit]),
                                   doubletR = ord(tree.Digits_RPC_doubletR[digit]),
                                   doubletPhi= ord(tree.Digits_RPC_doubletPhi[digit]),
                                   gasGap = ord(tree.Digits_RPC_gasGap[digit]),
                                   measuresPhi=tree.Digits_RPC_measuresPhi[digit],
                                   strip = ord(tree.Digits_RPC_strip[digit]),
                                   globPosX= tree.Digits_RPC_globalPosX[digit],
                                   globPosY= tree.Digits_RPC_globalPosY[digit],
                                   globPosZ= tree.Digits_RPC_globalPosZ[digit],
                                   
                                   locPosX = tree.Digits_RPC_localPosX[digit],
                                   locPosY = tree.Digits_RPC_localPosY[digit],
                                   time = tree.Digits_RPC_time[digit]
                                   ))
            if digits[-1].identify().stationName() == "BIS": 
                digits.pop()
        if len (digits): allDigits[evt] = digits
    return allDigits

def readSDOTree(in_file, treeName = "HitValidTree"):
    allSDOS = {}    
    t_file = TFile.Open(in_file, "READ")
    if not t_file:
        print(f"Failed to open {in_file}")
        exit(1)
    tree = t_file.Get(treeName) 
    if not tree:
        print("Failed to read tree {treeName} from {file}".format(treeName=treeName, file=t_file.GetName()))
        exit(1)
    allSDOS = {}
    for entry in range(tree.GetEntries()):
        tree.GetEntry(entry)
        evt= tree.eventNumber

        SDOs = []
        for sdo in range(len(tree.SDO_RPC_stationIndex)):
            SDOs.append(RpcSDO(stationIndex = ord(tree.SDO_RPC_stationIndex[sdo]),
                               stationEta =  ord(tree.SDO_RPC_stationEta[sdo]),
                               stationPhi = ord(tree.SDO_RPC_stationPhi[sdo]),
                               doubletR = ord(tree.SDO_RPC_doubletR[sdo]),
                               doubletPhi = ord(tree.SDO_RPC_doubletPhi[sdo]),
                               gasGap = ord(tree.SDO_RPC_gasGap[sdo]),
                               measuresPhi=tree.SDO_RPC_measuresPhi[sdo],
                               strip = ord(tree.SDO_RPC_strip[sdo]),
                               globPosX = tree.SDO_RPC_globalPosX[sdo],
                               globPosY = tree.SDO_RPC_globalPosY[sdo],
                               globPosZ = tree.SDO_RPC_globalPosZ[sdo],
                               
                               locPosX = tree.SDO_RPC_localPosX[sdo],
                               locPosY = tree.SDO_RPC_localPosY[sdo],
                               #time = tree.SDO_RPC_global_time[sdo],
                               barcode=tree.SDO_RPC_barcode[sdo]
                               ))

        if len(SDOs): 
            allSDOS[evt] = SDOs

    return allSDOS

if __name__ == "__main__":
    
    parser = ArgumentParser(prog="checkRpcDigits", description="Script comparing the Rpc digitization validation n-tuples")
    parser.add_argument("--refFile", help="Reference file location", default = "/media/slowSSD/jojungge/ATLPHYSVAL-1018/SimulationTest/24.0.35/NSWPRDValAlg.digi.ntuple.root", type = str)
    parser.add_argument("--testFile", help="Reference file location", default = "/media/slowSSD/jojungge/ATLPHYSVAL-1018/SimulationTest/25.0.5/NSWPRDValAlg.digi.ntuple.root", type = str)
    parser.add_argument("--outFile", help="Location where the out file is stored", default="DigitComparison.txt")
    args = parser.parse_args()
    
    refDigits = readDigitTree(args.refFile)
    testDigits = readDigitTree(args.testFile)

    refSDOs = readSDOTree(args.refFile)
    testSDOs = readSDOTree(args.testFile)

    outFile = open(args.outFile, "w")
    
    ### Loop over the SDOs & Digits
    allGood = True
    for event, refContent in refDigits.items():
        if not event in testDigits:
            print ("WARNING {event} is not part of the test file {testFile}".format(event = event, testFile=args.testFile))
            continue
        testContent = testDigits[event]

        for refObj in refContent:
            if refObj.identify().measuresPhi():
                continue
            if refObj not in testContent:                
                errorStr ="In Event {event}, {digit} cannot be found in the test sample. Potential candidates: \n".format(digit = str(refObj),
                                                                                                                          event = event)
                for testObj in testContent:
                    if testObj.identify().gasGapID() == refObj.identify().gasGapID():
                        errorStr += "     **** {digit} \n".format(digit = str(testObj))
                print(errorStr)
                outFile.write("{msg}\n".format(msg= errorStr))
                allGood = False
    
    exit(allGood == False)
        