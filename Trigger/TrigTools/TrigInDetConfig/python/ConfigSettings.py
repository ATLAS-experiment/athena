#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__author__ = "Mark Sutton, Matous Vozak"
__doc__    = "ConfigSettings"

import math
from TrigInDetConfig.ConfigSettingsBase import _ConfigSettingsBase
from TrigEDMConfig.TriggerEDM import recordable
from AthenaCommon.SystemOfUnits import GeV


class ConfigSettings_electron( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name      = "electron"
      self._suffix    = "Electron"
      self._roi       = "HLT_Roi_Electron"
      # this soze of 0.05 should be increased to 0.1
      self._etaHalfWidth        = 0.05
      self._phiHalfWidth        = 0.1
      self._doCloneRemoval      = True #Previously False in Run2!
      self._doSeedRedundancyCheck = True
      self._doTRT               = True
      self._keepTrackParameters = True
      self._electronPID         = True


class ConfigSettings_muon( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name      = "muon"
      self._suffix    = "Muon"
      self._roi       = "HLT_Roi_L2SAMuon"
      self._Triplet_D0Max       = 10.0
      self._doResMon            = True
      self._DoPhiFiltering      = False
      self._doSeedRedundancyCheck = True
      self._monPtMin            = 12*GeV


class ConfigSettings_muonIso( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name      = "muonIso"
      self._suffix    = "MuonIso"
      self._roi       = "HLT_Roi_MuonIso"
      self._etaHalfWidth        = 0.35
      self._phiHalfWidth        = 0.35
      self._zedHalfWidth        = 10.0


class ConfigSettings_tau( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "tau"
      self._suffix   = "Tau"
      self._roi      = "HLT_Roi_Tau"
      self._vertex   = "HLT_IDVertex_Tau"
      self._adaptiveVertex  = True
      self._pTmin           = 0.8*GeV
      self._etaHalfWidth    = 0.4
      self._phiHalfWidth    = 0.4
      self._doTRT           = True
      self._electronPID     = False
      # potential change coming up ...
      # self._minNSiHits_vtx = 6


class ConfigSettings_tauCore( _ConfigSettingsBase ):
    def __init__( self ):
       _ConfigSettingsBase.__init__(self)
       self._name     = "tauCore"
       self._suffix   = "TauCore"
       self._roi      = "HLT_Roi_TauCore"
       self._pTmin    = 0.8*GeV
       self._holeSearch_FTF = True

class ConfigSettings_tauIso( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "tauIso"
      self._suffix   = "TauIso"
      self._roi      = "HLT_Roi_TauIso"
      self._etaHalfWidth   = 0.4
      self._phiHalfWidth   = 0.4
      self._zedHalfWidth   = 7.0
      self._adaptiveVertex = True
      self._addSingleTrackVertices = True
      self._vertex         = "HLT_IDVertex_Tau"
      self._electronPID    = False
      self._pTmin          = 0.8*GeV
      # potential change coming up ...
      # self._minNSiHits_vtx = 6

   def tracks_IDTrig(self):
      if self._doRecord:
         return recordable('HLT_IDTrack_Tau_IDTrig')
      else:
         return 'HLT_IDTrack_Tau_IDTrig'


# inherit everythiong from the tauIso instance - only
# the Roi name is changed to protect the innocent
class ConfigSettings_tauIsoBDT( ConfigSettings_tauIso ):
   def __init__( self ):
      ConfigSettings_tauIso.__init__(self)
      self._roi      = "HLT_Roi_TauIsoBDT"


class ConfigSettings_bjet( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "bjet"
      self._suffix   = "Bjet"
      self._roi      = "HLT_Roi_Bjet"
      self._etaHalfWidth    = 0.4
      self._phiHalfWidth    = 0.4
      self._zedHalfWidth    = 10.0
      self._pTmin  = 0.8*GeV

class ConfigSettings_jetSuper( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "jetSuper"
      self._suffix   = "JetSuper"
      self._vertex   = "HLT_IDVertex_JetSuper"
      self._adaptiveVertex = True
      self._addSingleTrackVertices = True
      self._roi          = "HLT_Roi_JetSuper"
      self._etaHalfWidth = 0.3
      self._phiHalfWidth = 0.3
      self._zedHalfWidth = 180.0
      self._doFullScan   = True
      self._pTmin        = 1*GeV
      #-----
      self._doTRT           = False
      self._DoubletDR_Max   = 200
      self._SeedRadBinWidth = 10
      self._doSeedRedundancyCheck = True
      self._TripletDoPPS    = False
      self._nClustersMin    = 8
      self._UseTrigSeedML   = 4


class ConfigSettings_minBias( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "minBias"
      self._suffix   = "MinBias"
      self._roi      = "HLT_Roi_MinBias"
      self._doFullScan      = True
      self._pTmin           = 0.1*GeV # TODO: double check
      self._doTRT           = False      #backward compatibility with EFIDTracking.py:makeInDetPatternRecognition
      self._etaHalfWidth    = 3
      self._phiHalfWidth    = math.pi
      self._doZFinder       = True
      self._doZFinderOnly   = True



class ConfigSettings_beamSpot( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "beamSpot"
      self._suffix   = "BeamSpot"
      self._roi      = "HLT_Roi_FS"
      self._doFullScan      = True
      self._doZFinder       = True
      self._DoubletDR_Max   = 200
      self._SeedRadBinWidth = 10
      self._etaHalfWidth    = 3
      self._phiHalfWidth    = math.pi
      self._doTRT           = False
      self._doSeedRedundancyCheck = True
      self._doRecord        = False


class ConfigSettings_fullScan( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "fullScan"
      self._suffix   = "FS"
      self._roi      = "HLT_Roi_FS"
      self._vertex              = "HLT_IDVertex_FS"
      self._adaptiveVertex      = True
      # these are being evaluated and may be added
      # self._addSingleTrackVertices = True
      # self._TracksMaxZinterval = 3
      self._vertex_jet          = "HLT_IDVertex_FS"
      self._adaptiveVertex_jet  = True
      self._doFullScan      = True
      self._etaHalfWidth    = 3.
      self._phiHalfWidth    = math.pi
      self._doTRT           = False
      self._DoubletDR_Max   = 200
      self._SeedRadBinWidth = 10
      self._doSeedRedundancyCheck = True
      self._TripletDoPPS    = False
      self._nClustersMin    = 8
      self._UseTrigSeedML   = 4
      self._dodEdxTrk         = True
      self._doHitDV           = True
      self._doDisappearingTrk = True

class ConfigSettings_beamSpotFS( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "fullScan"
      self._suffix   = "FS"
      self._roi      = "HLT_Roi_FS"
      self._doFullScan      = True
      self._etaHalfWidth    = 3.
      self._phiHalfWidth    = math.pi
      self._doTRT           = False
      self._DoubletDR_Max   = 200
      self._SeedRadBinWidth = 10
      self._TripletDoPPS    = False
      self._nClustersMin    = 8
      self._UseTrigSeedML   = 4
      self._doRecord        = False


class ConfigSettings_cosmics( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name        = "cosmics"
      self._suffix      = "Cosmic"
      self._roi         = "HLT_Roi_Cosmics"
      self._Triplet_D0Max       = 1000.0
      self._Triplet_D0_PPS_Max  = 1000.0
      self._TrackInitialD0Max   = 1000.
      self._TrackZ0Max          = 1000.
      self._doTRT           = False      #no real reason except of backward compatibility with
                                         #EFIDTracking.py:makeInDetPatternRecognition
                                         #2023fix
      self._doFullScan      = True
      self._etaHalfWidth    = 3
      self._phiHalfWidth    = math.pi


class ConfigSettings_bmumux( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name      = "bphysics"
      self._suffix    = "Bmumux"
      self._roi       = "HLT_Roi_Bmumux"
      self._Triplet_D0Max       = 10.
      self._DoPhiFiltering      = False
      self._etaHalfWidth        = 0.75
      self._phiHalfWidth        = 0.75
      self._zedHalfWidth        = 50.
      self._doSeedRedundancyCheck = True
      self._SuperRoI = True

   @property
   def SuperRoI(self):
      return self._SuperRoI
       


class ConfigSettings_electronLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name       = "electronLRT"
      self._suffix     = "ElecLRT"
      self._roi        = "HLT_Roi_Electron"
      self._etaHalfWidth        = 0.1
      self._phiHalfWidth        = 0.4
      self._UsePixelSpacePoints = False
      self._Triplet_D0Max       = 300.
      self._TrackInitialD0Max   = 300.
      self._TrackZ0Max          = 500.
      self._zedHalfWidth        = 225.
      self._keepTrackParameters = True
      self._doSeedRedundancyCheck = True
      self._nClustersMin        = 8
      self._isLRT               = True
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpactEM   = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False

      
class ConfigSettings_muonLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name       = "muonLRT"
      self._suffix     = "MuonLRT"
      self._roi        = "HLT_Roi_Muon"
      self._UsePixelSpacePoints = False
      self._etaHalfWidth        = 0.2
      self._phiHalfWidth        = 0.4
      self._Triplet_D0Max       = 300.
      self._TrackInitialD0Max   = 300.
      self._TrackZ0Max          = 500.
      self._zedHalfWidth        = 225.
      self._doSeedRedundancyCheck = True
      self._nClustersMin        = 8
      self._isLRT               = True
      self._doResMon            = True
      self._DoPhiFiltering      = False
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpEM      = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False


class ConfigSettings_tauLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "tauLRT"
      self._suffix   = "TauLRT"
      self._roi      = "HLT_Roi_TauLRT"
      self._vertex   = "HLT_IDVertex_Tau" # TODO: does this need renaming?
      self._pTmin        = 0.8*GeV
      self._etaHalfWidth = 0.4
      self._phiHalfWidth = 0.4
      self._zedHalfWidth = 225.
      self._doTRT        = True
      self._UsePixelSpacePoints = False
      self._Triplet_D0Max       = 300.
      self._TrackInitialD0Max   = 300.
      self._TrackZ0Max          = 500.
      self._nClustersMin        = 8
      self._isLRT               = True
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpEM      = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False

class ConfigSettings_bjetLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "bjetLRT"
      self._suffix   = "BjetLRT"
      self._roi      = "HLT_Roi_Bjet"
      self._etaHalfWidth = 0.4
      self._phiHalfWidth = 0.4
      self._UsePixelSpacePoints = False
      self._Triplet_D0Max       = 300.
      self._TrackInitialD0Max   = 300.
      self._TrackZ0Max          = 500.
      self._nClustersMin        = 8
      self._isLRT               = True
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpactEM   = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False


class ConfigSettings_fullScanLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "fullScanLRT"
      self._suffix   = "FSLRT"
      self._roi      = "HLT_Roi_FS"
      self._doFullScan      = True
      self._etaHalfWidth    = 3.
      self._phiHalfWidth    = math.pi
      self._doTRT           = False
      self._doSeedRedundancyCheck = True
      self._UsePixelSpacePoints   = False
      self._Triplet_D0Max         = 300.
      self._TrackInitialD0Max     = 300.
      self._TrackZ0Max            = 500.
      self._Triplet_D0_PPS_Max    = 300.
      self._DoubletDR_Max         = 200
      self._nClustersMin          = 8
      self._isLRT                 = True
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpactEM   = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False


class ConfigSettings_DJetLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "DJetLRT"
      self._suffix   = "DJLRT"
      self._roi      = "HLT_Roi_DJ"
      self._doFullScan      = False
      self._etaHalfWidth    = 0.4
      self._phiHalfWidth    = 0.4
      self._zedHalfWidth    = 225.
      self._doTRT           = False
      self._doSeedRedundancyCheck = True
      self._UsePixelSpacePoints   = False
      self._Triplet_D0Max         = 300.
      self._TrackInitialD0Max     = 300.
      self._TrackZ0Max            = 500.
      self._Triplet_D0_PPS_Max    = 300.
      self._DoubletDR_Max         = 200
      self._nClustersMin          = 8
      self._isLRT                 = True
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpactEM   = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False



class ConfigSettings_DVtxLRT( _ConfigSettingsBase ):
   def __init__( self ):
      _ConfigSettingsBase.__init__(self)
      self._name     = "DVtxLRT"
      self._suffix   = "DVLRT"
      self._roi      = "HLT_Roi_DV"
      self._doFullScan      = False
      self._etaHalfWidth    = 0.35
      self._phiHalfWidth    = 0.35
      self._doTRT           = False
      self._doSeedRedundancyCheck = True
      self._UsePixelSpacePoints   = False
      self._Triplet_D0Max         = 300.
      self._TrackInitialD0Max     = 300.
      self._TrackZ0Max            = 500.
      self._Triplet_D0_PPS_Max    = 300.
      self._DoubletDR_Max         = 200
      self._nClustersMin          = 8
      self._isLRT                 = True
      #pt config
      self._maxZImpact        = 500.
      self._maxRPhiImpact     = 300.
      self._maxRPhiImpactEM   = 300.
      self._maxEta            = 2.7
      self._maxDoubleHoles    = 0
      self._maxSiHoles        = 2
      self._maxPixelHoles     = 1
      self._maxSCTHoles       = 1
      self._minSiClusters     = 8
      self._doEmCaloSeed      = False


ConfigSettingsInstances = {
   "electron"     : ConfigSettings_electron(),
   "photon"       : ConfigSettings_electron(),

    "muon"        : ConfigSettings_muon(),
    "muonIso"     : ConfigSettings_muonIso(),
    "muonIsoMS"   : ConfigSettings_muonIso(),
    "muonCore"    : ConfigSettings_muon(),
    "muonFS"      : ConfigSettings_muon(),
    "muonLate"    : ConfigSettings_muon(),

    "tau"         : ConfigSettings_tau(),
    "tauTau"      : ConfigSettings_tau(),
    "tauCore"     : ConfigSettings_tauCore(),
    "tauIso"      : ConfigSettings_tauIso(),
    "tauIsoBDT"   : ConfigSettings_tauIsoBDT(),

    "bjet"        : ConfigSettings_bjet(),

    "fullScan"    : ConfigSettings_fullScan(),
    "FS"          : ConfigSettings_fullScan(),
    "jetSuper"    : ConfigSettings_jetSuper(),

    "beamSpot"    : ConfigSettings_beamSpot(),
    "BeamSpot"    : ConfigSettings_beamSpot(),
    "beamSpotFS"  : ConfigSettings_beamSpotFS(),

    "cosmics"     : ConfigSettings_cosmics(),
    "bmumux"      : ConfigSettings_bmumux(),
    "minBias"     : ConfigSettings_minBias(),

    "electronLRT"    : ConfigSettings_electronLRT(),
    "muonLRT"        : ConfigSettings_muonLRT(),
    "tauLRT"         : ConfigSettings_tauLRT(),
    "bjetLRT"        : ConfigSettings_bjetLRT(),
    "fullScanLRT"    : ConfigSettings_fullScanLRT(),
    "DJetLRT"        : ConfigSettings_DJetLRT(),
    "DVtxLRT"        : ConfigSettings_DVtxLRT() }
