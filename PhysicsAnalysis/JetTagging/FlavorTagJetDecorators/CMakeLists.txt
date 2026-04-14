# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Jet-level decorator algorithms for flavour-tagging derivations.
#
# These algorithms pre-compute variables at derivation time and write
# them as decorations on jets/EventInfo.  Constituent-level variables
# (tracks, electrons, muons, calo clusters, flow elements) are handled
# by PassThrough model loaders in FlavorTagInference.

atlas_subdir( FlavorTagJetDecorators )

find_package( ROOT COMPONENTS Core MathCore )

if (NOT XAOD_STANDALONE)

  atlas_add_component( FlavorTagJetDecoratorsLib
    src/JetOverlapLeptonDecoratorAlg.cxx
    src/PrimaryVertexDecoratorAlg.cxx
    src/SoftElectronSelectionAlg.cxx
    src/components/FlavorTagJetDecorators_entries.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES
      AthenaBaseComps
      GaudiKernel
      AthContainers
      AthLinks
      JetInterface
      FlavorTagInference
      xAODBase
      xAODCaloEvent
      xAODCore
      xAODEgamma
      xAODEventInfo
      xAODJet
      xAODMuon
      xAODPFlow
      xAODTracking
      xAODTruth
      MCTruthClassifierLib
      TruthUtils
      ${ROOT_LIBRARIES}
  )

endif()
