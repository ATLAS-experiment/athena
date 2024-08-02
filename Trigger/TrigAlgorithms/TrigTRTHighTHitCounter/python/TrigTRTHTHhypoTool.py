# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def _IncTool(flags, name):
    
    monTool = GenericMonitoringTool(flags, "MonTool_"+name,
                                    HistPath = 'TrigTRTHTHhypo/'+name)
    monTool.defineHistogram('HTRatioRoad', type='TH1F', path='EXPERT', title="TrigTRTHTH Hypo HTRatioRoad", xbins=10, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('HTRatioWedge', type='TH1F', path='EXPERT', title="TrigTRTHTH Hypo HTRatioWedge", xbins=10, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('TRTHTHitsRoad', type='TH1F', path='EXPERT', title="TrigTRTHTH Hypo TRTHTHitsRoad", xbins=100, xmin=0, xmax=100)
    monTool.defineHistogram('TRTHTHitsWedge', type='TH1F', path='EXPERT', title="TrigTRTHTH Hypo TRTHTHitsWedge", xbins=100, xmin=0, xmax=100)

    tool = CompFactory.TrigTRTHTHhypoTool( name,
                                           AcceptAll = False,
                                           MinTRTHTHitsRoad = 20,
                                           MinHTRatioRoad = 0.4,
                                           MinTRTHTHitsWedge = 30,
                                           MinHTRatioWedge = 0.5,
                                           DoWedge = True,
                                           DoRoad = False,
                                           MonTool = monTool )
    return tool

def TrigTRTHTHhypoToolFromDict( flags, chainDict ):
    """ Use menu decoded chain dictionary to configure the tool """
    name = chainDict['chainName']
    return _IncTool( flags, name )
