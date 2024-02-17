/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file egammaD3PDAnalysis/src/egammaTruthAlg.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2014
 * @brief Select egtruth particles.
 */


#include "egammaTruthAlg.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkCaloExtension/CaloExtension.h"
#include "TrkCaloExtension/CaloExtensionHelpers.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

using Gaudi::Units::GeV;
using Gaudi::Units::MeV;






namespace D3PD {


/**
 * @brief Standard Gaudi algorithm constructor.
 * @param name The algorithm name.
 * @param svcloc The service locator.
 */
egammaTruthAlg::egammaTruthAlg (const std::string& name,
                                ISvcLocator* svcloc)
  : AthReentrantAlgorithm (name, svcloc)
{
  declareProperty ("AuxPrefix", m_auxPrefix,
                   "Prefix to add to aux data items.");

  declareProperty ("ElectronPtMin", m_electronPtMin = 2*GeV,
                   "Minimum pt for electrons.");
  declareProperty ("PhotonPtMin",   m_photonPtMin = 2*GeV,
                   "Minimum pt for photons.");
  declareProperty ("EtaMax",    m_etaMax = 2.5,
                   "Maximum eta.");
  declareProperty ("IsoCone",    m_isoCone = 0.2,
                   "Isolation cone width.");
  declareProperty ("PhotonEtIsoMax", m_photonEtIsoMax = 2*MeV,
                   "Maximum isolation cone energy allowed to keep a photon.");
}


/**
 * @brief Standard Gaudi @c initialize method.
 */
StatusCode egammaTruthAlg::initialize()
{
  CHECK( AthReentrantAlgorithm::initialize() );
  CHECK( m_exten.retrieve() );
  CHECK( m_inputKey.initialize() );
  CHECK( m_outputKey.initialize() );

  return StatusCode::SUCCESS;
}


/**
 * @brief Standard Gaudi @c execute method.
 */
StatusCode egammaTruthAlg::execute (const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::TruthParticleContainer> pin (m_inputKey, ctx);

  auto pout = std::make_unique<xAOD::TruthParticleContainer>();
  auto pout_aux = std::make_unique<xAOD::TruthParticleAuxContainer>();
  pout->setStore (pout_aux.get());

#define DECOR(TYPE,N) xAOD::TruthParticle::Decorator<TYPE> N (m_auxPrefix + #N)
  DECOR(float,        etaCalo);
  DECOR(float,        phiCalo);
  DECOR(float,        depthCalo);
  DECOR(float,        Etcone20);
#undef DECOR

  for (const xAOD::TruthParticle* tp : *pin) {
    float iso = -999;
    if (isAccepted (*tp, *pin, iso)) {
      pout->push_back (std::make_unique<xAOD::TruthParticle>());
      *pout->back() = *tp;

      CHECK( findImpact (*tp,
                         etaCalo(*pout->back()),
                         phiCalo(*pout->back()),
                         depthCalo(*pout->back())) );
      Etcone20(*pout->back()) = iso;
    }
  }

  SG::WriteHandle<xAOD::TruthParticleContainer> output (m_outputKey, ctx);
  CHECK( output.record (std::move(pout), std::move(pout_aux)) );

  return StatusCode::SUCCESS;
}


/**
 * @brief Test to see if we accept a particle.
 * @param tp The particle to test.
 * @param cont The container of particles.
 * @param iso[out] The isolation energy for this particle.
 */
bool egammaTruthAlg::isAccepted (const xAOD::TruthParticle& tp,
                                 const xAOD::TruthParticleContainer& cont,
                                 float& iso) const
{
  iso = -999;

  int id = tp.pdgId();
  int aid = abs(id);
  int barcode = tp.barcode();

  if (aid != abs(MC::ELECTRON) && !MC::isPhoton(aid)) return false;
  if (aid == abs(MC::ELECTRON) && tp.pt() < m_electronPtMin) return false;
  if (MC::isPhoton(aid) && tp.pt() < m_photonPtMin) return false;


  if (fabs(tp.eta()) > m_etaMax) return false;
  if ( (!((HepMC::is_simulation_particle(&tp) || MC::isZeroEnergyPhoton(&tp)) || ( MC::isStable(&tp) && MC::isSpecialNonInteracting(&tp)))) && MC::isStableOrSimDecayed(&tp)) return false;

  // Remove electrons/gammas decaying into themselves
  if( tp.hasDecayVtx() ) {
    const xAOD::TruthVertex* v = tp.decayVtx();
    size_t sz = v->nOutgoingParticles();
    for (size_t i = 0; i < sz; i++) {
      const xAOD::TruthParticle* child = v->outgoingParticle(i);
      if( child && child->pdgId()==id && child->barcode()!=barcode
          && (HepMC::generations(child) == 0))
      {
        return false;
      }
    }
  } // end decays into themselves

  // Isolation selection for photons.
  iso = computeIso (tp, cont);
  if (aid == abs(MC::PHOTON)) {
    if (iso > m_photonEtIsoMax)
      return false;
  }

  return true;
}


/**
 * @brief Compute isolation around a particle.
 * @param tp The particle for which we want isolation.
 * @param cont The container of particles.
 */
float egammaTruthAlg::computeIso (const xAOD::TruthParticle& tp,
                                  const xAOD::TruthParticleContainer& cont) const
{
  TLorentzVector sum;
  for (const xAOD::TruthParticle* p : cont) {
    if (p == &tp || p->barcode() == tp.barcode()) continue;
    if (!((HepMC::is_simulation_particle(p) || MC::isZeroEnergyPhoton(p) || ( MC::isStable(p) && MC::isSpecialNonInteracting(p))) && MC::isStableOrSimDecayed(p))) continue;
    if (tp.p4().DeltaR (p->p4()) < m_isoCone)
      sum += p->p4();
  }

  return sum.Pt();
}


/**
 * @brief Find the impact of a particle in the calorimeter.
 * @param p The particle to analyze.
 * @param etaCalo[out] Eta of the particle's impact with the calorimeter.
 * @param phiCalo[out] Phi of the particle's impact with the calorimeter.
 * @param depthCalo[out] Depth of the particle's impact with the calorimeter
 *                       (r for barrel and abs(z) for endcap).
 */
StatusCode egammaTruthAlg::findImpact (const xAOD::TruthParticle& tp,
                                       float& etaCalo,
                                       float& phiCalo,
                                       float& depthCalo) const
{
  etaCalo = -999;
  phiCalo = -999;
  depthCalo = -999;

  std::unique_ptr<Trk::CaloExtension> extension =
    m_exten->caloExtension(Gaudi::Hive::currentContext(), tp);
  if (!extension) {
    REPORT_MESSAGE (MSG::ERROR) <<  "Extension to calorimeter failed";
    return StatusCode::FAILURE;
  }

  CaloExtensionHelpers::EntryExitPerLayerVector lvec;
  CaloExtensionHelpers::entryExitPerLayerVector (*extension, lvec);
  for (const auto& [sampling, entry, exit] : lvec) {
    if (sampling == CaloSampling::EMB2) {
      etaCalo = entry.eta();
      phiCalo = entry.phi();
      depthCalo = entry.perp();
      break;
    }
    else if (sampling == CaloSampling::EME2) {
      etaCalo = entry.eta();
      phiCalo = entry.phi();
      depthCalo = std::abs(entry.z());
    }
  }

  return StatusCode::SUCCESS;
}


} // namespace D3PD
