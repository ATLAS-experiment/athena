# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import re

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def createIDCalibHypoAlg(flags, name):
    # Monitoring
    monTool = GenericMonitoringTool(flags, "IM_MonTool"+name,
                                    HistPath = 'IDCalibHypoAlg')
    monTool.defineHistogram('pt', type='TH1F', path='EXPERT', title="p_{T};p_{T} [GeV];Nevents", xbins=50, xmin=0, xmax=100)

    # Setup the hypothesis algorithm
    theHypo = CompFactory.IDCalibHypoAlg(name,
                                         MonTool = monTool)
    return theHypo


def IDCalibHypoToolFromDict( chainDict ):

    log = logging.getLogger('IDCalibHypoTool')

    """ Use menu decoded chain dictionary to configure the tool """
    cparts = [i for i in chainDict['chainParts'] if i['signature']=='Calib']

    name = "default_chain_name"
    if 'chainName' in chainDict:
        name = chainDict['chainName']
    else:
        log.error("chainName not in chain dictionary")

    # set thresholds
    m = re.search(r'trk(\d+)',[ cpart['hypo'] for cpart in cparts ][0])
    threshold = m.group(1)

    mult = [ cpart['multiplicity'] for cpart in cparts ][0]
    thresholds = [ float(threshold) for x in range(0,int(mult)) ]
    log.debug("Threshold values are: %s", ", ".join(str(THR) for THR in thresholds))

    tool = CompFactory.IDCalibHypoTool(name,
                                       cutTrackPtGeV = thresholds)
    return tool
