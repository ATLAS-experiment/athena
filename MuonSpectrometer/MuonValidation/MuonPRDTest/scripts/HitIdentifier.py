# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration



##    Basic class to represent a muon Identifier with the fields stationName, stationPhi, stationEta 
class StationIdentifier(object):
    ### basic constructor taking the stationIndex, stationPhi, station Eta of a muon identifier
    def __init__(self, stationIndex = -1, stationEta = -1, stationPhi = -1) -> None:
        self.__stationIndex = stationIndex
        self.__stationEta = stationEta if stationEta < 128 else stationEta - 255
        self.__stationPhi = stationPhi
    
    def __lt__(self, other) -> bool:
        if self.stationIndex() != other.stationIndex():
            return self.stationIndex() < other.stationIndex()
        if self.stationEta() != other.stationEta():
            return self.stationEta() < other.stationEta()
        return self.stationPhi() < other.stationPhi()
    def __eq__(self, other) -> bool:
        return self.stationIndex() == other.stationIndex() and \
                self.stationEta() == other.stationEta() and \
                self.stationPhi() == other.stationPhi()
        
    def __str__(self):
        return "{stationName}  eta: {stationEta:2} phi: {stationPhi: 2}".format(stationName = self.stationName(),
                                                                                stationEta = self.stationEta(),
                                                                                stationPhi = self.stationPhi())
    def stationIndex(self) -> int:
        return self.__stationIndex

    ### Translates the stationIndex into the stationName  
    def stationName(self) -> str:
        __transDict = { 0: "BIL", 1: "BIS", 7: "BIR",
                        2: "BML", 3: "BMS", 8: "BMF", 53: "BME", 54: "BMG", 52: "BIM",
                        4: "BOL", 5: "BOS", 9: "BOF", 10: "BOG",
                        6: "BEE", 14: "EEL", 15: "EES", 
                        13: "EIL", 
                        17: "EML", 18: "EMS", 
                        20: "EOL", 21: "EOS",
                        41: "T1F", 42: "T1E", 43: "T2F", 44: "T2E",
                        45: "T3F", 46: "T3E", 47: "T4F", 48: "T4E"
                        }
        return __transDict[self.stationIndex()]
    def stationEta(self)-> int:
        return self.__stationEta
    
    def stationPhi(self) -> int:
        return self.__stationPhi
    
### Extension of the station Identifier to cover the Rpc representation
class RpcIdentifier(StationIdentifier):
    def __init__(self, stationIndex=-1, stationEta=-1, stationPhi=-1,
                 doubletR = -1, doubletPhi = -1, doubletZ = -1,
                 gasGap =-1, measuresPhi = False, strip = -1) -> None:
        super().__init__(stationIndex, stationEta, stationPhi)

        self.__doubletR = doubletR
        self.__doubletPhi = doubletPhi
        self.__doubletZ = doubletZ
        self.__gasGap = gasGap
        self.__measuresPhi = measuresPhi
        self.__strip = strip
    
    def doubletR(self):
        return self.__doubletR
    def doubletPhi(self):
        return self.__doubletPhi
    def doubletZ(self):
        return self.__doubletZ
    def gasGap(self):
        return self.__gasGap
    def measuresPhi(self):
        return self.__measuresPhi
    def strip(self):
        return self.__strip
    def __str__(self):
        rpcIdStr = StationIdentifier.__str__(self)
        rpcIdStr+=" measuresPhi: {measPhi:2}".format(measPhi = "si" if self.measuresPhi() else "no")
        
        if self.doubletR() > 0: rpcIdStr+=" doubletR: {doubR:2}".format(doubR = self.doubletR())
        if self.doubletZ() > 0: rpcIdStr+=" doubletZ: {doubZ:2}".format(doubZ = self.doubletZ())
        if self.doubletPhi() > 0: rpcIdStr+=" doubletPhi: {doubPhi:2}".format(doubPhi = self.doubletPhi())
        if self.gasGap() > 0: rpcIdStr+=" gasGap: {gasGap:2}".format(gasGap = self.gasGap())
        if self.strip() > 0: rpcIdStr+=" strip: {strip:2}".format(strip = self.strip())
        return rpcIdStr
    def __lt__(self, other) -> bool:
        if  StationIdentifier.__lt__(self, other): 
            return True
        if self.doubletR() != other.doubletR():
            return self.doubletR() < other.doubletR()
        if self.doubletZ() != other.doubletZ():
            return self.doubletZ() < other.doubletZ()
        if self.doubletPhi() != other.doubletPhi():
            return self.doubletPhi() < other.doubletPhi()
        if self.gasGap() != other.gasGap():
            return self.gasGap() < other.gasGap()
        if self.measuresPhi() != other.measuresPhi():
            return self.measuresPhi()        
        return self.strip() < other.strip()

    def __eq__(self, other) -> bool:
        return StationIdentifier.__eq__(self, other) and \
               self.doubletR() == other.doubletR() and \
               self.doubletZ() == other.doubletZ() and \
               self.doubletPhi() == other.doubletPhi() and \
               self.gasGap() == other.gasGap() and \
               self.measuresPhi() == other.measuresPhi() and \
               self.strip() == other.strip()
    
    #### Returns an Identifier ignoring the strip number
    def gasGapID(self):
       return RpcIdentifier(stationIndex= self.stationIndex(),
                            stationPhi = self.stationPhi(),
                            stationEta = self.stationEta(),
                            doubletR = self.doubletR(),
                            doubletPhi = self.doubletPhi(),
                            doubletZ = self.doubletZ(),
                            gasGap = self.gasGap(),
                            measuresPhi=self.measuresPhi()) 