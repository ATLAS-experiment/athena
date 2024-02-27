# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TrigHLTJetHypo.FastReductionAlgToolFactory import toolfactory
from TrigHLTJetHypo.ConditionDefaults import defaults

from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG

import re

logger = logging.getLogger( __name__)
logger.setLevel(DEBUG)

pattern = r'^MAXMULT(?P<end>\d+)(?P<eta>[jacf]*)$'
rgx = re.compile(pattern)

etaRangeAbbrev = {
    "j":"0eta320", # default
    "a":"0eta490", 
    "c":"0eta240", 
    "f":"320eta490"
}

def prefilter_maxmult(pf_string):
    """calculate the parameters needed to generate a MaxMultFilter config 
    AlgTool starting from the prefilter substring if it appears in the 
    chain dict"""

    assert pf_string.startswith('MAXMULT'),\
        'routing error, module %s: bad prefilter %s' % (__name__, pf_string)

    m = rgx.match(pf_string)
    groupdict = m.groupdict()

    vals = {}

    # eta region
    eta_region = groupdict['eta']
    if not eta_region: eta_region = 'j'
    eta_sel = etaRangeAbbrev[eta_region]
    lo, hi = eta_sel.split('eta')
    vals = defaults('eta', lo=lo, hi=hi)

    # jet multiplicity 
    vals['end'] = int(groupdict['end'])  
    toolclass, name =  toolfactory('MaxMultFilterConfigTool')
    vals['name'] = name
    
    return toolclass(**vals)

