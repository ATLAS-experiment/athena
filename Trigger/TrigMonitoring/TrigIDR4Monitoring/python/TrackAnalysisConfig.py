#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


# from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
# from AthenaConfiguration.ComponentFactory import CompFactory
# from AthenaCommon.Logging import logging

# wrap the annoyoing CA to just get the tools, and not have to faff about
# with the CA directly

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def createTrackAnalysis( inflags, name="TrackAnalysis", chain="", mtool=None, kak=None ):

    # actually this won't work now - newer releases don't allow you to modify
    # flags during the configuration, so we sack off the input flags and just
    # create our own.
    flags = inflags.clone()
    
    #  from AthenaConfiguration.AllConfigFlags import initConfigFlags
    #  flags = initConfigFlags()
    
#   print( "in montool: ", mtool.name )

    from AthenaConfiguration.ComponentFactory import CompFactory

    # created this in case we need it later, but if we use my mktool wrapper
    # for all the tools, it probably won't be needed.
    # in principle, we may be able to get away with sharing some tools betwen
    # analyses in the same signature rather than all these independent tools
    # but we can sort that out later

#    if kak is None:
#        kak = ComponentAccumulator()

#  cnt  = ComponentAccumulator()
    tool = CompFactory.IDTPM.TrackAnalysis(name)
    #    cnt.setPrivateTools(tool)
        
    from TrigInDetAnalysisExample.chainString import chainString
    cs = chainString( chain )

    # actual trigger to be monitored
    tool.trigger = cs.head

    # test track collection ...
    tool.TriggerTracks = cs.tail

    # allocate the histograms ...
    tool.montool = mtool 
    
#    from InDetTrackPerfMon.InDetSelectionConfig import TrackQualitySelectionToolCfg
#    from InDetTrackPerfMon.InDetSelectionConfig import VertexQualitySelectionToolCfg
#    from InDetTrackPerfMon.InDetSelectionConfig import RoiSelectionToolCfg
#    from InDetTrackPerfMon.InDetSelectionConfig import TrackRoiSelectionToolCfg

#    from InDetTrackPerfMon.InDetMatchingConfig  import TrackMatchingToolCfg

    from InDetTrackPerfMon.InDetSelectionConfig import sanitise
    
    print( "RoiKey: ", cs.roi, "   :: ", name )
    
    flags.addFlag("RoiKey", cs.roi )

    # Stable SeltaR matching                                                                                                                                                              
    flags.addFlag("MatchingType", "StableDeltaRMatch")
    flags.addFlag("RefType",  "Offline")
    flags.addFlag("TestType", "Trigger")

    flags.lock()
    
    
#    tool.TrackQualitySelectionTool  = mktool( TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool_" + sanitise(name) ), kak )

    
#    tool.VertexQualitySelectionTool = mktool( VertexQualitySelectionToolCfg( flags, name="VertexQualitySelectionTool_" + sanitise(name) ), kak )
#    tool.RoiSelectionTool           = mktool( RoiSelectionToolCfg( flags, name="RoiSelectionTool_" + sanitise(name) ), kak )

#    tool.TrackRoiSelectionTool = mktool( TrackRoiSelectionToolCfg( flags, name="TrackRoiSelectionTool_" + sanitise(name) ), kak )


#    tool.TrackMatchingTool     = mktool( TrackMatchingToolCfg( flags, name="TrackMatchingTool_" + sanitise(name) ), kak )

    # VertexQualitySelectionTool = name="VertexQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag

#     mktool( cnt, kak )

#    print("kak: ", kak )
    
#    kak.printConfig(withDetails=True, summariseProps=True)
    
    return tool




# just lazy, and like the code to be neater 
def mktool( ca, kak=None ):
    if kak is not None:
        return kak.popToolsAndMerge(ca)
    else:
        return ComponentAccumulator().popToolsAndMerge(ca)



