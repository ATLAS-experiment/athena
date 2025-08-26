#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# 
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool


def setupFilterMonitoring( flags, filterAlg ):
    if not hasattr(filterAlg, "Input"):
        return

    monTool = GenericMonitoringTool(flags, 'MonTool',
                                    HistPath="HLTFramework/Filters")
    
    inputKeys = [str(i) for i in filterAlg.Input]
    monTool.defineHistogram( 'name,stat;'+filterAlg.getName(), path='EXPERT', type='TH2I',
                             title='Input activity fraction;;presence',
                             xbins=len(inputKeys), xmin=0, xmax=len(inputKeys)+2, xlabels=['exec', 'anyvalid']+inputKeys,
                             ybins=2, ymin=-0.5, ymax=1.5, ylabels=['no', 'yes'] )

    filterAlg.MonTool = monTool


def TriggerSummaryAlg( flags, name, **kwargs ):
    monTool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)
    monTool.defineHistogram('TIME_SinceEventStart', path='EXPERT', type='TH1F',
                                   title='Time since beginning of event processing;time [ms]',
                                   xbins=100, xmin=0, xmax=3.5e3   )

    alg = CompFactory.TriggerSummaryAlg( name,
                                         MonTool = monTool,
                                         **kwargs )
    return alg


def ComboHypoCfg( name ):
    acc = ComponentAccumulator()
    acc.addEventAlgo( CompFactory.ComboHypo(name) )
    return acc
