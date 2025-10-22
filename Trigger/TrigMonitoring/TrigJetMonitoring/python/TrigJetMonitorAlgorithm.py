#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''@file MTMonitoring.py
@authors P-A. Delsart, Jona Bossio
@date    03/04/2020
@brief   Python configuration for the Run III Trigger Jet Monitoring
'''

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)

from TrigDecisionTool.TrigDecisionToolConfig import getRun3NavigationContainerFromInput

import math
import re
import copy


#####################################
# constants
#####################################

copySuffix = "copied" # suffix for jet containters that are duplicated for monitoring

#####################################
# Offline jet collections to monitor
#####################################

OfflineJetCollections = dict()

OfflineJetCollections['pp'] = {
  'AntiKt4EMTopoJets'  : { 'MatchTo' : 'AntiKt4EMPFlowJets' },
 'AntiKt4EMPFlowJets' : { 'MatchTo' : 'NONE' },
 #'AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets' : { 'MatchTo' : 'NONE' }, # Remove until ATR-25800 is fixed
}

OfflineJetCollections['HI'] = {
  'AntiKt4HIJets'  : { 'MatchTo' : 'AntiKt4HIJets' },
}

###########################################
# L1 jet collections and chains to monitor
###########################################

# The MatchedTo list must be either empty of length 2, and contain the names of an offline collection
# and an HLT collection. These names can be the empty string.

# the strings in L1JetCollections are jet container names.
L1JetCollections = dict()

match_smallRL1_OfflineJets_List = ['AntiKt4EMPFlowJets', 'HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf']
# temporarily modified to use small-R offline jet in turn-on to fix tier0 jet mon crash ATR-25800!! - throws exception if < 2 jet collections provided
match_largeRL1_OfflineJets_List = ['AntiKt4EMPFlowJets', 'HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftf']
match_HIL1_OfflineJets_List = ['AntiKt4HIJets', 'HLT_AntiKt4HIJets']

L1JetCollections['pp'] = {
  'L1_jFexSRJetRoI': {'MatchTo': match_smallRL1_OfflineJets_List},
  'L1_gFexSRJetRoI': {'MatchTo': match_smallRL1_OfflineJets_List},
  'L1_gFexLRJetRoI': {'MatchTo': match_largeRL1_OfflineJets_List},
}

L1JetCollections['HI'] = {
  'L1_jFexSRJetRoI': {'MatchTo': match_HIL1_OfflineJets_List},
  'L1_gFexSRJetRoI': {'MatchTo': match_HIL1_OfflineJets_List},
}

for case in L1JetCollections.keys():
  try:
    items = L1JetCollections[case].items()
  except (AttributeError, TypeError):
    raise RuntimeError('L1JetCollections for %s is not a mapping type'%case)

  for k, d in items:
    try:
      d_items = d.items()
    except (AttributeError, TypeError):
      raise RuntimeError('L1JetCollections value for %s is not a mapping type'%case)

    if 'MatchTo' not in d:
      errmsg = 'L1Collections entry %s has no (possibly empty) MatchType list' % (
        str(k))
      raise RuntimeError(errmsg)

# Now seeing new L1 containers of differing types. These types
# are explicit in the C++ JetMatcher algorithm, and correspond
# top different attributes of that algorithm.
#
# l1Coll2MatcherKey supplies the python name of
# C++ component attribute.

l1Coll2MatcherKey = {
  'L1_jFexSRJetRoI': 'L1jFexSRJetRoIContainerName',
  'L1_gFexSRJetRoI': 'L1gFexJetRoIContainerName',
  'L1_gFexLRJetRoI': 'L1gFexJetRoIContainerName',
}

for case in L1JetCollections.keys():
  for k, d in L1JetCollections[case].items():
    if d['MatchTo']:  # exists by previous checks. check if empty.
      if k not in l1Coll2MatcherKey:
        errmsg = 'Match(es) to an L1 container requested  entry '\
        '%s but no C++ MatcherAlg attribute name provided' % (str(k),)
        raise RuntimeError(errmsg)
      

# the values of Chain2L1JetCollDict are keys of L1JetCollections.
# the keys of Chain2L1JetCollDict are used to select events before histograms are filled

Chain2L1JetCollDict = dict()

Chain2L1JetCollDict['pp'] = { # set L1 jet collection name for L1 jet chains
  
  'L1_jJ40': ['L1_jFexSRJetRoI'],
  'L1_jJ50': ['L1_jFexSRJetRoI'],
  'L1_jJ160': ['L1_jFexSRJetRoI'],
  'L1_jJ85p0ETA21_3jJ40p0ETA25': ['L1_jFexSRJetRoI'],
  'L1_3jJ70p0ETA23': ['L1_jFexSRJetRoI'],
  'L1_4jJ40': ['L1_jFexSRJetRoI'],


  'L1_gJ20p0ETA25': ['L1_gFexSRJetRoI'],
  'L1_gJ50p0ETA25': ['L1_gFexSRJetRoI'],
  'L1_gJ100p0ETA25': ['L1_gFexSRJetRoI'],
  'L1_gJ400p0ETA25': ['L1_gFexSRJetRoI'],

  'L1_gLJ80p0ETA25': ['L1_gFexLRJetRoI'],
  'L1_gLJ100p0ETA25': ['L1_gFexLRJetRoI'],
  'L1_gLJ140p0ETA25': ['L1_gFexLRJetRoI'],
  'L1_gLJ160p0ETA25': ['L1_gFexLRJetRoI'],

  'L1_SC111-CjJ40': ['L1_jFexSRJetRoI'],
  'L1_HT190-jJ40s5pETA21': ['L1_jFexSRJetRoI'],
}

Chain2L1JetCollDict['HI'] = {

  'L1_jJ40': ['L1_jFexSRJetRoI'],
  'L1_jJ55': ['L1_jFexSRJetRoI'],
  'L1_jJ80': ['L1_jFexSRJetRoI'],

  'L1_jJ40p30ETA49': ['L1_jFexSRJetRoI'],
}

############################################
# HLT jet collections and chains to monitor
############################################

# List of HLT jet collections (stating
# which should be matched and to which offline jet collection

JetCollections = dict()

JetCollections['pp'] = {
  'HLT_AntiKt4EMTopoJets_subjesIS'                                : { 'MatchTo' : 'AntiKt4EMPFlowJets'}, # default small-R EM
  'HLT_AntiKt4EMTopoJets_subjesIS_fastftag'                       : { 'MatchTo' : 'NONE'}, # small-R EM jets with RoI tracking & fast flavour tagging
  'HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf'                      : { 'MatchTo' : 'AntiKt4EMPFlowJets'}, # a4 calo jet w/ FTF
  'HLT_AntiKt4EMTopoJets_subjesgscIS_ftf'                         : { 'MatchTo' : 'AntiKt4EMPFlowJets'}, # a4 calo jet w/ calo+track GSC, reconstructed by MET
  'HLT_AntiKt4EMPFlowJets_subjesgscIS_ftf'                        : { 'MatchTo' : 'AntiKt4EMPFlowJets'}, # a4 pflow w/ calo+track GSC, reconstructed by MET
  'HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf'                     : { 'MatchTo' : 'AntiKt4EMPFlowJets'}, # a4 pflow w/ residual + calo+track GSC
  'HLT_AntiKt4EMPFlowJets_nojcalib_ftf'                           : { 'MatchTo' : 'NONE'},               # a4 pflow nojcalib
  'HLT_AntiKt10EMTopoRCJets_subjesIS'                             : { 'MatchTo' : 'NONE'},               # a10r
  'HLT_AntiKt10LCTopoJets_subjes'                                 : { 'MatchTo' : 'NONE'},               # a10
  'HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_jes'              : { 'MatchTo' : 'NONE'}, # a10t
  'HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_nojcalib_ftf' : { 'MatchTo' : 'NONE'},               # a10sd pflow cssk nojcalib
  'HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftf'      : { 'MatchTo' : 'NONE'},               # a10sd pflow cssk jes
}

JetCollections['HI'] = {
  'HLT_AntiKt4HIJets'  : {'MatchTo': 'AntiKt4HIJets'},
  'HLT_AntiKt4EMPFlowJets_jes_ftf' : {'MatchTo': 'AntiKt4HIJets'},
}


def getChains2Monitor(inputFlags, monMode):

  Chains2Monitor = dict()
  from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
  monAccess = getHLTMonitoringAccess(inputFlags)
  
  # set HLT jet collection, reference chain and offline jet collection
  # for turn-on curves 
  ListOfMonChains = monAccess.monitoredChains(signatures="jetMon", monLevels = ["shifter","t0"])

  default_dict = {"HLTColl": "NONE", "RefChain": "NONE" ,"OfflineColl": "NONE"}
  Chains2Monitor[monMode] = dict((chain, default_dict.copy()) for chain in ListOfMonChains)  

  if monMode == 'HI':
    for chainName in Chains2Monitor['HI']:
      if '_ion_' in chainName:
          Chains2Monitor['HI'][chainName]["HLTColl"] = "HLT_AntiKt4HIJets"
          Chains2Monitor['HI'][chainName]["OfflineColl"] = "AntiKt4HIJets"
      else:
          Chains2Monitor['HI'][chainName]["HLTColl"] = "HLT_AntiKt4EMPFlowJets_jes_ftf"
          Chains2Monitor['HI'][chainName]["OfflineColl"] = "AntiKt4EMPFlowJets"

    # only HLT_noalg get efficiency curves by default, so...
    # these are additional hard-coded chains for efficiency monitoring
    #Standard HI chains (obsolete for O+O)
    if Chains2Monitor['HI'].get('HLT_j60_ion_L1jJ40'): Chains2Monitor['HI']['HLT_j60_ion_L1jJ40'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})
    if Chains2Monitor['HI'].get('HLT_j75_ion_L1jJ55'): Chains2Monitor['HI']['HLT_j75_ion_L1jJ55'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})
    if Chains2Monitor['HI'].get('HLT_j85_ion_L1jJ55'): Chains2Monitor['HI']['HLT_j85_ion_L1jJ55'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})
    if Chains2Monitor['HI'].get('HLT_j150_ion_L1jJ80'): Chains2Monitor['HI']['HLT_j150_ion_L1jJ80'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})
    if Chains2Monitor['HI'].get('HLT_j200_ion_L1jJ80'): Chains2Monitor['HI']['HLT_j200_ion_L1jJ80'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})

    if Chains2Monitor['HI'].get('HLT_j50f_ion_L1jJ40p30ETA49'): Chains2Monitor['HI']['HLT_j50f_ion_L1jJ40p30ETA49'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})
    if Chains2Monitor['HI'].get('HLT_j60f_ion_L1jJ40p30ETA49'): Chains2Monitor['HI']['HLT_j60f_ion_L1jJ40p30ETA49'].update({"RefChain": "HLT_noalg_L1jTE50", "OfflineColl": "AntiKt4HIJets"})

  elif monMode == "pp":  
    # logic to define HLTColl, RefChain, OfflineColl
    for chainName in Chains2Monitor['pp']:
      Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt4EMTopoJets_subjesIS"
      if '_pf_' in chainName and 'a10' not in chainName: 
        Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf"
        Chains2Monitor['pp'][chainName]["OfflineColl"] = "AntiKt4EMPFlowJets"
      elif 'a10' in chainName:
        if 'a10t' in chainName: Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_jes"
        elif 'sd_cssk_pf' in chainName: Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftf"
        elif 'a10r' in chainName: Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt10EMTopoRCJets_subjesIS"
        else: Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt10LCTopoJets_subjes"
      elif '_noalg_' in chainName: 
        Chains2Monitor['pp'][chainName]["RefChain"] = "HLT_j45_pf_ftf_preselj20_L1jJ40" # temporarily modify to using small-R jet in turn-on for both small and large-R jets to fix tier0 jet mon crash ATR-25800!!
        Chains2Monitor['pp'][chainName]["OfflineColl"] = "AntiKt4EMPFlowJets"
        if 'gLJ' in chainName: Chains2Monitor['pp'][chainName]["HLTColl"] = "HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftf"
      else: continue


    if Chains2Monitor['pp'].get('HLT_j420_L1jJ160'): Chains2Monitor['pp']['HLT_j420_L1jJ160'].update({"RefChain": "HLT_j85_L1jJ50", "OfflineColl": "AntiKt4EMPFlowJets"})
    if Chains2Monitor['pp'].get('HLT_3j200_L1jJ160'): Chains2Monitor['pp']['HLT_3j200_L1jJ160'].update({"RefChain": "HLT_j85_L1jJ50", "OfflineColl": "AntiKt4EMPFlowJets"})    
    if Chains2Monitor['pp'].get('HLT_4j120_L13jJ90'): Chains2Monitor['pp']['HLT_4j120_L13jJ90'].update({"RefChain": "HLT_j85_L1jJ50", "OfflineColl": "AntiKt4EMPFlowJets"}) 
    if Chains2Monitor['pp'].get('HLT_5j80_pf_ftf_presel5j50_L14jJ40'): Chains2Monitor['pp']['HLT_5j80_pf_ftf_presel5j50_L14jJ40'].update({"RefChain": "HLT_j45_pf_ftf_preselj20_L1jJ40", "OfflineColl": "AntiKt4EMPFlowJets"})    
    if Chains2Monitor['pp'].get('HLT_j400_pf_ftf_L1jJ160'): Chains2Monitor['pp']['HLT_j400_pf_ftf_L1jJ160'].update({"RefChain": "HLT_j85_pf_ftf_preselj50_L1jJ50", "OfflineColl": "AntiKt4EMPFlowJets"})    
    if Chains2Monitor['pp'].get('HLT_j400_pf_ftf_preselj225_L1jJ160'): Chains2Monitor['pp']['HLT_j400_pf_ftf_preselj225_L1jJ160'].update({"RefChain": "HLT_j85_pf_ftf_preselj50_L1jJ50", "OfflineColl": "AntiKt4EMPFlowJets"})

  else: 
    errmsg = 'Returned empty Chains2Monitor due to invalid monMode'
    raise RuntimeError(errmsg)
  return Chains2Monitor

#########################################################
# Helpful functions
#########################################################

def getEtaRange(chain):
  etaMin,etaMax = 0,2.5 # central jets by default

  if 'eta490_j' in chain: #workaround for the UPC (ultra-peripheral) trigger chains
    etaMin,etaMax = 0,4.9
  elif 'eta' in chain:
    etaParts    = chain.split('eta')
    etaMinTemp  = etaParts[0].split('_')
    etaMin      = etaMinTemp[len(etaMinTemp)-1]
    etaMin      = int(etaMin)/10
    etaMax      = etaParts[1].split('_')[0]
    etaMax      = int(etaMax)/10

  # Check for 2-3 digit number before f_ for forward triggers
  if re.search(r'\d{2,3}f_', chain): #workaround for the HLT forward triggers with 2-3 digit numbers
    etaMin,etaMax = 3.2,4.9

  if 'a_' in chain: #new full eta range triggers
    etaMin,etaMax = 0,4.9
  
  return etaMin,etaMax

def getBinningFromThreshold(chain,varname):
  #default binning if nothing below applies
  xbins, xmin, xmax = 160,0.,800000.
  #pt and et binning based on threshold
  if varname == "pt" or varname == "et":
    if 'noalg' in chain:
        if 'jJ500' in chain or 'J400' in chain: return 160,xmin,800000 
        else: return 100,xmin,500000 # good enough for L1 jJ40 & jJ100
    else:
        #threshold = int(chain.split("_")[1].split('j')[1])
        threshold = int(re.search(r'\d+',chain.split("_")[1].split('j')[1]).group())
    if threshold < 50:
      return 40, 0., 100000.
    if threshold < 120:
      return 36, 20000., 200000.

    xbins = 40
    xmin = 50000.+100000.*(int(threshold/100)-1) #example: threshold = 330 -> 250 to 450; threshold = 420 -> 350 to 550
    if threshold % 100 == 0: #gives enough low bins if threshold is an exact divider of 100 GeV such as 3j200
      xmin = 1000.*(threshold - 100.)
    xmax = xmin + 200000.
    if "a10" in chain: # efficiency curve broader for large-R jets
      xmin = xmin - 50000. 
      xmax = xmax + 50000. 
      if "pf" in chain:
        xmax = xmax + 50000. # needed to include efficiency plateau for large-R PFlow chains
        if "smc" in chain:
          xmax = xmax + 50000. # efficiency plateau even higher for a10 pdf smc chains due to imperfect calibration
  #mass binning for large-R smc chains
  elif varname == "m":
    xbins, xmin, xmax = 35, 30000., 100000.
  return xbins, xmin, xmax

def getHTBinning(chain,binwidth):
  parts = chain.split('HT')
  threshold = parts[1].split('_')[0]
  if 'XX' in threshold:
    threshold = threshold.split('XX')[0]
  xmin = int(0.9 * int(threshold))  # xmin to make the threshold visible
  xmax = xmin + 500
  xbins = int((xmax-xmin)/binwidth)-1
  return xbins, xmin, xmax

# Add fast flavour-tag monitoring.
# Adds a 20 GeV jet pT cut to avoid FPE WARNINGS from jets below min jet pT for RoI track association
def addFlavourTagVariables(conf, network_prefix, flavs="cub"):
    cutname='pt20'
    fillerTools = []
    for f in flavs:
      xvar = f"{network_prefix}_p{f}"
      varname = f"ftag_p{f}"
      fillerTools += [HistoSpec(varname, xvar=xvar, bins=(70, -0.2, 1.2), title=f"{varname};{varname};;Entries")]
    fastDipsSelectSpec = SelectSpec(f"{network_prefix}_{cutname}", '20<pt:GeV&|eta|<3.2', path='NoTriggerSelection/'+cutname, FillerTools=fillerTools)
    conf.appendHistos(fastDipsSelectSpec)

#########################################################
# Schedule more histograms for dedicated jet collections
#########################################################
from JetMonitoring.JetMonitoringConfig import JetMonAlgSpec, HistoSpec, EventHistoSpec, SelectSpec, ToolSpec #VarSpec can be added to define specific/custom variables
from AthenaConfiguration.ComponentFactory import CompFactory

# All offline jet collections
ExtraOfflineHists = [
  "EMFrac",
  "HECFrac",
  "Jvt",
  "JVFCorr",
  "JvtRpt",
  "NumTrkPt1000[0]",
  "TrackWidthPt1000[0]",
  "SumPtTrkPt500[0]",
]

# All online small-R jet collections
ExtraSmallROnlineHists = [
  HistoSpec('et:GeV;eta',  (100,0,750, 50,-5,5) , title='#eta vs E_{T};E_{T} [GeV];#eta;Entries'),
  "EMFrac",
  "HECFrac",
  "DetectorEta",
  "ActiveArea", 
  "EM3Frac",
  "Tile0Frac",
  "LooseBad",
]

# All online large-R jet collections
ExtraLargeROnlineHists = [
]

ExtraOnlineNJetHists = [
  "njets",
]

# Kinematics at different scales for offline and small-R online jet collections
OfflineScaleMomenta = [ "ConstitScale", "EMScale", "PileupScale", "EtaJESScale"]
OnlineScaleMomenta  = [ "ConstitScale" ]
for var in [ "pt", "eta", "m" ]:
  for offlinescale in OfflineScaleMomenta:
    ExtraOfflineHists.append("Jet"+offlinescale+"Momentum_"+var)
  for onlinescale in OnlineScaleMomenta:
    ExtraSmallROnlineHists.append("Jet"+onlinescale+"Momentum_"+var)

OnlineScaleMomenta.append("") #Adding this for convenience in the jet matching loop below
OfflineScaleMomenta.append("")


def getJetCopyAlg(injets,outjets):
    '''
      Schedules JetCopier tool to make a shallow copy of
      the original offline/HLT jet container, for the JetMatcherAlg to decorate.
      This prevents our jet monitoring from decorating
      the original jet containers, which may end up being
      persistified in AOD/ESD (ATLASRECTS-7168,ATR-27980,ATR-26076)
    '''
    jcopy = CompFactory.JetCopier(
        "copier",
        InputJets = injets,
        DecorDeps=[],
        ShallowCopy=True,
        ShallowIO=True)

    jprovider = CompFactory.JetRecAlg(
        "jetalg_copy_"+outjets,
        Provider = jcopy,
        Modifiers = [],
        OutputContainer = outjets,
        MonTool = None)

    return jprovider

def getL1JetCopyAlg(injets,outjets):
    '''
      Schedules L1JetCopyAlgorithm to make a shallow copy of
      the original L1 jet container, for the JetMatcherAlg to decorate.
      This prevents our jet monitoring from decorating
      the original jet containers, which may end up being
      persistified in AOD/ESD (ATLASRECTS-7168,ATR-27980,ATR-26076).
      The L1JetCopyAlgorithm is a templated class (e.g. L1JetCopyAlgorithm<JTM_JetRoIContainer>).
      The python class name is what is generated by Athena during build time.
      The template types are defined in JTMContainers.h.
    '''
    jcopy_alg = None
    jcopy_alg_name = "l1jetcopy_alg_"+injets  
    if injets == "L1_jFexSRJetRoI":
        jcopy_alg = CompFactory.L1JetCopyAlgorithm_JTM_jFexSRJetRoIContainer_(jcopy_alg_name)    
    elif injets in ["L1_gFexSRJetRoI", "L1_gFexLRJetRoI"]:
        jcopy_alg = CompFactory.L1JetCopyAlgorithm_JTM_gFexJetRoIContainer_(jcopy_alg_name)    
    else:
        raise ValueError(f"L1 jet container {injets} not recognised")
    jcopy_alg.JetInContainerName = injets
    jcopy_alg.JetOutContainerName = outjets

    return jcopy_alg

def TrigJetMonConfig(inputFlags):

  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  cfg = ComponentAccumulator()

  monMode = 'pp'
  if inputFlags.Reco.EnableHI: monMode = 'HI'

  Chains2Monitor = getChains2Monitor(inputFlags, monMode)

  # Protections
  # Add missing jet collections to JetCollections dict
  # (this can happen if a given chain uses a jet collection that is not listed in JetCollections) 
  # TODO: make more general
  for chain,chaindict in Chains2Monitor[monMode].items():
    if chaindict['HLTColl'] not in JetCollections[case]: # chain will not be monitored unless HLT collection is present in JetCollections
      JetCollections[case][chaindict['HLTColl']] = {'MatchTo': 'NONE'}

  # Match HLT jets to offline jets
  CopiedJetCollections = copy.deepcopy(JetCollections)
  for hltColl,collDict in JetCollections[monMode].items():
    if collDict['MatchTo'] != 'NONE':
      copiedhltColl = f'{hltColl}_{copySuffix}'
      CopiedJetCollections[monMode][copiedhltColl] = CopiedJetCollections[monMode].pop(hltColl)
      jetcopyalg = getJetCopyAlg(hltColl,copiedhltColl)
      jetcopyalg.ExtraInputs.add(('xAOD::TrigCompositeContainer',
                                  'StoreGateSvc+%s' % getRun3NavigationContainerFromInput(inputFlags)))
      cfg.addEventAlgo(jetcopyalg)
      for jetcalibscale in OnlineScaleMomenta:
        scalestring = "_"+jetcalibscale if jetcalibscale != "" else ""
        name = 'Matching_{}{}_{}'.format(hltColl,scalestring,collDict['MatchTo'])
        alg = CompFactory.JetMatcherAlg(name,
                                        JetContainerName1=copiedhltColl,
                                        JetContainerName2=collDict['MatchTo'],
                                        JetCalibScale=jetcalibscale)
        
        alg.ExtraInputs.add(('xAOD::TrigCompositeContainer',
                             'StoreGateSvc+%s' % getRun3NavigationContainerFromInput(inputFlags)))
        cfg.addEventAlgo(alg)

  # Match offline to offline jets
  CopiedOfflineJetCollections = copy.deepcopy(OfflineJetCollections)
  for offjetColl,collDict in OfflineJetCollections[monMode].items():
    if collDict['MatchTo'] != 'NONE':
      copiedjetcoll = f'{offjetColl}_{copySuffix}'
      CopiedOfflineJetCollections[monMode][copiedjetcoll] = CopiedOfflineJetCollections[monMode].pop(offjetColl)
      jetcopyalg = getJetCopyAlg(offjetColl,copiedjetcoll)
      cfg.addEventAlgo(jetcopyalg)
      for jetcalibscale in OfflineScaleMomenta:
        scalestring = "_"+jetcalibscale if jetcalibscale != "" else ""
        name = 'Matching_{}{}_{}'.format(offjetColl,scalestring,collDict['MatchTo'])
        alg = CompFactory.JetMatcherAlg(name,
                                        JetContainerName1=copiedjetcoll,
                                        JetContainerName2=collDict['MatchTo'],
                                        JetCalibScale=jetcalibscale)
        
        alg.ExtraInputs.add(('xAOD::TrigCompositeContainer',
                             'StoreGateSvc+%s' % getRun3NavigationContainerFromInput(inputFlags)))
        cfg.addEventAlgo(alg)

  # Make copy of every L1 jet collection
  # Then match L1 to offline as well as HLT jets
  CopiedL1JetCollections = copy.deepcopy(L1JetCollections)
  for l1jetColl,collDict in L1JetCollections[monMode].items():
    copiedl1jetColl = f'{l1jetColl}_{copySuffix}'
    CopiedL1JetCollections[monMode][copiedl1jetColl] = CopiedL1JetCollections[monMode].pop(l1jetColl)
    l1jetcopyalg = getL1JetCopyAlg(l1jetColl,copiedl1jetColl)
    l1jetcopyalg.ExtraInputs.add(('xAOD::TrigCompositeContainer',
                                  'StoreGateSvc+%s' % getRun3NavigationContainerFromInput(inputFlags)))
    cfg.addEventAlgo(l1jetcopyalg)
    for matchjetcoll in collDict['MatchTo']:

      kwds = {'name': 'Matching_{}_{}'.format(l1jetColl,matchjetcoll),
              l1Coll2MatcherKey[l1jetColl]: copiedl1jetColl,
              'JetContainerName2': matchjetcoll,
              'MatchL1': True
             }
              
      alg = CompFactory.JetMatcherAlg(**kwds)
      alg.ExtraInputs.add(('xAOD::TrigCompositeContainer',
                           'StoreGateSvc+%s' % getRun3NavigationContainerFromInput(inputFlags)))
      cfg.addEventAlgo(alg)

  # The following class will make a sequence, configure algorithms, and link
  # them to GenericMonitoringTools
  from AthenaMonitoring import AthMonitorCfgHelper
  helper = AthMonitorCfgHelper(inputFlags,'TrigJetMonitorAlgorithm')
   # Configure filter tools                                                                                                                                                                     
  from AthenaMonitoring.EventFlagFilterToolConfig import EventFlagFilterToolCfg
  from AthenaMonitoring.BadLBFilterToolConfig import LArBadLBFilterToolCfg
  # Loop over L1 jet collections
  for jetcoll in CopiedL1JetCollections[monMode]:
    l1jetconf = l1JetMonitoringConfig(inputFlags,jetcoll,CopiedL1JetCollections,monMode,'',True)
    alg=l1jetconf.toAlg(helper)
    alg.FilterTools = [ EventFlagFilterToolCfg(inputFlags),helper.resobj.popToolsAndMerge(LArBadLBFilterToolCfg(inputFlags))]

  # Loop over L1 jet chains
  for chain,jetcolls in Chain2L1JetCollDict[monMode].items():
    for jetcoll in jetcolls:
      l1chainconf = l1JetMonitoringConfig(inputFlags,jetcoll,L1JetCollections,monMode,chain)
      alg=l1chainconf.toAlg(helper)
      alg.FilterTools = [ EventFlagFilterToolCfg(inputFlags),helper.resobj.popToolsAndMerge(LArBadLBFilterToolCfg(inputFlags))]

  # Loop over offline jet collections
  for jetcoll in CopiedOfflineJetCollections[monMode]:
    offlineMonitorConf = jetMonitoringConfig(inputFlags,jetcoll,CopiedOfflineJetCollections,monMode)
    alg=offlineMonitorConf.toAlg(helper)
    alg.FilterTools = [ EventFlagFilterToolCfg(inputFlags),helper.resobj.popToolsAndMerge(LArBadLBFilterToolCfg(inputFlags))]

  # Loop over HLT jet collections
  for jetcoll in CopiedJetCollections[monMode]:
    monitorConf = jetMonitoringConfig(inputFlags,jetcoll,CopiedJetCollections,monMode)
    # then we turn the full specification into properly configured algorithm and tools.
    # we use the method 'toAlg()' defined for the specialized dictionnary 'JetMonAlgSpec'
    monitorConf.toAlg(helper)

  # Loop over HLT jet chains
  for chain,chainDict in Chains2Monitor[monMode].items():
    jetcoll = chainDict['HLTColl']
    # kinematic plots
    # only use passing jets
    # skip for noalg chains
    if 'noalg' not in chain:
      chainMonitorConfT = jetChainMonitoringConfig(inputFlags,jetcoll,chain,True)
      alg=chainMonitorConfT.toAlg(helper)
      alg.FilterTools = [ EventFlagFilterToolCfg(inputFlags),helper.resobj.popToolsAndMerge(LArBadLBFilterToolCfg(inputFlags))]
    # all jets
    chainMonitorConfF = jetChainMonitoringConfig(inputFlags,jetcoll,chain,False)
    alg=chainMonitorConfF.toAlg(helper)
    alg.FilterTools = [ EventFlagFilterToolCfg(inputFlags),helper.resobj.popToolsAndMerge(LArBadLBFilterToolCfg(inputFlags))]
    # efficiency plots
    if chainDict['RefChain'] != 'NONE' and chainDict['OfflineColl'] != 'NONE':
      effMonitorConf = jetEfficiencyMonitoringConfig(inputFlags,jetcoll,chainDict['OfflineColl'],chain,chainDict['RefChain'])
      effMonitorConf.toAlg(helper)

  cfg.merge(helper.result())
  return cfg


# Basic selection of histograms common for online and offline jets
def basicJetMonAlgSpec(jetcoll,isOnline):
  # we use a specialized dictionnary (JetMonAlgSpec) which will be translated into the final C++ tool
  path = 'NoTriggerSelection' if isOnline else 'standardHistos/'
  minNjetBin = 1 if isOnline else 0

  TopLevelDir  = 'HLT/JetMon/'
  TopLevelDir += 'Online/' if isOnline else 'Offline/'

  jetcollFolder = jetcoll
  jetcollFolder=jetcoll.replace(f"_{copySuffix}","")
  Conf = JetMonAlgSpec(jetcoll+"Mon",JetContainerName = jetcoll, defaultPath = path, topLevelDir=TopLevelDir, bottomLevelDir=jetcollFolder, failureOnMissingContainer=False)

  # Now start filling the histo spec list
  Conf.appendHistos(
    
    #See knownHistos in JetStandardHistoSpecs.py for the list of standard specification.
    #or we can directly add our custom histo specification in the form of a HistoSpec:
    #the basic call is : HistoSpec( variable, histobins, title='histotile;xtitle,ytitle')
    
    #Say we want a 2nd 'pt' plot but with a different binning than in the standard spec.
    #WARNING : we can not re-use the same spec name in a given JetMonitoringAlg !!!
    #so we give a new name AND we specify the actual variable with the argument 'xvar'
    #(the ':GeV' means the variable is to be set at GeV scale)
    #HistoSpec( 'lowpt',  (100,0,150) , title='p_{T};p_{T} [GeV];', xvar='pt:GeV'),            
    #An equivalent solution would have been to clone the existing spec like in :
    #knownHistos.pt.clone('lowpt',bins= (100,0,200) ),

    #2D histos are usually refered to by concatenating vars with a ';' as in 'varx;vary' 
    #if the 'vax;vary' alias doesn't exist in knownHistos but 'varx' and 'vary'
    #do exist, then a spec fot 'vax;vary' will be automatically generated.
    

    #Jet multiplicity histograms can be added by using an EventHistoSpec
    #Their specifications (pT cut, ET cut, eta cuts) must be defined in the knownEventVar dictionary within JetStandardHistoSpecs.py
    #The following line is an example for a jet multiplicity histogram with ET>40 GeV, 1.0<|eta|<2.0, and binning of (10,0,10):
    #EventHistoSpec('njetsEt40Eta1_2', (10,0,10), title='NJetsEt40Eta1_2;NJetsEt40Eta1_2;Entries' ),

    #To select on multiple variables simultaneously, simply combine the selection strings via &
    #Example below to select on ET > 100 GeV and |eta| > 3.2:
    #SelectSpec( 'ETeta', '100<et:GeV&|eta|<3.2', path, FillerTools = ["pt","et","m","eta"] )

    #TProfile2D : just use 3 variables. For now the sytem will automatically
    #interpret it as a TProfile2D (the 3rd variable being profiled)
    #"phi;eta;e",  --> Average Energy vs pt and eta
     
    #another possible selections : only sub-leading jets and highJVF
    #SelectSpec( 'subleading',
    #           '', # no selection on variables
    #           SelectedIndex=1, # force 2nd (sub-leading) jet (we would set 0 for leading jets)
    #           path='standardHistos', # force the path where the histos are saved in the final ROOT file
    #           FillerTools = [
    #               "pt",
    #               "m",
    #           ] ),
    #SelectSpec( 'highJVF',
    #           '0.3<JVF[0]', # JVF is a vector<float> for each jets. Here we cut on the 0th entry of this vector
    #           FillerTools = [
    #               "pt",
    #           ] ),

    #1D histos
    "pt",  
    "m",
    "eta",
    "phi",
    "e",
    "et",
    
    #2D histos
    "pt;m",    # mass vs pt
    "eta;phi", # phi vs eta
    "eta;e",   # energy vs eta
    "phi;e",   # energy vs phi

    #Event selection
    SelectSpec( 'central', '|eta|<3.2', path, FillerTools = ["pt","et","m"] ),
    SelectSpec( 'forward', '3.2<|eta|', path, FillerTools = ["pt","et","m"] ),
    SelectSpec( 'lowmu', 'avgMu<30', path, isEventVariable=True, FillerTools = ["pt","et","m","phi","eta"]),
    SelectSpec( 'highmu', '30<avgMu', path, isEventVariable=True, FillerTools = ["pt","et","m","phi","eta"]),
    
    EventHistoSpec('njets', (25,minNjetBin,25), title='NJets;NJets;Entries' ),
    EventHistoSpec('njetsPt20', (25,minNjetBin,25), title='NJetsPt20;NJetsPt20;Entries' ),
    EventHistoSpec('njetsPt50', (25,minNjetBin,25), title='NJetsPt50;NJetsPt50;Entries' ),
    
  )

  return Conf

# Basic selection of histograms common for heavy ion online and offline jets
def basicHIJetMonAlgSpec(jetcoll,isOnline):
  # we use a specialized dictionnary (JetMonAlgSpec) which will be translated into the final C++ tool
  path = 'NoTriggerSelection' if isOnline else 'standardHistos/'

  TopLevelDir  = 'HLT/JetMon/'
  TopLevelDir += 'Online/' if isOnline else 'Offline/'

  jetcollFolder = jetcoll
  jetcollFolder=jetcoll.replace(f"_{copySuffix}","")
  Conf = JetMonAlgSpec(jetcoll+"Mon",JetContainerName = jetcoll, defaultPath = path, topLevelDir=TopLevelDir, bottomLevelDir=jetcollFolder, failureOnMissingContainer=False)

  # Now start filling the histo spec list
  Conf.appendHistos(
    
    #See knownHistos in JetStandardHistoSpecs.py for the list of standard specification.
    #or we can directly add our custom histo specification in the form of a HistoSpec:
    #the basic call is : HistoSpec( variable, histobins, title='histotile;xtitle,ytitle')
    
    #Say we want a 2nd 'pt' plot but with a different binning than in the standard spec.
    #WARNING : we can not re-use the same spec name in a given JetMonitoringAlg !!!
    #so we give a new name AND we specify the actual variable with the argument 'xvar'
    #(the ':GeV' means the variable is to be set at GeV scale)
    #HistoSpec( 'lowpt',  (100,0,150) , title='p_{T};p_{T} [GeV];', xvar='pt:GeV'),            
    #An equivalent solution would have been to clone the existing spec like in :
    #knownHistos.pt.clone('lowpt',bins= (100,0,200) ),

    #2D histos are usually refered to by concatenating vars with a ';' as in 'varx;vary' 
    #if the 'vax;vary' alias doesn't exist in knownHistos but 'varx' and 'vary'
    #do exist, then a spec fot 'vax;vary' will be automatically generated.

    #Jet multiplicity histograms can be added by using an EventHistoSpec
    #Their specifications (pT cut, ET cut, eta cuts) must be defined in the knownEventVar dictionary within JetStandardHistoSpecs.py
    #The following line is an example for a jet multiplicity histogram with ET>40 GeV, 1.0<|eta|<2.0, and binning of (10,0,10):
    #EventHistoSpec('njetsEt40Eta1_2', (10,0,10), title='NJetsEt40Eta1_2;NJetsEt40Eta1_2;Entries' ),

    #To select on multiple variables simultaneously, simply combine the selection strings via &
    #Example below to select on ET > 100 GeV and |eta| > 3.2:
    #SelectSpec( 'ETeta', '100<et:GeV&|eta|<3.2', path, FillerTools = ["pt","et","m","eta"] )

    #TProfile2D : just use 3 variables. For now the sytem will automatically
    #interpret it as a TProfile2D (the 3rd variable being profiled)
    #"phi;eta;e",  --> Average Energy vs pt and eta
     
    #another possible selections : only sub-leading jets and highJVF
    #SelectSpec( 'subleading',
    #           '', # no selection on variables
    #           SelectedIndex=1, # force 2nd (sub-leading) jet (we would set 0 for leading jets)
    #           path='standardHistos', # force the path where the histos are saved in the final ROOT file
    #           FillerTools = [
    #               "pt",
    #               "m",
    #           ] ),
    #SelectSpec( 'highJVF',
    #           '0.3<JVF[0]', # JVF is a vector<float> for each jets. Here we cut on the 0th entry of this vector
    #           FillerTools = [
    #               "pt",
    #           ] ),

    #1D histos
    "pt",  
    "m",
    "eta",
    "phi",
    "et",
    
    #2D histos
    "eta;phi", # phi vs eta
    HistoSpec( 'pt:GeV;m:GeV',  (100,0,400, 100,0,400) , title='p_{T} vs mass;p_{T} [GeV];m [GeV];Entries'),
    HistoSpec( 'eta;et:GeV',  (100,-5,5, 100,0,400) , title='#eta vs e_{T};#eta;E_{T} [GeV];Entries'),
    HistoSpec( 'phi;et:GeV',  (60,-math.pi,math.pi, 100,0,400) , title='#phi vs E_{T};#phi;E_{T} [GeV];Entries'),

    #Event selection
    SelectSpec( 'central', '|eta|<3.2', path, FillerTools = ["pt","et","m"] ),
    SelectSpec( 'forward', '3.2<|eta|', path, FillerTools = ["pt","et","m"] ),
    SelectSpec( 'pt60', '60<pt:GeV', path, FillerTools = ["m","phi","eta","eta;phi"]),
    
  )

  return Conf

def jetMonitoringConfig(inputFlags,jetcoll,jetCollDict,monMode):
   '''Function to configures some algorithms in the monitoring system.'''

   isOnline  = True if 'HLT' in jetcoll else False

   if monMode == 'HI': # Heavy ion jet monitoring histos
      conf      = basicHIJetMonAlgSpec(jetcoll,isOnline)
   else:
      conf      = basicJetMonAlgSpec(jetcoll,isOnline)

   jetCollMonDetails = jetCollDict[monMode][jetcoll]

   # Declare a configuration dictionnary for a JetContainer
   if isOnline:
     if 'AntiKt4' in jetcoll or 'a4tcem' in jetcoll:
       if monMode == 'pp': #Use extra histos for pp only
        for hist in ExtraSmallROnlineHists: conf.appendHistos(hist)
       if 'ftf' in jetcoll: # dedicated histograms for FTF chains
         conf.appendHistos("Jvt")
         conf.appendHistos("JVFCorr")
         conf.appendHistos("JvtRpt")
         conf.appendHistos("SumPtTrkPt500[0]")
         conf.appendHistos("NumTrkPt1000[0]")
         conf.appendHistos("TrackWidthPt1000[0]")
         if 'PF' in jetcoll: # dedicated histograms for online PFlow jets
           conf.appendHistos("SumPtChargedPFOPt500[0]")
           conf.appendHistos("fCharged")
           if "subresjesgscIS" in jetcoll:
               addFlavourTagVariables(conf,"fastDIPS20211215")
               addFlavourTagVariables(conf,"GN120230331")
               addFlavourTagVariables(conf,"tlaGN220240122")
       if 'fastftag' in jetcoll:
           addFlavourTagVariables(conf,"fastDips")
           addFlavourTagVariables(conf, "fastGN120230327")
           addFlavourTagVariables(conf,"fastGN220240122")
           addFlavourTagVariables(conf,"fastGNTau20240216", flavs=["tau", "u"])
       if 'EMTopo' in jetcoll: #dedicated histograms for online EMTopo jets
           conf.appendHistos("Timing")
     else:
       for hist in ExtraLargeROnlineHists: conf.appendHistos(hist)
     # Add matched jets plots
     if jetCollMonDetails['MatchTo'] != 'NONE':
       def defineHistoForHLTJetMatch(conf, parentAlg, monhelper , path):
           # create a monitoring group with the histo path starting from the parentAlg
           group = monhelper.addGroup(parentAlg, conf.Group, conf.topLevelDir+'/'+conf.bottomLevelDir+'/NoTriggerSelection/')
           # define the histograms
           for histname in [ 'ptdiff', 'energydiff', 'massdiff' ]: #defines which variable difference will be plotted
             group.defineHistogram(histname,title=histname, type="TH1F",
                                   path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                   xbins=100 , xmin=-100000., xmax=100000. ,)
             
           for histname in [ 'ptresp', 'energyresp', 'massresp' ]:
             group.defineHistogram(histname,title=histname, type="TH1F",
                                   path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                   xbins=100 , xmin=-2., xmax=2. ,)
             
           group.defineHistogram('ptresp,ptref;ptresp_vs_ptRef',title='ptresponse vs ptRef', type="TH2F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=10 , xmin=-2., xmax=2., ybins=10, ymin=0., ymax=500000.,)
           
           group.defineHistogram('ptresp,etaref;ptresp_vs_etaRef',title='ptresponse vs etaRef', type="TH2F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=10 , xmin=-2., xmax=2., ybins=10, ymin=-5., ymax=5.,)

           group.defineHistogram('ptref,ptresp;ptRef_vs_ptresp',title='ptRef vs ptresponse', type="TH2F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=100 , xmin=0., xmax=400000., ybins=80, ymin=-2., ymax=2.,)
                                
           group.defineHistogram('etaref,ptresp;etaRef_vs_ptresp',title='etaRef vs ptresponse', type="TH2F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=100 , xmin=-5., xmax=5., ybins=80, ymin=-2., ymax=2.,)
           
       matchedJetColl   = jetCollMonDetails['MatchTo']

       # we can get specific calibration scales by adding e.g. '_EtaJESScale' to the strings
       jetmatchKey      = '{}.matched_{}'.format(jetcoll,matchedJetColl)
       jetptdiffKey     = '{}.ptdiff_{}'.format(jetcoll,matchedJetColl)
       jetenergydiffKey = '{}.energydiff_{}'.format(jetcoll,matchedJetColl)
       jetmassdiffKey   = '{}.massdiff_{}'.format(jetcoll,matchedJetColl)
       jetptrespKey     = '{}.ptresp_{}'.format(jetcoll,matchedJetColl)
       jetenergyrespKey = '{}.energyresp_{}'.format(jetcoll,matchedJetColl)
       jetmassrespKey   = '{}.massresp_{}'.format(jetcoll,matchedJetColl)
       jetptrefKey      = '{}.ptRef_{}'.format(jetcoll,matchedJetColl)
       jetetarefKey     = '{}.etaRef_{}'.format(jetcoll,matchedJetColl)
       name = 'jetMatched_{}_{}'.format(jetcoll,matchedJetColl)
       conf.appendHistos(ToolSpec('JetHistoMatchedFiller', name,
                                  JetMatchedKey=jetmatchKey, JetPtDiffKey=jetptdiffKey,
                                  JetEnergyDiffKey=jetenergydiffKey,
                                  JetMassDiffKey=jetmassdiffKey, JetPtRespKey=jetptrespKey,
                                  JetEnergyRespKey=jetenergyrespKey, JetMassRespKey=jetmassrespKey,
                                  JetPtRefKey=jetptrefKey,JetEtaRefKey=jetetarefKey,
                                  defineHistoFunc=defineHistoForHLTJetMatch,Group='matchedJets_'+jetcoll)
       )
   else: # offline
     for hist in ExtraOfflineHists: conf.appendHistos(hist)
     if 'AntiKt4' in jetcoll and monMode=="pp":
         conf.appendHistos(SelectSpec('LooseBadFailedJets', 'LooseBad',
                                      InverseJetSel=True,
                                      FillerTools = ["pt",
                                                     "phi",
                                                     "eta"])) #cleaning variables not applicable for large-R collections
     
         if 'PF' in jetcoll: # dedicated histograms for offline PFlow jets
             conf.appendHistos("SumPtChargedPFOPt500[0]")
             conf.appendHistos("fCharged")
         elif 'EMTopo' in jetcoll:
             conf.appendHistos("Timing")
     if jetCollMonDetails['MatchTo'] != 'NONE':
       def defineHistoForOfflineJetMatch(conf, parentAlg, monhelper , path):
         # create a monitoring group with the histo path starting from the parentAlg
         group = monhelper.addGroup(parentAlg, conf.Group, conf.topLevelDir+'/'+conf.bottomLevelDir+'/standardHistos/')
         # define the histograms
         for histname in [ 'ptdiff', 'energydiff', 'massdiff' ]: #defines which variable difference will be plotted
           group.defineHistogram(histname,title=histname, type="TH1F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=100 , xmin=-100000., xmax=100000. ,)
           
         for histname in [ 'ptresp', 'energyresp', 'massresp' ]:
           group.defineHistogram(histname,title=histname, type="TH1F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=100 , xmin=-2., xmax=2. ,)
           
         group.defineHistogram('ptresp,ptref;ptresp_vs_ptRef',title='ptresp vs ptRef', type="TH2F",
                               path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                               xbins=10 , xmin=-2., xmax=2., ybins=10, ymin=0., ymax=500000.,)
         
         group.defineHistogram('ptresp,etaref;ptresp_vs_etaRef',title='ptresp vs etaRef', type="TH2F",
                               path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                               xbins=10 , xmin=-2., xmax=2., ybins=10, ymin=-5., ymax=5.,)

         group.defineHistogram('ptref,ptresp;ptRef_vs_ptresp',title='ptRef vs ptresponse', type="TH2F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=100 , xmin=0., xmax=400000., ybins=80, ymin=-2., ymax=2.,)
                                
         group.defineHistogram('etaref,ptresp;etaRef_vs_ptresp',title='etaRef vs ptresponse', type="TH2F",
                                 path='MatchedJets_{}'.format(jetCollMonDetails['MatchTo']),
                                 xbins=100 , xmin=-5., xmax=5., ybins=80, ymin=-2., ymax=2.,)
         
       matchedJetColl   = jetCollMonDetails['MatchTo']
       jetmatchKey      = '{}.matched_{}'.format(jetcoll,matchedJetColl)
       jetptdiffKey     = '{}.ptdiff_{}'.format(jetcoll,matchedJetColl)
       jetenergydiffKey = '{}.energydiff_{}'.format(jetcoll,matchedJetColl)
       jetmassdiffKey   = '{}.massdiff_{}'.format(jetcoll,matchedJetColl)
       jetptrespKey     = '{}.ptresp_{}'.format(jetcoll,matchedJetColl)
       jetenergyrespKey = '{}.energyresp_{}'.format(jetcoll,matchedJetColl)
       jetmassrespKey   = '{}.massresp_{}'.format(jetcoll,matchedJetColl)
       jetptrefKey      = '{}.ptRef_{}'.format(jetcoll,matchedJetColl)
       jetetarefKey     = '{}.etaRef_{}'.format(jetcoll,matchedJetColl)
       name = 'jetMatched_{}_{}'.format(jetcoll,matchedJetColl)
       conf.appendHistos(ToolSpec('JetHistoMatchedFiller',name,
                                  JetMatchedKey=jetmatchKey, JetPtDiffKey=jetptdiffKey,
                                  JetEnergyDiffKey=jetenergydiffKey,
                                  JetMassDiffKey=jetmassdiffKey, JetPtRespKey=jetptrespKey,
                                  JetEnergyRespKey=jetenergyrespKey,
                                  JetMassRespKey=jetmassrespKey,
                                  JetPtRefKey=jetptrefKey, JetEtaRefKey=jetetarefKey,
                                  defineHistoFunc=defineHistoForOfflineJetMatch,Group='matchedJets_'+jetcoll)
       )

   return conf

def l1JetMonitoringConfig(inputFlags,jetColl, jetDict, monMode,chain='',matched=False):

  from TrigJetMonitoring.L1JetMonitoringConfig import L1JetMonAlg
  name = jetColl if chain=='' else jetColl+'_'+chain

  jetCollKey = jetColl
  jetColl = jetColl.replace(f"_{copySuffix}","")

  if not jetDict[monMode][jetCollKey]['MatchTo']:
    conf = L1JetMonAlg(name,jetColl,jetCollKey,chain)
  else:
    assert  len(jetDict[monMode][jetCollKey]['MatchTo']) == 2
    
    conf = L1JetMonAlg(name,jetColl,jetCollKey,chain,
                       matched,jetDict[monMode][jetCollKey]['MatchTo'][0],
                       jetDict[monMode][jetCollKey]['MatchTo'][1])

  return conf

def jetChainMonitoringConfig(inputFlags,jetcoll,chain,onlyUsePassingJets=True):
   '''Function to configures some algorithms in the monitoring system.'''

   jetcollFolder = jetcoll
   chainFolder = chain

   #if not athenaMT:
   #  onlyUsePassingJets = False #does not work for legacy samples yet
   jetMonAlgSpecName = chain+"TrigMon"
   if not onlyUsePassingJets:
     chainFolder = chainFolder + "/ExpertHistos"
     jetMonAlgSpecName = jetMonAlgSpecName + "_ExpertHistos"

   # Define helper functions to automatize ET & eta selection strings for NJet histograms of chains
   def getThreshold(parts):
     return parts[1].split('_')[0]

   def getEtaRangeString(chain):
     etaMin, etaMax = 0, 32
     if 'eta490_j' in chain: #workaround for the upc chains
       etaMin, etaMax = 0, 49
     elif 'eta' in chain:
       etaParts    = chain.split('eta')
       etaMinTemp  = etaParts[0].split('_')
       etaMin      = etaMinTemp[len(etaMinTemp)-1]
       etaMax      = etaParts[1].split('_')[0]
       if int(etaMin) > 0 : etaMin = str(int(int(etaMin)/10))
       if int(etaMax) > 0 : etaMax = str(int(int(etaMax)/10))
     if 'f_ion' in chain:
       etaMin, etaMax = 32, 49
     return 'Eta{}_{}'.format(etaMin,etaMax)

   def getNjetHistName(chain):
     NjetHistName = 'NONE'
     parts         = chain.split('j')
     # check if it is a multi-threshold multijet chain or a single-threshold multijet chain
     multiplicity  = parts[0].split('_')[1] # for single-threshold multijet chains
     if (chain.count('_j')-chain.count('_jes')) > 1  or multiplicity != '':
       NjetHistName = 'njetsEt{}{}'.format(getThreshold(parts),getEtaRangeString(chain))
     return NjetHistName


   trigConf = JetMonAlgSpec( # the usual JetMonAlgSpec 
     jetMonAlgSpecName,
     JetContainerName = jetcoll,
     TriggerChain = chain,
     defaultPath = chainFolder,
     topLevelDir="HLT/JetMon/Online/",
     bottomLevelDir=jetcollFolder,
     failureOnMissingContainer=True,
     onlyPassingJets=onlyUsePassingJets,
     isExpressStreamJob=inputFlags.Common.doExpressProcessing,
   )
   
   trigConf.appendHistos(
           "pt",
           "m",
           "eta",
           "et",
           "phi",
   )
   for hist in ExtraOnlineNJetHists: trigConf.appendHistos(EventHistoSpec(hist, (20,0,25), title=hist+';'+hist+';Entries'))
   # Add NjetEt and NjetPt histograms for simple scenarios
   if 'ht' not in chain and 'HT' not in chain and 'dijet' not in chain and 'DIJET' not in chain and 'fbdj' not in chain and 'noalg' not in chain:
     NjetHistName = getNjetHistName(chain)
     from JetMonitoring.JetStandardHistoSpecs import knownEventVar
     if knownEventVar.get(NjetHistName,None) is not None and NjetHistName not in ExtraOnlineNJetHists: #avoids duplication warnings for some chains
       trigConf.appendHistos(
         EventHistoSpec(NjetHistName, (25,0,25), title=NjetHistName+';'+NjetHistName+';Entries' ),
       )
     NjetHistName = NjetHistName.replace('Et','Pt')
     if knownEventVar.get(NjetHistName,None) is not None and NjetHistName not in ExtraOnlineNJetHists:
       trigConf.appendHistos(
         EventHistoSpec(NjetHistName, (25,0,25), title=NjetHistName+';'+NjetHistName+';Entries' ),
       )
   if 'ftf' in chain and 'a10' not in chain: # track-based JVT variables for FTF chains
     trigConf.appendHistos("Jvt")
     trigConf.appendHistos("JVFCorr")
     trigConf.appendHistos("JvtRpt")

   if 'ht' in chain or 'HT' in chain:
     def defineHistoForHTChain(conf, parentAlg, monhelper , path):
         # create a monitoring group with the histo path starting from the parentAlg
         group = monhelper.addGroup(parentAlg, conf.Group, conf.topLevelDir+jetcollFolder+'/')
         # define the histograms
         xbins, xmin, xmax = getHTBinning(chain,25) # bin width in GeV
         group.defineHistogram("jetHT;HT",title="Jet HT;H_{T} [GeV];Entries", type="TH1F", path=chainFolder, xbins=xbins , xmin=xmin, xmax=xmax ,)
     trigConf.appendHistos(ToolSpec('JetHistoHTFiller','JetHistoHTFiller_'+chain,MinPt=30.,MaxEta=3.2,FailureOnMissingContainer=False,
                                  defineHistoFunc=defineHistoForHTChain,Group='jetHT_'+jetcoll))

   return trigConf

def jetEfficiencyMonitoringConfig(inputFlags,onlinejetcoll,offlinejetcoll,chain,refChain):
   '''Function to configures some algorithms in the monitoring system.'''

   jetcollFolder = onlinejetcoll
   chainFolder = chain

   # We schedule a new JetAlg which will be acting only when a TriggerChain fired (using the TriggerChain from the base classes).
   # We'll plot 1 histo build by a dedicated JetHistoTriggEfficiency tool.
   # So we'll have to explicitely give a specification via the generic dicionnary 'ToolSpec'
   # This implies defining a little function which declares to the monitoring framework which variables to histogram and how.
   #  this is done here.
   def defineHistoForJetTrigg(conf, parentAlg, monhelper , path):
       # create a monitoring group with the histo path starting from the parentAlg
       group = monhelper.addGroup(parentAlg, conf.Group, conf.topLevelDir+jetcollFolder+'/')
       # define the histogram, give them individual names so they don't overwrite each other
       append = "offlineCut_"+conf.name.split("_")[-1] if "offlineCut" in conf.name else "noOfflineCut"
       histname = "trigEff_vs_"+conf.Var.Name+"_"+append
       xbins, xmin, xmax = getBinningFromThreshold(chain,conf.Var.Name)
       group.defineHistogram('trigPassed,jetVar;'+histname, title=histname, type="TEfficiency",
                             path=chainFolder,
                             xbins=xbins , xmin=xmin, xmax=xmax,)
       
   # Get jet index and eta selection for offline jets
   validchain = chain.replace('noalg','j0') 
   parts        = validchain.split('j')
   multiplicity = parts[0].split('_')[1]
   if multiplicity != '': index = int(multiplicity) - 1 # single-threhold multijet chains
   else: index = 0 # single-jet chain
   etaMin,etaMax = getEtaRange(chain)

   from JetMonitoring.JetMonitoringConfig import retrieveVarToolConf
   trigConf = JetMonAlgSpec( # the usual JetMonAlgSpec 
       chain+"TrigEffMon",
       JetContainerName          = offlinejetcoll,
       TriggerChain              = refChain, # reference chain
       defaultPath               = chainFolder,
       topLevelDir               = "HLT/JetMon/Online/",
       bottomLevelDir            = jetcollFolder,
       failureOnMissingContainer = True,
       onlyPassingJets           = False,
       )
   trigConf.appendHistos(
       SelectSpec( 'eff', '{}<|eta|<{}'.format(etaMin,etaMax), chainFolder, SelectedIndex=index, FillerTools = [
           # we pass directly the ToolSpec
           ToolSpec('JetHistoTriggEfficiency', chain,
                    # below we pass the Properties of this JetHistoTriggEfficiency tool :
                    Group='jetTrigGroup_'+chain,
                    Var=retrieveVarToolConf("pt"), # In this context we can not just pass a str alias to describe a histo variable
                                                   # so we use retrieveVarToolConf("pt") which returns a full specification for the "pt" histo variable.
                    ProbeTrigChain=chain,defineHistoFunc=defineHistoForJetTrigg),
       ] ),
   )

   if 'smc' in chain:
     trigConf.appendHistos(
             SelectSpec( 'm50', '50<m:GeV&{}<|eta|<{}'.format(etaMin,etaMax), chainFolder, SelectedIndex=index, FillerTools = [
               ToolSpec('JetHistoTriggEfficiency', chain+'_offlineCut_m50',
                 Group='jetTrigGroup_'+chain+'_m50',
                 Var=retrieveVarToolConf("pt"), # In this context we can not just pass a str alias to describe a histo variable
                 ProbeTrigChain=chain,defineHistoFunc=defineHistoForJetTrigg
               ),
             ] ),
             SelectSpec( 'et500', '500<et:GeV&{}<|eta|<{}'.format(etaMin,etaMax), chainFolder, SelectedIndex=index, FillerTools = [
               ToolSpec('JetHistoTriggEfficiency', chain+'_offlineCut_et500',
                 Group='jetTrigGroup_'+chain+'_et500',
                 Var=retrieveVarToolConf("m"), # In this context we can not just pass a str alias to describe a histo variable
                 SortJets=True,
                 ProbeTrigChain=chain,defineHistoFunc=defineHistoForJetTrigg
               ),
             ] ),
     )

   return trigConf

if __name__=='__main__':

  import sys,argparse

  # Read arguments
  parser = argparse.ArgumentParser()
  parser.add_argument('--runTruthReco',        action='store_true', dest='runTruthReco',        default=False)
  parser.add_argument('--genOfflineR10PF',     action='store_true', dest='genOfflineR10PF',     default=False)
  parser.add_argument('--printDetailedConfig', action='store_true', dest='printDetailedConfig', default=False)
  parser.add_argument('--input',               action='store',      dest='inputFile')
  args                = parser.parse_args()
  RunTruth            = args.runTruthReco
  GenOfflineR10PF     = args.genOfflineR10PF
  PrintDetailedConfig = args.printDetailedConfig

  # Input file
  if args.inputFile is not None: inputFile = args.inputFile
  else:
    logger.error('ERROR: No input file provided, exiting')
    sys.exit(0)

  # Setup logs
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import INFO #,DEBUG 
  log.setLevel(INFO)

  # Set the Athena configuration flags
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()
  flags.Input.Files = [inputFile]
  flags.Input.isMC = True
  flags.Output.HISTFileName = 'AthenaMTMonitorOutput.root' 
  flags.lock()

  monMode = 'pp'
  if flags.Reco.EnableHI: monMode = 'HI'

  Chains2Monitor = getChains2Monitor(flags, monMode)

  # Protections
  # Add missing jet collections to JetCollections dict
  # (this can happen if a given chain uses a jet collection that is not listed in JetCollections) 
  # TODO: make more general
  for chain,chaindict in Chains2Monitor[monMode].items():
    if chaindict['HLTColl'] not in JetCollections[case]: # chain will not be monitored unless HLT collection is present in JetCollections
      JetCollections[case][chaindict['HLTColl']] = {'MatchTo': 'NONE'}  

  # Initialize configuration object, add accumulator, merge, and run.
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
  from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
  cfg = MainServicesCfg(flags)

  # Define the output list
  outputlist = ["xAOD::EventInfo#*","xAOD::VertexContainer#*","xAOD::JetContainer#AntiKt4*Jets","xAOD::JetAuxContainer#AntiKt4*JetsAux.-PseudoJet","xAOD::JetContainer#HLT_*","xAOD::JetAuxContainer#HLT_*Aux.-PseudoJet","xAOD::ShallowAuxContainer#HLT_*Aux.-PseudoJet"]
  # Reconstruct small-R truth jets
  if RunTruth:
    from JetRecConfig.StandardSmallRJets import AntiKt4Truth # import the standard definitions
    # Add the components from our jet reconstruction job
    from JetRecConfig.JetRecConfig import JetRecCfg
    comp = JetRecCfg(AntiKt4Truth,flags)
    cfg.merge(comp)
    # add jets to the output list
    key = "{0}Jets".format(AntiKt4Truth.basename)
    outputlist += ["xAOD::JetContainer#"+key,"xAOD::JetAuxContainer#"+key+"Aux.-PseudoJet"]

  # Reconstruct offline large-R PFlow CSSK+SD jets
  if GenOfflineR10PF:
    from JetRecConfig.JetDefinition import JetConstitSeq, JetDefinition, xAODType
    EMPFlowCSSK         = JetConstitSeq("EMPFlowCSSK", xAODType.ParticleFlow, ["CorrectPFO","CS","SK","CHS"], "JetETMissParticleFlowObjects", "CSSKParticleFlowObjects", label="EMPFlowCSSK")
    AntiKt10EMPFlowCSSK = JetDefinition("AntiKt",1.0,EMPFlowCSSK,ptmin=2e3,)
    AntiKt10EMPFlowCSSK.modifiers = ["ConstitFourMom","Sort","Filter:2000"]
    from JetRecConfig.JetGrooming import JetSoftDrop
    from JetRecConfig.StandardLargeRJets import standardrecomods,substrmods
    AntiKt10EMPFlowCSSKSoftDrop = JetSoftDrop(AntiKt10EMPFlowCSSK,modifiers=standardrecomods+substrmods,ZCut=0.1,Beta=1.0) # standard SoftDrop
    # Add the components from our jet reconstruction job
    from JetRecConfig.JetRecConfig import JetRecCfg
    comp = JetRecCfg(AntiKt10EMPFlowCSSKSoftDrop,flags)
    cfg.merge(comp)
    # add jets to the output list
    key = "{0}Jets".format(AntiKt10EMPFlowCSSKSoftDrop.basename)
    outputlist += ["xAOD::JetContainer#"+key,"xAOD::JetAuxContainer#"+key+"Aux.-PseudoJet"]

  # Write new jet collections to AOD
  if RunTruth or GenOfflineR10PF:
    # Get the output stream components
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags,"xAOD",ItemList=outputlist))

  cfg.merge(PoolReadCfg(flags))

  # The following class will make a sequence, configure algorithms, and link
  # them to GenericMonitoringTools
  from AthenaMonitoring import AthMonitorCfgHelper
  helper = AthMonitorCfgHelper(flags,'TrigJetMonitorAlgorithm')
  cfg.merge(helper.result()) # merge it to add the sequence needed to add matchers

  # Match HLT jets to offline jets
  for hltColl,collDict in JetCollections[monMode].items():
    if collDict['MatchTo'] != 'NONE':
      for jetcalibscale in OnlineScaleMomenta:
        scalestring = "_"+jetcalibscale if jetcalibscale != "" else ""
        name = 'Matching_{}{}_{}'.format(hltColl,scalestring,collDict['MatchTo'])
        alg = CompFactory.JetMatcherAlg(name, JetContainerName1=hltColl,JetContainerName2=collDict['MatchTo'],JetCalibScale=jetcalibscale)
        alg.ExtraInputs.add(('xAOD::TrigCompositeContainer','StoreGateSvc+%s' % getRun3NavigationContainerFromInput(flags)))
        cfg.addEventAlgo(alg,sequenceName='AthMonSeq_TrigJetMonitorAlgorithm') # Add matchers to monitoring alg sequence

  # Match offline to offline jets
  for offjetColl,collDict in OfflineJetCollections[monMode].items():
    if collDict['MatchTo'] != 'NONE':
      for jetcalibscale in OfflineScaleMomenta:
        scalestring = "_"+jetcalibscale if jetcalibscale != "" else ""
        name = 'Matching_{}{}_{}'.format(offjetColl,scalestring,collDict['MatchTo'])
        alg = CompFactory.JetMatcherAlg(name, JetContainerName1=offjetColl,JetContainerName2=collDict['MatchTo'],JetCalibScale=jetcalibscale)
        alg.ExtraInputs.add(('xAOD::TrigCompositeContainer','StoreGateSvc+%s' % getRun3NavigationContainerFromInput(flags)))
        cfg.addEventAlgo(alg,sequenceName='AthMonSeq_TrigJetMonitorAlgorithm')

  # Match L1 to offline as well as HLT jets
  for l1jetColl,collDict in L1JetCollections[monMode].items():
    for matchjetcoll in collDict['MatchTo']:
      if matchjetcoll != 'NONE':
        name = 'Matching_{}_{}'.format(l1jetColl,matchjetcoll)
        alg = CompFactory.JetMatcherAlg(name, L1JetContainerName1=l1jetColl,JetContainerName2=matchjetcoll,MatchL1=True)
        alg.ExtraInputs.add(('xAOD::TrigCompositeContainer','StoreGateSvc+%s' % getRun3NavigationContainerFromInput(flags)))
        cfg.addEventAlgo(alg,sequenceName='AthMonSeq_TrigJetMonitorAlgorithm')
  
  # Loop over L1 jet collectoins
  for jetcoll in L1JetCollections[monMode]:
    l1jetconf = l1JetMonitoringConfig(flags,jetcoll,L1JetCollections,monMode,'',True)
    l1jetconf.toAlg(helper)

  # Loop over L1 jet chains
  for chain,jetcoll in Chain2L1JetCollDict[monMode].items():
    l1chainconf = l1JetMonitoringConfig(flags,jetcoll,L1JetCollections,monMode,chain)
    l1chainconf.toAlg(helper)

  # Loop over offline jet collections
  for jetcoll in OfflineJetCollections[monMode]:
    offlineMonitorConf = jetMonitoringConfig(flags,jetcoll,OfflineJetCollections,monMode)
    offlineMonitorConf.toAlg(helper)

  # Loop over HLT jet collections
  for jetcoll in JetCollections[monMode]:
    monitorConf = jetMonitoringConfig(flags,jetcoll,JetCollections,monMode)
    # then we turn the full specification into properly configured algorithm and tools.
    # we use the method 'toAlg()' defined for the specialized dictionnary 'JetMonAlgSpec'
    monitorConf.toAlg(helper)

  # Loop over HLT jet chains
  for chain,chainDict in Chains2Monitor[monMode].items():
    jetcoll = chainDict['HLTColl']
    # kinematic plots
    # only passing jets
    # skip for noalg chains
    if 'noalg' not in chain:
      chainMonitorConfT = jetChainMonitoringConfig(flags,jetcoll,chain,True)
      chainMonitorConfT.toAlg(helper)
    # all jets
    chainMonitorConfF = jetChainMonitoringConfig(flags,jetcoll,chain,False)
    chainMonitorConfF.toAlg(helper)
    # efficiency plots
    if chainDict['RefChain'] != 'NONE' and chainDict['OfflineColl'] != 'NONE':
      effMonitorConf = jetEfficiencyMonitoringConfig(flags, jetcoll,
                                                     chainDict['OfflineColl'], chain,
                                                     chainDict['RefChain'])
      effMonitorConf.toAlg(helper)

  cfg.merge(helper.result())
  
  # Print config
  cfg.printConfig(withDetails=PrintDetailedConfig)

  cfg.run()
