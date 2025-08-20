# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )
log.debug("Importing %s",__name__)

from copy import deepcopy
import itertools

#==========================================================
# This is stored in chainDict['Signature']
#==========================================================

# this dictionary contains all the informations about the signatures, needed to create the Chaindicitonary. It has the shape of:
# 'signature': ('substring', 'group')
# if the substring is '', the signature is not mapped to the chain name
# if the group is '', the signature is not mapped to any group
SignatureDict = {
    'Electron': ('e','AllTag'),
    'Photon'  : ('g','AllTag'),
    'Muon'    : ('mu','AllTag'),
    'Bphysics': ('', 'AllTag'),
    'Tau'     : ('tau','JetMET'),
    'Jet'     : ('j',  'JetMET'),
    'Bjet'    : ('', 'JetMET'),
    'MET'     : ('xe', 'JetMET'),
    'XS'      : ('xs', 'JetMET'),
    'TE'      : ('te', 'JetMET'),
    'MinBias' : ('mb', 'MinBias'),
    'HeavyIon' : ('hi', 'MinBias'),
    'Cosmic'  : ('cosmic', ''),
    'Calib'   : ('calib', ''),
    'Streaming' : ('streamer', ''),
    'Monitor'   : ('mon', ''),
    'Beamspot'  : ('beamspot','Beamspot'),
    'MuonnoL1'  : ( '', 'MuonnoL1'),
    'EnhancedBias' : ('eb', ''),
    'UnconventionalTracking'  : (['isotrk', 'fslrt', 'dedxtrk', 'hitdvjet', 'fsvsi', 'distrk', 'dispjet', 'dispvtx'], 'JetMET'),
    'Test'          : ('TestChain', ''),
    'Electronprobe': ('', 'AllProbe'),
    'Photonprobe'  : ('', 'AllProbe'),
    'Tauprobe'     : ('', 'AllProbe'),
    'Muonprobe'    : ('', 'AllProbe')
}


def getSignatureDict():
    # removes the grouping from the dict and creates a new one signature : string
    new_dict = {key: value[0] for key, value in SignatureDict.items() if value[0] != ''}
    return new_dict

SliceIDDict = getSignatureDict()

def getSignatureGroupingDict():
    # removes the substring from the dict and creates a new one signature : group
    new_dict = {key: value[1] for key, value in SignatureDict.items() if value[1] != ''}
    return new_dict

def getListOfSignatureStrings():   
    ''' returns the list of substrings representing the signautres in the chain name'''

    list_of_strings = list(SliceIDDict.values())      # this is a list of lists
    flattened_list = list(itertools.chain.from_iterable((item if isinstance(item, list) else [item]) for item in list_of_strings))
    return flattened_list

def getListOfSignatures():   
    ''' returns the list of substrings representing the signautres in the chain name'''
    return SliceIDDict.keys()



class ChainStore(dict):
    """Class to hold list of chains for each signature (dictionary with fixed set of keys)"""
    _allowedSignatures = ['Egamma', 'Muon', 'Jet', 'Bjet', 'Bphysics', 'MET', 'Tau', 
                          'HeavyIon', 'Beamspot', 'Cosmic', 'EnhancedBias',
                          'Monitor', 'Calib', 'Streaming', 'Combined', 'MinBias',
                          'UnconventionalTracking', 'Test']

    def __init__(self):
        # Create dicionary with fixed set of keys in the orignal order
        super().__init__({s : [] for s in self._allowedSignatures})

    def __setitem__(self, key, value):
        if key not in self:
            raise RuntimeError(f"'{key}' is not in the list of allowed signatures: {self._allowedSignatures}")
        else:
            dict.__setitem__(self, key, value)


#==========================================================
# ---- Generic Template for all chains          ----
# ---- chainParts specific information given in ----
# ---- signature specific dictionaries below    ----
#==========================================================
ChainDictTemplate = {
    'chainName'       : '',
    'L1item'          : '',
    'topo'            : '',
    'signatures'      : [],
    'alignmentGroups' : [],
    'stream'          : '',
    'groups'          : [],
    'EBstep'          : '',
    'chainParts'      : [],
    'sigDicts'        : {},
    'sigFolder'       : [],
    'subSigs'         : [],
    'extraComboHypos' : []
}

#==========================================================
# Test chains
#==========================================================
# ---- Test Dictionary of all allowed Values ----
TestChainParts = {
    'L1threshold'    : '',
    'signature'      : ['Test'],
    'alignmentGroup' : ['Test'],
    'chainPartName'  : '',
    'multiplicity'   : '',
    'extra'          : ['mv1', 'mv1step', 'mv2', 'ev1', 'ev2', 'ev3', 'gv1', 'mEmpty1', 'mEmpty2', 'mEmpty3', 'ev1dr', 'mv1dr','merge'],
    'trigType'       : ['TestChain'],
    'threshold'      : '',
    'addInfo'        : [''],
    'sigFolder'     : ['Test'],
    'subSigs'       : ['Test'],
    'chainPartIndex': list(range(0,10))
}

# ---- Test Dictionary of default Values ----
TestChainParts_Default = {
    'signature'      : ['Test'],
    'alignmentGroup' : ['Test'],
    'L1threshold'    : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'threshold'      : '',
    'addInfo'        : [],
    'sigFolder'     : ['Test'],
    'subSigs'       : ['Test'],
    'chainPartIndex': 0
}

#==========================================================
# Jet
#==========================================================
AllowedTopos_jet = []
# List of keys that pertain to jet reconstruction
# as opposed to the hypo configuration
JetRecoKeys = ['recoAlg','constitType','clusterCalib','constitMod','jetCalib','trkopt','ionopt']
# ---- Jet Dictionary of all allowed Values ----
JetChainParts = {
    # Information common to all signatures
    'signature'     : ['Jet'],
    'alignmentGroup': ['Jet','JetMET'],
    'L1threshold'   : '',
    'chainPartName' : '',
    'threshold'     : '',
    'multiplicity'  : '',
    'trigType'     : ['j'],
    'topo'          : AllowedTopos_jet,
    'extra'        : [],
    'addInfo'      : ['perf'],
    'sigFolder'     : ['Jet'],
    'subSigs'       : ['Jet'],
    'chainPartIndex': list(range(0,10)),
    # Information unique to the jet slice
    # Reco information
    'recoAlg'      : # Jet clustering algorithm
      ['a2', 'a4', 'a10', 'a10r', 'a10t', 'a10sd'],
    'constitType'  : # Jet input type
      ['tc','pf'], # 'ufo' might be added at some point
    'clusterCalib' : # Topocluster calibration
      ['em', 'lcw'],
    'constitMod'   : # Constituent modifiers
      ['sk', 'cssk'],
    'jetCalib'     : # Jet calibration
      ['jes', 'subjes', 'subjesIS', 'subjesgscIS', 'subresjesgscIS', 'subjesgsc', 'subresjesgsc', 'nojcalib'],
    'scan'         : # No longer used?
      ['FS',],
    'ionopt'       : # Heavy ion configuration
      ['noion','ion','ionp'],
    'trkopt'       : # Tracking configuration
      ['notrk','ftf','roiftf'],
    'trkpresel'    : # Tracking preselection
      ['nopresel',
       # Single jet
       'preselj50emf72',
       'preselj30emf72',
       'preselj20emf72',
       'preselj20emf60',
       'preselj20emf48',
       'preselj20emf24',
       'preselj20emf18',
       'preselj20emf12',
       'preselj20emf6', 
       'preselj20',
       'preselj50',
       'preselj80',
       'preselj120',
       'preselj140',
       'preselj180',
       'preselj190',
       'preselj160',
       'preselj200',
       'preselj225',
       # Multijets
       'presel2j180',
       'presel2j225',
       'presel3c30',
       'presel3c40',
       'presel3c45',
       'presel3j45',
       'presel3j150',
       'presel4j20',
       'presel4c20',
       'presel4c25',
       'presel4c30',
       'presel4c35',
       'presel4c45',
       'presel4j25',
       'presel4c25',
       'presel4j40',
       'presel4c40',
       'presel4j45',
       'presel4j50',
       'presel4j85',
       'presel5c20',
       'presel5j25',
       'presel5j50',
       'presel5j55',
       'presel5c50',
       'presel6j40',
       'presel6j45',
       'presel7j30',
       # Multiple threshold
       'preselj60XXj40',
       'preselj140XXj45',
       'preselj140XX2j45',
       'preselj100XX2j45',
       'preselj120XX2j45',
       'preselj80XX2j45',
       'presel2j180XXj80',
       # Nonstandard eta regions
       'presel5c55',
       'presel6c20',
       'presel6c25',
       'presel6c45',
       'preselj45XX2f40',
       'preselc60XXc45XXc25XXc20', # L1J45p0ETA21_3J15p0ETA25
       'preselc60XXj45XXf40',
       'preselj60XXj45XXf40',
       'presela60XXa40XX2a25',
       'preseljHT400',
       'preselcHT400',
       'preseljHT450',
       'preselcHT450',
       'preseljHT500',
       'preselcHT500',
       'preseljHT600',
       'preselcHT600',
       'preselcHT650',
       'preselcHT850',
       #b-jet preselections
       'presel1c100XX2c20bgtwo85',
       'presel1c120XX2c20bgtwo90',
       'presel1c120CXX1c20XX1c20bgtwo85',
       'presel1c160XX1c20bgtwo90',
       'presel1c160XX1c20bgtwo85',
       'presel2c20XX2c20b85',
       'presel2c20XX2c20b82',
       'presel2c20XX2c20b80',
       'presel2c20XX2c20bgtwo85',
       'presel2c20XX2c20bg85',
       'presel2c20XX2c20bg82',
       'presel2c20XX2c20bg80',
       'presel2c20XX2c20b90',
       'presel3c20XX1c20b85',
       'presel3c20XX1c20bg85',
       # (b+)tau preselections
       'presel4c20',
       'presel3c20XX1c20bgtwo85',
       'presel2c20XX1c20bgtwo85XX1c20gntau90',
       'presel2c20XX1c20bgtwo85XX1c20gntau85',
       'presel2c20XX1c20bgtwo82XX1c20gntau85',
       'presel2c20XX1c20bgtwo82XX1c20gntau80',
       'presel2c20XX1c20bgtwo80XX1c20gntau80',
       'presel2c20XX1c20bgtwo85XX1c20uht1tau90',
       'presel2c20XX1c20bgtwo85XX1c20uht1tau85',
       'presel2c20XX1c20bgtwo85XX1c20uht1tau82',
       'presel2c20XX1c20bgtwo85XX1c20uht1tau80',
       'presel5c25XXc25bgtwo85',
       'presel3j45bgtwo95',
       'presel4j25bgtwo95',
       'presel2j25XX2j25bgtwo85',
       'presel3j25XX2j25bgtwo85',
       'preselj50bgtwo85XX3j50',
       'preselj80XX2j45bgtwo90',
       'preselj140bgtwo85XXj45bgtwo85',
       'presel2a20bgtwo90XX2a20',
       'presela20bgtwo85XX3a20',
       'presel3c20XX1c20gntau90',
       'presel3c20XX1c20gntau85',
       'preselj20b95',
       'preselj2b77',
       'preselj20b77',
       'presel3j45b95',
       'presel4j25b95',
       'presel2j25XX2j25b85',
       'presel3j25XX2j25b85',
       'preselj50b85XX3j50',
       'preselj80XX2j45b90',
       'preselj140b85XXj45b85',
       'presel5c25XXc25b85',
       'presel2a20b90XX2a20',
       'presela20b85XX3a20',
       #beamspot preselction option
       'presel2c20b85',
       #DIPZ preselection
       'preselZ128XX4c20',
       'preselZ120XX4c20',
       'preselZ116XX4c20',
       'preselZ167MAXMULT5cXX4c20',
       'preselZ138MAXMULT5cXX4c20',
       'preselZ126MAXMULT5cXX4c20',
       'preselZ120MAXMULT20cXX4c85',
       'preselZ87XX3c20',
       'preselZ84XX3c20',
       'preselZ82XX3c20',
       'preselZ120XX2c20XX2c20b85',
       'preselZ138MAXMULT5cXX2c20XX2c20b85',
       'preselZ84XX1c20XX2c20b85',
       'preselZ120XX4c85',
       'preselZ116XX4c20',
       'preselZ138XX4c20',
       'preselZ120MAXMULT20cXX4c20',
       'preselZ84MAXMULT20cXX3c20',
       'preselZ116MAXMULT5cXX4c20',
       'preselZ116MAXMULT20cXX4c20',
       'preselZ84XX1c20XX2c20b85',
       'preselZ128XX2c20XX2c20b85',
       'preselZ128MAXMULT20cXX4c85',
       'preselZ128XX4c20XX1j20',
       'preselZ128XX3c20XX1c20bg85',
       'preselZ116XX3c20XX1c20bg85',
       'preselZ128XX4c85',
       'preselZ219XX6c20',
       'preselZ197XX6c20',
       'preselZ182XX6c20',
       'preselZ142XX5c20',
       'preselZ134XX5c20',
       'preselZ124XX5c20'
     ],
    # Hypo information
    #   If hypoScenario is 'simple', then hypo configuration is handled based on the
    #   other dict contents. If it is not 'simple', then the configuration is 100%
    #   from the hypoScenario specification, and all other hypo entries are ignored.
    #   Complete scenario names for aliases can be found in Trigger/TrigHypothesis/TrigHLTJetHypo/python/hypoConfigBuilder.py
    'hypoScenario' : ['simple', # Independent selections on individual jets, multiplicity+threshold cuts
                      # 'fbdj' (forward-backward + dijet) scenario:
                      #   default eta selection for dijet mass cut is 0eta490
                      'FBDJSHARED',  # Forward backward jets + dijet, default parameters, fb and dj can share
                      'FBDJNOSHARED10etXX20etXX34massXX50fbet', # f/b jets + dijet, expl. parameters, fb and dj do not share
                      # 'dijet' scenario applies always a mass cut (deta and dphi cuts are optional)
                      #   0eta490 is the default eta selections for j1/j2
                      #   j12et sets the same et cuts for j1et and j2et
                      #   j12eta sets the same eta cuts for j1eta and j2eta
                      #   order:
                      #     et cuts    (mandatory)
                      #     eta cuts   (optional, if missing will use default)
                      #     djmass sel (mandatory)
                      #     djdphi sel (optional)
                      #     djdeta sel (optional)
                      #
                      # pt threshold cuts
                      'DJMASS300j35', # alias
                      'DJMASS500j35', # alias
                      'DJMASS700j35', # alias
                      'DJMASS1000j35', # alias
                      'DJMASS900j50', # alias
                      'DJMASS1000j50', # alias
                      'DJMASS1000j50dphi240', # alias
                      'DJMASS1000j50dphi200x400deta', # alias
                      'DJMASS900j50dphi200x400deta', # alias
                      'DJMASS1000j50dphi260x200deta', # alias
                      'DJMASS900j50dphi260x200deta' , # alias
                      'DJMASS1000j50dphi260', # alias
                      'DJMASS900j50dphi260', # alias
                      'DJMASS1000j50x200deta', # alias
                      'DJMASS900j50x200deta', # alias
                      'DJMASS1000j30dphi260x200deta', # alias
                      'DJMASS900j30dphi260x200deta', # alias
                      'DIJET70j12etXX1000djmassXXdjdphi200XX400djdeta', # needed for hypoToolTests.py
                      'DIJET80j12etXX0j12eta240XX700djmass', # needed for hypoToolTests.py

                      # 'ht' category applies a cut on HT (HT>value) computed by aggregation over single jets (default filtering: 30et and 0eta320)
                      'HT0',
                      'HT1000',
                      'HT290',
                      'HT300',
                      'HT500',
                      'HT940',
                      'HT50',
                      'HT300XX10ptXX0eta490',
                      'HT300XX10ptXX0eta490XXveto',
                      'HT300XX15ptXX0eta490',
                      'HT300XX15ptXX0eta490XXveto',
                      'HT400XX15ptXX0eta490',
                      'HT400XX15ptXX0eta490XXveto',
                      'HT500XX0eta240',
                      'HT650XX0eta240',
                      'HT850XX0eta240',
                      'HT940XX0eta240',
                      'HT940XX020jvt',
                      'HT940XX0eta240XX020jvt',
                      'HT1000XX0eta240',
                      'HT1000XX020jvt',
                      'HT1000XX0eta240XX020jvt',
                      'HT50XX10etXX0eta320',  # needed for hypoToolTests.py
                      # DIPZ for testing only
                      'Z120XX4c20',
                      'Z120XX4c120',
                      'Z128XX4c20',
                      'Z120XX5c70',
                      'Z120XX6c55',
                      'Z120XX10c40',
                      'Z219XX6c20',
                      'Z197XX6c20',
                      'Z182XX6c20',
                      'Z142XX5c20',
                      'Z134XX5c20',
                      'Z124XX5c20',
                      # 'MULT' hypoScenario applies a cut on the number of jets
                      # in the input container after filtering on pt, eta.
                      'MULT0mult11XX10ptXX0eta490', # Heavy Ions
                      'MULT0mult11XX15ptXX0eta490', # Heavy Ions
                      ],
    'exotHypo' : ['emergingPTF0p2dR1p2', 'emergingPTF0p1dR1p2', 'emergingPTF0p09dR1p2', 'emergingPTF0p08dR1p2', 'emergingPTF0p075dR1p2', 'emergingPTF0p07dR1p2', 'emergingPTF0p0dR1p2',
                  'emergingPTF0p2dR0p4', 'emergingPTF0p1dR0p4', 'emergingPTF0p09dR0p4', 'emergingPTF0p08dR0p4', 'emergingPTF0p075dR0p4', 'emergingPTF0p07dR0p4', 'emergingPTF0p0dR0p4',
                  'tracklessdR1p2',      'tracklessdR0p4',
                  'calratio','calratiormbib','calratiovar','calratiovarrmbib',  # Exotics CalRatio jets (trackless and low-EMF, with option to clean out BIB)
                  'calratiovar103','calratiovar82','calratiovar59', 'calratiovar186', 'calratiovar150', 'calratiovar165' # Exotics CalRatio Jets ( requested by DPJ Team for alternative cut on ratio )
              ],

    # Simple hypo configuration. Single property cuts defined as MINvarMAX
    'etaRange'      :
      # These atypical explicit values are allowed to be in chain names.
      # Otherwise use ['','a','c','f'] ==> [0eta320, 0eta490, 0eta240, 320eta490]
      # suffix after threshold e.g. j420 == j420_0eta320, 6j55c == 6j55_0eta240
      ['0eta290', '0eta200', '0eta180', '0eta160', '0eta140']
      +['320eta490'], # TODO: Kept temporarily for validation
    'jvt'           : # Jet Vertex Tagger pileup discriminant
      ['010jvt', '011jvt', '015jvt', '020jvt', '050jvt', '059jvt'],
    'nnJvt'         : # NN Jet Vertex Tagger pileup discriminant
      ['nnJvtv1'], # No range cuts, boolean pass/fail
    'momCuts'       : # Generic moment cut on single jets
       ['050momemfrac100','momemfrac006','momemfrac024','momemfrac012', 'momhecfrac010', '050momemfrac100XXmomhecfrac010', 'momemfrac072', 'momemfrac048' ],
    'timing'        : # delayed jets, with absolute delay requirement [ns]
    ['2timing','2timing15'],
    'timeSig'       : # delayed jets, based on pT-dependent significance of delay [sigma]
    ['1timeSig', '1p5timeSig', '2timeSig', '3timeSig','2timeSig15','3timeSig15'],
    'prefilters'      : # Pre-hypo jet selectors (including cleaning)
    ['CLEANlb', 'CLEANllp', 'MASK300ceta210XX300nphi10',
     # ptrangeXrY (X, Y matches regex \d+)  triggers a prehypo selection of
     # jets by ordering by pt, and selecting those with indices in [X,Y]
     'PTRANGE0r1',
     'PTRANGE2r3',
     'MAXMULT20c',
     'MAXMULT6c',],
    'bsel': [ '95bdips','90bdips','85bdips','80bdips','77bdips'
            , '95bgnone','90bgnone','85bgnone','80bgnone','77bgnone'
            , '60bgntwoxt', '65bgntwoxt', '70bgntwoxt', '75bgntwoxt'
            , '80bgntwoxt', '85bgntwoxt', '90bgntwoxt', '95bgntwoxt'
            , '79bgntwox', '86bgntwox', '91bgntwox', '96bgntwox'
            , '95bgntwo','90bgntwo','85bgntwo','80bgntwo','82bgntwo','77bgntwo','75bgntwo','60bgntwo'
            ],
    'tausel':
        [ '75gntau' , '80gntau', '85gntau' , '90gntau'
        , '75uht1tau' , '80uht1tau', '85uht1tau' , '90uht1tau'
        ],
    'smc'           : # "Single mass condition" -- rename?
      ['30smcINF', '35smcINF', '40smcINF', '50smcINF', '60smcINF', 'nosmc'],
    # Setup for alternative data stream readout
    # B-tagging information
    'bTag'         : ['boffperf'  ,
                      # GN1 series
                      'bgn160', 'bgn165', 'bgn170', 'bgn172',
                      'bgn175', 'bgn177', 'bgn180', 'bgn182',
                      'bgn185',
                      'bgn182bb96', 'bgn177bb96', 'bgn175bb90',
                      # GN2 series
                      'bgn260', 'bgn265', 'bgn270', 'bgn272',
                      'bgn275', 'bgn277', 'bgn280', 'bgn282',
                      'bgn285',                       ],
    'ditauTag'     : ['ditauOmni0Trk3',  'ditauOmni0Trk4',  'ditauOmni0Trk5',  'ditauOmni0Trk9', 
                      'ditauOmni1Trk3',  'ditauOmni1Trk4',  'ditauOmni1Trk5',  'ditauOmni1Trk9', 
                      'ditauOmni2Trk3',  'ditauOmni2Trk4',  'ditauOmni2Trk5',  'ditauOmni2Trk9', 
                      'ditauOmni3Trk3',  'ditauOmni3Trk4',  'ditauOmni3Trk5',  'ditauOmni3Trk9', 
                      'ditauOmni4Trk3',  'ditauOmni4Trk4',  'ditauOmni4Trk5',  'ditauOmni4Trk9', 
                      'ditauOmni5Trk3',  'ditauOmni5Trk4',  'ditauOmni5Trk5',  'ditauOmni5Trk9', 
                      'ditauOmni6Trk3',  'ditauOmni6Trk4',  'ditauOmni6Trk5',  'ditauOmni6Trk9', 
                      'ditauOmni7Trk3',  'ditauOmni7Trk4',  'ditauOmni7Trk5',  'ditauOmni7Trk9', 
                      'ditauOmni8Trk3',  'ditauOmni8Trk4',  'ditauOmni8Trk5',  'ditauOmni8Trk9', 
                      'ditauOmni9Trk3',  'ditauOmni9Trk4',  'ditauOmni9Trk5',  'ditauOmni9Trk9',
                      'ditauOmni01Trk3', 'ditauOmni01Trk4', 'ditauOmni01Trk5', 'ditauOmni01Trk9',
                      'ditauOmni02Trk3', 'ditauOmni02Trk4', 'ditauOmni02Trk5', 'ditauOmni02Trk9',
                      'ditauOmni03Trk3', 'ditauOmni03Trk4', 'ditauOmni03Trk5', 'ditauOmni03Trk9',
                      'ditauOmni04Trk3', 'ditauOmni04Trk4', 'ditauOmni04Trk5', 'ditauOmni04Trk9',
                      'ditauOmni05Trk3', 'ditauOmni05Trk4', 'ditauOmni05Trk5', 'ditauOmni05Trk9',
                      'ditauOmni06Trk3', 'ditauOmni06Trk4', 'ditauOmni06Trk5', 'ditauOmni06Trk9',
                      'ditauOmni07Trk3', 'ditauOmni07Trk4', 'ditauOmni07Trk5', 'ditauOmni07Trk9',
                      'ditauOmni08Trk3', 'ditauOmni08Trk4', 'ditauOmni08Trk5', 'ditauOmni08Trk9',
                      'ditauOmni09Trk3', 'ditauOmni09Trk4', 'ditauOmni09Trk5', 'ditauOmni09Trk9',
                     ],
    'bTracking'    : [],
    'bConfig'      : ['split',],
    'bMatching'    : ['antimatchdr05mu'],
    'tboundary'    : ['SHARED'], # simple scenario tree boundary marker

    # beamspot
    'beamspotChain'     : ['beamspotVtx'],
    'pileuprm'       : # scedule pileup removal algo for  single jet, the m_min LogR value is minimul criteria to for jet LogR to start removal algorithm, the m_max LogR is the desired logR cut to pass for jetschedule pileup removal algo for single jet, the m_min LogR value is minimal criteria for jet LogR to start the removal algorithm, the n_max LogR is the desired logR cut to pass for jet
       ['n041pileuprmn015' ],# left value is min LogR,right is max LogR, n stands for negative (for example n041 means -0.41 ) 
}

# ---- Jet Dictionary of default Values ----
JetChainParts_Default = {
    'signature'     : ['Jet'],
    'alignmentGroup': ['Jet'],
    'L1threshold'   : '',
    'threshold'     : '',
    'multiplicity'  : '',
    'trigType'      : '',
    'topo'          : [],
    'extra'         : '',
    'addInfo'       : [],
    'sigFolder'     : ['Jet'],
    'subSigs'       : ['Jet'],
    'chainPartIndex': 0,
    #
    'recoAlg'       :'a4',
    'constitType'   :'tc',
    'clusterCalib'  :'em',
    'constitMod'    :'',
    'jetCalib'      :'default',
    'scan'          :'FS',
    'ionopt'        : 'noion',
    'trkopt'        : 'notrk',
    'trkpresel'     : 'nopresel',
    #
    'etaRange'      : '0eta320',
    'jvt'           : '',
    'nnJvt'         : '',
    'momCuts'       : '',
    'timing'        : '',
    'timeSig'       : '',
    'prefilters'    : [],
    'bsel'          : '',
    'tausel'        : '',
    'hypoScenario'  : 'simple',
    'exotHypo'      : [],
    'smc'           : 'nosmc',
    #
    'bTag'          : '',
    'ditauTag'      : '',
    'bTracking'     : '',
    'bConfig'       : [],
    'bMatching'     : [],
    #
    'tboundary'     : '',

    'beamspotChain' : '',
    'pileuprm'     : '',
    }

# ---- bJet Dictionary of default Values that are different to the ones for normal jet chains ----
bJetChainParts_Default = {
    'etaRange' : '0eta290',
    'sigFolder'     : ['Bjet'],
    'subSigs'       : ['Bjet'],
}

ditauJetChainParts_Default = {
    'sigFolder'     : ['Tau'],
    'subSigs'       : ['Ditau'],
}
# ---- Beamspot Dictionary for chains confiugred through jets
BeamspotJetChainParts_Default = {
    'signature'      : 'Beamspot',
    'alignmentGroup' : ['Beamspot'],
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Beamspot'],
    'beamspotChain' : '',
    'chainPartIndex': 0
    }

#==========================================================
# Muon
#==========================================================
AllowedTopos_mu = [
    'b7invmAB9vtx20', 'b11invmAB60vtx20', 'b11invmAB24vtx20', 'b24invmAB60vtx20',
    '50invmAB130' # Zmumu
    ]

# ---- Muon Dictionary of all allowed Values ----
MuonChainParts = {
    'signature'      : ['Muon'],
    'alignmentGroup' : ['Muon','MuonnoL1'],
    'L1threshold'    : '',
    'chainPartName'  : [],
    'multiplicity'   : '',
    'trigType'       : ['mu'],
    'etaRange'       : ['0eta105'],
    'threshold'      : '',
    'tnpInfo'        : ['probe'],
    'extra'          : ['noL1', 'lateMu', "muoncalib" ,'noL2Comb','vtx','mucombTag'],
    'IDinfo'         : [],
    'isoInfo'        : ['ivarloose', 'ivarmedium', 'ivarperf','iloosems'],
    'l2AlgInfo'      : ['l2io','l2mt'],
    'lrtInfo'        : ['d0loose','d0medium','d0tight'],
    'invMassInfo'    : ['invmJPsiOS','invmDimu'],
    'msonlyInfo'     : ['msonly'],
    'addInfo'        : ['idperf','LRT','3layersEC','cosmic',"muonqual","nscan","nscan10","nscan20","nscan30","nscan40",'idtp', 'idReuse'],
    'topo'           : AllowedTopos_mu,
    'flavour'        : [],
    'sigFolder'     : ['Muon'],
    'subSigs'       : ['Muon'],
    'chainPartIndex': list(range(0,10))
}
# ---- MuonDictionary of default Values ----
MuonChainParts_Default = {
    'signature'      : ['Muon'],
    'alignmentGroup' : ['Muon'],
    'L1threshold'    : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'etaRange'       : '0eta250',
    'threshold'      : '',
    'tnpInfo'        : '',
    'extra'          : '',
    'IDinfo'         : '',
    'isoInfo'        : '',
    'l2AlgInfo'      : [],
    'lrtInfo'        : [],
    'addInfo'        : [],
    'invMassInfo'    : '',
    'msonlyInfo'     : [],
    'topo'           : [],
    'flavour'        : '',
    'sigFolder'     : ['Muon'],
    'subSigs'       : ['Muon'],
    'chainPartIndex': 0
}

#==========================================================
# Bphysics
#==========================================================
AllowedTopos_Bphysics = [
    'bJpsimumu','bJpsi','bJpsimutrk','bUpsimumu','bUpsi','bBmumu','bDimu','bDimu2700','bDimu6000','bPhi','bTau','b3mu',
    'bBmumux', 'bBmux', 'b0dRAB12vtx20', 'b0dRAB127invmAB22vtx20', 'b0dRAB207invmAB22vtx20', 'b7invmAB22vtx20',

    ##### TO BE REMOVED ONCE IMPLEMENTED IN SIGNATURE CODE
    # topoVariants
    'Bidperf','BsmumuPhi','BpmumuKp','BcmumuPi','BdmumuKst','LbPqKm','BcmumuDsloose','BcmumuDploose','BcmumuD0Xloose','BcmumuDstarloose',
    'BpmuD0X','BdmuDpX','BdmuDstarX','BsmuDsX','LbmuLcX',
    # topoExtras
    'Lxy0','sigmaLxy3','noos','nocut','lowpt'
    #########Remove until here############

]
AllowedTopos_Bphysics_topoVariant = [
    'Bidperf','BsmumuPhi','BpmumuKp','BcmumuPi','BdmumuKst','LbPqKm','BcmumuDsloose','BcmumuDploose','BcmumuD0Xloose','BcmumuDstarloose',
    'BpmuD0X','BdmuDpX','BdmuDstarX','BsmuDsX','LbmuLcX'
]
AllowedTopos_Bphysics_topoExtra = ['Lxy0','noos','nocut','lowpt']
AllAllowedTopos_Bphysics = AllowedTopos_Bphysics_topoVariant+AllowedTopos_Bphysics_topoExtra+AllowedTopos_Bphysics

# ---- Bphysics Dictionary of all allowed Values ----
BphysicsChainParts = deepcopy(MuonChainParts)
BphysicsChainParts['signature'] = ['Bphysics']
BphysicsChainParts['sigFolder'] = ['Bphysics']
BphysicsChainParts['subSigs'] = ['Bphysics']
BphysicsChainParts['topo'] = AllowedTopos_Bphysics

# ---- Bphysics Dictionary of default Values ----
BphysicsChainParts_Default = deepcopy(MuonChainParts_Default)
BphysicsChainParts_Default['signature'] = ['Bphysics']
BphysicsChainParts_Default['sigFolder'] = ['Bphysics']
BphysicsChainParts_Default['subSigs'] = ['Bphysics']
BphysicsChainParts_Default['topo'] = []


#==========================================================
# Taus
#==========================================================
AllowedTopos_tau = []

# ---- Tau Dictionary of all allowed Values ----
TauChainParts = {
    'signature'     : ['Tau'],
    'alignmentGroup': ['Tau', 'JetMET'],
    'extra'         : [],
    'L1threshold'   : '',
    'chainPartName' : '',
    'threshold'     : '',
    'reconstruction': [
                        # BRT calibration (no-tracking)
                        'ptonly',

                        # Standard reconstruction triggers
                        # 2-step FTF (Core + Iso) + PT
                        # Split in different sequences to avoid running unnecesary TauIDs
                        'tracktwoMVA', # GNTau, DeepSet and MesonCuts triggers
                        'tracktwoLLP', # RNNLLP triggers

                        # LRT reconstruction triggers
                        # 1-step FTF (LRT) + PT
                        'trackLRT', # RNNLLP triggers
                      ],
    'jet'           : ['lc', 'pf'], # Only use LCTopo jets for now
    'preselection'  : [],
    'selection'     : [
                        'idperf', # No selection
                        'perf', # NTrk selection

                        # GNTau ID WPs:
                        'verylooseGNTau', 'looseGNTau', 'mediumGNTau', 'tightGNTau',

                        # RNN/DeepSet ID WPs (for tracktwoMVA/LLP/LRT reco with DeepSet/RNNLLP TauIDs):
                        'looseRNN', 'mediumRNN', 'tightRNN',

                        # Meson b-phys triggers (used with tracktwoMVA reco without cutting on the RNN/DeepSet score):
                        'kaonpi1', 'kaonpi2', 'dipion1', 'dipion2', 'dipion3', 'dipion4', 'dikaonmass', 'singlepion',
                      ],
    'multiplicity'  : '',
    'trigType'      : ['tau'],
    'tnpInfo'       : ['probe'],
    'topo'          : AllowedTopos_tau,
    'sigFolder'     : ['Tau'],
    'subSigs'       : ['Tau'],
    'chainPartIndex': list(range(0,10))
}
TauChainParts_Default = {
    'signature'     : ['Tau'],
    'alignmentGroup': ['Tau'],
    'extra'         : '',
    'L1threshold'   : '',
    'chainPartName' : '',
    'threshold'     : '',
    'reconstruction': 'tracktwoMVA',
    'jet'           : 'lc',
    'preselection'  : '',
    'selection'     : '',
    'multiplicity'  : '',
    'trigType'      : '',
    'tnpInfo'       : '',
    'topo'          : [],
    'sigFolder'     : ['Tau'],
    'subSigs'       : ['Tau'],
    'chainPartIndex': 0
}


#==========================================================
# MET
#==========================================================
AllowedTopos_xe = []
# ---- Met Dictionary of all allowed Values ----
METChainParts = {
    'signature'      : ['MET'],
    'alignmentGroup' : ['MET','JetMET'],
    'L1threshold'    : '',
    'chainPartName'  : '',
    'threshold'      : '',
    'multiplicity'   : '',
    'topo'           : AllowedTopos_xe,
    'trigType'       : ['xe'],
    'extra'          : ['noL1'],
    'calib'          : ['lcw','em'],
    'jetCalib'       : JetChainParts['jetCalib'],
    'L2recoAlg'      : [],
    'EFrecoAlg'      : ['cell', 'tc', 'tcpufit', 'mht', 'trkmht', 'pfsum', 'cvfpufit', 'pfopufit', 'mhtpufit', 'nn'],
    'constitType'    : JetChainParts['constitType'],
    'nSigma'         : ["default",  "sig30", "sig35", "sig40", "sig45", "sig50", "sig55", "sig60"],
    'L2muonCorr'     : [],
    'EFmuonCorr'     : [],
    'addInfo'        : ['FStracks'],
    'sigFolder'      : ['MET'],
    'subSigs'        : ['MET'],
    'constitmod'     : ['cssk', 'vssk'],
    'chainPartIndex': list(range(0,10))
}
# ---- MetDictionary of default Values ----
METChainParts_Default = {
    'signature'      : ['MET'],
    'alignmentGroup' : ['MET'],
    'L1threshold'    : '',
    'trigType'       : '',
    'threshold'      : '',
    'extra'          : '',
    'calib'          : 'lcw',
    'jetCalib'       : JetChainParts_Default['jetCalib'],
    'nSigma'         : "default",
    'L2recoAlg'      : '',
    'EFrecoAlg'      : '',
    'L2muonCorr'     : '',
    'EFmuonCorr'     : '',
    'addInfo'        : '',
    'constitType'    : 'tc',
    'constitmod'     : '',
    'sigFolder'     : ['MET'],
    'subSigs'       : ['MET'],
    'chainPartIndex': 0
}

#==========================================================
# XS
#==========================================================
# ---- xs Dictionary of all allowed Values ----
XSChainParts = METChainParts
XSChainParts['signature'] = ['XS']
XSChainParts['trigType']  = ['xs']

# ---- xs Dictionary of default Values ----
XSChainParts_Default = METChainParts_Default
XSChainParts_Default['signature'] = ['XS']
XSChainParts_Default['trigType']  = ''

#==========================================================
# TE
#==========================================================
# ---- te Dictionary of all allowed Values ----
TEChainParts = METChainParts
TEChainParts['signature'] = ['TE']
TEChainParts['trigType']  = ['te']

# ---- te Dictionary of default Values ----
TEChainParts_Default = METChainParts_Default
TEChainParts_Default['signature'] = ['TE']
TEChainParts_Default['trigType']  = ''

#==========================================================
# Electron Chains
#==========================================================
AllowedTopos_e = ['Jpsiee','Zeg','Zee','Heg','bBeeM6000']
# ---- Electron Dictionary of all allowed Values ----
ElectronChainParts = {
    'signature'      : ['Electron'],
    'alignmentGroup' : ['Electron','Egamma'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'tnpInfo'        : ['probe'],
    'extra'          : ['ion'],
    'multiplicity'   : '',
    'trigType'       : ['e'],
    'threshold'      : '',
    'etaRange'       : [],
    'IDinfo'         : ['dnnloose','dnnmedium','dnntight','lhvloose','lhloose','lhmedium','lhtight','vloose','loose','medium','tight', 'mergedtight'],
    'isoInfo'        : ['ivarloose','ivarmedium','ivartight'],
    'idperfInfo'     : ['idperf'],
    'gsfInfo'        : ['nogsf'],
    'lrtInfo'        : ['lrtloose','lrtmedium','lrttight','lrtxtight','lrtvxtight'],
    'caloInfo'       : [],
    'lhInfo'         : ['nod0', 'nopix'],
    'L2IDAlg'        : ['noringer'],
    'addInfo'        : [ 'etcut', 'etcut1step',"fwd",'nopid'],
    'sigFolder'     : ['Egamma'],
    'subSigs'       : ['Electron'],
    'topo'          : AllowedTopos_e,
    'chainPartIndex': list(range(0,10))
}

# ---- Egamma Dictionary of default Values ----
ElectronChainParts_Default = {
    'signature'      : ['Electron'],
    'alignmentGroup' : ['Electron'],
    'multiplicity'   : '',
    'L1threshold'         : '',
    'trigType'       : '',
    'threshold'      : '',
    'etaRange'       : '0eta250',
    'tnpInfo'        : '',
    'extra'          : '',
    'IDinfoType'     : '',
    'IDinfo'         : '',
    'isoInfo'        : '',
    'reccalibInfo'   : '',
    'idperfInfo'     : '',
    'gsfInfo'        : '',
    'lrtInfo'        : '',
    'caloInfo'       : '',
    'lhInfo'         : '',
    'L2IDAlg'        : '',
    'hypoInfo'       : '',
    'recoAlg'        : '',
    'FSinfo'         : '',
    'addInfo'        : [],
    'sigFolder'     : ['Egamma'],
    'subSigs'       : ['Electron'],
    'topo'          : [],
    'chainPartIndex': 0
}

#==========================================================
# Photon chains
#==========================================================
# ---- Photon Dictionary of all allowed Values ----
AllowedTopos_g = ['dPhi25', 'm80']
PhotonChainParts = {
    'L1threshold'    : '',
    'signature'      : ['Photon'],
    'alignmentGroup' : ['Photon','Egamma'],
    'chainPartName'  : '',
    'multiplicity'   : '',
    'trigType'       : ['g'],
    'threshold'      : '',
    'tnpInfo'        : ['probe'],
    'extra'          : ['hiptrt', 'ion'],
    'IDinfo'         : ['etcut','loose','medium','tight'],
    'isoInfo'        : ['noiso', 'icaloloose','icalomedium','icalotight'],
    'reccalibInfo'   : [],
    'trkInfo'        : [],
    'caloInfo'       : [],
    'L2IDAlg'        : ['noringer','ringer'],
    'hypoInfo'       : '',
    'recoAlg'        : [],
    'FSinfo'         : [],
    'addInfo'        : ['etcut','nopid'],
    'sigFolder'     : ['Egamma'],
    'subSigs'       : ['Photon'],
    'topo'          : AllowedTopos_g,
    'chainPartIndex': list(range(0,10)),
    }

# ---- Photon Dictionary of default Values ----
PhotonChainParts_Default = {
    'signature'      : ['Photon'],
    'alignmentGroup' : ['Photon'],
    'L1threshold'    : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'threshold'      : '',
    'tnpInfo'        : '',
    'extra'          : '',
    'IDinfo'         : '',
    'isoInfo'        : '',
    'reccalibInfo'   : '',
    'trkInfo'        : '',
    'caloInfo'       : '',
    'L2IDAlg'        : '',
    'hypoInfo'       : '',
    'recoAlg'        : '',
    'FSinfo'         : '',
    'addInfo'        : [],
    'sigFolder'     : ['Egamma'],
    'subSigs'       : ['Photon'],
    'topo'          : [],
    'chainPartIndex': 0
    }

#==========================================================
# MinBias chains
#==========================================================
# ---- MinBias Dictionary of all allowed Values ----
MinBiasChainParts = {
    'signature'      : ['MinBias'],
    'alignmentGroup' : ['MinBias'],
    'L1threshold'    : '',
    'chainPartName'  : '',
    'multiplicity'   : '',
    'trigType'       : ['mb'],
    'threshold'      : '',
    'extra'          : ['noisesup', 'vetombts2in', 'vetombts1side2in',  'vetospmbts2in', "vetosp" ,'ion', 'ncb', 'blayer', 'dijet', 'all', 'q2'], #ncb = non collision background, blayer = only sum innermost pix layer
    'IDinfo'         : [],
    'ZDCinfo'        : ['lg', 'hg'],
    'trkInfo'        : ['hlttr', 'ftk', 'costr'],
    'hypoSPInfo'     : ['sp2', 'sp3', 'sp5', 'sp10', 'sp15', 'sp50', 'sp100', 'sp300', 'sp400', 'sp500', 'sp600', 'sp700', 'sp800', 'sp900',
                        'sp1000', 'sp1100', 'sp1200', 'sp1300', 'sp1400', 'sp1500', 'sp1600', 'sp1700', 'sp1800',
                        'sp2000', 'sp2100', 'sp2200', 'sp2300', 'sp2400', 'sp2500', 'sp2700', 'sp2800', 'sp2900', 'sp3000',
                        'sp3100', 'sp3500', 'sp4100', 'sp4500', 'sp4800', 'sp5000', 'sp5200',
                        'vpix15', 'vpix30', 'vpix35', 'vpix40', 'vpix45', 'vpix50', 'vpix55', 'vpix60',
                        'pix20','pix50','pix100', 'pix200', 'pix500', 'pix1000',
                        'nototpix20', 'nototpix30','nototpix50', 'nototpix70', 'nototpix100', 'nototpix200', 'nototpix500'],
    'pileupInfo'     : ['pusup0', 'pusup7', 'pusup10', 'pusup15', 'pusup20', 'pusup30', 'pusup40','pusup50','pusup60', 'pusup70', 'pusup80', 'pusup90', 'pusup100', 'pusup110', 'pusup120', 'pusup130', 'pusup150', 'pusup180', 'pusup190',
                        'pusup200', 'pusup220', 'pusup240', 'pusup250', 'pusup260', 'pusup270', 'pusup280', 'pusup290', 'pusup300'],
    'hypoTrkInfo'    : ['trk3','trk5','trk10','trk15',  'trk20', 'trk25',  'trk30', 'trk35', 'trk40', 'trk45', 'trk50', 'trk55', 'trk60', 'trk65', 'trk70', 'trk75', 'trk80', 'trk90',
                        'trk100', 'trk110', 'trk120', 'trk130', 'trk140', 'trk150', 'trk160', 'trk180', 'trk200', 'trk220', 'trk240', 'trk260', 'trk280', 'trk290',
                         '2trk6', '1trk4', '1trk5', '1trk2', '0trk2'], #ranges for exclusive tracks
    'hypoPtInfo'     : [ 'pt0p2', 'pt0p5', 'pt1', 'pt2', 'pt4', 'pt6', 'pt8', 'pt10' ],
    'recoAlg'        : ['mbts', 'sptrk', 'sp', 'noalg', 'perf', 'hmt', 'hmtperf', 'idperf', 'zdcperf', 'afprec', 'afptof', 'afpdz5', 'afpdz10', 'excl', 'pixsptrk'],
    'addInfo'        : ['peb', 'pc'],
    'sigFolder'     : ['MinBias'],
    'subSigs'       : ['MinBias'],
    'chainPartIndex': list(range(0,10))
    }
# ---- MinBiasDictionary of default Values ----
MinBiasChainParts_Default = {
    'signature'      : ['MinBias'],
    'alignmentGroup' : ['MinBias'],
    'L1threshold'    : '',
    'chainPartName'  : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'threshold'      : '',
    'extra'          : '',
    'IDinfo'         : '',
    'ZDCinfo'        : '',
    'trkInfo'        : '',
    'hypoSPInfo'     : '',
    'pileupInfo'     : '',
    'hypoTrkInfo'     : '',
    'hypoPtInfo'     : '',
    'hypoSumEtInfo': '',
    'recoAlg'        : [],
    'addInfo'        : [],
    'sigFolder'     : ['MinBias'],
    'subSigs'       : ['MinBias'],
    'chainPartIndex': 0
    }

#==========================================================
# HeavyIon chains
#==========================================================
# ---- HeavyIon Dictionary of all allowed Values ----
HeavyIonChainParts = {
    'signature'      : ['HeavyIon'],
    'alignmentGroup' : ['HeavyIon'],
    'L1threshold'    : '',
    'chainPartName'  : '',
    'multiplicity'   : '',
    'trigType'       : ['hi'],
    'threshold'      : '',
    'extra'          : [],
    'IDinfo'         : [],
    'trkInfo'        : [],
    'eventShape'     : [],
    'eventShapeVeto' : [],
    'hypoL2Info'     : [],
    'pileupInfo'     : [],
    'hypoEFInfo'     : [],
    'hypoEFsumEtInfo': [],
    'hypoFgapInfo'   : ['FgapAC3', 'FgapAC5', 'FgapAC10', 'FgapA3', 'FgapA5', 'FgapA10', 'FgapC3', 'FgapC5', 'FgapC10'],
    'hypoUCCInfo'    : ['uccTh1','uccTh2','uccTh3'],
    'recoAlg'        : [],
    'addInfo'        : [],
    'sigFolder'     : ['HeavyIon'],
    'subSigs'       : ['HeavyIon'],
    'chainPartIndex': list(range(0,10))
    }

# ---- HeavyIonDictionary of default Values ----
HeavyIonChainParts_Default = {
    'signature'      : ['HeavyIon'],
    'alignmentGroup' : ['HeavyIon'],
    'L1threshold'    : '',
    'chainPartName'  : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'threshold'      : '',
    'extra'          : '',
    'IDinfo'         : '',
    'trkInfo'        : '',
    'eventShape'     : '',
    'eventShapeVeto' : '',
    'hypoL2Info'     : '',
    'pileupInfo'     : '',
    'hypoEFInfo'     : '',
    'hypoEFsumEtInfo': '',
    'hypoFgapInfo'   : [],
    'hypoUCCInfo'    : [],
    'recoAlg'        : [],
    'addInfo'        : [],
    'sigFolder'     : ['HeavyIon'],
    'subSigs'       : ['HeavyIon'],
    'chainPartIndex': 0
    }

#==========================================================
# ---- CosmicDef chains -----
#==========================================================
AllowedCosmicChainIdentifiers = ['larps','larhec',
                                 'sct',  'id',]

# ---- Cosmic Chain Dictionary of all allowed Values ----
CosmicChainParts = {
    'signature'      : ['Cosmic'],
    'alignmentGroup' : ['Cosmic'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'purpose'        : AllowedCosmicChainIdentifiers,
    'addInfo'        : ['cosmicid','noise', 'beam', 'laser', 'AllTE', 'central', 'ds','CIS'], #'trtd0cut'
    'trackingAlg'    : ['idscan', 'sitrack', 'trtxk'],
    'hits'           : ['4hits'],
    'threshold'      : '',
    'multiplicity'   : '',
    'trigType'       : 'cosmic',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Cosmic'],
    'chainPartIndex': list(range(0,10))
    }

# ---- Cosmic Chain Default Dictionary of all allowed Values ----
CosmicChainParts_Default = {
    'signature'      : ['Cosmic'],
    'alignmentGroup' : ['Cosmic'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'purpose'        : [],
    'addInfo'        : [],
    'trackingAlg'    : [],
    'hits'           : [],
    'threshold'      : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Cosmic'],
    'chainPartIndex': 0
    }

#==========================================================
# ---- StreamingDef chains -----
#==========================================================
AllowedStreamingChainIdentifiers = ['noalg']

# ---- Streaming Chain Dictionary of all allowed Values ----
StreamingChainParts = {
    'signature'      : ['Streaming'],
    'alignmentGroup' : ['Streaming'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'threshold'      : '',
    'multiplicity'   : '',
    # No effect on configuration, used in special cases for
    # disambiguation or to allow events from the same L1 seed
    # to be written to different streams
    # New cases should be discussed with Menu Coordinators
    'streamingInfo'  : ['laser', 'CIS','idmon','mb','l1calo', 'cosmicmuons', 'bkg','vdm', 'zb', 'eb'],
    'trigType'       : 'streamer',
    'extra'          : '',
    'streamType'     : AllowedStreamingChainIdentifiers,
    'algo' : ['NoAlg'],
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Streaming'],
    'chainPartIndex': list(range(0,10))
    }

# ---- Cosmic Chain Default Dictionary of all allowed Values ----
StreamingChainParts_Default = {
    'signature'      : ['Streaming'],
    'alignmentGroup' : ['Streaming'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'threshold'      : '',
    'multiplicity'   : '',
    'streamingInfo'  : '',
    'trigType'       : '',
    'extra'          : '',
    'streamType'     : '',
    'algo' : [],
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Streaming'],
    'chainPartIndex': 0
    }

#==========================================================
# ---- CalibDef chains -----
#==========================================================
AllowedCalibChainIdentifiers = ['csccalib',     'larcalib',
                                'idcalib',      'l1calocalib',
                                'tilelarcalib',
                                'larnoiseburst','ibllumi',
                                'l1satmon',     'zdcpeb',
                                'calibAFP', 'larpsallem', 'larpsall',
                                'acceptedevts', 'metcalo', 'mettrk',
                                ]

# ---- Calib Chain Dictionary of all allowed Values ----
##stramingInfo not use in ChainConfiguration, only to distinguish streaming

CalibChainParts = {
    'signature'      : ['Calib'],
    'alignmentGroup' : ['Calib'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'purpose'        : AllowedCalibChainIdentifiers,
    'location'       : ['central', 'fwd'],
    'addInfo'        : ['loose','noise','beam'],
    'hypo'           : ['trk4','trk9', 'trk16', 'trk29', 'conej40', 'conej165', 'conej75_320eta490', 'conej140_320eta490','satu20em'],
    'streamingInfo'  : ['vdm',],
    'threshold'      : '',
    'multiplicity'   : '',
    'trigType'       : ['trk'],
    'extra'          : ['bs',''],
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Calib'],
    'chainPartIndex': list(range(0,10))
    }


# ---- Calib Chain Default Dictionary of all allowed Values ----
CalibChainParts_Default = {
    'signature'      : ['Calib'],
    'alignmentGroup' : ['Calib'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'purpose'        : [],
    'addInfo'        : [],
    'hypo'           : '',
    # 'hits'           : [],
    'streamingInfo'  : [],
    'threshold'      : '',
    'multiplicity'   : '',
    'location'   : '',
    'trigType'       : '',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Calib'],
    'chainPartIndex': 0
    }

#==========================================================
# ---- MonitorDef chains -----
#==========================================================
AllowedMonitorChainIdentifiers = ['robrequest', 'timeburner',
                                  'idmon','larsupercellmon',
                                  'l1calooverflow', 'l1topoPh1debug',
                                  'mistimemonl1bccorr','mistimemonl1bccorrnomu',
                                  'mistimemoncaltimenomu','mistimemoncaltime',
                                  'mistimemonj400', 'caloclustermon']

# ---- Monitor Chain Dictionary of all allowed Values ----
MonitorChainParts = {
    'signature'      : ['Monitor'],
    'alignmentGroup' : ['Monitor'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'monType'        : AllowedMonitorChainIdentifiers,
    'hypo'           : ['trkFS',],
    'threshold'      : '',
    'multiplicity'   : '',
    'isLegacyL1'     : ['legacy'],
    'trigType'       : 'mon',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Monitor'],
    'chainPartIndex': list(range(0,10))
    }

# ---- Monitor Chain Default Dictionary of all allowed Values ----
MonitorChainParts_Default = {
    'signature'      : ['Monitor'],
    'alignmentGroup' : ['Monitor'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'monType'        : [],
    'hypo'           : '',
    'threshold'      : '',
    'multiplicity'   : '',
    'isLegacyL1'     : [],
    'trigType'       : '',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Monitor'],
    'chainPartIndex': 0
    }

#==========================================================
# ---- EB chains -----
#==========================================================
AllowedEBChainIdentifiers = ['eb']

# ---- Enhanced Bias Chain Dictionary of all allowed Values ----
EnhancedBiasChainParts = {
    'signature'      : ['EnhancedBias'],
    'alignmentGroup' : ['EnhancedBias'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'algType'        : ['medium','firstempty','empty','unpairediso','unpairednoniso', 'low'],
    'threshold'      : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['EnhancedBias'],
    'chainPartIndex': list(range(0,10))
    }

# ---- EnhancedBias Chain Default Dictionary of all allowed Values ----
EnhancedBiasChainParts_Default = {
    'signature'      : ['EnhancedBias'],
    'alignmentGroup' : ['EnhancedBias'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'algType'        : 'physics',
    'threshold'      : '',
    'multiplicity'   : '',
    'trigType'       : '',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['EnhancedBias'],
    'chainPartIndex': 0
    }

#==========================================================
# ---- BeamspotDef chains -----
#==========================================================
AllowedBeamspotChainIdentifiers = ['beamspot',]
BeamspotChainParts = {
    'signature'      : ['Beamspot'],
    'alignmentGroup' : ['Beamspot'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'monType'        : AllowedBeamspotChainIdentifiers,
    'location'       : ['vtx'],
    'addInfo'        : ['trkFS', 'allTE', 'activeTE','idperf'],
    'hypo'           : [],
    'l2IDAlg'        : ['trkfast'],
    'threshold'      : '',
    'multiplicity'   : '',
    'trigType'       : 'beamspot',
    'extra'          : '',
    'sigFolder'     : ['CalibCosmicMon'],
    'subSigs'       : ['Beamspot'],
    'chainPartIndex': list(range(0,10)),
    'beamspotChain' : [],
    }

# ---- Beamspot Chain Default Dictionary of all allowed Values ----
BeamspotChainParts_Default = {
    'signature'      : ['Beamspot'],
    'alignmentGroup' : ['Beamspot'],
    'chainPartName'  : '',
    'L1threshold'    : '',
    'monType'        : [],
    'addInfo'        : [],
    'hypo'           : [],
    'l2IDAlg'        : [],
    'threshold'      : '',
    'multiplicity'   : '',
    'location'       : 'vtx',
    'trigType'       : '',
    'extra'          : '',
    'sigFolder'      : ['CalibCosmicMon'],
    'subSigs'        : ['Beamspot'],
    'chainPartIndex' : 0,
    'beamspotChain'  : '',
    }

#==========================================================
# Unconventional Tracking
#==========================================================
# ---- Unconventional Tracking Dictionary of all allowed Values ----
UnconventionalTrackingChainParts = {
    'signature'      : ['UnconventionalTracking'],
    'alignmentGroup' : ['UnconventionalTracking'],
    'L1threshold'    : '',
    'chainPartName'  : [],
    'multiplicity'   : '',
    'trigType'       : ['isotrk', 'fslrt', 'dedxtrk', 'hitdvjet', 'fsvsi', 'distrk', 'dispjet', 'dispvtx'],
    'threshold'      : '',
    'IDinfo'         : ['loose','medium','tight','vloose'],
    'isoInfo'        : ['iaggrmedium','iaggrloose','imedium','iloose'],
    'extra'          : '',
    'addInfo'        : ['perf'],
    'dispjetConfig'  : ['3d2p', '1p', 'x3d1p', '2p'],
    'sigFolder'     : ['UnconventionalTracking'],
    'subSigs'       : ['UnconventionalTracking'],
    'chainPartIndex': list(range(0,10))
}
# ---- Unconventional Tracking Dictionary of default Values ----
UnconventionalTrackingChainParts_Default = {
    'signature'      : ['UnconventionalTracking'],
    'alignmentGroup' : ['UnconventionalTracking'],
    'L1threshold'    : '',
    'chainPartName'  : [],
    'multiplicity'   : '',
    'IDinfo'         : '',
    'trigType'       : '',
    'threshold'      : '',
    'isoInfo'        : '',
    'extra'          : '',
    'addInfo'        : '',
    'dispjetConfig'  : '',
    'sigFolder'     : ['UnconventionalTracking'],
    'subSigs'       : ['UnconventionalTracking'],
    'chainPartIndex': 0
}

#==========================================================
# Combined Chains
#==========================================================
AllowedTopos_comb = [
    'idZmumu','idJpsimumu',
    'dRAA12', 'dRAB15', '03dRAB','02dRAB10','03dRAB10','03dRAB30','03dRAB35','dRAD04', 'dRAF04','dRAB03','dRAB04', 'dRAB05', '02dRAB','02dRAC','03dRAC30','03dRAC35','02dRBC','15dRBC45','50invmAB','60invmAB','afpdijet','18dphiAB','18dphiAC','80mTAC','80mTAD',
    'anomdet','anomdetL','anomdetM','anomdetT',
    '29dphiAA', '29dphiAB', '30dphiAA', '30dphiAB', # g-2 tau triggers
    '90invmAB',# TEST
    '1invmAB5','50invmAB130','50invmBC130', # Jpsiee, Zee/Zeg
    '25dphiAA','25dphiBB','25dphiCC','invmAA80', # Low-mass diphoton
    '10invmAA70', # Low-mass dimuon
    'invmAB10', '10invmAB70',
    '7invmAB9', '11invmAB60', '11invmAB24', '24invmAB60', '7invmAA9', '11invmAA60', '11invmAA24', '24invmAA60',
    '20detaAA' # Low mass Drell-Yan
    ]

# ---- Combined Dictionary of all allowed Values ----
CombinedChainParts = deepcopy(PhotonChainParts)
CombinedChainParts['signature'] = ['Photon','Muon']
CombinedChainParts['chainParts'] = ['g','mu'],
CombinedChainParts['topo'] = AllowedTopos_comb
# ---- Combined Dictionary of default Values ----
CombinedChainParts_Default = deepcopy(PhotonChainParts_Default)
CombinedChainParts_Default['signature'] = ['Photon','Muon']
CombinedChainParts_Default['chainParts'] = ['g','mu'],
CombinedChainParts_Default['trigType'] = ''
CombinedChainParts_Default['topo'] = []

#==========================================================
# ----- Allowed HLT Topo Keywords (also: generic topos like DR, DETA, DPHI...)
#==========================================================
#NOTE: removed jets from list, special case for VBF triggers
AllowedTopos = AllowedTopos_e + AllowedTopos_g + AllowedTopos_mu + AllowedTopos_Bphysics + AllowedTopos_xe + AllowedTopos_tau + AllowedTopos_comb

#==========================================================
# Obtain signature type
#==========================================================
def getSignatureNameFromToken(chainpart):
    import re
    theMatchingTokens = []
    reverseSliceIDDict = { subvalue: key for key, value in SliceIDDict.items() for subvalue in ([value] if not isinstance(value, list) else value) } #reversed SliceIDDict
    for sig,token in SliceIDDict.items():
        token = token if isinstance(token, list) else [token]
        for subtoken in token:
            if re.match(r'^\d*'+subtoken+r'\d*\w*$', chainpart):
                theMatchingTokens += [subtoken]
    if len(theMatchingTokens) > 0:
        return reverseSliceIDDict[sorted(theMatchingTokens, key=lambda x: len(x), reverse=True)[0]]
    else:
        log.error('No signature matching chain part %s was found.', chainpart)

    raise Exception('[getSignatureNameFromToken] Cannot find signature from chain name, exiting.')

    return False


#==========================================================
# Signature dictionaries to use
#==========================================================
def getSignatureInformation(signature):
    if signature == 'Electron':
        return [ElectronChainParts_Default, ElectronChainParts]
    if signature == 'Photon':
        return [PhotonChainParts_Default, PhotonChainParts]
    if signature == "Jet":
        return [JetChainParts_Default, JetChainParts]
    if signature == "Bjet":
        return [bJetChainParts_Default, JetChainParts]
    if signature == "Beamspot_Jet":
        return [BeamspotJetChainParts_Default, JetChainParts]
    if signature == "Tau":
        return [TauChainParts_Default, TauChainParts]
    if signature == "Ditau":
        return [ditauJetChainParts_Default, JetChainParts]
    if (signature == "Muon"):
        return [MuonChainParts_Default, MuonChainParts]
    if  (signature == "Bphysics"):
        return [BphysicsChainParts_Default, BphysicsChainParts]
    if  (signature == "Combined"):
        return [CombinedChainParts_Default, CombinedChainParts]
    if signature == "MET":
        return [METChainParts_Default, METChainParts]
    if signature == "XS":
        return [XSChainParts_Default, XSChainParts]
    if signature == "TE":
        return [TEChainParts_Default, TEChainParts]
    if signature == "MinBias":
        return [MinBiasChainParts_Default, MinBiasChainParts]
    if signature == "HeavyIon":
        return [HeavyIonChainParts_Default, HeavyIonChainParts]
    if signature == "Cosmic":
        return [CosmicChainParts_Default, CosmicChainParts]
    if signature == "Calib":
        return [CalibChainParts_Default, CalibChainParts]
    if signature == "Streaming":
        return [StreamingChainParts_Default, StreamingChainParts]
    if signature == "Monitor":
        return [MonitorChainParts_Default, MonitorChainParts]
    if signature == "Beamspot":
        return [BeamspotChainParts_Default, BeamspotChainParts]
    if signature == "EnhancedBias":
        return [EnhancedBiasChainParts_Default, EnhancedBiasChainParts]
    if signature == "UnconventionalTracking":
        return [UnconventionalTrackingChainParts_Default, UnconventionalTrackingChainParts]
    if signature == "Test":
        return [TestChainParts_Default, TestChainParts]
    else:
        raise RuntimeError("ERROR Cannot find corresponding dictionary for signature", signature)

#==========================================================
# Analysis the base pattern: <mult><signatureType><threshold><extraInfo>
#==========================================================
def getBasePattern():
    import re
    allTrigTypes = []
    for v in SliceIDDict.values():
        if isinstance(v, list):
            allTrigTypes += v
        else:
            allTrigTypes.append(v)

    possibleTT = '|'.join(allTrigTypes)
    pattern = re.compile(r"(?P<multiplicity>\d*)(?P<trigType>(%s))(?P<threshold>\d+)(?P<extra>\w*)" % (possibleTT))
    return pattern
