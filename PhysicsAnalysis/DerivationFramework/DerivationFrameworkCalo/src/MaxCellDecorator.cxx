/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MaxCellDecorator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkCalo/MaxCellDecorator.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

#include <string>
#include <vector>
namespace {}

// Constructor
DerivationFramework::MaxCellDecorator::MaxCellDecorator(const std::string& t,
                                                        const std::string& n,
                                                        const IInterface* p)
  : base_class(t, n, p)
{
}

// Destructor
DerivationFramework::MaxCellDecorator::~MaxCellDecorator() = default;

// Athena initialize and finalize
StatusCode
DerivationFramework::MaxCellDecorator::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  ATH_CHECK(m_cablingKey.initialize());

  if (!m_SGKey_electrons.key().empty()) {
    const std::string key = m_SGKey_electrons.key();
    ATH_MSG_INFO("Using " << key << " for electrons");
    ATH_CHECK(m_SGKey_electrons.initialize());

    // setup vector of decorators
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_time");
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_energy");
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_gain");
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_onlId");
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_x");
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_y");
    m_SGKey_electrons_decorations.emplace_back(key + ".maxEcell_z");

    if (!m_SGKey_egammaClusters.key().empty()) {
      ATH_MSG_INFO("Using " << m_SGKey_egammaClusters.key() << " to try to match a cluster to the LRT egamma cluster");
      ATH_CHECK(m_SGKey_egammaClusters.initialize());
      m_SGKey_electrons_decorations.emplace_back(key + ".dR");
    }

    ATH_CHECK(m_SGKey_electrons_decorations.initialize());
  }

  if (!m_SGKey_photons.key().empty()) {
    const std::string key = m_SGKey_photons.key();
    ATH_MSG_INFO("Using " << key << " for photons");
    ATH_CHECK(m_SGKey_photons.initialize());

    // setup vector of decorators
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_time");
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_energy");
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_gain");
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_onlId");
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_x");
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_y");
    m_SGKey_photons_decorations.emplace_back(key + ".maxEcell_z");
    ATH_CHECK(m_SGKey_photons_decorations.initialize());
  }

  if (!m_SGKey_taus.key().empty()) {
    const std::string key = m_SGKey_taus.key();
    ATH_MSG_INFO("Using " << key << " for taus");
    ATH_CHECK(m_SGKey_taus.initialize());

    // setup vector of decorators
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_time");
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_energy");
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_gain");
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_onlId");
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_x");
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_y");
    m_SGKey_taus_decorations.emplace_back(key + ".maxEcell_z");
    ATH_CHECK(m_SGKey_taus_decorations.initialize());
  }

  if (!m_SGKey_jets.key().empty()) {
    const std::string key = m_SGKey_jets.key();
    ATH_MSG_INFO("Using " << key << " for jets");
    ATH_CHECK(m_SGKey_jets.initialize());

    // setup vector of decorators
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_time");
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_energy");
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_gain");
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_onlId");
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_x");
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_y");
    m_SGKey_jets_decorations.emplace_back(key + ".maxEcell_z");
    ATH_CHECK(m_SGKey_jets_decorations.initialize());
  }

  return StatusCode::SUCCESS;
}

StatusCode
DerivationFramework::MaxCellDecorator::finalize()
{
  return StatusCode::SUCCESS;
}

// The decoration itself
StatusCode
DerivationFramework::MaxCellDecorator::addBranches(const EventContext& ctx) const
{

  if (!m_SGKey_photons.key().empty()) {
    // Retrieve photon container
    SG::ReadHandle<xAOD::EgammaContainer> photonContainer(m_SGKey_photons, ctx);
    // setup vector of decorators
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationPh0(
      m_SGKey_photons_decorations[0], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationPh1(
      m_SGKey_photons_decorations[1], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, int> decorationPh2(
      m_SGKey_photons_decorations[2], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, uint64_t> decorationPh3(
      m_SGKey_photons_decorations[3], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationPh4(
      m_SGKey_photons_decorations[4], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationPh5(
      m_SGKey_photons_decorations[5], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationPh6(
      m_SGKey_photons_decorations[6], ctx);

    const xAOD::EgammaContainer* importedPhotons = photonContainer.ptr();
    for (const auto* egamma : *importedPhotons) {
      const xAOD::CaloCluster *cluster = egamma->caloCluster();
      DerivationFramework::MaxCellDecorator::calculation res =
        decorateObject(cluster, ctx);
      decorationPh0(*egamma) = res.maxEcell_time;
      decorationPh1(*egamma) = res.maxEcell_energy;
      decorationPh2(*egamma) = res.maxEcell_gain;
      decorationPh3(*egamma) = res.maxEcell_onlId;
      decorationPh4(*egamma) = res.maxEcell_x;
      decorationPh5(*egamma) = res.maxEcell_y;
      decorationPh6(*egamma) = res.maxEcell_z;
    }
  }

  if (!m_SGKey_electrons.key().empty()) {
    // Retrieve electron container
    SG::ReadHandle<xAOD::EgammaContainer> electronContainer(m_SGKey_electrons,
                                                            ctx);

    //
    std::optional<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>  odecorationEl7;
    const xAOD::CaloClusterContainer* egClContainer(nullptr);
    if (!m_SGKey_egammaClusters.key().empty()) {
      SG::ReadHandle<xAOD::CaloClusterContainer> egClContainerRH(
        m_SGKey_egammaClusters, ctx);
      egClContainer = egClContainerRH.ptr();
      odecorationEl7.emplace(m_SGKey_electrons_decorations[7], ctx);
    }

    // setup vector of decorators
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationEl0(
      m_SGKey_electrons_decorations[0], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationEl1(
      m_SGKey_electrons_decorations[1], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, int> decorationEl2(
      m_SGKey_electrons_decorations[2], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, uint64_t> decorationEl3(
      m_SGKey_electrons_decorations[3], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationEl4(
      m_SGKey_electrons_decorations[4], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationEl5(
      m_SGKey_electrons_decorations[5], ctx);
    SG::WriteDecorHandle<xAOD::EgammaContainer, float> decorationEl6(
      m_SGKey_electrons_decorations[6], ctx);

    const xAOD::EgammaContainer* importedElectrons = electronContainer.ptr();
    for (const auto* egamma : *importedElectrons) {
      const xAOD::CaloCluster *cluster = egamma->caloCluster();
      if (!m_SGKey_egammaClusters.key().empty()) {
        double dRMin = 9e9;
        const xAOD::CaloCluster *matchedCluster(nullptr);
        for (const auto *clus : *egClContainer) {
          double dR = clus->p4().DeltaR(cluster->p4());
          if (dR < dRMin && dR < m_dRLRTegClusegClusMax) {
            dRMin = dR;
            matchedCluster = clus;
          }
        }
        cluster = matchedCluster;
        odecorationEl7.value()(*egamma) = dRMin;
      }
      DerivationFramework::MaxCellDecorator::calculation res =
        decorateObject(cluster, ctx);
      decorationEl0(*egamma) = res.maxEcell_time;
      decorationEl1(*egamma) = res.maxEcell_energy;
      decorationEl2(*egamma) = res.maxEcell_gain;
      decorationEl3(*egamma) = res.maxEcell_onlId;
      decorationEl4(*egamma) = res.maxEcell_x;
      decorationEl5(*egamma) = res.maxEcell_y;
      decorationEl6(*egamma) = res.maxEcell_z;
    }
  }
	
  if (!m_SGKey_taus.key().empty()) {
    // Retrieve tau container
    SG::ReadHandle<xAOD::TauJetContainer> tauJetContainer(m_SGKey_taus, ctx);
    // setup vector of decorators
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> decorationTau0(
      m_SGKey_taus_decorations[0], ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> decorationTau1(
      m_SGKey_taus_decorations[1], ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> decorationTau2(
      m_SGKey_taus_decorations[2], ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, uint64_t> decorationTau3(
      m_SGKey_taus_decorations[3], ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> decorationTau4(
      m_SGKey_taus_decorations[4], ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> decorationTau5(
      m_SGKey_taus_decorations[5], ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> decorationTau6(
      m_SGKey_taus_decorations[6], ctx);
    
    const xAOD::TauJetContainer* importedTaus = tauJetContainer.ptr();
    for (const auto* tau : *importedTaus) {
      DerivationFramework::MaxCellDecorator::calculation res;
      res.maxEcell_energy = -9999.;
      for (size_t i = 0;i<tau->nClusters();++i) {
        // get particle
        const xAOD::IParticle* part = tau->cluster(i);
        if ( not part ) {
          ATH_MSG_WARNING("Tau particle link invalid");
          continue;
        }
        
        const xAOD::CaloCluster* cluster=dynamic_cast<const xAOD::CaloCluster*> (part);
        if ( not cluster ) {
          ATH_MSG_WARNING("Tau cluster link invalid");
          continue;
        }
        
        DerivationFramework::MaxCellDecorator::calculation resCand =
          decorateObject(cluster, ctx);
        if (resCand.maxEcell_energy > res.maxEcell_energy) {
          res = resCand;
        }
      }
      
      decorationTau0(*tau) = res.maxEcell_time;
      decorationTau1(*tau) = res.maxEcell_energy;
      decorationTau2(*tau) = res.maxEcell_gain;
      decorationTau3(*tau) = res.maxEcell_onlId;
      decorationTau4(*tau) = res.maxEcell_x;
      decorationTau5(*tau) = res.maxEcell_y;
      decorationTau6(*tau) = res.maxEcell_z;
    }
  }
	
  if (!m_SGKey_jets.key().empty()) {
    // Retrieve jet container
    SG::ReadHandle<xAOD::JetContainer> jetContainer(m_SGKey_jets, ctx);
    // setup vector of decorators
    SG::WriteDecorHandle<xAOD::JetContainer, float> decorationJet0(
      m_SGKey_jets_decorations[0], ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decorationJet1(
      m_SGKey_jets_decorations[1], ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, int> decorationJet2(
      m_SGKey_jets_decorations[2], ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, uint64_t> decorationJet3(
      m_SGKey_jets_decorations[3], ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decorationJet4(
      m_SGKey_jets_decorations[4], ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decorationJet5(
      m_SGKey_jets_decorations[5], ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decorationJet6(
      m_SGKey_jets_decorations[6], ctx);

    const xAOD::JetContainer* importedJets = jetContainer.ptr();
    for (const auto* jet : *importedJets) {
      if (jet->numConstituents() == 0) continue;

      DerivationFramework::MaxCellDecorator::calculation res;
      res.maxEcell_energy = -9999.;
      std::vector<const xAOD::CaloCluster*> clusterList;
      clusterList.clear();

      xAOD::Type::ObjectType ctype = jet->rawConstituent( 0 )->type();

      if (ctype  == xAOD::Type::FlowElement) {
        // Particle Flow jets
        for (size_t i=0;i<jet->numConstituents();++i) {
          if(jet->rawConstituent(i)->type() != xAOD::Type::FlowElement) {
            ATH_MSG_WARNING("Tried to call fillEperSamplingFE with a jet constituent that is not a FlowElement!");
            continue;
          }

          const xAOD::FlowElement* constit = static_cast<const xAOD::FlowElement*>(jet->rawConstituent(i));
          if (constit) {
            const SG::AuxElement::ConstAccessor< ElementLink<xAOD::IParticleContainer> > originalObject("originalObjectLink");
            auto originalFE = dynamic_cast<const xAOD::FlowElement*>(*originalObject(*constit));
            if(originalFE && !originalFE->isCharged()){
              const xAOD::CaloCluster* cluster = dynamic_cast<const xAOD::CaloCluster*>(originalFE->otherObject(0));
              if (cluster) {
                clusterList.push_back(cluster);
              }
            }
          }
        }
      } else if (ctype  == xAOD::Type::CaloCluster) {
        // Topo jets
        for (size_t i=0;i<jet->numConstituents();++i) {
          if(jet->rawConstituent(i)->type() != xAOD::Type::CaloCluster) {
            ATH_MSG_WARNING("Tried to call fillEperSamplingCluster with a jet constituent that is not a cluster!");
            continue;
          }

          const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>(jet->rawConstituent(i));
          if (cluster) {
            clusterList.push_back(cluster);
          }
        }
      } else {
        // PFlow (old)
        for (size_t i=0;i<jet->numConstituents();++i) {
          if(jet->rawConstituent(i)->type() != xAOD::Type::ParticleFlow) {
            continue;
          }
          const xAOD::PFO* iPFO = static_cast<const xAOD::PFO*>(jet->rawConstituent(i));
          if ( iPFO ) {
            for (unsigned int cidx=0;cidx<iPFO->nCaloCluster();++cidx) {
              if ( iPFO->cluster(cidx) ) {
                clusterList.push_back(iPFO->cluster(cidx));
              }
            }
          }
        }
      }
      
      for (auto cluster : clusterList) {
        DerivationFramework::MaxCellDecorator::calculation resCand =
          decorateObject(cluster, ctx);
        if (resCand.maxEcell_energy > res.maxEcell_energy) {
          res = resCand;
        }
      }
                    
      decorationJet0(*jet) = res.maxEcell_time;
      decorationJet1(*jet) = res.maxEcell_energy;
      decorationJet2(*jet) = res.maxEcell_gain;
      decorationJet3(*jet) = res.maxEcell_onlId;
      decorationJet4(*jet) = res.maxEcell_x;
      decorationJet5(*jet) = res.maxEcell_y;
      decorationJet6(*jet) = res.maxEcell_z;
    }
  }

  return StatusCode::SUCCESS;
}

DerivationFramework::MaxCellDecorator::calculation
DerivationFramework::MaxCellDecorator::decorateObject(
  const xAOD::CaloCluster* cluster,
  const EventContext& ctx) const
{

  DerivationFramework::MaxCellDecorator::calculation result;

  if (cluster) {
    if (!cluster->getCellLinks()) {
      ATH_MSG_DEBUG("CellLinks not found");
      return result;
    }

    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{ m_cablingKey, ctx };
    const LArOnOffIdMapping* cabling{ *cablingHdl };
    if (!cabling) {
      ATH_MSG_ERROR("Do not have mapping object " << m_cablingKey.key());
      return result;
    }

    float emax = -9999.;

    const CaloCell* cell_maxE = nullptr;
    for (const CaloCell* cell : *cluster) {
      int sampling = cell->caloDDE()->getSampling();
      if (sampling == CaloCell_ID::EMB2 || sampling == CaloCell_ID::EME2) {
        if ((cell->provenance() & 0x2000)) {
          if (cell->energy() > emax) {
            emax = cell->energy();
            cell_maxE = cell;
          }
        }
      }
    }

    if (cell_maxE) {
      const CaloDetDescrElement* caloDDEl = cell_maxE->caloDDE();
      result.maxEcell_time = cell_maxE->time();
      result.maxEcell_energy = cell_maxE->energy();
      result.maxEcell_gain = (int)cell_maxE->gain();
      result.maxEcell_onlId =
        (uint64_t)(cabling->createSignalChannelID(caloDDEl->identify()))
          .get_compact();
      result.maxEcell_x = caloDDEl->x();
      result.maxEcell_y = caloDDEl->y();
      result.maxEcell_z = caloDDEl->z();
    }
  }
  return result;
}
