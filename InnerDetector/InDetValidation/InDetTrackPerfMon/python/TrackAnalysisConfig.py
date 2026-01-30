#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


# from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
# from AthenaConfiguration.ComponentFactory import CompFactory
# from AthenaCommon.Logging import logging

# wrap the annoyoing CA to just get the tools, and not have to faff about
# with the CA directly

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# just lazy, and like the code to be neater 
def mktool( ca ):
    return ComponentAccumulator().popToolsAndMerge(ca)



def createTrackAnalysis( inflags, name="TrackAnalysis", chain="", mtool=None ):

    flags = inflags.clone()
    
#   print( "in montool: ", mtool.name )

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    # created this in case we need it later, but if we use my mktool wrapper
    # for all the tools, it probably won't be needed.
    # in principle, we may be able to get away with sharing some tools betwen
    # analyses in the same signature rather than all these independent tools
    # but we can sort that out later
    kak = ComponentAccumulator()

    tool = CompFactory.IDTPM.TrackAnalysis(name )

    from TrigInDetAnalysisExample.chainString import chainString
    cs = chainString( chain )

    tool.trigger = cs.head
    tool.montool = mtool 

    from InDetTrackPerfMon.InDetSelectionConfig import TrackQualitySelectionToolCfg
    from InDetTrackPerfMon.InDetSelectionConfig import VertexQualitySelectionToolCfg
    from InDetTrackPerfMon.InDetSelectionConfig import RoiSelectionToolCfg

    from InDetTrackPerfMon.InDetSelectionConfig import sanitise
    
    print( "RoiKey: ", cs.roi, "   :: ", name )
    
    flags.addFlag("RoiKey", cs.roi )
    
    tool.TrackQualitySelectionTool  = mktool( TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool_" + sanitise(name) ) )
    tool.VertexQualitySelectionTool = mktool( VertexQualitySelectionToolCfg( flags, name="VertexQualitySelectionTool_" + sanitise(name) ) )
    tool.RoiSelectionTool           = mktool( RoiSelectionToolCfg( flags, name="RoiSelectionTool_" + sanitise(name) ) )


#    # VertexQualitySelectionTool = name="VertexQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag
    
    
    return tool


