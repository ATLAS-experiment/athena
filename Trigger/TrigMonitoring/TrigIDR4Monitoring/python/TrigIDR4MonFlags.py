## noqa: ATL902
##
##    @file  TrigIDR4MonFlags.py
##
##           Flag configuration for TrigIDR4Mon.  Sets up IDTPM2 signature
##           nodes with default values, optionally applies a JSON overlay,
##           and prints a configuration summary.
##
##           Typical usage in run.py:
##
##             from TrigIDR4Monitoring.TrigIDR4MonFlags import TrigIDR4MonFlags
##
##             # all signatures (data or MC as appropriate)
##             TrigIDR4MonFlags(tflags)
##
##             # selected signatures only
##             TrigIDR4MonFlags(tflags, signatures=["Electrons", "Muons"])
##
##             # with a JSON overlay
##             TrigIDR4MonFlags(tflags, cfgFile="myconfig.json")
##
##           JSON format (all keys optional):
##             {
##               "Electrons": { "DeltaR": 0.03,
##                              "RefSelector": { "minPt": 2000 } },
##               "Muons":     { "MatcherType": "DeltaRZ" }
##             }
##
##   @author  sutt and simpson
##   @date    Sun 14 Sep 2026
##
##  $Id: TrigIDR4MonFlags.py, v0.0   Sun 14 Sep 2026  sutt $
##
## Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
##

_ALL_SIGS  = ["Electrons", "Muons", "Bjets"]
_DATA_SIGS = ["Taus"]
_MC_SIGS   = ["Offline", "MuonTruth", "ElectronTruth", "TauTruth", "BjetTruth"]

from TrigIDR4Monitoring.NodeUtils import decodeFlags

def TrigIDR4MonFlags( inflags, signatures=None, cfgFile=None) :
  """
  Top-level driver for TrigIDR4Mon flag configuration.

  Parameters
  ----------
  iflags     : Node   root node tree from decode_flags
  signatures : list or None
      Signatures to configure; None runs all appropriate for input type.
      Example: signatures=["Electrons", "Muons"]
  cfgFile    : str or None
      Path to a JSON config file that overrides Python defaults.
  """
  
  if type(inflags).__name__ == "Node": iflags = inflags
  else: iflags = decodeFlags( inflags )                             
  
  flags = iflags.addNode("IDTPM2")

  flags.useDecorations = False
    
  if cfgFile is not None :

    print( f"configuring from json file : {cfgFile}" )
    print( "---------------------------------------------" )

    loadJsonConfig( flags, cfgFile, signatures )
    
  else : 

    print( "configuring from flags ..." )
    print( "--------------------------" )

    loadPythonConfig( flags, signatures )
    
  _printConfig( flags, cfgFile )

  return iflags


# ---------------------------------------------------------------------------
# Common defaults applied to every signature node
# ---------------------------------------------------------------------------

def _setCommonDefaults( node ) :

  node.SliceTag = f"HLT/TRIDT/{node.name()}/Expert"

  # eta-phi-z (DeltaRZSinTheta) matching as default per meeting notes

  node.addNode( "Matcher")
  node.Matcher.MatcherType = "DeltaR"
  node.Matcher.DeltaR      =  0.05

  node.addNode("RefSelector")
  node.addNode("TestSelector")

  node.useDecorations = False

  node.RefSelector.addNode("InDetSelector")

  node.RefSelector.InDetSelector.minNPixelHits  = 2
  node.RefSelector.InDetSelector.maxNPixelHoles = 1
  node.RefSelector.InDetSelector.minNSctHits    = 3

  node.RefSelector.minPt  = 1000.0
  node.RefSelector.minEta =   -2.5
  node.RefSelector.maxEta =    2.5

  node.RefSelector.maxD0  =    4.0

  # offline selector for reference tracks, none for test tracks
  node.RefSelector.UseOfflineTool  = True
  node.TestSelector.UseOfflineTool = False


# ---------------------------------------------------------------------------
# Per-signature flag setters
# ---------------------------------------------------------------------------

def _setElectronFlags( flags ) :

  flags.Chains = [
    "HLT_e.*idperf.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",
    "HLT_e.*idperf.*:key=HLT_IDTrack_Electron_IDTrig",
    "HLT_e.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag:te=0",
    "HLT_e.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe:te=1"
  ]

  flags.Reference = "Electrons:Tight"

  flags.MinTagPT   = 14000
  flags.MinTagMass = 75
  flags.MaxTagMass = 105

  flags.RefSelector.minET     = 14000
  flags.RefSelector.minPt     = 1000
  flags.RefSelector.minETovpt = 0.8


def _setMuonFlags( flags ) :

  flags.Chains = [
    "HLT_mu.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
    "HLT_mu.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
    "HLT_mu.*_mu14.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
    "HLT_mu.*_mu14.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
    "HLT_mu.*_mu14.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu1_tag:te=0",
    "HLT_mu.*_mu14.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu1_probe:te=1"
  ]

  flags.Reference = "Muons:Medium"

  flags.MinTagPT   = 14000
  flags.MinTagMass = 75
  flags.MaxTagMass = 105

  flags.RefSelector.minET     = 14000
  flags.RefSelector.minPt     = 1000
  flags.RefSelector.minETovpt = 0


def _setTauFlags( flags ) :

  flags.Chains = [
    "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
    "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
    "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=tau1_tag:te=0",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:extra=tau1_probe:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=tau0_tag:te=0",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:extra=tau0_probe:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=tau_tag:te=0",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:extra=tau_probe:te=1"
  ]

  flags.Reference    = "TauJets:Loose:1Prong"
  flags.ReferenceTag = "Muons:Medium"

  flags.MinTagPT   = 14000
  flags.MinTagMass = 75
  flags.MaxTagMass = 105

  flags.RefSelector.minET     = 14000
  flags.RefSelector.minPt     = 1000
  flags.RefSelector.minETovpt = 0

  
def _setBjetFlags( flags ) :

  flags.Chains =  [
    "HLT_j.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",
    "HLT_j.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
    "HLT_j180.*dis.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",
    "HLT_j180.*dis.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS"
  ]
  
  flags.Matcher.MatcherType = "DeltaRZ"
  flags.Matcher.DeltaEta    =  0.05
  flags.Matcher.DeltaPhi    =  0.05
  flags.Matcher.DeltaZed    =  2

  flags.Matcher.delete("DeltaR")
  
  flags.Reference = "InDetTrackParticles"

  flags.RefSelector.minPt     = 1000
  flags.RefSelector.minETovpt = 0



  
def _setMuonTruthFlags( flags ) :

  flags.Chains = [
    "HLT_mu.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_tag",
    "HLT_mu.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_probe",
    "HLT_mu.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu1_tag",
    "HLT_mu.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu1_probe"
  ]

  flags.Reference = "TruthParticles"

  flags.MinTagPT   = 14000
  flags.MinTagMass = 75
  flags.MaxTagMass = 105

  flags.ReferencePdgIdTag = 13

  flags.RefSelector.InDetSelector.minNPixelHits  = -1
  flags.RefSelector.InDetSelector.maxNPixelHoles = -1
  flags.RefSelector.InDetSelector.minNSctHits    = -1

  flags.RefSelector.PdgId = 13
  # flags.RefSelector.FromPdgId = 13

  flags.RefSelector.minPt  = 10000.0
  flags.RefSelector.minEta =    -2.5
  flags.RefSelector.maxEta =     2.5

  flags.RefSelector.maxD0  =    4.0

  flags.RefSelector.UseOfflineTool  = False
  flags.RefSelector.addNode("TruthSelector")
  flags.TestSelector.UseOfflineTool = False


def _setElectronTruthFlags( flags ) :

  flags.Chains = [
    "HLT_e.*idperf.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",
    "HLT_e.*idperf.*:key=HLT_IDTrack_Electron_IDTrig",
    "HLT_e.*_idperf.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag",
    "HLT_e.*_idperf.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe"
  ]

  flags.Reference         = "TruthParticles"
  flags.ReferencePdgIdTag = 11

  flags.MinTagPT   = 14000
  flags.MinTagMass = 75
  flags.MaxTagMass = 105

  flags.RefSelector.InDetSelector.minNPixelHits  = -1
  flags.RefSelector.InDetSelector.maxNPixelHoles = -1
  flags.RefSelector.InDetSelector.minNSctHits    = -1

  flags.RefSelector.PdgId = 11
  # flags.RefSelector.FromPdgId = 11

  flags.RefSelector.minPt  = 10000.0
  flags.RefSelector.minEta =    -2.5
  flags.RefSelector.maxEta =     2.5

  flags.RefSelector.maxD0  =     4.0

  flags.RefSelector.UseOfflineTool  = False
  flags.RefSelector.addNode("TruthSelector")
  flags.TestSelector.UseOfflineTool = False



def _setTauTruthFlags( flags ) :

  flags.Chains = [
    "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
    "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
    "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=tau1_tag:te=0",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:extra=tau1_probe:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=tau0_tag:te=0",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:extra=tau0_probe:te=1",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=tau_tag:te=0",
    "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:extra=tau_probe:te=1"
  ]

  flags.Reference         = "TruthParticles"
  flags.ReferencePdgIdTag = 11

  flags.MinTagPT   = 14000
  flags.MinTagMass = 75
  flags.MaxTagMass = 105

  flags.RefSelector.InDetSelector.minNPixelHits  = -1
  flags.RefSelector.InDetSelector.maxNPixelHoles = -1
  flags.RefSelector.InDetSelector.minNSctHits    = -1

  # flags.RefSelector.PdgId = 13
  flags.RefSelector.FromPdgId = 15

  flags.RefSelector.minPt  = 10000.0
  flags.RefSelector.minEta =    -2.5
  flags.RefSelector.maxEta =     2.5

  flags.RefSelector.maxD0  =    4.0

  flags.RefSelector.addNode("TruthSelector")

  flags.RefSelector.UseOfflineTool  = False
  flags.TestSelector.UseOfflineTool = False

  
def _setBjetTruthFlags( flags ) :
  
  flags.Chains =  [
    "HLT_j.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",
    "HLT_j.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
    "HLT_j180.*dis.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",
    "HLT_j180.*dis.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS"
  ]

  flags.Reference = "TruthParticles"

  flags.RefSelector.InDetSelector.minNPixelHits  = -1
  flags.RefSelector.InDetSelector.maxNPixelHoles = -1
  flags.RefSelector.InDetSelector.minNSctHits    = -1

  flags.RefSelector.minPt  = 1000.0
  flags.RefSelector.minEta =   -2.5
  flags.RefSelector.maxEta =    2.5

  flags.RefSelector.maxD0  =    4.0

  flags.RefSelector.UseOfflineTool  = False
  flags.RefSelector.addNode("TruthSelector")
  flags.TestSelector.UseOfflineTool = False

  flags.RefSelector.minPt     = 1000
  flags.RefSelector.minETovpt = 0


def _setOfflineFlags( flags ) :

  flags.SliceTag = "Offline"
    
  flags.Matcher.MatcherType = "OfflineTruth"
  flags.Matcher.Prob = 0.5
  
  flags.Chains =  [ "InDetTrackParticles" ]

  flags.Reference = "TruthParticles"

  flags.RefSelector.UseOfflineTool  = False
  flags.RefSelector.addNode("TruthSelector")

  flags.RefSelector.minPt     = 1000
  flags.RefSelector.minETovpt = 0


_configs = {
  "Electrons"     : _setElectronFlags,
  "Muons"         : _setMuonFlags,
  "Taus"          : _setTauFlags,
  "Bjets"         : _setBjetFlags,
  "ElectronTruth" : _setElectronTruthFlags,
  "MuonTruth"     : _setMuonTruthFlags,
  "TauTruth"      : _setTauTruthFlags,
  "BjetTruth"     : _setBjetTruthFlags,
  "Offline"       : _setOfflineFlags,
}


# ---------------------------------------------------------------------------
# standard python flags configuration
# ---------------------------------------------------------------------------

def loadPythonConfig( flags, signatures=None ) :
  
    all_signatures = list(_ALL_SIGS)

    if flags.Input.isMC : all_signatures += _MC_SIGS
    else:                 all_signatures += _DATA_SIGS 

    found = False

    if signatures is not None: 
      for signame in signatures :
        if signame not in all_signatures : print( f"WARNING: signature {signame} not available" ) 
    
    for signame in all_signatures:

      if signatures is not None and signame not in signatures : continue
      
      found = True
      
      node = flags.addNode(signame)

      _setCommonDefaults(node)
      fun = _configs.get(node.name())
      if fun is not None : fun(node)
        
    if found is False : raise RuntimeError( f"no matching signatures found for {signatures}" )


# ---------------------------------------------------------------------------
# Optional JSON configuration
# ---------------------------------------------------------------------------

def loadJsonConfig( flags, filename, signatures=None ) :
  """
  Optional: overlay a JSON config file onto existing flag defaults.

  The JSON should be a dict keyed by signature name (e.g. "Electrons"),
  with nested dicts for sub-nodes (e.g. "RefSelector") and scalar
  values for flags. Any value set here overrides the Python defaults
  set by TrigIDR4MonFlags().

  Example JSON:
      {
        "Electrons": { "RefSelector": { "minPt": 2000 } },
        "Muons":     { "MatchingDeltaR": 0.03 }
      }
  """
  import json

  print("loading json config ...")

  from TrigIDR4Monitoring.TrigIDR4MonConfig import file_exists
  file_exists( filename )
  
  with open(filename, "r") as f:
    data = json.load(f)

  found = False

  matched = []
  
  for signame, sigdata in data.items():
    if signatures is None or signame in signatures :
      found = True
      matched.append( signame )
      flags.addNode(signame)  
      _applyDict(flags.node(signame), sigdata)
      
  if found is False : raise RuntimeError( f"no matching signatures found for {signatures}" )

  if signatures is not None :
    for signame in signatures :
      if signame not in matched : print( f"WARNING: signature {signame} not available" )  


      
def _applyDict( node, d ) :
  """Recursively apply a dict of values onto a Node."""
  for key, value in d.items():
    if isinstance(value, dict):
      if not node.hasNode(key): node.addNode(key)
      _applyDict(node.node(key), value)
    else:
      setattr(node, key, value)





# ---------------------------------------------------------------------------
# Configuration summary printout
# ---------------------------------------------------------------------------

def _printConfig(flags, cfgFile) :
  """ Print a human-readable summary of the active flag configuration. """

  nodes = list(flags.nodes())

  print()
  print("  TrigIDR4MonFlags: configuration summary")
  print("  ---------------------------------------")

  if cfgFile is None : print( "  JSON config : none  (using Python defaults)")
  else :               print(f"  JSON config : {cfgFile}")

  print(f"  Signatures  : {', '.join(n.name() for n in nodes)}"
        f"  ({len(nodes)} active)")

  print("")

  for node in nodes :
    print(f"  {node.name()}:" )
    for key, value in node.Matcher.node_flags() : print(f"    Matcher   : {key} {value}") 
    print(f"    Reference : {node.Reference      if node.has('Reference') else '?'}")
    print(f"    Patterns  : {len(node.Chains)    if node.has('Chains')    else 0} pattern(s)")
    print()
