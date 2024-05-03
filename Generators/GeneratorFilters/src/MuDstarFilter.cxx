/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// -------------------------------------------------------------
// File: GeneratorFilters/MuDstarFilter.cxx
// Description:
//
//   Allows the user to search for (mu D*) combinations
//   with both same and opposite charges.
//   For D*+-, the decay D*+ -> D0 pi_s+ is selected
//   with D0 in the nominal decay mode, D0 -> K- pi+ (Br = 3.947%),
//   and, if "D0Kpi_only=false", 14 main other decays to 2 or 3 particles (except nu_e and nu_mu)
//   in case they can immitate the nominal decay:
//   D0 -> K-mu+nu, K-e+nu, pi-mu+nu, pi-e+nu,
//         K-mu+nu pi0, K-e+nu pi0, pi-mu+nu pi0, pi-e+nu pi0,
//         pi-pi+, K-K+, K-pi+pi0, K-K+pi0, pi-pi+pi0, K-pi+gamma
//         Doubly Cabbibo supressed modes are also considered
//   Requirements for non-nominal decay modes:
//         D*+ -> ("K-" "pi+") pi_s+ (+c.c.) charges
//         mKpiMin < m("K" "pi") < mKpiMax
//         m("K" "pi" pi_s) - m("K" "pi") < delta_m_Max
//
// AuthorList:
//   L K Gladilin (gladilin@mail.cern.ch)  March 2023
// Versions:
// 1.0.0; 2023.04.01 (no jokes) - baseline version
// 1.0.1; 2023.04.14 - allow photons for "m_D0Kpi_only" for a case of Photos
// 1.0.2; 2023.04.17 - skip history Photos lines (status=3)
//         
// Header for this module:-

#include "GeneratorFilters/MuDstarFilter.h"

// Framework Related Headers:-
#include "GaudiKernel/MsgStream.h"


// Other classes used by this class:-
#include <math.h>
#include <limits>

#include "CLHEP/Vector/LorentzVector.h"

#include "TLorentzVector.h"

#include "TruthUtils/HepMCHelpers.h"

//--------------------------------------------------------------------------
MuDstarFilter::MuDstarFilter(const std::string & name,
  ISvcLocator * pSvcLocator): GenFilter(name, pSvcLocator) {
  //--------------------------------------------------------------------------    
  declareProperty("PtMinMuon", m_PtMinMuon = 2500.);
  declareProperty("PtMaxMuon", m_PtMaxMuon = 1e9);
  declareProperty("EtaRangeMuon", m_EtaRangeMuon = 2.7);
  //
  declareProperty("PtMinDstar", m_PtMinDstar = 4500.);
  declareProperty("PtMaxDstar", m_PtMaxDstar = 1e9);
  declareProperty("EtaRangeDstar", m_EtaRangeDstar = 2.7);
  declareProperty("RxyMinDstar", m_RxyMinDstar = -1e9);
  //
  declareProperty("PtMinPis", m_PtMinPis = 450.);
  declareProperty("PtMaxPis", m_PtMaxPis = 1e9);
  declareProperty("EtaRangePis", m_EtaRangePis = 2.7);
  //
  declareProperty("PtMinKpi", m_PtMinKpi = 900.);
  declareProperty("PtMaxKpi", m_PtMaxKpi = 1e9);
  declareProperty("EtaRangeKpi", m_EtaRangeKpi = 2.7);
  //
  declareProperty("D0Kpi_only", m_D0Kpi_only = false);
  //
  declareProperty("mKpiMin", m_mKpiMin = 1665.);
  declareProperty("mKpiMax", m_mKpiMax = 2065.);
  //
  declareProperty("delta_m_Max", m_delta_m_Max = 220.);
  //
  declareProperty("DstarMu_m_Max", m_DstarMu_m_Max = 12000.);
}

//--------------------------------------------------------------------------
MuDstarFilter::~MuDstarFilter() {
  //--------------------------------------------------------------------------

}

//---------------------------------------------------------------------------
StatusCode MuDstarFilter::filterInitialize() {
  //---------------------------------------------------------------------------
 ATH_MSG_INFO( "MuDstarFilter v1.02 INITIALISING "  );
 
 ATH_MSG_INFO( "PtMinMuon = " << m_PtMinMuon );
 ATH_MSG_INFO( "PtMaxMuon = " << m_PtMaxMuon );
 ATH_MSG_INFO( "EtaRangeMuon = " << m_EtaRangeMuon );
    
 ATH_MSG_INFO( "PtMinDstar = " << m_PtMinDstar );
 ATH_MSG_INFO( "PtMaxDstar = " << m_PtMaxDstar );
 ATH_MSG_INFO( "EtaRangeDstar = " << m_EtaRangeDstar );

 ATH_MSG_INFO( "RxyMinDstar = " << m_RxyMinDstar );
 
 ATH_MSG_INFO( "PtMinPis = " << m_PtMinPis );
 ATH_MSG_INFO( "PtMaxPis = " << m_PtMaxPis );
 ATH_MSG_INFO( "EtaRangePis = " << m_EtaRangePis );
 
 ATH_MSG_INFO( "D0Kpi_only = " << m_D0Kpi_only );
 ATH_MSG_INFO( "PtMinKpi = " << m_PtMinKpi );   
 ATH_MSG_INFO( "PtMaxKpi = " << m_PtMaxKpi );
 ATH_MSG_INFO( "EtaRangeKpi = " << m_EtaRangeKpi );
 
 ATH_MSG_INFO( "mKpiMin = " << m_mKpiMin );
 ATH_MSG_INFO( "mKpiMax = " << m_mKpiMax );
 
 ATH_MSG_INFO( "delta_m_Max = " << m_delta_m_Max );  
    
 ATH_MSG_INFO( "DstarMu_m_Max = " << m_DstarMu_m_Max );

 return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode MuDstarFilter::filterFinalize() {
  //---------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode MuDstarFilter::filterEvent() {
  //---------------------------------------------------------------------------

  // Loop over all events in McEventCollection 
  McEventCollection::const_iterator itr;

  ATH_MSG_DEBUG(" MuDstarFilter filtering ");

  for (itr = events_const() -> begin(); itr != events_const() -> end(); ++itr) {

    double primx = 0.;
    double primy = 0.;

    #ifdef HEPMC3
    HepMC::ConstGenVertexPtr vprim = * (( * itr) -> vertices().begin());
    #else
    HepMC::GenVertexPtr vprim = * (( * itr) -> vertices_begin());
    #endif

    primx = vprim -> position().x();
    primy = vprim -> position().y();

    ATH_MSG_DEBUG("MuDstarFilter: PV x, y = " << primx << " , " << primy);

    int NumMuons = 0;
    int NumDstars = 0;

    std::vector < HepMC::ConstGenParticlePtr > Muons;

    // Loop over all particles in the event
    const HepMC::GenEvent * genEvt = ( * itr);
    for (const auto & pitr: * genEvt) {

      if (pitr -> status() == 3) continue; // photos history line

      // muons
      if (MC::isMuon(pitr)) {
        if ((pitr -> momentum().perp() >= m_PtMinMuon) &&
          (pitr -> momentum().perp() < m_PtMaxMuon) &&
          (std::abs(pitr -> momentum().eta()) < m_EtaRangeMuon)) {
          ++NumMuons;
          Muons.push_back(pitr);
        }
      }
    }

    ATH_MSG_DEBUG( "MuDstarFilter: NumMuons = " << NumMuons);

    if (NumMuons == 0) break;
    for (const auto & pitr: * genEvt) {

      if (pitr -> status() == 3) continue; // photos history line

      // Dstars
      if (std::abs(pitr -> pdg_id()) == MC::DSTAR) {
        if ((pitr -> momentum().perp() >= m_PtMinDstar) &&
          (pitr -> momentum().perp() < m_PtMaxDstar) &&
          (std::abs(pitr -> momentum().eta()) < m_EtaRangeDstar)) {

          //Check if has end_vertex
          if (!(pitr -> end_vertex())) continue;

          double Rxy = std::sqrt(pow(pitr -> end_vertex() -> position().x() - primx, 2) + pow(pitr -> end_vertex() -> position().y() - primy, 2));

          ATH_MSG_DEBUG("MuDstarFilter: R_xy(Dstar) = " << Rxy);

          if (Rxy < m_RxyMinDstar) continue;

          // Child
          #ifdef HEPMC3
          auto firstChild = pitr -> end_vertex() -> particles_out().begin();
          auto endChild = pitr -> end_vertex() -> particles_out().end();
          auto thisChild = firstChild;
          #else
          HepMC::GenVertex::particle_iterator firstChild =
            pitr -> end_vertex() -> particles_begin(HepMC::children);
          HepMC::GenVertex::particle_iterator endChild =
            pitr -> end_vertex() -> particles_end(HepMC::children);
          HepMC::GenVertex::particle_iterator thisChild = firstChild;
          #endif

          if (( * firstChild) -> pdg_id() == pitr -> pdg_id()) continue;

          TLorentzVector p4_K;
          TLorentzVector p4_pi;
          TLorentzVector p4_pis;
          TLorentzVector p4_D0;
          TLorentzVector p4_Dstar;
          TLorentzVector p4_Mu;
          TLorentzVector p4_DstarMu;

          int pis_pdg = 0;
          int K_pdg = 0;

          int NumD0 = 0;
          int NumPis = 0;

          for (; thisChild != endChild; ++thisChild) {

            if (( * thisChild) -> status() == 3) continue; // photos history line

            if (std::abs(( * thisChild) -> pdg_id()) == MC::PIPLUS) {
              if ((( * thisChild) -> momentum().perp() >= m_PtMinPis) &&
                (( * thisChild) -> momentum().perp() < m_PtMaxPis) &&
                (std::abs(( * thisChild) -> momentum().eta()) < m_EtaRangePis)) {
                ++NumPis;

                p4_pis.SetPtEtaPhiM(( * thisChild) -> momentum().perp(), ( * thisChild) -> momentum().eta(), ( * thisChild) -> momentum().phi(), m_PionMass);
                pis_pdg = ( * thisChild) -> pdg_id();
              }
            }
          } // thisChild

          ATH_MSG_DEBUG("MuDstarFilter: NumPis = " << NumPis );

          if (NumPis != 1) continue;

          HepMC::ConstGenParticlePtr D0Child1 = 0;
          HepMC::ConstGenParticlePtr D0Child2 = 0;
          HepMC::ConstGenParticlePtr D0ChildMu = 0;

          int NumChildD0 = 0;
          int NumChildD0Charged = 0;
          int NumChildD0neutrinos = 0;
          int NumChildD0gammas = 0;
          int ChargeD0Child1 = 0;
          int ChargeD0Child2 = 0;
          int NumChildD0K = 0;
          int NumChildD0pi = 0;
          int NumChildD0mu = 0;

          for (thisChild = firstChild; thisChild != endChild; ++thisChild) {

            if (( * thisChild) -> status() == 3) continue; // photos history line

            if (std::abs(( * thisChild) -> pdg_id()) == MC::D0) {
              if ((( * thisChild) -> end_vertex())) {
                #ifdef HEPMC3
                auto firstChild1 = ( * thisChild) -> end_vertex() -> particles_out().begin();
                auto endChild1 = ( * thisChild) -> end_vertex() -> particles_out().end();
                auto thisChild1 = firstChild1;
                #else
                HepMC::GenVertex::particle_iterator firstChild1 =
                  ( * thisChild) -> end_vertex() -> particles_begin(HepMC::children);
                HepMC::GenVertex::particle_iterator endChild1 =
                  ( * thisChild) -> end_vertex() -> particles_end(HepMC::children);
                HepMC::GenVertex::particle_iterator thisChild1 = firstChild1;

                #endif

                for (; thisChild1 != endChild1; ++thisChild1) {

                  if (( * thisChild1) -> status() == 3) continue; // photos history line

                  if (std::abs(( * thisChild1) -> pdg_id()) == MC::ELECTRON || std::abs(( * thisChild1) -> pdg_id()) == MC::MUON ||
                    std::abs(( * thisChild1) -> pdg_id()) == MC::PIPLUS || std::abs(( * thisChild1) -> pdg_id()) == MC::KPLUS) {

                    ++NumChildD0;

                    if ((( * thisChild1) -> momentum().perp() >= m_PtMinKpi) &&
                      (( * thisChild1) -> momentum().perp() < m_PtMaxKpi) &&
                      (std::abs(( * thisChild1) -> momentum().eta()) < m_EtaRangeKpi)) {
                      ++NumChildD0Charged;

                      if (NumChildD0Charged == 1) {
                        D0Child1 = ( * thisChild1);
                        if (( * thisChild1) -> pdg_id() == -MC::ELECTRON || ( * thisChild1) -> pdg_id() == -MC::MUON ||
                          ( * thisChild1) -> pdg_id() == MC::PIPLUS || ( * thisChild1) -> pdg_id() == MC::KPLUS) {
                          ChargeD0Child1 = +1;
                        } else {
                          ChargeD0Child1 = -1;
                        }
                      }
                      if (NumChildD0Charged == 2) {
                        D0Child2 = ( * thisChild1);
                        if (( * thisChild1) -> pdg_id() == -MC::ELECTRON || ( * thisChild1) -> pdg_id() == -MC::MUON ||
                          ( * thisChild1) -> pdg_id() == MC::PIPLUS || ( * thisChild1) -> pdg_id() == MC::KPLUS) {
                          ChargeD0Child2 = +1;
                        } else {
                          ChargeD0Child2 = -1;
                        }
                      }
                      if (std::abs(( * thisChild1) -> pdg_id()) == MC::MUON) {
                        ++NumChildD0mu;
                        D0ChildMu = ( * thisChild1);
                      }
                      if (std::abs(( * thisChild1) -> pdg_id()) == MC::PIPLUS) ++NumChildD0pi;
                      if (std::abs(( * thisChild1) -> pdg_id()) == MC::KPLUS) {
                        ++NumChildD0K;
                        K_pdg = ( * thisChild1) -> pdg_id();
                      }
                    }

                  } else if (std::abs(( * thisChild1) -> pdg_id()) == MC::PI0) {
                    ++NumChildD0;
                  } else if (std::abs(( * thisChild1) -> pdg_id()) == MC::NU_E || std::abs(( * thisChild1) -> pdg_id()) == MC::NU_MU) {
                    ++NumChildD0neutrinos;
                  } else if (std::abs(( * thisChild1) -> pdg_id()) == MC::PHOTON) {
                    ++NumChildD0gammas;
                  } else if (std::abs(( * thisChild1) -> pdg_id()) == MC::K0 || std::abs(( * thisChild1) -> pdg_id()) == MC::K0L ||
                    std::abs(( * thisChild1) -> pdg_id()) == MC::K0S) {
                    ++NumChildD0;
                    ++NumChildD0;
                  } else if ((( * thisChild1) -> end_vertex())) {

                    //
                    #ifdef HEPMC3
                    auto firstChild2 = ( * thisChild1) -> end_vertex() -> particles_out().begin();
                    auto endChild2 = ( * thisChild1) -> end_vertex() -> particles_out().end();
                    auto thisChild2 = firstChild2;
                    #else
                    HepMC::GenVertex::particle_iterator firstChild2 =
                      ( * thisChild1) -> end_vertex() -> particles_begin(HepMC::children);
                    HepMC::GenVertex::particle_iterator endChild2 =
                      ( * thisChild1) -> end_vertex() -> particles_end(HepMC::children);
                    HepMC::GenVertex::particle_iterator thisChild2 = firstChild2;

                    #endif
                    for (; thisChild2 != endChild2; ++thisChild2) {

                      if (( * thisChild2) -> status() == 3) continue; // photos history line

                      if (std::abs(( * thisChild2) -> pdg_id()) == MC::ELECTRON || std::abs(( * thisChild2) -> pdg_id()) == MC::MUON ||
                        std::abs(( * thisChild2) -> pdg_id()) == MC::PIPLUS || std::abs(( * thisChild2) -> pdg_id()) == MC::KPLUS) {

                        ++NumChildD0;

                        if ((( * thisChild2) -> momentum().perp() >= m_PtMinKpi) &&
                          (( * thisChild2) -> momentum().perp() < m_PtMaxKpi) &&
                          (std::abs(( * thisChild2) -> momentum().eta()) < m_EtaRangeKpi)) {
                          ++NumChildD0Charged;

                          if (NumChildD0Charged == 1) {
                            D0Child1 = ( * thisChild2);
                            if (( * thisChild2) -> pdg_id() == -MC::ELECTRON || ( * thisChild2) -> pdg_id() == -MC::MUON ||
                              ( * thisChild2) -> pdg_id() == MC::PIPLUS || ( * thisChild2) -> pdg_id() == MC::KPLUS) {
                              ChargeD0Child1 = +1;
                            } else {
                              ChargeD0Child1 = -1;
                            }
                          }
                          if (NumChildD0Charged == 2) {
                            D0Child2 = ( * thisChild2);
                            if (( * thisChild2) -> pdg_id() == -MC::ELECTRON || ( * thisChild2) -> pdg_id() == -MC::MUON ||
                              ( * thisChild2) -> pdg_id() == MC::PIPLUS || ( * thisChild2) -> pdg_id() == MC::KPLUS) {
                              ChargeD0Child2 = +1;
                            } else {
                              ChargeD0Child2 = -1;
                            }
                          }
                          if (std::abs(( * thisChild2) -> pdg_id()) == MC::MUON) {
                            ++NumChildD0mu;
                            D0ChildMu = ( * thisChild2);
                          }
                        }

                      } else if (std::abs(( * thisChild2) -> pdg_id()) == MC::PI0) {
                        ++NumChildD0;
                      } else if (std::abs(( * thisChild2) -> pdg_id()) == MC::NU_E || std::abs(( * thisChild2) -> pdg_id()) == MC::NU_MU) {
                        ++NumChildD0neutrinos;
                      } else if (std::abs(( * thisChild2) -> pdg_id()) == MC::PHOTON) {
                        ++NumChildD0gammas;
                      } else if (std::abs(( * thisChild2) -> pdg_id()) == MC::K0 || std::abs(( * thisChild2) -> pdg_id()) == MC::K0L ||
                        std::abs(( * thisChild2) -> pdg_id()) == MC::K0S) {
                        ++NumChildD0;
                        ++NumChildD0;
                      } else if ((( * thisChild2) -> end_vertex())) {
                        ++NumChildD0;
                        ++NumChildD0;
                      } else {
                        ++NumChildD0;
                        ++NumChildD0;
                        ATH_MSG_DEBUG("MuDstarFilter: unexpected D0 granddaughter = " << (*thisChild2)->pdg_id() );
                      }
                    } // thisChild2

                  } else {
                    ++NumChildD0;
                    ++NumChildD0;
                    ATH_MSG_DEBUG("MuDstarFilter: unexpected D0 daughter = " << (*thisChild1)->pdg_id() );
                  }
                } // thisChild1

                ATH_MSG_DEBUG("MuDstarFilter: NumChildD0, NumChildD0Charged = " << NumChildD0 << " , " << NumChildD0Charged);

                if (NumChildD0 <= 3 && NumChildD0Charged == 2 && ChargeD0Child1 * ChargeD0Child2 < 0) {
                  if (m_D0Kpi_only) {
                    if (NumChildD0 == 2 && NumChildD0K == 1 && NumChildD0pi == 1) ++NumD0;
                  } else {
                    ++NumD0;
                  }
                }
              } // enVertex
            } // D0

            ATH_MSG_DEBUG("MuDstarFilter: NumD0 = " << NumD0 );

            if (NumD0 == 1) {

              if (pis_pdg * ChargeD0Child1 < 0) {

                p4_K.SetPtEtaPhiM(D0Child1 -> momentum().perp(), D0Child1 -> momentum().eta(), D0Child1 -> momentum().phi(), m_KaonMass);
                p4_pi.SetPtEtaPhiM(D0Child2 -> momentum().perp(), D0Child2 -> momentum().eta(), D0Child2 -> momentum().phi(), m_PionMass);

              } else {

                p4_K.SetPtEtaPhiM(D0Child2 -> momentum().perp(), D0Child2 -> momentum().eta(), D0Child2 -> momentum().phi(), m_KaonMass);
                p4_pi.SetPtEtaPhiM(D0Child1 -> momentum().perp(), D0Child1 -> momentum().eta(), D0Child1 -> momentum().phi(), m_PionMass);
              }

              p4_D0 = p4_K + p4_pi;
              double mKpi = p4_D0.M();

              ATH_MSG_DEBUG("MuDstarFilter: mKpi = " << mKpi );

              if (mKpi >= m_mKpiMin && mKpi <= m_mKpiMax) {

                p4_Dstar = p4_D0 + p4_pis;

                double delta_m = p4_Dstar.M() - mKpi;

                ATH_MSG_DEBUG("MuDstarFilter: delta_m = " << delta_m );

                if (delta_m <= m_delta_m_Max) {
                  ++NumDstars;

                  ATH_MSG_DEBUG("MuDstarFilter: NumDstars = " << NumDstars );

                  for (size_t i = 0; i < Muons.size(); ++i) {

                    if (NumChildD0mu == 1) {
                      ATH_MSG_DEBUG("MuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->momentum().perp() << " , " << D0ChildMu->momentum().perp());
                      if (std::fabs(Muons[i] -> momentum().perp() - D0ChildMu -> momentum().perp())< std::numeric_limits<double>::epsilon()) continue;
                      ATH_MSG_DEBUG("MuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->momentum().perp() << " , " << D0ChildMu->momentum().perp() ) ;
                    }

                    p4_Mu.SetPtEtaPhiM(Muons[i] -> momentum().perp(), Muons[i] -> momentum().eta(), Muons[i] -> momentum().phi(), m_MuonMass);

                    p4_DstarMu = p4_Dstar + p4_Mu;

                    ATH_MSG_DEBUG("MuDstarFilter: p4_DstarMu.M() = " << p4_DstarMu.M());

                    if (p4_DstarMu.M() <= m_DstarMu_m_Max) {

                      ATH_MSG_INFO("MuDstarFilter: MuDstar candidate found" );
                      ATH_MSG_INFO("MuDstarFilter: p4_DstarMu.M() = " << p4_DstarMu.M() );
                      ATH_MSG_INFO("MuDstarFilter: NumChildD0, NumChildD0Charged = " << NumChildD0 << " , " << NumChildD0Charged );
                      ATH_MSG_INFO("MuDstarFilter: NumChildD0K, NumChildD0pi, NumChildD0mu = " << NumChildD0K << " , " << NumChildD0pi << " , " << NumChildD0mu );

                      if (NumChildD0mu == 1) {

                        ATH_MSG_DEBUG("MuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->momentum().perp() << " , " << D0ChildMu->momentum().perp() );
                      }

                      ATH_MSG_INFO("MuDstarFilter: NumChildD0neutrinos, NumChildD0gammas = " << NumChildD0neutrinos << " , " << NumChildD0gammas );
                      ATH_MSG_INFO("MuDstarFilter: pis_pdg, K_pdg, ChargeD0Child1, ChargeD0Child2 = " << pis_pdg << " , " << K_pdg << " , " << ChargeD0Child1 << " , " << ChargeD0Child2 );

                      setFilterPassed(true);
                      return StatusCode::SUCCESS;
                    }
                  } // for i

                } //delta_m		 
              } // mKpi
            } // NumD0	     
          } // thisChild
        } // PtMinDstar
      } // DSTAR
    } // pitr
  } // itr

  //
  // if we get here we have failed
  //
  setFilterPassed(false);
  return StatusCode::SUCCESS;
}

