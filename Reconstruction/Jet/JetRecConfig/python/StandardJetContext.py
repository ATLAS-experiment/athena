# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
""" 
This module defines the standard 'jet contexts'. 
A jet context is a set of options (mainly related to Tracks) shared by several 
components (modifier tools, input tools/algs, ...).

Other contexts are expected to be defined, for example in the trigger to deal with oher track collection, or for 
analysis having non-default PV0 choices.

Setting a context to a JetDefinition ensures identical properties are  consistently set across the components configured by the JetDefinition. 

Contexts are defined as dictionaries and are centralized under the AthConfigFlags used to configure the jobs under 'flags.Jet.Context'.
"""
from JetRecConfig.Utilities import ldict
from AthenaConfiguration.Enums import LHCPeriod
from PyUtils.moduleExists import moduleExists

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createJetContextFlags():
    flags = AthConfigFlags()

    #***********************
    run3context=ldict(
        Tracks =               "InDetTrackParticles",
        JetTracks =            "JetSelectedTracks", #used for e.g. ghost tracks (no track quality criteria applied)
        JetTracksQualityCuts = "JetSelectedTracks_trackSelOpt", #used for track-jets (trackSelOpt quality criteria applied)
        Vertices =             "PrimaryVertices",
        TVA =                  "JetTrackVtxAssoc",
        GhostTracks =          "PseudoJetGhostTrack",
        GhostTracksLabel =     "GhostTrack",
        EventDensity =         "EventDensity",
        GhostTrackCutLevel =   'NoCut', # The track selection level for ghost-associated tracks. This is different from the cutlevel we apply when performing actual calculations such as JVT or tack moments.

        # options passed to InDet::InDetTrackSelectionTool.
        #   Note : these are the standard options used for track calculations. Tracks selected for ghost-associaton have CutLevel=NoCut by default : see ghostTrackCutLevel above
        trackSelOptions = ldict( CutLevel = "Loose", minPt=500, maxAbsEta=2.5 ),
    )

    flags.addFlag("Jet.Context.Run3" , run3context)
    #***********************
    flags.addFlag("Jet.Context.Run4" , run3context.clone(
        trackSelOptions = run3context["trackSelOptions"].clone(maxAbsEta=4.0) # set range of track selection up to eta=4
    ))
    flags.addFlag("Jet.Context.HL_LHC", flags.Jet.Context.Run4) 

    #***********************
    # The default context is Run3 or Run4 according to other global flags :
    def _defaultFlag(prevFlags):
        try:
            run = prevFlags.GeoModel.Run
        except (ValueError, RuntimeError) : # several exceptions can be thrown... catch any of them
            # No GeoModel.Run -> we are in a Truth job, return an empty context.
            return {}                
        return prevFlags.Jet.Context.Run3 if run <= LHCPeriod.Run3 else prevFlags.Jet.Context.Run4
    flags.addFlag("Jet.Context.default", _defaultFlag)



    
    #***********************
    # Alternative context for AntiKt4LCTopo_EleRM jets used for the electron removed tau reconstruction  
    flags.addFlag("Jet.Context.EleRM" , run3context.clone(
        Tracks = "InDetTrackParticles_EleRM",
        TVA    = "JetTrackVtxAssoc_EleRM",
        JetTracks = "JetSelectedTracks_EleRM",
        GhostTracks = "PseudoJetGhostTrack_EleRM",
        EventDensity = "EleRM_EventDensity",
    ))


    #**********************************
    # This is not a jet context, but the list of keys related to track (Used in trigger config).
    # We store it under Context since it is a related central place. 
    flags.addFlag("Jet.Context.CommonTrackKeys",["Tracks", "Vertices", "TVA", "GhostTracks", "GhostTracksLabel", "JetTracks", "JetTracksQualityCuts"],)


    # ****************
    # Add Jet trigger context :
    if moduleExists("TriggerMenuMT"):
        from TriggerMenuMT.HLT.Jet.JetRecoCommon import addJetContextFlags
        addJetContextFlags(flags)

    # ****************
    # Add HIGG1D1 context :
    if moduleExists("DerivationFrameworkHiggs"):
        from DerivationFrameworkHiggs.HIGG1D1CustomJetsConfig import addJetContextFlags
        addJetContextFlags(flags)

    # ****************
    # Add PHYS context :
    if moduleExists("DerivationFrameworkPhys"):
        from DerivationFrameworkPhys.GNNVertexConfig import addJetContextFlags
        addJetContextFlags(flags)
        
    return flags



def propFromContext(propName):
    """Some properties might depend on the context for which jets are configured.
    This function returns a helper function which gives the value of the property propName according to the jet context.    
    """
    def getProp(jetdef, spec):
        contextDic = jetdef._contextDic
        if isinstance(spec, str):
            # user may have passed explicitly a str : allow to force an other context if jetdef.context if non void
            contextDic = jetdef._cflags.Jet.Context[spec or jetdef.context]
        return contextDic[propName]
    return getProp

def inputsFromContext(inputKey, prefix="", suffix=""):
    """Some prerequisites might depend on the context for which jets are configured.
    This function returns a helper function which gives a list of input prerequisites according to 'inputKey' in the current jetdef.context.    
    """
    def getPrereqs(jetdef):
        return f"input:{prefix}{jetdef._contextDic[inputKey]}{suffix}"
    return getPrereqs
