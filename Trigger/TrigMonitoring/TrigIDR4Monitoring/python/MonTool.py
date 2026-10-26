#
#  @file   MonTool.py
#
#  @author sutt@cern.ch
#  @date   Tue  7 Jul 2026 12:53:51 BST
#
#      create all the histograms for each analysis - this will get called once for each 
#      configured chain - we can set the HistPath either already here, or from the c++ 
#      code
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

import sys


from TrigInDetAnalysisExample.TIDAMonTool import monGroup


# histogram filler factory, so we can book
# only the relevant fillers ...

from            TrigIDR4Monitoring.Tier0Config import            Tier0_builder
from  TrigIDR4Monitoring.TrackParametersConfig import  TrackParameters_builder
from      TrigIDR4Monitoring.ResolutionsConfig import      Resolutions_builder
from     TrigIDR4Monitoring.EfficienciesConfig import     Efficiencies_builder
from         TrigIDR4Monitoring.VerticesConfig import         Vertices_builder
from        TrigIDR4Monitoring.SiHitPlotConfig import        SiHitPlot_builder
from          TrigIDR4Monitoring.TRTPlotConfig import          TRTPlot_builder

# for when we want to configure histograms from json files ...
# from           JsonConfig import         JsonHist_builder




_factory = {
    "Tier0" :                     Tier0_builder,
    "Resolutions" :         Resolutions_builder,
    "Efficiencies" :       Efficiencies_builder,
    "Vertices" :               Vertices_builder, 
    "TrackParameters" : TrackParameters_builder, 
    "SiHits"          :       SiHitPlot_builder, 
    "TRTHits"         :         TRTPlot_builder 
}




# actually create the monTool to go along with a specific chain
# stores the histogram, binning, creates all the histograms etc

def createMonTool( flags, slicetag, chain, fillers ) :

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool( flags, chain )

    histpath = slicetag + monGroup(chain)

    from TrigInDetAnalysisExample.chainString import chainString
    cs = chainString( chain )
    
    # monTool.HistPath would this be better set in the c++ code ?
    
    if  flags is None:
        mypath = "EXPERT"
        monTool.HistPath = histpath
    else:
        #   mypath = flags.DQ.FileKey
        # can't set the "path" in the histograms as that gets apended 
        # to the histogram name when using the official monitoring 
        # framework and we absolutely don't want that
        mypath = ""
        monTool.HistPath = "/" + flags.DQ.FileKey + "/" + histpath
        # monTool.HistPath = histpath
        
#   do we need this ? 
#   monTool.UseCache = True 
        
    monTool.convention      = 'OFFLINE'
    monTool.defaultDuration = 'run'

    # now add the requested histograms fro the builders ...
    # should this function beome a class with the factory and
    # builders as elements ?
    
    for filler in fillers:
        toolbuilder = _factory.get(filler)
        if toolbuilder is None:
            print(f"Unknown filler '{filler}'")
            sys.exit(1)
        toolbuilder( monTool, mypath, chain, cs )

    #    JsonHist_builder( monTool, mypath, "/afs/cern.ch/user/s/sutt/b/Trigger/TrigMonitoring/TrigIDR4Monitoring/data/NtracksPlotsDef.json" )
        
    return monTool
    
