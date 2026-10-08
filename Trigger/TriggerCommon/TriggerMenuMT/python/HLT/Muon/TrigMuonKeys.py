#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDM import recordable

class muonNames(object):

    def __init__(self):
        #EFSA and EFCB containers have different names 
        #for RoI and FS running. Other containers are 
        #produced in RoIs only.

        #Fast reconstruction muon containers
        self.L2SAMuons = recordable("HLT_MuonL2SAInfo")
        self.L2SAMuonsPhII = recordable("HLT_FastMuons_RoI")
        self.L2CBMuons = recordable("HLT_MuonL2CBInfo")

        #EFSA muon containers
        self.EFSAMuons = "Muons"
        self.EFSAMuonsPhIINewFast = "Muons"
        self.EFSAMuonsPhIIMlbkt = "Muons"

        #EFSA track particle containers
        self.EFSATrackParticlesPhII = "Muons_trackParticles"
        self.EFSATrackParticlesPhIINewFast = "Muons_trackParticles"
        self.EFSATrackParticlesPhIIMlbkt = "Muons_trackParticles"

        #EFCB muon containers
        self.EFCBMuons = "MuonsCB"
        self.EFCBOutInMuons = "MuonsCBOutsideIn"
        self.EFCBInOutMuons = "HLT_MuonsCBInsideOut"
        self.EFIsoMuons = recordable("HLT_MuonsIso")
        self.L2forIDName   = "RoIs_fromL2SAViews"

        #EFCB track particle containers
        self.EFCBTrackParticles = "MuonsCB_trackParticles"

    def getNames(self, name):

        if "FS" in name:
            self.EFSAMuons = recordable("HLT_Muons_FS")
            self.EFSAMuonsPhIINewFast = recordable("HLT_Muons_FSPhII_newFast")
            self.EFSAMuonsPhIIMlbkt = recordable("HLT_Muons_FSPhII_mlbkt")

            self.EFSATrackParticlesPhII = recordable("HLT_MuonMSTrackParticles_FSPhII")
            self.EFSATrackParticlesPhIINewFast = recordable("HLT_MuonMSTrackParticles_FSPhII_newFast")
            self.EFSATrackParticlesPhIIMlbkt = recordable("HLT_MuonMSTrackParticles_FSPhII_mlbkt")

            self.EFCBMuons = recordable("HLT_MuonsCB_FS")
            self.EFCBOutInMuons = "MuonsCBOutsideIn_FS"
            self.EFCBTrackParticles = recordable("HLT_CBCombinedMuon_FSTrackParticles")

        if "RoI" in name:
            self.EFSAMuons = recordable("HLT_Muons_RoI")
            self.EFSAMuonsPhIINewFast = recordable("HLT_Muons_RoIPhII_newFast")
            self.EFSAMuonsPhIIMlbkt = recordable("HLT_Muons_RoIPhII_mlbkt")

            self.EFSATrackParticlesPhII = recordable("HLT_MuonMSTrackParticles_RoIPhII")
            self.EFSATrackParticlesPhIINewFast = recordable("HLT_MuonMSTrackParticles_RoIPhII_newFast")
            self.EFSATrackParticlesPhIIMlbkt = recordable("HLT_MuonMSTrackParticles_RoIPhII_mlbkt")

            self.EFCBMuons = recordable("HLT_MuonsCB_RoI")
            self.EFCBTrackParticles = recordable("HLT_CBCombinedMuon_RoITrackParticles")

        if "LRT" in name:
            self.L2CBMuons = recordable("HLT_MuonL2CBInfoLRT")
            self.EFSAMuons = recordable("HLT_Muons_RoI")
            self.EFCBMuons = recordable("HLT_MuonsCB_LRT")
        return self