/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>
#include <limits>

// Framework includes
#include "AthContainers/ConstDataVector.h"
#include "AsgTools/CurrentContext.h"
#include "AsgDataHandles/ReadHandle.h"

// Local includes
#include "AssociationUtils/AltMuJetOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace
{
  /// Unit conversion constants
  const double GeV = 1e3; // FIXME local unit definition!!
}

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  AltMuJetOverlapTool::AltMuJetOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("BJetLabel", m_bJetLabel = "",
                    "Input b-jet flag. Disabled by default.");
    // Disabled by default
    declareProperty("NumJetTrk", m_numJetTrk = std::numeric_limits<int>::max(),
                    "Min number of jet tracks to keep jet and remove muon");
    // Disabled by default
    declareProperty("MuJetPtRatio", m_muJetPtRatio = 0.,
                    "Max PT ratio to keep jet and remove muon");
    declareProperty("InnerDR", m_innerDR = 0.2,
                    "Inner cone for removing jets");
    declareProperty("SlidingDRC1", m_slidingDRC1 = 0.04,
                    "The constant offset for sliding dR");
    declareProperty("SlidingDRC2", m_slidingDRC2 = 10.*GeV,
                    "The inverse muon pt factor for sliding dR");
    declareProperty("SlidingDRMaxCone",
                    m_slidingDRMaxCone = std::numeric_limits<double>::max(),
                    "Maximum allowed size of sliding dR cone");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
    declareProperty("PVContainerName", m_PVContName = "PrimaryVertices",
                    "PV Container to use");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode AltMuJetOverlapTool::initializeDerived()
  {
    // Initialize the b-jet helper
    if(!m_bJetLabel.empty())
      resetAccessor (m_accessors->m_bJetAcc, *m_accessors, m_bJetLabel);

    // Initialize the inner cone dR matcher
    m_dRMatchCone1 =
      std::make_unique<DeltaRMatcher> (m_innerDR, m_useRapidity);
    ATH_CHECK (m_dRMatchCone1->setObjectTypes (xAODType::ObjectType::Muon, xAODType::ObjectType::Jet));
    addSubtool(*m_dRMatchCone1);
    // Initialize the sliding dR matcher
    m_dRMatchCone2 =
      std::make_unique<SlidingDeltaRMatcher>
        (m_slidingDRC1, m_slidingDRC2, m_slidingDRMaxCone, m_useRapidity);
    ATH_CHECK (m_dRMatchCone2->setObjectTypes (xAODType::ObjectType::Muon, xAODType::ObjectType::Jet));
    addSubtool(*m_dRMatchCone2);

    resetAccessor (m_accessors->m_vtxContainerAcc, *m_accessors, m_PVContName, {.addMTDependency = true});

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode AltMuJetOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId eventContext) const
  {
    // Check the container types
    ATH_CHECK (checkForXAODContainer<xAOD::MuonContainer>(cont1, "First container arg is not of type MuonContainer!"));
    ATH_CHECK (checkForXAODContainer<xAOD::JetContainer>(cont2, "Second container arg is not of type JetContainer!"));

    ATH_CHECK( internalFindOverlaps(cont1, cont2, eventContext));
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode AltMuJetOverlapTool::
  internalFindOverlaps(columnar::Particle1Range muons,
                       columnar::Particle2Range jets,
                       columnar::EventContextId eventContext) const
  {
    ATH_MSG_DEBUG("Removing overlapping muons and jets");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(muons);
    initializeDecorations(jets);

    // Remove jets that overlap with muons in first cone.
    for(const auto muon : muons){
      if(!isSurvivingObject(muon)) continue;

      for(const auto jet : jets){
        if(!isSurvivingObject(jet)) continue;
        // User-defined jet criteria include b-tagging,
        // numTrack, and the mu/jet PT ratio
        if(!m_bJetLabel.empty() && acc.m_bJetAcc(jet)) continue;
        if(getNumTracks(jet, eventContext) >= m_numJetTrk) continue;
        float ptRatio = muon(acc.m_muonPtAcc) / jet(acc.m_jetPtAcc);
        if(ptRatio < m_muJetPtRatio) continue;

        if(m_dRMatchCone1->objectsMatch(jet, muon)){
          ATH_CHECK( handleOverlap(jet, muon) );
        }
      }
    }

    // Remove muons from remaining overlapping jets
    for(const auto jet : jets){
      if(!isSurvivingObject(jet)) continue;

      for(const auto muon : muons){
        if(!isSurvivingObject(muon)) continue;

        if(m_dRMatchCone2->objectsMatch(muon, jet)){
          ATH_CHECK( handleOverlap(muon, jet) );
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Retrieve the primary vertex
  //---------------------------------------------------------------------------
  int AltMuJetOverlapTool::getPrimVtxIndex(columnar::EventContextId eventContext) const
  {
    auto& acc = *m_accessors;
    if (!acc.m_vtxContainerAcc.isAvailable(eventContext)) {
      ATH_MSG_WARNING("Primary vertex container is not available");
      return -1;
    }
    auto vertices = acc.m_vtxContainerAcc(eventContext);
    for(auto vtx : vertices) {
      if(vtx(acc.m_vertexTypeAcc) == xAOD::VxType::PriVtx)
        return vertices.getIndexInRange(vtx);
    }
    ATH_MSG_WARNING("No primary vertex found");
    return -1;
  }

  //---------------------------------------------------------------------------
  // Retrieve the primary vertex
  //---------------------------------------------------------------------------
  int AltMuJetOverlapTool::getNumTracks(columnar::Particle2Id jet, columnar::EventContextId eventContext) const
  {
    // Find the primary vertex
    auto& acc = *m_accessors;
    auto vtx = getPrimVtxIndex(eventContext);
    if(vtx == -1) return -1;
    return acc.m_numTrkPt500Acc(jet)[vtx];
  }

} // namespace ORUtils
