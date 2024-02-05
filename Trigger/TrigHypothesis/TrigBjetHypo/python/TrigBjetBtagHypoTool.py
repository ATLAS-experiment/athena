# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import re
from TrigBjetHypo.TrigBjetMonitoringConfig import TrigBjetBtagHypoToolMonitoring
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigBjetBtagHypoTool')

####################################################################################################

# DL1r (Place Holder while we wait for WPs to be defined)
# Values taken from https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingBenchmarksRelease21#DL1rnn_tagger
bTaggingWP = \
  { "dl1d40" : 6.957
  , "dl1d45" : 6.344
  , "dl1d50" : 5.730
  , "dl1d55" : 5.121
  , "dl1d60" : 4.512
  , "dl1d65" : 3.882
  , "dl1d70" : 3.251
  , "dl1d72" : 2.964
  , "dl1d75" : 2.489
  , "dl1d77" : 2.157
  , "dl1d80" : 1.626
  , "dl1d82" : 1.254
  , "dl1d85" : 0.634
  , "dl1d90" : -0.465
  , "dl1d95" : -1.616

  , "gn140": 7.370
  , "gn145": 6.748
  , "gn150": 6.094
  , "gn155": 5.425
  , "gn160": 4.757
  , "gn165": 4.095
  , "gn170": 3.423
  , "gn172": 3.141
  , "gn175": 2.703
  , "gn177": 2.392
  , "gn180": 1.884
  , "gn182": 1.510
  , "gn185": 0.881
  , "gn190": -0.351
  , "gn195": -1.794

  , "gn240": 7.718
  , "gn245": 7.017
  , "gn250": 6.241
  , "gn255": 5.440
  , "gn260": 4.640
  , "gn265": 3.889
  , "gn270": 3.163
  , "gn272": 2.863
  , "gn275": 2.412
  , "gn277": 2.087
  , "gn280": 1.5866
  , "gn282": 1.186
  , "gn285": 0.5355
  , "gn290": -0.741
  , "gn295": -2.242



  , "offperf" : -999
  }

bbTaggingWP = \
  { "gn182bb96": -1.201
  , "gn177bb96": -1.201
  , "gn175bb90": -0.590
  }


####################################################################################################
def TrigBjetBtagHypoToolFromDict( flags, chainDict ):

    chainPart = chainDict['chainParts'][0]
    conf_dict = { 'threshold'    : chainPart['threshold'],
                  'multiplicity' : '1' if len(chainPart['multiplicity']) == 0 else chainPart['multiplicity'],
                  'bTag' :         chainPart['bTag'][1:],
                  'bConfig' :      'EF' if len(chainPart['bConfig']) == 0 else chainPart['bConfig'][0],
                  'minEta' :       chainPart['etaRange'].split('eta')[0],
                  'maxEta' :       chainPart['etaRange'].split('eta')[1]}
    name = chainDict['chainName']
    # TODO the chain dict can be probably passed over down the line here

    MonTool = None
    nolegname = re.sub("(^leg.*?_)", "", name)
    if 'bJetMon:online' in chainDict['monGroups']:
        MonTool = TrigBjetBtagHypoToolMonitoring(flags, f'TrigBjetOnlineMonitoring/{nolegname}')
    tool = getBjetBtagHypoConfiguration( name,conf_dict, MonTool )

    return tool


####################################################################################################  

def decodeThreshold( threshold_btag ):
    """ decodes the b-tagging thresholds """

    log.debug("decoded b-jet threshold: b%s", threshold_btag)

    tagger = "offperf" if threshold_btag == "offperf" else re.findall("(.*)[0-9]{2}",threshold_btag)[0]

    allowedTaggers = ["offperf", "dl1d", "gn182bb", "gn177bb", "gn175bb", "gn1","gn2"]
    if tagger not in allowedTaggers:
        log.debug("tagger = %s not amidst allowed taggers ",threshold_btag)
        assert False, "Can't recognize tagger during TrigBjetHypoTool configuration. Tagger = "+threshold_btag
        return None


    btagger = "DL1d20211216"
    bbtagger = "dl1dbb20230314"

    bbcut = bbTaggingWP.get(threshold_btag)

    # remove the bb part to get the b-only cut
    threshold_btag = threshold_btag.split("bb", maxsplit=1)[0]

    # for chains wanting to use GN1
    if "gn1" in threshold_btag:
        btagger = "GN120220813"

    if "gn2" in threshold_btag:
        btagger = "GN220240122"
    bcut = bTaggingWP[threshold_btag]

    return [btagger, bcut] , [bbtagger, bbcut]

####################################################################################################

def getBjetBtagHypoConfiguration( name,conf_dict, MonTool ):

    tool = CompFactory.TrigBjetBtagHypoTool( name )
    if MonTool is not None:
        tool.MonTool = MonTool

    [btagger, bcut] , [bbtagger, bbcut] = decodeThreshold( conf_dict['bTag'] )
    tool.monitoredFloats = {f'{btagger}_p{x}':f'btag_p{x}' for x in 'cub'}

    if conf_dict['bTag'] == "offperf":
        # we shoudln't be worried about rates blowing up due to bad
        # b-tagging in the boffperf chains
        tool.vetoBadBeamspot = False
        return tool

    btagTool = CompFactory.BJetThreeValueCheck(
        f'{name}_btag',
        b=f'{btagger}_pb',
        c=f'{btagger}_pc',
        u=f'{btagger}_pu',
        cFraction=0.018,
        threshold=bcut)
    if MonTool is not None:
        btagTool.MonTool = MonTool
    tool.checks.append(btagTool)

    if bbcut is not None:
        bbTool = CompFactory.BJetTwoValueCheck(
            f'{name}_bbtag',
            numerator=f'{bbtagger}_pb',
            denominator=f'{bbtagger}_pbb',
            threshold=bbcut)
        if MonTool is not None:
            bbTool.MonTool = MonTool
        tool.checks.append(bbTool)
        tool.monitoredFloats.update({
            f'{bbtagger}_p{x}':f'bbtag_p{x}' for x in ['b','bb']})

    return tool
