#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDM import recordable

class muonNames(object):

    def __init__(self):
        #EFSA and EFCB containers have different names 
        #for RoI and FS running. Other containers are 
        #produced in RoIs only.

        self.L2SAName = recordable("HLT_MuonL2SAInfo")
        self.L2SAPhIIName= recordable("HLT_FastMuons_RoI")
        self.L2CBName = recordable("HLT_MuonL2CBInfo")
        self.EFSAName = "Muons"
        self.EFSAPhIIName = "Muons"
        self.EFSAPhIINewFastName = "Muons"
        self.EFSAPhIIMlbktName = "Muons"
        self.EFCBName = "MuonsCB"
        self.EFCBOutInName = "MuonsCBOutsideIn"
        self.EFCBInOutName = "HLT_MuonsCBInsideOut"
        self.EFIsoMuonName = recordable("HLT_MuonsIso")
        self.L2forIDName   = "RoIs_fromL2SAViews"

    def getNames(self, name):

        if "FS" in name:
            self.EFSAName = recordable("HLT_Muons_FS")
            self.EFSAPhIIName = recordable("HLT_Muons_FSPhII")
            self.EFSAPhIINewFastName = recordable("HLT_Muons_FSPhII_newFast")
            self.EFSAPhIIMlbktName = recordable("HLT_Muons_FSPhII_mlbkt")
            self.EFCBName = recordable("HLT_MuonsCB_FS")
            self.EFCBOutInName = "MuonsCBOutsideIn_FS"
        if "RoI" in name:
            self.EFSAName = recordable("HLT_Muons_RoI")
            self.EFSAPhIIName = recordable("HLT_Muons_RoIPhII")
            self.EFSAPhIINewFastName = recordable("HLT_Muons_RoIPhII_newFast")
            self.EFSAPhIIMlbktName = recordable("HLT_Muons_RoIPhII_mlbkt")
            self.EFCBName = recordable("HLT_MuonsCB_RoI")
        if "LRT" in name:
            self.L2CBName = recordable("HLT_MuonL2CBInfoLRT")
            self.EFSAName = recordable("HLT_Muons_RoI")
            self.EFCBName = recordable("HLT_MuonsCB_LRT")
        return self