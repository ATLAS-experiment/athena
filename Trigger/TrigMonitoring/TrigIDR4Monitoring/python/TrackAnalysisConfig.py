#
# @file     TrackAnalysisConfig
#
# @author   sutt@cern.ch
# @date     Tue  7 Jul 2026 12:10:43 BST
#
#           Configures the track analyses for IDTPM2,
#           currently quite trigger biased, but can easily
#           be modified to look less trigger specific
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
# from AthenaConfiguration.ComponentFactory import CompFactory
# from AthenaCommon.Logging import logging

from TrigIDR4Monitoring.MonTool import createMonTool

from TrigInDetAnalysisExample.chainString import chainString


def createTrackAnalysis( flags, name="TrackAnalysis", chains=[], label="", kak=None ):

    # newer releases don't allow you to modify flags during the configuration,
    # so we sack off the input flags and just duplicate and create our own.
    # need two at the moment, since we use the same tool for the
    # reference track selection, and the test track selection, so need
    # different settings of the flags for each
    # we are implementing a better solution at the moment, where we can
    # actually tell tools what the domain of the flags is that they should use

    #  from AthenaConfiguration.AllConfigFlags import initConfigFlags
    #  flags = initConfigFlags()
    
    #    if isinstance( chains, list ) :
    #        print( "" )
    #        for ch in chains : print( "list chain: ", ch )
    
            
    from AthenaConfiguration.ComponentFactory import CompFactory
    
    if isinstance( chains, list ) : chain = chains[0]
    else : chain = chains

    cs = chainString( chain )

    # print( "RoiKey: ", cs.roi, "   :: ", name )
    
    from TrigIDR4Monitoring.ParticleSelectorConfig import ParticleSelectorCfg

    from TrigIDR4Monitoring.MatchingConfig import TrackViewMatchingToolCfg
    
    #    flags.addFlag("RoiKey", cs.roi ) # do we still need that at all ? 

    # for electrons ...
    # setFlagIfNotSet( flags, "minET",     14000 )
    # setFlagIfNotSet( flags, "minETovpt",   0.8 )

    # flags2.addFlag("UseOfflineTool", False )

    tool = CompFactory.IDTPM2.TrackAnalysis(name)

    # for electrons ...

    if flags.has("Reference") : tool.Reference    = flags.Reference

    if flags.has("ReferencePdgIdTag") : tool.ReferencePdgId = flags.ReferencePdgIdTag
    if flags.has("MinTagPT")   : tool.MinTagPT   = flags.MinTagPT
    if flags.has("MinTagMass") : tool.MinTagMass = flags.MinTagMass
    if flags.has("MaxTagMass") : tool.MaxTagMass = flags.MaxTagMass

    if flags.has("useDecorations") : tool.useDecorations  = flags.useDecorations

    
    if len(chains) > 1 :
        tool.TagChain = chains[1]
        if flags.has("ReferenceTag") :      tool.ReferenceTag      = flags.ReferenceTag
        if flags.has("ReferencePdgIdTag") : tool.ReferencePdgIdTag = flags.ReferencePdgIdTag

    # don't want the vertex analysis if no vertices are specified
    if cs.vtx : tool.Fillers = [ "Tier0", "TrackParameters", "Resolutions", "Efficiencies", "SiHits", "TRTHits", "Vertices" ]
    else :      tool.Fillers = [ "Tier0", "TrackParameters", "Resolutions", "Efficiencies", "SiHits", "TRTHits" ]

    # tool.Fillers = [ "Resolutions" ]
    
    montool = createMonTool( flags, label, chain, tool.Fillers )

    #   print( "in montool: ", montool.name )

    tool.montool = montool
    
#    tool.ReferenceSelector  = mktool( TrackSelectorCfg(  flags.RefSelector, name="ReferenceSelector_" + sanitise(name) ), kak )
#    tool.TestSelector       = mktool( TrackSelectorCfg( flags.TestSelector, name="TestSelector_" + sanitise(name) ), kak )

    tool.ReferenceSelector  = mktool( ParticleSelectorCfg(  flags.RefSelector, name="ReferenceSelector_" + sanitise(name) ), kak )
    tool.TestSelector       = mktool( ParticleSelectorCfg( flags.TestSelector, name="TestSelector_" + sanitise(name) ), kak )

    tool.TrackMatchingTool  = mktool( TrackViewMatchingToolCfg( flags.Matcher, name="TrackViewMatchingTool_" + sanitise(name) ), kak )

    #  tool.VertexQualitySelectionTool = mktool( VertexQualitySelectionToolCfg( flags, name="VertexQualitySelectionTool_" + sanitise(name) ), kak )    

    #  VertexQualitySelectionTool = name="VertexQualitySelectionTool"+flags.PhysVal.IDTPM2.currentTrkAna.anaTag

    #  kak.printConfig(withDetails=True, summariseProps=True)
    
    return tool



def setFlagIfNotSet( flags, attr, value ) : 
    if flags.hasFlag(attr):
        if flags._flagdict[attr].get(value) == -9999:
            flags._flagdict[attr].set(value)
        return
    else: 
        flags.addFlag(attr,value)


# just lazy, and like the code to be neater 
def mktool( ca, kak=None ):
    if kak is not None:
        return kak.popToolsAndMerge(ca)
    else:
        return ComponentAccumulator().popToolsAndMerge(ca)


def sanitise(s: str) -> str:
    """
    Replace all ':' and '=' characters in the string with '_'.
    """
    return s.replace(":", "_").replace("=", "_")


