#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''
@file    ParticleSelectorConfig.py
@author  sutt @ cern.sh
@date    2026-08-17
@brief 
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# This is called ParticleSelector, rather than "TrackSelector",
# or "OfflineTrackSelector" etc as it selects from TrackParticles,
# or TruthParticles, and can be used for any tracks, not just
# "offline" tracks or "Truth" particles

def ParticleSelectorCfg( flags, name="ParticleSelector", **kwargs ) :

    # NB: for this selector, we have changed the way the flags are
    #     controlled so we don't need "offl" or "truth" in front of 
    #     the flags to distinguish what type of tool they are meant
    #     for, ie we will want a flags.referenceTracks and testTracks
    #     flags for the track collection names,
    #     and then RefSelector, TestSelector etc to specify
    #     which types of selector we want, and we will want
    #     RefSelector.minPT, TestSelector.minPT etc ( or
    #     maybe, even Reference.minPT without the "Selector")
    #     so the selectors can run on test, or refrence, without 
    #     leaking their ultimate use into the names of their flags

    acc = ComponentAccumulator()

    # actually, this setting of some parameters from an offline tool,
    # and some parameters from some other tool is not very helpful
    # - why not set all selection parameters consistently ?
    # pretty much the offline  tool should be limited to just setting
    # the working points, not the other cuts - it is a mess having some
    # cuts applied in tool X and some others in tool Y, and cuts on 
    # some of the same variables in tools X and Y

    if flags.has("UseOfflineTool") and flags.UseOfflineTool : 

        kwargs_InDetTrackSelectionTool = {}

        #    print_obj( kwargs_InDetTrackSelectionTool )

        for key, value in flags.InDetSelector.node_flags():
            kwargs_InDetTrackSelectionTool.setdefault( key, value )
        
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import InDetTrackSelectionToolCfg
        offlineSelectionTool = acc.popToolsAndMerge( InDetTrackSelectionToolCfg( flags, **kwargs_InDetTrackSelectionTool) )
        
        kwargs.setdefault( "offlineTool", offlineSelectionTool )  

    # any number of ways to set this, eg could base it on whether the Refselector
    # node itself has a node TruthSelector - as it does for the flags which it is
    # setting but then we would have to explicitly set some, rather than setting
    # the six "default" values that we are actually setting in the TruthSelectorBaseCfg
    # that might actuallky be better than using a custom UseTruthTool flag

    if flags.Input.isMC and ( flags.has("UseTruthTool", True) or flags.hasNode("TruthSelector") ) : 

        # TruthSelectionBaseTool properties
        # ---------------------------------
        # we shuld NOT have several different ways to select on PT, ie our selection, some
        # offline tools selection, so truth selection ??? There should be one, and only one
        # way for any specific selection, eg if the Offline "Offline" or "Truth" selectors
        # require it, then we should not select it in our tools
        # of perhaps better, might be to replace all that selection to our tool ... 

        kwargs_base = {}

        if flags.hasNode("TruthSelector"):
            for key, value in flags.TruthSelector.node_flags():
                kwargs_base.setdefault( key. value )
            
        # from TrigIDR4Monitoring.SelectorConfig import TruthSelectorBaseCfg
        from TrigIDR4Monitoring.TruthSelectorBase import TruthSelectorBaseCfg

        # ... for instance here, do we need this external tool ? And if so, why are
        # we bothering with our own tool to do the same thing ? why aren't we just deferring
        #  the entire selection to that tool
    
        kwargs.setdefault("truthTool", acc.popToolsAndMerge( TruthSelectorBaseCfg( flags, name="TruthSelector_"+name, **kwargs_base ) ) )

    # now the remaining generic flags ...
        
    for key, value in flags.node_flags():
        kwargs.setdefault( key, value )
    
    acc.setPrivateTools( CompFactory.IDTPM2.ParticleSelector( name, **kwargs ) )
    
    return acc

