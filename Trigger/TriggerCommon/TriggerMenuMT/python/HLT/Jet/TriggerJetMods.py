# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from JetRecConfig.JetDefinition import JetModifier

########################################################################
# Special modifier setups used by jet trigger, but not in offline
# We just extend the stdJetModifiers from the standard config.


# HLT settings for Jet vertex tagger with neural network.
def getTrkAugNNJvtTool(jetdef, modspec):
    nnjvt_trkaug = CompFactory.getComp("JetPileupTag::JetVertexNNTagger")(
        "nnjvt_trkaugv1",
        VertexContainer = jetdef._cflags.Jet.Context[modspec or jetdef.context]["Vertices"],
        SuppressInputDependence = True,
        UseTrkAugNN = True,
        NNConfigDir = "JetPileupTag/NNJvt/HLT-2025-02-05",
        NNParamFile = "TrkAugNNJVT.Network.graph.HLT.json",
        NNCutFile = "TrkAugNNJVT.Cuts.HLT.json",
    )
    return nnjvt_trkaug

# Many JetMoment tools need to know the name of the container they operate on.
# We set the function below as the 'JetContainer' property so the config system
# can assign the right name to the c++ tool.
def _jetname(jetdef,modspec):
    return jetdef.fullname()

from JetRecConfig.StandardJetMods import stdJetModifiers
stdJetModifiers.update(

    # No need for the special momentum scales, just copy the basic four-vec
    # to "DetectorEtaPhi", because we're not doing origin correction
    # and certainly not with the offline collection names    
    ConstitFourMom_copy  = JetModifier("JetConstitFourMomTool", "constitfourmom_copy",
                                       JetScaleNames = ["DetectorEtaPhi"],
                                       AltConstitColls = [""],
                                       AltConstitScales = [0],
                                       AltJetScales = ["JetConstitScaleMomentum"]                                       
                                       ),
    Cleaning = JetModifier("JetCleaningTool","jetcleaning_{modspec}",
                           # This allows to set the modifier using a string like
                           # "Cleaning:CLEAN_LEVEL" as defined in JetCleaningTool
                           # (example "Cleaning:LooseBad")
                           CutLevel=lambda _, modspec: str(modspec),
                           prereqs=[f"mod:{mod}" for mod in ['CaloQuality']],
                           JetContainer = _jetname,
    ),
    # Note that in the current implementation, if we were to change only the NN model
    # and keep the same input variables (or some subset of them), there would be
    # repeated attempts to write the input branches, causing the job to fail.
    #
    # Suffixes could be added on the inputs, but this would add a lot of redundant
    # info, so it may be more practical at some stage to refactor and generate the
    # input decorations in an independent tool run as a precursor.
    NNJVT_TrkAugV1 = JetModifier("JetVertexNNTagger", "nnjvt_trkaugv1",
                        createfn=getTrkAugNNJvtTool,
                        prereqs = [ "mod:TrackMoments" ],
                        JetContainer = _jetname,
                        JVTName = 'NNJvtTrkAugV1',
                        passJvtName = 'NNJvtTrkAugV1Pass',
                        )
)
