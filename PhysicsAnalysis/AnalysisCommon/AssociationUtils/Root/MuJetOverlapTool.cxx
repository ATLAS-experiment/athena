/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"
#include "AthContainers/ConstAccessor.h"
#include "AsgTools/CurrentContext.h"
#include "AsgDataHandles/ReadHandle.h"

// Local includes
#include "AssociationUtils/MuJetOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"
#include "AssociationUtils/MuJetGhostDRMatcher.h"

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
  MuJetOverlapTool::MuJetOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("BJetLabel", m_bJetLabel = "",
                    "Input b-jet flag. Disabled by default.");
    declareProperty("NumJetTrk", m_numJetTrk = 3,
                    "Min number of jet tracks to keep jet and remove muon");
    declareProperty("ApplyRelPt", m_applyRelPt = false,
                    "Toggle mu/jet relative PT requirements to prioritize jet");
    declareProperty("MuJetPtRatio", m_muJetPtRatio = 0.5,
                    "Mu/jet PT ratio threshold to remove a jet");
    declareProperty("MuJetTrkPtRatio", m_muJetTrkPtRatio = 0.7,
                    "Mu/jetTrk PT ratio threshold to remove a jet");
    declareProperty("JetNumTrackDecoration", m_jetNumTrkDec = "",
                    "User-defined decoration for jet numTrack");
    declareProperty("JetSumTrackPTDecoration", m_jetSumTrkPtDec = "",
                    "User-defined decoration for jet sumTrackPT");
    declareProperty("UseGhostAssociation", m_useGhostAssociation = true,
                    "Activate ghost-association mapping for jet removals");
    declareProperty("InnerDR", m_innerDR = 0.2,
                    "Flat inner cone for removing jets");
    declareProperty("OuterDR", m_outerDR = 0.4,
                    "Flat outer cone for removing muons");
    declareProperty("UseSlidingDR", m_useSlidingDR = false,
                    "Enables sliding dR outer cone = c1 + c2/MuPt");
    declareProperty("SlidingDRC1", m_slidingDRC1 = 0.04,
                    "The constant offset for sliding dR");
    declareProperty("SlidingDRC2", m_slidingDRC2 = 10.*GeV,
                    "The inverse muon pt factor for sliding dR");
    declareProperty("SlidingDRMaxCone", m_slidingDRMaxCone = 0.4,
                    "Maximum allowed size of sliding dR cone");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
    declareProperty("AllowNoPV", m_allowNoPV = false,
                    "Allow events with no primary vertex");
    declareProperty("PVContainerName", m_PVContName = "PrimaryVertices",
                    "PV Container to use");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode MuJetOverlapTool::initializeDerived()
  {
    // Initialize the b-jet helper
    if(!m_bJetLabel.empty()) {
      ATH_MSG_DEBUG("Configuring btag-aware OR with btag label: " << m_bJetLabel);
      resetAccessor (m_bJetAcc, *this, m_bJetLabel);
    }

    // Initialize the matcher for the 'inner' cone.
    if(m_useGhostAssociation) {
      ATH_MSG_DEBUG("Configuring ghost association + dR matching for jet-mu OR "
                    "with inner cone size " << m_innerDR);
      m_dRMatchCone1 = std::make_unique<MuJetGhostDRMatcher>(m_innerDR, m_useRapidity);
    }
    else {
      ATH_MSG_DEBUG("Configuring mu-jet inner cone size " << m_innerDR);
      m_dRMatchCone1 = std::make_unique<DeltaRMatcher>(m_innerDR, m_useRapidity);
    }
    ATH_CHECK (m_dRMatchCone1->setObjectTypes (xAODType::ObjectType::Muon, xAODType::ObjectType::Jet));
    addSubtool(*m_dRMatchCone1);

    // Use sliding dR or flat dR for the 'outer' cone.
    if(m_useSlidingDR) {
      ATH_MSG_DEBUG("Configuring sliding outer cone for mu-jet OR with " <<
                    "constants C1 = " << m_slidingDRC1 << ", C2 = " <<
                    m_slidingDRC2 << ", MaxCone = " << m_slidingDRMaxCone);
      m_dRMatchCone2 =
        std::make_unique<SlidingDeltaRMatcher>
          (m_slidingDRC1, m_slidingDRC2, m_slidingDRMaxCone, m_useRapidity);
    }
    else {
      ATH_MSG_DEBUG("Configuring mu-jet outer cone size " << m_outerDR);
      m_dRMatchCone2 = std::make_unique<DeltaRMatcher>(m_outerDR, m_useRapidity);
    }
    ATH_CHECK (m_dRMatchCone2->setObjectTypes (xAODType::ObjectType::Muon, xAODType::ObjectType::Jet));
    addSubtool(*m_dRMatchCone2);

    // Additional config printouts
    ATH_MSG_DEBUG("Mu-jet matching config: NumJetTrk " << m_numJetTrk <<
                  " ApplyRelPt " << m_applyRelPt <<
                  " MuJetPtRatio " << m_muJetPtRatio <<
                  " MuJetTrkPtRatio " << m_muJetTrkPtRatio);
    if(!m_jetNumTrkDec.empty()) {
      ATH_MSG_DEBUG("Using user-defined JetNumTrackDecoration " << m_jetNumTrkDec);
    }
    if(!m_jetSumTrkPtDec.empty()) {
      ATH_MSG_DEBUG("Using user-defined JetSumTrackPTDecoration " << m_jetSumTrkPtDec);
    }

    ATH_MSG_DEBUG("AllowNoPV set to " << m_allowNoPV);
    resetAccessor (m_accessors->m_vtxContainerAcc, *m_accessors, m_PVContName, {.addMTDependency = true});
    if (!m_jetNumTrkDec.empty())
      resetAccessor (m_accessors->m_jetNumTrkAcc, *m_accessors, m_jetNumTrkDec);
    else
      resetAccessor (m_accessors->m_jetNumTrkPt500Acc, *m_accessors, "NumTrkPt500");
    if (!m_jetSumTrkPtDec.empty())
      resetAccessor (m_accessors->m_jetSumTrkPtAcc, *m_accessors, m_jetSumTrkPtDec);
    else
      resetAccessor (m_accessors->m_jetSumTrkPt500Acc, *m_accessors, "SumPtTrkPt500");
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode MuJetOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId eventContext) const
  {
    // Check the container types
    ATH_CHECK( checkForXAODContainer<xAOD::MuonContainer>(cont1, "First container arg is not of type MuonContainer!") );
    ATH_CHECK( checkForXAODContainer<xAOD::JetContainer>(cont2, "Second container arg is not of type JetContainer!") );

    ATH_CHECK( internalFindOverlaps(cont1, cont2, eventContext) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode MuJetOverlapTool::
  internalFindOverlaps(columnar::Particle1Range muons,
                       columnar::Particle2Range jets,
                       columnar::EventContextId eventContext) const
  {
    ATH_MSG_DEBUG("Removing overlapping muons and jets");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(muons);
    initializeDecorations(jets);

    // Retrieve the primary vertex for later reference
    auto vtxIdx = getPrimVtxIdx(eventContext);
    if(m_jetNumTrkDec.empty() && m_jetSumTrkPtDec.empty()) {
      if (!vtxIdx)
      {
        if (!m_allowNoPV) {
          ATH_MSG_ERROR("No primary vertex found! Cannot proceed with Mu-Jet OR.");
          return StatusCode::FAILURE;
        }
        else {
          ATH_MSG_DEBUG("No primary vertex found. Skipping the PV-dependent jet track info.");
        }
      }
    }

    // Remove suspicious jets that overlap with muons.
    for(const auto muon : muons){
      if(!isSurvivingObject(muon)) continue;

      for(const auto jet : jets){
        if(!isSurvivingObject(jet)) continue;

        // Don't reject user-defined b-tagged jets
        if(!m_bJetLabel.empty() && m_bJetAcc(jet)) continue;

        if (vtxIdx) {
          // Get the number of tracks and the sumPT of those tracks
          int nTrk = getNumTracks(jet, vtxIdx.value());
          float sumTrkPt = getSumTrackPt(jet, vtxIdx.value());

          // Don't reject jets with high track multiplicity and
          // high relative PT ratio
          bool highNumTrk = nTrk >= m_numJetTrk;
          bool highRelPt = false;

	        if (sumTrkPt < FLT_MIN){
	          highRelPt = (muon(acc.m_muonPtAcc)/jet(acc.m_jetPtAcc) < m_muJetPtRatio);
	        }
	        else{
	          highRelPt = (muon(acc.m_muonPtAcc)/jet(acc.m_jetPtAcc) < m_muJetPtRatio || muon(acc.m_muonPtAcc)/sumTrkPt < m_muJetTrkPtRatio);
	        }

          if(highNumTrk && (!m_applyRelPt || highRelPt)) continue;
        } // end part related to PV

        if(m_dRMatchCone1->objectsMatch(muon, jet)){
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
  std::optional<std::size_t> MuJetOverlapTool::getPrimVtxIdx(columnar::EventContextId eventContext) const
  {
    auto& acc = *m_accessors;
    // the old version of this code would allow for a missing PV
    // container, but then fail later on. So the code now requires a
    // primary vertex here.
    auto vertices = acc.m_vtxContainerAcc(eventContext);
    for(auto vtx : vertices) {
      if(vtx(acc.m_vertexTypeAcc) == xAOD::VxType::PriVtx)
        return vertices.getIndexInRange(vtx);
    }
    // No PV found. Printing a debug message only and leaving it to the
    // caller to do the right thing. There is no point checking here
    // whether we will fail or not.
    ATH_MSG_DEBUG("No primary vertex in the " << m_PVContName << " container!");
    return std::nullopt;
  }

  //---------------------------------------------------------------------------
  // Get the number of tracks in a jet
  //---------------------------------------------------------------------------
  int MuJetOverlapTool::getNumTracks(columnar::Particle2Id jet, size_t vtxIdx) const
  {
    auto& acc = *m_accessors;
    // Use the user decoration if configured
    if(!m_jetNumTrkDec.empty()) {
      return acc.m_jetNumTrkAcc(jet);
    }
    return acc.m_jetNumTrkPt500Acc(jet)[vtxIdx];
  }

  //---------------------------------------------------------------------------
  // Get the sum track pt of a jet
  //---------------------------------------------------------------------------
  float MuJetOverlapTool::getSumTrackPt(columnar::Particle2Id jet, size_t vtxIdx) const
  {
    auto& acc = *m_accessors;
    // Use the user decoration if configured
    if(!m_jetSumTrkPtDec.empty()) {
      return acc.m_jetSumTrkPtAcc(jet);
    }
    return acc.m_jetSumTrkPt500Acc(jet)[vtxIdx];
  }

} // namespace ORUtils
