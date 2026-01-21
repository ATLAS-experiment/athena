# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from TrkConfig.TrackingPassFlags import createTrackingPassFlags,createITkTrackingPassFlags
from ActsConfig.ActsTrackingPassFlags import createActsTrackingPassFlags
from TrigEDMConfig.TriggerEDM import recordable

from AthenaCommon.Logging import logging
import AthenaCommon.SystemOfUnits as Units
import math
from copy import deepcopy
from collections.abc import Callable

log = logging.getLogger("__name__")

def defaultTrigTrackingFlags(flags : AthConfigFlags):

  flags.addFlag("input_name",           "")
  flags.addFlag("name",                 "")
  flags.addFlag("suffix",               "")

  flags.addFlag("pTmin",                1.*Units.GeV)      #fix - consolidate pTmin and minPT
  flags.addFlag("TripletDoPPS",         True)
  flags.addFlag("Triplet_D0Max",        4.0)
  flags.addFlag("Triplet_D0_PPS_Max",   1.7)
  flags.addFlag("DoPhiFiltering",       True)
  flags.addFlag("doZFinder",            False)
  flags.addFlag("doZFinderOnly",        False)
  flags.addFlag("doResMon",             False)
  flags.addFlag("doCloneRemoval",       True)
  flags.addFlag("doSeedRedundancyCheck",False)
  flags.addFlag("DoubletDR_Max",        270)
  flags.addFlag("SeedRadBinWidth",      2)
  flags.addFlag("holeSearch_FTF",       False)
  flags.addFlag("electronPID",          False)
  flags.addFlag("etaHalfWidth",         0.1)
  flags.addFlag("phiHalfWidth",         0.1)
  flags.addFlag("zedHalfWidth",        -999)    # don't set this parameter unless it is >= 0)
  flags.addFlag("doFullScan",           False)
  flags.addFlag("monPS",                1)
  flags.addFlag("monPtMin",             1*Units.GeV)
  flags.addFlag("doTRT",                True)
  flags.addFlag("keepTrackParameters",  False) # Keep track parameters in conversion to TrackParticles
  flags.addFlag("UsePixelSpacePoints",  True)
  flags.addFlag("TrackInitialD0Max",    20.0)
  flags.addFlag("TrackZ0Max",           300.0)
  flags.addFlag("isLRT",                False)
  flags.addFlag("UseTrigSeedML",        0)
  flags.addFlag("nClustersMin",         7)
  flags.addFlag("roi",                  "")
  flags.addFlag("LRT_D0Min",            2.0)
  flags.addFlag("LRT_HardPtMin",        1.0*Units.GeV)
  flags.addFlag("doRecord",             True)
  flags.addFlag("vertex",               "")
  flags.addFlag("adaptiveVertex",       False)
  flags.addFlag("addSingleTrackVertices", False)
  flags.addFlag("TracksMaxZinterval",   1) #mm
  flags.addFlag("minNSiHits_vtx",       10)        #from vtxCuts
  flags.addFlag("vertex_jet",           "")
  flags.addFlag("adaptiveVertex_jet",   False)
  flags.addFlag("dodEdxTrk",            False)
  flags.addFlag("doHitDV",              False)
  flags.addFlag("doDisappearingTrk",    False)
  flags.addFlag("useDynamicRoiZWidth",  False)
  
  
  #precision tracking configuration values
  #__provisional change__:
  #the following settings are incorrect but this is what is being used in the production running
  #at the moment. Setting them explicitly here will prevent trigger count differences in
  #https://gitlab.cern.ch/atlas/athena/-/merge_requests/56607
  flags.maxEta           = 2.7
  flags.addFlag("minSiClusters", 7)
  flags.addFlag("maxSiHoles", 5)
  flags.maxPixelHoles   = 5
  flags.addFlag("maxSCTHoles", 5)                   #fix2024 - consolidate names maxSctHoles and others in addFlag here
  flags.maxDoubleHoles  = 2
  flags.addFlag("doEmCaloSeed", False)

  flags.useSeedFilter         = False
  flags.doBremRecoverySi = False                    #fix2023 setTrue for electron once validated

  flags.addFlag("refitROT", True) 
  flags.addFlag("trtExtensionType", "xf") 
  flags.addFlag("doTruth",  False)  
  flags.addFlag("perigeeExpression","BeamLine")   #always use beamline regardless of Reco.EnableHI
  flags.addFlag("SuperRoI",  False)               #TBD - move to bphys/menu
  
  flags.addFlag("trkTracks_FTF",   "")
  flags.addFlag("trkTracks_IDTrig","")
  flags.addFlag("tracks_FTF",      "")        
  flags.addFlag("tracks_IDTrig",   "")  
  flags.addFlag("useGBTSeedingTool",  False)

def defaultInDetTrigTrackingFlags() -> AthConfigFlags:

  flags = createTrackingPassFlags()
  defaultTrigTrackingFlags(flags)
  
  flags.minPT = flags.pTmin   #hack to sync pT threshold used in offline and trigger
    
  flags.minClusters         = 7   #hardcoded to preserve trigger settings (not used for FTF config)
  flags.minSiNotShared      = 5
  flags.maxShared           = 2
  flags.Xi2max              = 9. 
  flags.Xi2maxNoAdd         = 25.
  flags.nHolesMax           = 2
  flags.nHolesGapMax        = 2
  flags.nWeightedClustersMin= 6
  flags.roadWidth           =10.
      

  flags.maxPrimaryImpact = 10.
  flags.maxEMImpact      = 50.
  flags.maxZImpact       = 250.

  flags.minTRTonTrk          =9

  #TODO - simple ambiguitues
  flags.useTIDE_Ambi = False  

  #2023fix - it should read 2
  flags.maxSiHoles           = 5
  flags.maxSCTHoles          = 5
  #end 

  return flags  

def defaultITkTrigTrackingFlags() -> AthConfigFlags:  
  flags = createITkTrackingPassFlags()
  defaultTrigTrackingFlags(flags)
  
  flags.minPT               = [1.0*Units.GeV,0.45*Units.GeV,0.45*Units.GeV] #ITk flags have eta dependant settings
  flags.minClusters         = [9,8,7]       #offline defaults are [9,8,7]
  flags.Xi2max              = [9.]
  flags.Xi2maxNoAdd         = [25.]
  flags.nHolesMax           = [2]
  flags.nHolesGapMax        = [2]
  flags.nWeightedClustersMin= [6]
  flags.maxDoubleHoles      = [2]
  flags.maxPixelHoles       = [5]
  flags.maxZImpact          = [250.0]
  flags.doTRT               = False
  flags.doZFinder           = False
  flags.DoPhiFiltering      = True
  flags.UsePixelSpacePoints = True          # In LRT they use only SCT SP, but for ITk we want pixel SP
  flags.doDisappearingTrk   = False         # Not working yet for ITk
  flags.doCaloSeededBremSi  = False
  flags.doCaloSeededAmbiSi  = False
  flags.DoubletDR_Max       = 150.0
  flags.useTIDE_Ambi        = False  
  flags.maxEta = 4.0
  return flags

def defaultITkActsTrigTrackingFlags() -> AthConfigFlags:
    flags = createActsTrackingPassFlags()
    defaultTrigTrackingFlags(flags)
    
    flags.minPT               = [0.9*Units.GeV, 0.4*Units.GeV, 0.4*Units.GeV]
    flags.minClusters         = [9, 8, 7]
    flags.doTRT = False
    flags.maxEta = 4.0
    flags.maxShared = [2]
    flags.maxHoles = [1]
    flags.maxPixelHoles = [2]
    flags.maxSctHoles = [2]
    flags.maxShared = [2]
    flags.maxDoubleHoles = [1]
    return flags

def defaultModeTrigTrackingFlags(flags: AthConfigFlags) -> AthConfigFlags:
  return flags

def signatureTrigTrackingFlags(mode : str) -> AthConfigFlags:

  signatureSet = {
    "electron"      : electron,
    "photon"        : electron,
    
    "muon"          : muon,
    "muonIso"       : muonIso,
    "muonIsoMS"     : muonIso,
    "muonCore"      : muon,
    "muonFS"        : muon,
    "muonLate"      : muon,
    
    "tauCore"       : tauCore,
    "tauIso"        : tauIso,
    
    "diTau"         : diTau,
    
    "bjet"          : bjet,
    
    "fullScan"      : fullScan,
    "FS"            : fullScan,
    "jetSuper"      : jetSuper,

    "beamSpot"      : beamSpot,
    "BeamSpot"      : beamSpot,
    "beamSpotFS"    : beamSpotFS,
                
    "cosmics"      : cosmics,
    "bmumux"       : bmumux,
    "bhh"          : bhh,

    "minBias"      : minBias,
    "minBiasPixel" : minBiasPixel,
    
    "electronLRT"  : electronLRT,
    "muonLRT"      : muonLRT,
    "tauLRT"       : tauLRT,
    "bjetLRT"      : bjetLRT,
    "fullScanLRT"  : fullScanLRT,
    "DJetLRT"      : DJetLRT,
    "DVtxLRT"      : DVtxLRT,
  }

  flags = AthConfigFlags()

  if   mode == "InDet":                       
    category = 'Trigger.InDetTracking'        
    defaults = defaultInDetTrigTrackingFlags  
  elif mode == "ITk":                         
    category = 'Trigger.ITkTracking'          
    defaults = defaultITkTrigTrackingFlags
  elif mode == "Acts":
    category = "Trigger.ActsTracking"
    defaults = defaultITkActsTrigTrackingFlags
  else:                                       
    log.error("Unsupported reconstruction mode %s", mode)       
                                            
     
  class categoryGeneratorWrapper():
    """ wrap function which can be consumed by addFlagsCategory and provide its args """
    def __init__(self, fun : Callable[[AthConfigFlags, str, str], AthConfigFlags], 
                 flags : AthConfigFlags, signatureInstance : str, recoMode : str):
      self.flags = flags
      self.sig = signatureInstance
      self.fun = fun
      self.mode= recoMode
    def run(self):
      return self.fun(self.flags,self.sig,self.mode)
    
  for i in signatureSet.keys():
    trackingflags = deepcopy(defaults())
    a = categoryGeneratorWrapper(signatureSet[i],trackingflags,i,mode)
    signatureCategory = "{}.{}".format(category,i)
    flags.addFlagsCategory(signatureCategory,a.run,prefix=True)

  addGlobalFlags(flags, category)        # they should not be needed / backward compatibility
  
  return flags


def signatureActions(func):
  """ convenience decorator to automate config steps """
  def invokeSteps(*args, **kwargs):
    flagsSig = func(*args, **kwargs)     #invoke signature specific code
    recoMode = args[2]
    derivedFromSignatureFlags(flagsSig,recoMode)  #invoke code dependant on signature flags
    return flagsSig
  return invokeSteps


def tsetter(var, value):
  """ use previous type of the var and convert value to it
      for the moment just makes list of a value if needed
  """
  type2set = type(var)
  typeOfValue = type(value)
  if type2set == typeOfValue:
    var = value
  else:
    basic = (bool, str, int, float, type(None))
    if isinstance(var,basic):
      var = value
    else:
      var = [value]

  return var
    
  
@signatureActions
def electron(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:
  
  flags.input_name = instanceName
  flags.name      = "electron"
  flags.suffix    = "Electron"
  flags.roi       = "HLT_Roi_Electron"
  flags.etaHalfWidth        = 0.05   # this size should be increased to 0.1
  flags.phiHalfWidth        = 0.1
  flags.doCloneRemoval      = True 
  flags.doSeedRedundancyCheck = True
  if recoMode=="InDet":
    flags.doTRT             = True
  flags.keepTrackParameters = True
  flags.electronPID         = True
  return flags

@signatureActions
def muon(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:
  
  flags.input_name = instanceName
  flags.name      = "muon"
  flags.suffix    = "Muon"
  flags.roi       = "HLT_Roi_L2SAMuon"
  flags.Triplet_D0Max       = 10.0
  flags.doResMon            = True
  flags.DoPhiFiltering      = False
  flags.doSeedRedundancyCheck = True
  flags.monPtMin            = 12*Units.GeV
  return flags
  
@signatureActions
def muonIso(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name      = "muonIso"
  flags.suffix    = "MuonIso"
  flags.roi       = "HLT_Roi_MuonIso"
  flags.etaHalfWidth        = 0.35
  flags.phiHalfWidth        = 0.35
  flags.zedHalfWidth        = 10.0
  return flags

@signatureActions
def tauCore(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:
  
  flags.input_name = instanceName
  flags.name     = "tauCore"
  flags.suffix   = "TauCore"
  flags.roi      = "HLT_Roi_TauCore"
  flags.pTmin    = 0.8*Units.GeV
  flags.minPT    = tsetter(flags.minPT, flags.pTmin)

  flags.holeSearch_FTF = True
  return flags
  
@signatureActions
def tauIso(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:
  
  flags.input_name = instanceName
  flags.name     = "tauIso"
  flags.suffix   = "TauIso"
  flags.roi      = "HLT_Roi_TauIso"
  flags.etaHalfWidth   = 0.4
  flags.phiHalfWidth   = 0.4
  flags.zedHalfWidth   = 7.0
  flags.adaptiveVertex = True
  flags.addSingleTrackVertices = True
  flags.vertex         = "HLT_IDVertex_Tau"
  flags.electronPID    = False
  flags.pTmin          = 0.8*Units.GeV
  flags.minPT = tsetter(flags.minPT, flags.pTmin)
  return flags

@signatureActions
def diTau(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:
  
  flags.input_name = instanceName
  flags.name     = "diTau"
  flags.suffix   = "DiTau"
  flags.roi      = "HLT_Roi_DiTau"
  flags.etaHalfWidth   = 1.0
  flags.phiHalfWidth   = 1.0
  flags.zedHalfWidth   = 7.0
  flags.adaptiveVertex = True
  flags.addSingleTrackVertices = True
  flags.vertex         = "HLT_IDVertex_DiTau"
  flags.electronPID    = False
  flags.pTmin          = 0.8*Units.GeV
  flags.minPT = tsetter(flags.minPT, flags.pTmin)
  return flags

@signatureActions
def bjet(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "bjet"
  flags.suffix   = "Bjet"
  flags.roi      = "HLT_Roi_Bjet"
  flags.etaHalfWidth    = 0.4
  flags.phiHalfWidth    = 0.4
  flags.zedHalfWidth    = 10.0
  flags.pTmin           = 0.8*Units.GeV
  flags.minPT = tsetter(flags.minPT, flags.pTmin)
  flags.Xi2max = tsetter(flags.Xi2max,12.)
  return flags

@signatureActions
def jetSuper(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "jetSuper"
  flags.suffix   = "JetSuper"
  flags.vertex   = "HLT_IDVertex_JetSuper"
  flags.adaptiveVertex = True
  flags.addSingleTrackVertices = True
  flags.roi          = "HLT_Roi_JetSuper"
  flags.etaHalfWidth = 0.3
  flags.phiHalfWidth = 0.3
  flags.doFullScan   = True
  flags.pTmin        = 1*Units.GeV
  flags.minPT = tsetter(flags.minPT, flags.pTmin)
  #-----
  flags.doTRT           = False
  flags.DoubletDR_Max   = 200
  flags.SeedRadBinWidth = 10
  flags.doSeedRedundancyCheck = True
  flags.TripletDoPPS    = False
  flags.nClustersMin    = 8
  flags.UseTrigSeedML   = 4
  flags.roadWidth =         5.
  return flags

@signatureActions
def minBias(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "minBias"
  flags.suffix   = "MinBias"
  flags.roi      = "HLT_Roi_MinBias"
  flags.doFullScan      = True
  flags.pTmin    = 0.1*Units.GeV # TODO: double check
  flags.minPT = tsetter(flags.minPT, flags.pTmin)

  flags.doTRT           = False
  flags.etaHalfWidth    = 3
  flags.phiHalfWidth    = math.pi
  flags.doZFinder       = True
  flags.doZFinderOnly   = True
  
  flags.nClustersMin        = 5
  flags.useSeedFilter       = True
  flags.maxPrimaryImpact    = tsetter(flags.maxPrimaryImpact, 10.*Units.mm)
  flags.maxZImpact          = tsetter(flags.maxZImpact, 150.*Units.mm)
  flags.roadWidth           = 20
  flags.usePrdAssociationTool = False     #for backward compatibility #2023fix?
  return flags


@signatureActions
def minBiasPixel(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "minBiasPixel"
  flags.suffix   = "MinBiasPixel"
  flags.roi      = "HLT_Roi_MinBias"
  flags.doFullScan      = True
  flags.pTmin    = 0.1*Units.GeV # this is good for HI which is the client of this setup
  flags.minPT    = tsetter(flags.minPT, flags.pTmin)


  flags.maxHoles         = 0 # HI setup
  flags.maxPixelHoles    = 0 # HI setup
  flags.maxSctHoles      = 0
  flags.maxDoubleHoles   = 0
  flags.minSiNotShared   = 3
  flags.maxShared        = 0
  flags.nHolesMax        = 0
  flags.nHolesGapMax     = 0
  if recoMode=="InDet":
    flags.useSCT           = False
    flags.useSCTSeeding    = False
    flags.useTRT           = False

    flags.roadWidth        = 12.0
    flags.Xi2max           = 15
    flags.Xi2maxNoAdd      = 35.0
    flags.nWeightedClustersMin = 6
    flags.doBremRecoverySi        = False
    flags.RunPixelPID = False # we use these tracks only for counting

  flags.doTRT           = False
  flags.etaHalfWidth    = 3
  flags.phiHalfWidth    = math.pi
  flags.doZFinder       = False
  flags.doZFinderOnly   = True

  flags.minClusters         = 3
  flags.nClustersMin        = 3
  flags.useSeedFilter       = True
  flags.maxPrimaryImpact    = 10.*Units.mm
  flags.maxZImpact          = 150.*Units.mm
  flags.roadWidth           = 20
  flags.usePrdAssociationTool = False
  return flags



@signatureActions
def beamSpot(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "beamSpot"
  flags.suffix   = "BeamSpot"
  flags.roi      = "HLT_Roi_FS"
  flags.doFullScan      = True
  flags.doZFinder       = True
  flags.DoubletDR_Max   = 200
  flags.SeedRadBinWidth = 10
  flags.etaHalfWidth    = 3
  flags.phiHalfWidth    = math.pi
  flags.doTRT           = False
  flags.doSeedRedundancyCheck = True
  flags.doRecord        = False
  return flags


@signatureActions
def fullScan(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "fullScan"
  flags.suffix   = "FS"
  flags.roi      = "HLT_Roi_FS"
  flags.vertex              = "HLT_IDVertex_FS"
  flags.adaptiveVertex      = True
  # these are being evaluated and may be added
  # flags.addSingleTrackVertices = True
  # flags.TracksMaxZinterval = 3
  flags.vertex_jet          = "HLT_IDVertex_FS"
  flags.adaptiveVertex_jet  = True
  flags.doFullScan      = True
  flags.etaHalfWidth    = 3.
  flags.phiHalfWidth    = math.pi
  flags.doTRT           = False
  flags.DoubletDR_Max   = 200
  flags.SeedRadBinWidth = 10
  flags.doSeedRedundancyCheck = True
  flags.TripletDoPPS    = False
  flags.nClustersMin    = 8
  flags.UseTrigSeedML   = 4
  flags.dodEdxTrk         = True
  flags.doHitDV           = True
  flags.doDisappearingTrk = True if recoMode=="InDet" else False
  flags.roadWidth =         5.
  return flags


@signatureActions
def beamSpotFS(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "fullScan"
  flags.suffix   = "FS"
  flags.roi      = "HLT_Roi_FS"
  flags.doFullScan      = True
  flags.etaHalfWidth    = 3.
  flags.phiHalfWidth    = math.pi
  flags.doTRT           = False
  flags.DoubletDR_Max   = 200
  flags.SeedRadBinWidth = 10
  flags.TripletDoPPS    = False
  flags.nClustersMin    = 8
  flags.UseTrigSeedML   = 4
  flags.doRecord        = False
  return flags


@signatureActions
def cosmics(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name        = "cosmics"
  flags.suffix      = "Cosmic"
  flags.roi         = "HLT_Roi_Cosmics"
  flags.Triplet_D0Max       = 1000.0
  flags.Triplet_D0_PPS_Max  = 1000.0
  flags.TrackInitialD0Max   = 1000.
  flags.TrackZ0Max          = 1000.
  flags.doTRT           = False      
  flags.doFullScan      = True
  flags.etaHalfWidth    = 3
  flags.phiHalfWidth    = math.pi

  flags.minPT = tsetter(flags.minPT, 0.5*Units.GeV)

  flags.nClustersMin        = 4
  flags.minSiNotShared      = 3
  flags.maxShared           = 0
  flags.nHolesMax           = 3
  flags.maxSiHoles          = 3
  flags.maxSCTHoles         = 3
  flags.maxPixelHoles       = tsetter(flags.maxPixelHoles,3)
  flags.maxPrimaryImpact    = tsetter(flags.maxPrimaryImpact, 1000.*Units.mm)

  flags.Xi2max         = tsetter(flags.Xi2max,        60.)
  flags.Xi2maxNoAdd    = tsetter(flags.Xi2maxNoAdd,   100.)
  flags.maxDoubleHoles = tsetter(flags.maxDoubleHoles,1)
  
  flags.nWeightedClustersMin= 8
  flags.useSeedFilter       = True
  flags.usePrdAssociationTool = False     #for backward compatibility #2023fix?
  flags.roadWidth =        75.
  flags.maxZImpact=        tsetter(flags.maxZImpact,    10000.*Units.mm)
  if recoMode=="InDet":
    flags.minTRTonTrk         = 20

  return flags

@signatureActions
def bmumux(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name      = "bphysics"
  flags.suffix    = "Bmumux"
  flags.roi       = "HLT_Roi_Bmumux"
  flags.Triplet_D0Max       = 10.
  flags.DoPhiFiltering      = False
  flags.etaHalfWidth        = 0.75
  flags.phiHalfWidth        = 0.75
  flags.zedHalfWidth        = 50.
  flags.doSeedRedundancyCheck = True
  flags.SuperRoI = True
  return flags

@signatureActions
def bhh(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name      = "bhh"
  flags.suffix    = "Bhh"
  flags.roi       = "HLT_Roi_Bhh"
  flags.Triplet_D0Max       = 10.
  flags.DoPhiFiltering      = False
  flags.etaHalfWidth        = 2.5
  flags.phiHalfWidth        = math.pi-1.e-5
  flags.zedHalfWidth        = 30.
  flags.pTmin               = 2*Units.GeV
  flags.minPT = tsetter(flags.minPT, flags.pTmin)
  flags.doSeedRedundancyCheck = True
  flags.SuperRoI = True
  return flags

@signatureActions
def electronLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name       = "electronLRT"
  flags.suffix     = "ElecLRT"
  flags.roi        = "HLT_Roi_Electron"
  flags.etaHalfWidth        = 0.1
  flags.phiHalfWidth        = 0.4
  flags.UsePixelSpacePoints = False
  flags.Triplet_D0Max       = 300.
  flags.TrackInitialD0Max   = 300.
  flags.TrackZ0Max          = 500.
  flags.zedHalfWidth        = 225.
  flags.keepTrackParameters = True
  flags.doSeedRedundancyCheck = True
  flags.nClustersMin        = 8
  flags.isLRT               = True
  #pt config
  flags.maxPrimaryImpact    = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact         = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False
  return flags


@signatureActions
def muonLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name       = "muonLRT"
  flags.suffix     = "MuonLRT"
  flags.roi        = "HLT_Roi_Muon"
  flags.UsePixelSpacePoints = False
  flags.etaHalfWidth        = 0.2
  flags.phiHalfWidth        = 0.4
  flags.Triplet_D0Max       = 300.
  flags.TrackInitialD0Max   = 300.
  flags.TrackZ0Max          = 500.
  flags.zedHalfWidth        = 225.
  flags.doSeedRedundancyCheck = True
  flags.nClustersMin        = 8
  flags.isLRT               = True
  flags.doResMon            = True
  flags.DoPhiFiltering      = False
  #pt config
  flags.maxPrimaryImpact    = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact         = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False

  return flags



@signatureActions
def tauLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "tauLRT"
  flags.suffix   = "TauLRT"
  flags.roi      = "HLT_Roi_TauLRT"
  flags.vertex   = "HLT_IDVertex_Tau" # TODO: does this need renaming?
  flags.pTmin    = 0.8*Units.GeV
  flags.minPT = tsetter(flags.minPT, flags.pTmin)
  flags.etaHalfWidth = 0.4
  flags.phiHalfWidth = 0.4
  flags.zedHalfWidth = 225.
  flags.UsePixelSpacePoints = False
  flags.Triplet_D0Max       = 300.
  flags.TrackInitialD0Max   = 300.
  flags.TrackZ0Max          = 500.
  flags.nClustersMin        = 8
  flags.isLRT               = True
  #pt config
  flags.maxPrimaryImpact    = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact         = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False
  if recoMode=="InDet":
    flags.doTRT        = True
    
  return flags


@signatureActions
def bjetLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "bjetLRT"
  flags.suffix   = "BjetLRT"
  flags.roi      = "HLT_Roi_Bjet"
  flags.etaHalfWidth = 0.4
  flags.phiHalfWidth = 0.4
  flags.UsePixelSpacePoints = False
  flags.Triplet_D0Max       = 300.
  flags.TrackInitialD0Max   = 300.
  flags.TrackZ0Max          = 500.
  flags.nClustersMin        = 8
  flags.isLRT               = True
  #pt config
  flags.maxPrimaryImpact    = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact         = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False

  return flags


@signatureActions
def fullScanLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "fullScanLRT"
  flags.suffix   = "FSLRT"
  flags.roi      = "HLT_Roi_FS"
  flags.doFullScan      = True
  flags.etaHalfWidth    = 3.
  flags.phiHalfWidth    = math.pi
  flags.doTRT           = False
  flags.doSeedRedundancyCheck = True
  flags.UsePixelSpacePoints   = False
  flags.Triplet_D0Max         = 300.
  flags.TrackInitialD0Max     = 300.
  flags.TrackZ0Max            = 500.
  flags.Triplet_D0_PPS_Max    = 300.
  flags.DoubletDR_Max         = 200
  flags.nClustersMin          = 8
  flags.isLRT                 = True
  #pt config
  flags.maxPrimaryImpact      = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact           = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False
  
  return flags


@signatureActions
def DJetLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "DJetLRT"
  flags.suffix   = "DJLRT"
  flags.roi      = "HLT_Roi_DJ"
  flags.doFullScan      = False
  flags.etaHalfWidth    = 0.4
  flags.phiHalfWidth    = 0.4
  flags.zedHalfWidth    = 225.
  flags.doTRT           = False
  flags.doSeedRedundancyCheck = True
  flags.UsePixelSpacePoints   = False
  flags.Triplet_D0Max         = 300.
  flags.TrackInitialD0Max     = 300.
  flags.TrackZ0Max            = 500.
  flags.Triplet_D0_PPS_Max    = 300.
  flags.DoubletDR_Max         = 200
  flags.nClustersMin          = 8
  flags.isLRT                 = True
  #pt config
  flags.maxPrimaryImpact      = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact           = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False
  
  return flags


@signatureActions
def DVtxLRT(flags: AthConfigFlags, instanceName: str, recoMode: str) -> AthConfigFlags:

  flags.input_name = instanceName
  flags.name     = "DVtxLRT"
  flags.suffix   = "DVLRT"
  flags.roi      = "HLT_Roi_DV"
  flags.doFullScan      = False
  flags.etaHalfWidth    = 0.35
  flags.phiHalfWidth    = 0.35
  flags.doTRT           = False
  flags.doSeedRedundancyCheck = True
  flags.UsePixelSpacePoints   = False
  flags.Triplet_D0Max         = 300.
  flags.TrackInitialD0Max     = 300.
  flags.TrackZ0Max            = 500.
  flags.Triplet_D0_PPS_Max    = 300.
  flags.DoubletDR_Max         = 200
  flags.nClustersMin          = 8
  flags.isLRT                 = True
  #pt config
  flags.maxPrimaryImpact      = tsetter(flags.maxPrimaryImpact, 300.*Units.mm)
  flags.maxEMImpact           = tsetter(flags.maxEMImpact, 300.*Units.mm)
  flags.maxEta            = 2.7
  flags.doEmCaloSeed      = False

  return flags


def derivedFromSignatureFlags(flags: AthConfigFlags, recoMode : str):

  flags.trkTracks_FTF     = f'HLT_IDTrkTrack_{flags.suffix}_FTF'
  flags.trkTracks_IDTrig  = f'HLT_IDTrkTrack_{flags.suffix}_IDTrig'
  flags.tracks_FTF    = collToRecordable(flags, f'HLT_IDTrack_{flags.suffix}_FTF')
  # ToDo: shouldn't be setting flags using this if type structures, the flags should be
  #       actually set somewhere in appropriate config functions
  flags.tracks_IDTrig = collToRecordable(flags,"HLT_IDTrack_{}_IDTrig".format(flags.suffix if flags.input_name != "tauIso" else "Tau"))

  if recoMode == "Acts":
    flags.trkTracks_FTF     = f'HLT_Acts_{flags.suffix}_Tracks'
    flags.trkTracks_IDTrig  = f'HLT_Acts_{flags.suffix}_Ambi_Tracks'

  if flags.isLRT:             # to be moved to a separate function once LRTs differ 
    flags.minClusters         = tsetter(flags.minClusters         , 8)
    flags.nHolesGapMax        = tsetter(flags.nHolesGapMax        , 1)
    flags.nWeightedClustersMin= tsetter(flags.nWeightedClustersMin, 8)
    flags.maxSiHoles          = tsetter(flags.maxSiHoles          , 2)
    flags.maxSCTHoles         = tsetter(flags.maxSCTHoles         , 1)
    flags.maxPixelHoles       = tsetter(flags.maxPixelHoles       , 1)
    flags.maxDoubleHoles      = tsetter(flags.maxDoubleHoles      , 0)
    flags.maxZImpact          = tsetter(flags.maxZImpact          , 500.)
    
    
  if recoMode == "ITk":
    flags.extension           = flags.input_name       #needed in the ITk mode?
    
    if flags.isLRT:
      flags.UsePixelSpacePoints = True                 #In LRT cases they use only SCT SP, but for ITk we want pixel SP


def collToRecordable(flags,name):
  # ToDo: should just be a flag set per signature in the per signature config
  #       and not setting parameters using tests on the signature name
  ret = name
  signature = flags.input_name
  firstStage = True if "FTF" in name else False
  record = True
  if firstStage:
    if signature in ["minBias","minBiasPixel","bjetLRT",
                     "beamSpot","BeamSpot"]:
      record = False
  else:
    if signature in ["tauCore","tauIso","tauIsoBDT",
                     "jet","fullScan","FS","jetSuper","bhh",
                     "beamSpot", "BeamSpot","beamSpotFS",
                     "bjetLRT","DJetLRT","DVtxLRT"]:
      record = False

  if record:
    ret = recordable(name)
      
  return ret

def addGlobalFlags(flags: AthConfigFlags, category : str):
  flags.addFlag(f'{category}.RoiZedWidthDefault', 180.0 * Units.mm)
  flags.addFlag(f'{category}.doGPU', False)
  flags.addFlag(f'{category}.UseTrigTrackFollowing', False)
  flags.addFlag(f'{category}.UseTrigRoadPredictor', False)
  flags.addFlag(f'{category}.UseTracklets', False)
  flags.addFlag(f'{category}.trackletPoints', 1)
  flags.addFlag(f'{category}.PixelClusterCacheKey',    "PixelTrigClustersCache")
  flags.addFlag(f'{category}.SCTClusterCacheKey',      "SCT_ClustersCache")
  flags.addFlag(f'{category}.SpacePointCachePix',      "PixelSpacePointCache")
  flags.addFlag(f'{category}.SpacePointCacheSCT',      "SctSpacePointCache")
  flags.addFlag(f'{category}.SCTBSErrCacheKey',        "SctBSErrCache")
  flags.addFlag(f'{category}.SCTFlaggedCondCacheKey',  "SctFlaggedCondCache")
  flags.addFlag(f'{category}.SCTRDOCacheKey',          "SctRDOCache")
  flags.addFlag(f'{category}.PixRDOCacheKey',          "PixRDOCache")
  flags.addFlag(f'{category}.PixBSErrCacheKey',        "PixBSErrCache")
  flags.addFlag(f'{category}.TRTRDOCacheKey',          "TrtRDOCache")
  flags.addFlag(f'{category}.TRT_DriftCircleCacheKey', "TRT_DriftCircleCache")
  flags.addFlag(f'{category}.TRT_DriftCircleKey',      "TRT_TrigDriftCircles")
  flags.addFlag(f'{category}.PixClustersAmbiMap',      "TrigPixelClusterAmbiguitiesMap")
  flags.addFlag(f'{category}.ClusterAmbiguitiesMap',   "TrigPixelClusterAmbiguitiesMap")
  
import unittest

class FlagValuesTest(unittest.TestCase):
    def setUp(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Trigger.InDetTracking.electron.pTmin=3.
        self.newflags = flags.cloneAndReplace('Tracking.ActiveConfig', 'Trigger.InDetTracking.electron',  
                                              keepOriginal = True)
        self.newflags2 = flags.cloneAndReplace('Tracking.ActiveConfig', 'Trigger.InDetTracking.muonLRT',
                                               keepOriginal = True)
        
    def runTest(self):
        self.assertEqual(self.newflags.Tracking.ActiveConfig.pTmin,  3.,             msg="Preset value lost")        
        self.assertEqual(self.newflags.Tracking.ActiveConfig.input_name, "electron", msg="Incorrect config")
        self.assertEqual(self.newflags2.Tracking.ActiveConfig.input_name, "muonLRT", msg="Incorrect config")

  
if __name__ == "__main__":
    unittest.main()
