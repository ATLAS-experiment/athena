#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__author__ = "Mark Sutton, Matous Vozak"
__doc__    = "ConfigSettingsBase"




from TrigEDMConfig.TriggerEDM import recordable
from AthenaCommon.SystemOfUnits import GeV

class _ConfigSettingsBase() :
   def __init__( self ) :
     # default values
      self._input_name          = None
      self._name                = None
      self._suffix              = None
      self._pTmin               = 1.*GeV
      self._TripletDoPPS        = True
      self._Triplet_D0Max       = 4.0
      self._Triplet_D0_PPS_Max  = 1.7
      self._DoPhiFiltering      = True
      self._doZFinder           = False
      self._doZFinderOnly       = False
      self._doResMon            = False
      self._doCloneRemoval      = True
      self._doSeedRedundancyCheck = False
      self._DoubletDR_Max       = 270
      self._SeedRadBinWidth     = 2
      self._holeSearch_FTF      = False
      self._electronPID         = False
      self._etaHalfWidth        = 0.1
      self._phiHalfWidth        = 0.1
      self._zedHalfWidth        = -999 # don't set this parameter unless it is >= 0
      self._doFullScan          = False
      self._monPS               = 1
      self._monPtMin            = 1*GeV
      self._doTRT               = True
      self._keepTrackParameters = False # Keep track parameters in conversion to TrackParticles
      self._UsePixelSpacePoints = True
      self._TrackInitialD0Max   = 20.0
      self._TrackZ0Max          = 300.0
      self._isLRT               = False
      self._UseTrigSeedML       = 0
      self._nClustersMin        = 7
      self._roi                 = None
      self._isLRT               = False
      self._LRT_D0Min            = 2.0
      self._LRT_HardPtMin        = 1.0*GeV
      self._doRecord            = True
      self._vertex              = None
      self._adaptiveVertex      = False
      self._addSingleTrackVertices = False
      self._TracksMaxZinterval  = 1 #mm
      self._minNSiHits_vtx      = 10        #from vtxCuts
      self._vertex_jet          = None
      self._adaptiveVertex_jet  = False
      self._dodEdxTrk           = False 
      self._doHitDV             = False 
      self._doDisappearingTrk   = False
      self._useDynamicRoiZWidth = False

      #precision tracking configuration values
      #__provisional change__:
      #the following settings are incorrect but this is what is being used in the production running
      #at the moment. Setting them explicitly here will prevent trigger count differences in
      #https://gitlab.cern.ch/atlas/athena/-/merge_requests/56607
      self._maxRPhiImpact   = 10.
      self._maxRPhiImpactEM = 50. # mm
      self._maxZImpact      = 250.
      self._maxEta          = 2.7
      self._minSiClusters   = 7
      self._maxSiHoles      = 5
      self._maxPixelHoles   = 5
      self._maxSCTHoles    = 5
      self._maxDoubleHoles  = 2
      self._usePixel        = None
      self._useSCT          = None
      self._doEmCaloSeed    = None
      self._minTRTonTrk     = None
      self._useSiSPSeededTrackFinder = False

      if hasattr(self.__class__, 'override') and callable(getattr(self.__class__, 'override')) :
         self.override()

   # assign to this override method to add additional global functionality 
   # to the base class, such as to globally override any of the 
   # variables above
   # def override(self):   
   #      pass

   def tracks_FTF(self):
      if not self._suffix:
         raise Exception( "ID Trigger configuration:  called with non existent slice: ", self._name, self._input_name  )
      if self._doRecord:
         return recordable('HLT_IDTrack_{}_FTF'.format( self._suffix ))
      else:
         return 'HLT_IDTrack_{}_FTF'.format( self._suffix )

   def tracks_IDTrig(self):
      if self._doRecord:
         return recordable('HLT_IDTrack_{}_IDTrig'.format( self._suffix ))
      else:
         return 'HLT_IDTrack_{}_IDTrig'.format( self._suffix )


   def trkTracks_FTF(self):
         return 'HLT_IDTrkTrack_{}_FTF'.format( self._suffix )

   def trkTracks_IDTrig(self):
         return 'HLT_IDTrkTrack_{}_IDTrig'.format( self._suffix )

   @property
   def name(self):
      return self._name

   @property
   def input_name(self):
      return self._input_name

   @property
   def suffix(self):
      return self._suffix

   @property
   def pTmin(self):
      return self._pTmin

   @property
   def TripletDoPPS(self):
      return self._TripletDoPPS

   @property
   def Triplet_D0Max(self):
      return self._Triplet_D0Max

   @property
   def Triplet_D0_PPS_Max(self):
      return self._Triplet_D0_PPS_Max

   @property
   def DoPhiFiltering(self):
      return self._DoPhiFiltering

   @property
   def doZFinder(self):
      return self._doZFinder

   @property
   def doZFinderOnly(self):
      return self._doZFinderOnly

   @property
   def doResMon(self):
      return self._doResMon

   @property
   def electronPID(self):
      return self._electronPID

   @property
   def doCloneRemoval(self):
      return self._doCloneRemoval

   @property
   def doSeedRedundancyCheck(self):
      return self._doSeedRedundancyCheck

   @property
   def DoubletDR_Max(self):
      return self._DoubletDR_Max

   @property
   def SeedRadBinWidth(self):
      return self._SeedRadBinWidth

   @property
   def etaHalfWidth(self):
      return self._etaHalfWidth

   @property
   def phiHalfWidth(self):
      return self._phiHalfWidth

   @property
   def zedHalfWidth(self):
      return self._zedHalfWidth

   @property
   def doFullScan(self):
      return self._doFullScan

   @property
   def monPS(self):
      return self._monPS

   @property
   def monPtMin(self):
      return self._monPtMin

   @property
   def doTRT(self):
      return self._doTRT

   @property
   def keepTrackParameters(self):
      return self._keepTrackParameters

   @property
   def UsePixelSpacePoints(self):
       return self._UsePixelSpacePoints

   @property
   def TrackInitialD0Max(self):
       return self._TrackInitialD0Max

   @property
   def holeSearch_FTF(self):
       return self._holeSearch_FTF

   @property
   def TrackZ0Max(self):
       return self._TrackZ0Max

   @property
   def isLRT(self):
       return self._isLRT

   @property
   def LRT_D0Min(self):
       return self._LRT_D0Min

   @property
   def LRT_HardMinPt(self):
       return self._LRT_HardPtMin

   @property
   def roi(self):
      return self._roi

   @property
   def UseTrigSeedML(self):
      return self._UseTrigSeedML

   @property
   def nClustersMin(self):
      return self._nClustersMin

   @property
   def isRecordable(self):
      return self._doRecord

   @property
   def doRecord(self):
      return self._doRecord

   @property
   def vertex(self):
      if not self._vertex:
         raise Exception( "ID Trigger configuration: vertex not defined for slice: ", self._name, self._input_name  )
      if self._doRecord:
         return recordable(self._vertex)
      else:
         return self._vertex

   @property
   def vertex_jet(self):
      if not self._vertex_jet:
         raise Exception( "ID Trigger configuration: vertex_jet not defined for slice: ", self._name, self._input_name  )
      if self._doRecord:
         return recordable(self._vertex_jet)
      else:
         return self._vertex_jet

   @property
   def adaptiveVertex(self):
       return self._adaptiveVertex

   @property
   def adaptiveVertex_jet(self):
       return self._adaptiveVertex_jet

   @property
   def addSingleTrackVertices(self):
       return self._addSingleTrackVertices

   @property
   def minNSiHits_vtx(self):
       return self._minNSiHits_vtx

   @property
   def TracksMaxZinterval(self):
      return self._TracksMaxZinterval

   @property
   def dodEdxTrk(self):
       return self._dodEdxTrk

   @property
   def doHitDV(self):
       return self._doHitDV

   @property
   def doDisappearingTrk(self):
       return self._doDisappearingTrk

   @property
   def usePixelNN(self):
     raise Exception( "ID Trigger configuration:  usePixelNN not implemented temporarily")

   @property
   def maxRPhiImpact(self):
      return self._maxRPhiImpact

   @property
   def maxRPhiImpactEM(self):
      return self._maxRPhiImpactEM

   @property
   def maxZImpact(self):
      return self._maxZImpact

   @property
   def maxEta(self):
      return self._maxEta

   @property
   def minSiClusters(self):
      return self._minSiClusters

   @property
   def maxSiHoles(self):
      return self._maxSiHoles

   @property
   def maxPixelHoles(self):
      return self._maxPixelHoles

   @property
   def maxSCTHoles(self):
      return self._maxSCTHoles

   @property
   def maxDoubleHoles(self):
      return self._maxDoubleHoles

   @property
   def usePixel(self):
      return self._usePixel
   
   @property
   def useSCT(self):
      return self._useSCT
   
   @property
   def doEmCaloSeed(self):
      return self._doEmCaloSeed

   @property
   def minTRTonTrk(self):
      return self._minTRTonTrk

   @property
   def useDynamicRoiZWidth(self):
      return self._useDynamicRoiZWidth

   @property
   def useSiSPSeededTrackFinder(self):
      return self._useSiSPSeededTrackFinder

   def printout(self):
      from AthenaCommon.Logging import logging
      log = logging.getLogger("InDetTrigConfig: ")

      log.info( " %s :", self._name )
      log.info( " %s :", self._input_name )
      log.info( "   pTmin                 : %s", self._pTmin )
      log.info( "   TripletDoPPS          : %s", self._TripletDoPPS )
      log.info( "   Triplet_D0Max         : %s", self._Triplet_D0Max )
      log.info( "   Triplet_D0_PPS_Max    : %s", self._Triplet_D0_PPS_Max )
      log.info( "   doZFinder             : %s", self._doZFinder )
      log.info( "   doResMon              : %s", self._doResMon )
      log.info( "   DoPhiFiltering        : %s", self._DoPhiFiltering )
      log.info( "   doCloneRemoval        : %s", self._doCloneRemoval )
      log.info( "   doSeedRedundancyCheck : %s", self._doSeedRedundancyCheck )
      log.info( "   DoubletDR_Max         : %s", self._DoubletDR_Max )
      log.info( "   SeedRadBinWidth       : %s", self._SeedRadBinWidth )
      log.info( "   etaHalfWidth          : %s", self._etaHalfWidth )
      log.info( "   phiHalfWidth          : %s", self._phiHalfWidth )
      log.info( "   doFullScan            : %s", self._doFullScan )
      log.info( "   monPS                 : %s", self._monPS )
      log.info( "   monPtMin              : %s", self._monPtMin )
      log.info( "   doTRT                 : %s", self._doTRT )
      log.info( "   keepTrackParameters   : %s", self._keepTrackParameters )
      log.info( "   UsePixelSpacePoints   : %s", self._UsePixelSpacePoints )
      log.info( "   TrackInitialD0Max     : %s", self._TrackInitialD0Max )
      log.info( "   TrackZ0Max            : %s", self._TrackZ0Max )
      log.info( "   adaptiveVertex        : %s", self._adaptiveVertex )
      log.info( "   isLRT                 : %s", self._isLRT )
      log.info( "   LRTD0Min              : %s", self._LRT_D0Min )
      log.info( "   LRTHardPtmin          : %s", self._LRT_HardPtMin )
      log.info( "   doHitDV               : %s", self._doHitDV )
      log.info( "   nClustersMin          : %s", self._nClustersMin )
      log.info( "   suffix                : %s", self._suffix )
      log.info( "   roi                   : %s", self._roi )
      log.info( "   addSingleTrackVertices: %s", self._addSingleTrackVertices )
      log.info( "   maxRPhiImpact         : %s", self._maxRPhiImpact )
      log.info( "   maxRPhiImpactEM       : %s", self._maxRPhiImpactEM )
      log.info( "   maxZImpact            : %s", self._maxZImpact )
      log.info( "   maxEta                : %s", self._maxEta )
      log.info( "   maxSiHoles            : %s", self._maxSiHoles )
      log.info( "   maxPixelHoles         : %s", self._maxPixelHoles )
      log.info( "   maxSCTHoles           : %s", self._maxSCTHoles )
      log.info( "   maxDoubleHoles        : %s", self._maxDoubleHoles )
      log.info( "   usePixel              : %s", self._usePixel )
      log.info( "   useSCT                : %s", self._useSCT )
      log.info( "   doEmCaloSeed          : %s", self._doEmCaloSeed )
      log.info( "   minTRTonTrk           : %s", self._minTRTonTrk )
      log.info( "   useDynamicRoiZWidth   : %s", self._useDynamicRoiZWidth )
      log.info( "   useSiSPSeededTrackFinder : %s", self._useSiSPSeededTrackFinder )

if __name__=='__main__':
   config = _ConfigSettingsBase()
   config.printout()
