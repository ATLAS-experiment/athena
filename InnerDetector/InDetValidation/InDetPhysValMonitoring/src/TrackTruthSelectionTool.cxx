/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// InDetPhysValMonitoring includes
#include "TrackTruthSelectionTool.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthVertex.h"
#include <cmath> // std::fabs


TrackTruthSelectionTool::TrackTruthSelectionTool(const std::string& name) :
  asg::AsgTool(name) {
  declareInterface<IAsgSelectionTool>(this);
}

TrackTruthSelectionTool::~TrackTruthSelectionTool() = default;

StatusCode
TrackTruthSelectionTool::initialize() {
  if (asg::AsgTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  // Define cut names and descriptions
  m_cuts.clear();
  if (m_maxEta > -1) {
    m_cuts.emplace_back("eta", "Cut on (absolute) particle eta");
  }
  if (m_minPt > -1) {
    m_cuts.emplace_back("min_pt", "Cut on minimum particle pT");
  }
  if (m_maxPt > -1) {
    m_cuts.emplace_back("max_pt", "Cut on maximum particle pT");
  }

  if (m_requireOnlyPrimary) {
    m_cuts.emplace_back("OnlyPrimary", "Cut on origin");
  }

  if (m_requireCharged) {
    m_cuts.emplace_back("charged", "Require charged particle");
  }
  if (m_requireStable) {
    m_cuts.emplace_back("stable", "Particle stable");
  }

  if (m_maxProdVertRadius > 0.) {
    m_cuts.emplace_back("decay_before_pixel", "Decays before first pixel layer");
  }

  if (m_pdgId > -1) {
    m_cuts.emplace_back("pdgId", "Pdg Id cut"); // 3-18-16 normally enabled, disabled for testing
  }
  // Add cuts to the AcceptInfo
  for (const auto& cut : m_cuts) {
    if (m_accept.addCut(cut.first, cut.second) < 0) {
      ATH_MSG_ERROR("Failed to add cut " << cut.first << " because the AcceptInfo object is full.");
      return StatusCode::FAILURE;
    }
  }

  // Initialise counters
  m_numTruthPassedCuts.resize(m_accept.getNCuts(), 0);

  return StatusCode::SUCCESS;
}

const asg::AcceptInfo&
TrackTruthSelectionTool::getAcceptInfo( ) const {
  return m_accept;
}

asg::AcceptData
TrackTruthSelectionTool::accept(const xAOD::IParticle* p) const// Is this perhaps supposed to be xAOD::TruthParticle?
{
  // Check if this is a track:
  if (!p) {
    ATH_MSG_ERROR("accept(...) Function received a null pointer");
    return asg::AcceptData (&m_accept);
  }
  if (p->type() != xAOD::Type::TruthParticle) {
    ATH_MSG_ERROR("accept(...) Function received a non-TruthParticle");
    return asg::AcceptData (&m_accept);
  }

  // Cast it to a track (we have already checked its type so we do not have to dynamic_cast):
  const xAOD::TruthParticle* truth = static_cast< const xAOD::TruthParticle* >(p);

  // Let the specific function do the work:
  return accept(truth);
}

asg::AcceptData
TrackTruthSelectionTool::accept(const xAOD::TruthParticle* p) const {
  asg::AcceptData acceptData (&m_accept);

  // Check cuts
  if (m_maxEta > -1) {
    acceptData.setCutResult("eta", (p->pt() > 1e-7 ? (std::fabs(p->eta()) < m_maxEta) : false));
  }
  if (m_minPt > -1) {
    acceptData.setCutResult("min_pt", (p->pt() > m_minPt));
  }
  if (m_maxPt > -1) {
    acceptData.setCutResult("max_pt", (p->pt() < m_maxPt));
  }
  if (m_requireOnlyPrimary) {
    acceptData.setCutResult("OnlyPrimary", (!HepMC::is_simulation_particle(p)));
  }

  if (m_requireCharged) {
    acceptData.setCutResult("charged", (not (p->isNeutral())));
  }
  if (m_requireStable) {
    acceptData.setCutResult("stable", (MC::isStable(p)));
  }
  if (m_maxProdVertRadius > 0.) {
    acceptData.setCutResult("decay_before_pixel", (!p->hasProdVtx() || p->prodVtx()->perp() < m_maxProdVertRadius));
  }
  if (m_pdgId > -1) {
    acceptData.setCutResult("pdgId", (std::fabs(p->pdgId()) == m_pdgId));// 3-18-16 normally on, disabled for testing
  }
  std::lock_guard<std::mutex> lock{m_mutex}; // To guard m_numTruthPassedCuts
  // Book keep cuts
  for (const auto& cut : m_cuts) {
    unsigned int pos = acceptData.getCutPosition(cut.first);
    if (acceptData.getCutResult(pos)) {
      m_numTruthPassedCuts[pos]++;
    }
  }
  m_numTruthProcessed++;
  if (acceptData) {
    m_numTruthPassed++;
  }

  return acceptData;
}

StatusCode
TrackTruthSelectionTool::finalize() {
  ATH_MSG_INFO("Finalizing " << name() << "...");

  if (m_numTruthProcessed == 0) {
    ATH_MSG_INFO("No tracks processed in selection tool.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_INFO(m_numTruthPassed << " / " << m_numTruthProcessed << " = "
                                << m_numTruthPassed * 100. / m_numTruthProcessed << "% passed all cuts.");
  for (const auto& cut : m_cuts) {
    ULong64_t numPassed = m_numTruthPassedCuts.at(m_accept.getCutPosition(cut.first));
    ATH_MSG_INFO(numPassed << " = " << numPassed * 100. / m_numTruthProcessed << "% passed "
                           << cut.first << " cut.");
  }

  return StatusCode::SUCCESS;
}
