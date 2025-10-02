/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthIsolationTool.cxx
// Author: Kevin Finelli (kevin.finelli@cern.ch)
// Calculate isolation at truth level for given lists of truth particles

#include "DerivationFrameworkMCTruth/TruthIsolationTool.h"
#include "TruthUtils/HepMCHelpers.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// Constructor
DerivationFramework::TruthIsolationTool::TruthIsolationTool(const std::string& t,
        const std::string& n,
        const IInterface* p ) :
  base_class(t,n,p)
{
}

// Destructor
DerivationFramework::TruthIsolationTool::~TruthIsolationTool() {
}

// Athena initialize
StatusCode DerivationFramework::TruthIsolationTool::initialize()
{

    // Initialise input keys
    ATH_CHECK(m_isoParticlesKey.initialize());
    ATH_CHECK(m_allParticlesKey.initialize());

    //sort (descsending) the cone sizes vector to optimize calculation
    m_coneSizesSort = m_coneSizes;
    std::sort(m_coneSizesSort.begin(), m_coneSizesSort.end(), [](float a, float b){return a>b;});

    // Decorations depend on the list of cone sizes
    for ( auto csize_itr : m_coneSizesSort ) { 
      std::ostringstream sizess;
      if (m_variableR) sizess << "var";
      sizess << m_isoVarNamePrefix.value() << (int)((csize_itr)*100.);
      m_isoDecorKeys.emplace_back(m_isoParticlesKey.key()+"."+sizess.str());
    }
    ATH_CHECK(m_isoDecorKeys.initialize());

    return StatusCode::SUCCESS;
}

// Function to do isolation calc, implements interface in IAugmentationTool
StatusCode DerivationFramework::TruthIsolationTool::addBranches() const
{
    // Event context 
    const EventContext& ctx = Gaudi::Hive::currentContext(); 

    // Retrieve the truth collections
    SG::ReadHandle<xAOD::TruthParticleContainer> isoTruthParticles(m_isoParticlesKey, ctx);
    if (!isoTruthParticles.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve collection with name " << m_isoParticlesKey);
      return StatusCode::FAILURE;
    }
  
    SG::ReadHandle<xAOD::TruthParticleContainer> allTruthParticles(m_allParticlesKey, ctx);
    if (!allTruthParticles.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve collection with name " << m_allParticlesKey);
      return StatusCode::FAILURE;
    }

    //get struct of helper functions
    DerivationFramework::DecayGraphHelper decayHelper;

    // Isolation is applied to selected particles only
    std::vector<const xAOD::TruthParticle*> listOfParticlesForIso;
    decayHelper.constructListOfFinalParticles(isoTruthParticles.ptr(), listOfParticlesForIso, m_listOfPIDs);

    // Vectors to store particles which will be dressed
    std::vector<const xAOD::TruthParticle*>  candidateParticlesList;

    std::vector<int> emptyList;
    //make a list of all candidate particles that could fall inside the cone of the particle of interest from listOfParticlesForIso
    decayHelper.constructListOfFinalParticles(allTruthParticles.ptr(), candidateParticlesList, emptyList, true, m_chargedOnly);

    std::vector<SG::WriteDecorHandle< xAOD::TruthParticleContainer, float > >
      decorator_iso = m_isoDecorKeys.makeHandles (ctx);

    //All isolation must filled for all Particles. 
    ///Even if this is with some dummy value 
    for ( unsigned int icone = 0; icone < m_coneSizesSort.size(); ++icone ) {
      for (const auto* part : *isoTruthParticles) {
        decorator_iso.at(icone)(*part) = -1;
      }
    }

    // Standard particle loop over final state particles of interest
    for (const auto* part : listOfParticlesForIso) {
      std::vector<float> isolationsCalcs(m_coneSizesSort.size(), 0.0);
      calcIsos(part, candidateParticlesList, isolationsCalcs);
      for ( unsigned int icone = 0; icone < m_coneSizesSort.size(); ++icone ) {
        decorator_iso.at(icone)(*part) = isolationsCalcs.at(icone);
      }
    }

    return StatusCode::SUCCESS;
}

void DerivationFramework::TruthIsolationTool::calcIsos(const xAOD::TruthParticle* particle,
        const std::vector<const xAOD::TruthParticle*> &candidateParticlesList,
        std::vector<float> &isoCalcs) const
{
    //given a bare TruthParticle particle, calculate the isolation(s) using the
    //particles in candidateParticlesList, filling isoCalcs in order corresponding
    //to the sorted cone sizes

    float part_eta = particle->eta();
    float part_phi = particle->phi();
    for (const auto* cand_part : candidateParticlesList) {
      if (find(m_excludeFromCone.begin(), m_excludeFromCone.end(), cand_part->pdgId()) != m_excludeFromCone.end()) {
        //skip if we find a particle in the exclude list
        continue;
      }
      if (!m_includeNonInteracting && !MC::isInteracting(cand_part->pdgId())){
        // Do not include non-interacting particles, and this particle is non-interacting
        continue;
      }
      if (!HepMC::is_same_particle(cand_part,particle)) {
        //iteration over sorted cone sizes
        for ( unsigned int icone = 0; icone < m_coneSizesSort.size(); ++icone ) {
          const float dr2 = calculateDeltaR2(cand_part, part_eta, part_phi);
          const float coneSize = m_coneSizesSort.at(icone);
          if (dr2 < coneSize*coneSize &&
              (!m_variableR || dr2*particle->pt()*particle->pt() < 100000000.)) {
            //sum the transverse momenta
            isoCalcs.at(icone) = isoCalcs.at(icone) + cand_part->pt();
          } else {
            //break out of the loop safely since the cone sizes are sorted descending
            break;
          }
        }
      }
    }
    return;
}

float DerivationFramework::TruthIsolationTool::calculateDeltaR2(const xAOD::IParticle *p1, float eta2, float phi2) 
{
  //calculate dR^2 this way to hopefully do fewer sqrt and TVector3::Pseudorapidity calls
  float phi1 = p1->phi();
  float eta1 = p1->eta();
  float deltaPhi = std::abs(phi1-phi2);
  if (deltaPhi>M_PI) deltaPhi = 2.0*M_PI - deltaPhi;
  float deltaPhiSq = deltaPhi * deltaPhi;
  float deltaEtaSq = (eta1-eta2)*(eta1-eta2);
  return deltaPhiSq+deltaEtaSq;
}
