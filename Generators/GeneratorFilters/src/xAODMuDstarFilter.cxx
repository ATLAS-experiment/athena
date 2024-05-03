/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// -------------------------------------------------------------
// File: GeneratorFilters/xAODMuDstarFilter.cxx
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

#include "GeneratorFilters/xAODMuDstarFilter.h"

// Framework Related Headers:-
#include "GaudiKernel/MsgStream.h"

// Other classes used by this class:-
#include <math.h>
#include <limits>

#include "TLorentzVector.h"

#include "TruthUtils/AtlasPID.h"

//--------------------------------------------------------------------------
xAODMuDstarFilter::xAODMuDstarFilter(const std::string &name,
                                     ISvcLocator *pSvcLocator) : GenFilter(name, pSvcLocator)
{
  //--------------------------------------------------------------------------
}

xAODMuDstarFilter::~xAODMuDstarFilter() = default;

//---------------------------------------------------------------------------
StatusCode xAODMuDstarFilter::filterInitialize()
{
  //---------------------------------------------------------------------------
  ATH_MSG_INFO("xAODMuDstarFilter v1.02 INITIALISING ");

  ATH_MSG_INFO("PtMinMuon = " << m_PtMinMuon);
  ATH_MSG_INFO("PtMaxMuon = " << m_PtMaxMuon);
  ATH_MSG_INFO("EtaRangeMuon = " << m_EtaRangeMuon);

  ATH_MSG_INFO("PtMinDstar = " << m_PtMinDstar);
  ATH_MSG_INFO("PtMaxDstar = " << m_PtMaxDstar);
  ATH_MSG_INFO("EtaRangeDstar = " << m_EtaRangeDstar);

  ATH_MSG_INFO("RxyMinDstar = " << m_RxyMinDstar);

  ATH_MSG_INFO("PtMinPis = " << m_PtMinPis);
  ATH_MSG_INFO("PtMaxPis = " << m_PtMaxPis);
  ATH_MSG_INFO("EtaRangePis = " << m_EtaRangePis);

  ATH_MSG_INFO("D0Kpi_only = " << m_D0Kpi_only);
  ATH_MSG_INFO("PtMinKpi = " << m_PtMinKpi);
  ATH_MSG_INFO("PtMaxKpi = " << m_PtMaxKpi);
  ATH_MSG_INFO("EtaRangeKpi = " << m_EtaRangeKpi);

  ATH_MSG_INFO("mKpiMin = " << m_mKpiMin);
  ATH_MSG_INFO("mKpiMax = " << m_mKpiMax);

  ATH_MSG_INFO("delta_m_Max = " << m_delta_m_Max);

  ATH_MSG_INFO("DstarMu_m_Max = " << m_DstarMu_m_Max);

  ATH_CHECK(m_xaodTruthParticleContainerNameGenKey.initialize());

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode xAODMuDstarFilter::filterEvent()
{
  //---------------------------------------------------------------------------

// Retrieve TruthGen container from xAODTruthParticleSlimmerGen, contains all particles witout barcode_zero and
// duplicated barcode ones
   const EventContext& context = Gaudi::Hive::currentContext();
   SG::ReadHandle<xAOD::TruthParticleContainer>
      xTruthParticleContainer(
          m_xaodTruthParticleContainerNameGenKey, context);
   if (!xTruthParticleContainer.isValid()) {
    ATH_MSG_ERROR("Could not retrieve xAOD::TruthParticleGenContainer with key:"
                  << m_xaodTruthParticleContainerNameGenKey.key());

    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG(" xAODMuDstarFilter filtering ");
  double primx = 0.;
  double primy = 0.;
  int NumMuons = 0;  
  int NumDstars = 0; 
    
  std::vector<const xAOD::TruthParticle *> Muons;

  unsigned int nPart = xTruthParticleContainer->size();   
  ATH_MSG_DEBUG("xAODMuDstarFilter:number of particles " << nPart);

  // Loop over all particles in the event
    for (const xAOD::TruthParticle* pitr : *xTruthParticleContainer) {

      if (pitr->status() == 4){
        const xAOD::TruthVertex *vprim = (pitr)->decayVtx();    
        primx = vprim->x();
        primy = vprim->y();

        ATH_MSG_DEBUG("xAODMuDstarFilter: PV x, y = " << primx << " , " << primy);
        }
  
      if (pitr->status() == 3)
        continue; // photos history line
   
      // muons
      if (pitr->isMuon())
      {
        if ((pitr->pt() >= m_PtMinMuon) &&
            (pitr->pt() < m_PtMaxMuon) &&
            (std::abs(pitr->eta()) < m_EtaRangeMuon))
        {
          NumMuons++;
          Muons.push_back(pitr);
        }
      }
    }

    if (NumMuons == 0){
       setFilterPassed(false);
       return StatusCode::SUCCESS;
      }

    ATH_MSG_DEBUG("xAODMuDstarFilter: NumMuons = " << NumMuons );

    for (const xAOD::TruthParticle* pitr : *xTruthParticleContainer) {     
      if (pitr->status() == 3)
        continue; // photos history line
     
      // Dstars
      if (std::abs(pitr->pdgId()) == DSTAR)
      {
        if ((pitr->pt() >= m_PtMinDstar) &&
            (pitr->pt() < m_PtMaxDstar) &&
            (std::abs(pitr->eta()) < m_EtaRangeDstar))
        {

          //Check if has end_vertex
          if (!(pitr->decayVtx()))
            continue;
          double Rxy = std::hypot(pitr->decayVtx()->x() - primx, pitr->decayVtx()->y() - primy);

          ATH_MSG_DEBUG("xAODMuDstarFilter: R_xy(Dstar) = " << Rxy);
         
          if (Rxy < m_RxyMinDstar)
            continue;

          auto firstChild = pitr->decayVtx()->outgoingParticle(0);
          if (firstChild->pdgId() == pitr->pdgId())
            continue;

          TLorentzVector p4_pis;

          int pis_pdg = 0;
          int K_pdg = 0;

          int NumD0 = 0;
          int NumPis = 0;

          for (size_t thisChild_id = 0; thisChild_id < pitr->decayVtx()->nOutgoingParticles(); thisChild_id++)
          {

            auto thisChild = pitr->decayVtx()->outgoingParticle(thisChild_id);
            if (thisChild->status() == 3)
              continue; // photos history line

            if (std::abs(thisChild->pdgId()) == PIPLUS)
            {
              if ((thisChild->pt() >= m_PtMinPis) &&
                  (thisChild->pt() < m_PtMaxPis) &&
                  (std::abs(thisChild->eta()) < m_EtaRangePis))
              {
                NumPis++;

                p4_pis.SetPtEtaPhiM(thisChild->pt(), thisChild->eta(), thisChild->phi(), m_PionMass);
                pis_pdg = thisChild->pdgId();
              }
            }
          } // thisChild

          if (NumPis == 0)
            continue;

          ATH_MSG_DEBUG("xAODMuDstarFilter: NumPis = " << NumPis);

          const xAOD::TruthParticle *D0Child1{nullptr}, *D0Child2{nullptr}, *D0ChildMu{nullptr};
          
          int NumChildD0 = 0;
          int NumChildD0Charged = 0;
          int NumChildD0neutrinos = 0;
          int NumChildD0gammas = 0;
          int ChargeD0Child1 = 0;
          int ChargeD0Child2 = 0;
          int NumChildD0K = 0;
          int NumChildD0pi = 0;
          int NumChildD0mu = 0;

          for (size_t thisChild_id = 0; thisChild_id < pitr->decayVtx()->nOutgoingParticles(); thisChild_id++)
          {
            auto thisChild = pitr->decayVtx()->outgoingParticle(thisChild_id);
            if (thisChild->status() == 3)
              continue; // photos history line

            if (std::abs(thisChild->pdgId()) == D0)
            {
              if (! thisChild->decayVtx()) continue; 
              
              for (size_t thisChild1_id = 0; thisChild1_id < thisChild->decayVtx()->nOutgoingParticles(); thisChild1_id++)
                {
                  auto thisChild1 = thisChild->decayVtx()->outgoingParticle(thisChild1_id);
                  if (thisChild1->status() == 3)
                    continue; // photos history line

                  if (thisChild1->isElectron() || thisChild1->isMuon() ||
                      std::abs(thisChild1->pdgId()) == PIPLUS || std::abs(thisChild1->pdgId()) == KPLUS)
                  {

                    NumChildD0++;
                    if ((thisChild1->pt() >= m_PtMinKpi) &&
                        (thisChild1->pt() < m_PtMaxKpi) &&
                        (std::abs(thisChild1->eta()) < m_EtaRangeKpi))
                    {
                      NumChildD0Charged++;

                      if (NumChildD0Charged == 1)
                      {
                        D0Child1 = thisChild1;
                        ChargeD0Child1 = D0Child1->charge();
                      }
                      if (NumChildD0Charged == 2)
                      {
                        D0Child2 = thisChild1;
                        ChargeD0Child2 = D0Child2->charge();
                      }
                      if (thisChild1->isMuon())
                      {
                        NumChildD0mu++;
                        D0ChildMu = thisChild1;
                      }
                      if (std::abs(thisChild1->pdgId()) == PIPLUS)
                        NumChildD0pi++;
                      if (std::abs(thisChild1->pdgId()) == KPLUS)
                      {
                        NumChildD0K++;
                        K_pdg = thisChild1->pdgId();
                      }
                    }
                  }
                  else if (std::abs(thisChild1->pdgId()) == PI0)
                  {
                    NumChildD0++;
                  }
                  else if (thisChild1->isNeutrino())
                  {
                    NumChildD0neutrinos++;
                  }
                  else if (thisChild1->isPhoton())
                  {
                    NumChildD0gammas++;
                  }
                  else if (std::abs(thisChild1->pdgId()) == K0 || std::abs(thisChild1->pdgId()) == K0L ||
                           std::abs(thisChild1->pdgId()) == K0S)
                  {
                    NumChildD0++;
                    NumChildD0++;
                  }
                  else if (thisChild1->decayVtx())
                  {
                    for (size_t thisChild2_id = 0; thisChild2_id < thisChild1->decayVtx()->nOutgoingParticles(); thisChild2_id++)
                    {

                      auto thisChild2 = thisChild1->decayVtx()->outgoingParticle(thisChild2_id);
                      if (thisChild2->status() == 3)
                        continue; // photos history line

                      if (thisChild2->isElectron() || thisChild2->isMuon() ||
                          std::abs(thisChild2->pdgId()) == PIPLUS || std::abs(thisChild2->pdgId()) == KPLUS)
                      {
                        NumChildD0++;

                        if ((thisChild2->pt() >= m_PtMinKpi) &&
                            (thisChild2->pt() < m_PtMaxKpi) &&
                            (std::abs(thisChild2->eta()) < m_EtaRangeKpi))
                        {
                          NumChildD0Charged++;

                          if (NumChildD0Charged == 1)
                          {
                            D0Child1 = thisChild2;
                            ChargeD0Child2 = D0Child1->charge();
                          }
                          if (NumChildD0Charged == 2)
                          {
                            D0Child2 = thisChild2;
                            ChargeD0Child2 = D0Child2->charge();
                          }
                          if (thisChild2->isMuon())
                          {
                            NumChildD0mu++;
                            D0ChildMu = thisChild2;
                          }
                        }
                      }
                      else if (std::abs(thisChild2->pdgId()) == PI0)
                      {
                        NumChildD0++;
                      }
                      else if (thisChild2->isNeutrino())
                      {
                        NumChildD0neutrinos++;
                      }
                      else if (thisChild2->isPhoton())
                      {
                        NumChildD0gammas++;
                      }
                      else if (std::abs(thisChild2->pdgId()) == K0 || std::abs(thisChild2->pdgId()) == K0L ||
                               std::abs(thisChild2->pdgId()) == K0S)
                      {
                        NumChildD0++;
                        NumChildD0++;
                      }
                      else if (thisChild2->decayVtx())
                      {
                        NumChildD0++;
                        NumChildD0++;
                      }
                      else
                      {
                        NumChildD0++;
                        NumChildD0++;
                        ATH_MSG_DEBUG("xAODMuDstarFilter: unexpected D0 granddaughter = " << thisChild2->pdgId());
                      }
                    } // thisChild2
                  }
                  else
                  {
                    NumChildD0++;
                    NumChildD0++;
                    ATH_MSG_DEBUG("xAODMuDstarFilter: unexpected D0 daughter = " << thisChild1->pdgId());
                  }
                } // thisChild1

                ATH_MSG_DEBUG("xAODMuDstarFilter: NumChildD0, NumChildD0Charged = " << NumChildD0 << " , " << NumChildD0Charged);

                if (NumChildD0 <= 3 && NumChildD0Charged == 2 && ChargeD0Child1 * ChargeD0Child2 < 0)
                {
                  if (m_D0Kpi_only)
                  {
                    if (NumChildD0 == 2 && NumChildD0K == 1 && NumChildD0pi == 1){
                      NumD0++;
                      }
                  }
                  else
                  {
                    NumD0++;
                  }
                }
            }   // D0

            ATH_MSG_DEBUG("xAODMuDstarFilter: NumD0 = " << NumD0);

            if (NumD0 == 1)
            {

              if (pis_pdg * ChargeD0Child1 > 0) std::swap(D0Child1, D0Child2);
              TLorentzVector p4_K;
              p4_K.SetPtEtaPhiM(D0Child1->pt(), D0Child1->eta(), D0Child1->phi(), m_KaonMass);
              TLorentzVector p4_pi;
              p4_pi.SetPtEtaPhiM(D0Child2->pt(), D0Child2->eta(), D0Child2->phi(), m_PionMass);

              TLorentzVector  p4_D0 = p4_K + p4_pi;
              double mKpi = p4_D0.M();

              ATH_MSG_DEBUG("xAODMuDstarFilter: mKpi = " << mKpi);

              if (mKpi >= m_mKpiMin && mKpi <= m_mKpiMax)
              {

                TLorentzVector  p4_Dstar = p4_D0 + p4_pis;

                double delta_m = p4_Dstar.M() - mKpi;

                ATH_MSG_DEBUG("xAODMuDstarFilter: delta_m = " << delta_m);

                if (delta_m <= m_delta_m_Max)
                {
                  NumDstars++;

                  ATH_MSG_DEBUG("xAODMuDstarFilter: NumDstars = " << NumDstars);

                  for (size_t i = 0; i < Muons.size(); ++i)
                  {

                    if (NumChildD0mu == 1)
                    {
                      if (std::fabs(Muons[i]->pt() - D0ChildMu->pt()) < std::numeric_limits<double>::epsilon())
                        continue;
                      ATH_MSG_DEBUG("xAODMuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->pt() << " , " << D0ChildMu->pt());
                    }

                    TLorentzVector p4_Mu;
                    p4_Mu.SetPtEtaPhiM(Muons[i]->pt(), Muons[i]->eta(), Muons[i]->phi(), m_MuonMass);

                    TLorentzVector p4_DstarMu = p4_Dstar + p4_Mu;

                    ATH_MSG_DEBUG("xAODMuDstarFilter: p4_DstarMu.M() = " << p4_DstarMu.M());

                    if (p4_DstarMu.M() <= m_DstarMu_m_Max)
                    {

                      ATH_MSG_DEBUG("xAODMuDstarFilter: MuDstar candidate found");
                      ATH_MSG_DEBUG("xAODMuDstarFilter: p4_DstarMu.M() = " << p4_DstarMu.M());
                      ATH_MSG_DEBUG("xAODMuDstarFilter: NumChildD0, NumChildD0Charged = " << NumChildD0 << " , " << NumChildD0Charged);
                      ATH_MSG_DEBUG("xAODMuDstarFilter: NumChildD0K, NumChildD0pi, NumChildD0mu = " << NumChildD0K << " , " << NumChildD0pi << " , " << NumChildD0mu);

                      if (NumChildD0mu == 1)
                      {

                        ATH_MSG_DEBUG("xAODMuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->pt() << " , " << D0ChildMu->pt());
                      }

                      ATH_MSG_DEBUG("xAODMuDstarFilter: NumChildD0neutrinos, NumChildD0gammas = " << NumChildD0neutrinos << " , " << NumChildD0gammas);
                      ATH_MSG_DEBUG("xAODMuDstarFilter: pis_pdg, K_pdg, ChargeD0Child1, ChargeD0Child2 = " << pis_pdg << " , " << K_pdg << " , " << ChargeD0Child1 << " , " << ChargeD0Child2);

                      setFilterPassed(true);
                      return StatusCode::SUCCESS;
                    }
                  } // for i

                } //delta_m
              }   // mKpi
            }     // NumD0
          }       // thisChild
        }         // PtMinDstar
        
      } // DSTAR

    } // pitr

    

  //
  // if we get here we have failed
  //
  setFilterPassed(false);
  return StatusCode::SUCCESS;
}


