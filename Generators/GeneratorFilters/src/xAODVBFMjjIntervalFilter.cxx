/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Header for this module
#include "GeneratorFilters/xAODVBFMjjIntervalFilter.h"

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"

// Pt  High --> Low
class High2LowByJetClassPt
{
public:
    bool operator()(const xAOD::Jet *t1, const xAOD::Jet *t2) const
    {
        return (t1->pt() > t2->pt());
    }
};

StatusCode xAODVBFMjjIntervalFilter::filterInitialize()
{
    CHECK(m_TruthJetContainerName.initialize());
    CHECK(m_truthPartContKey.initialize());
    ATH_MSG_INFO("Configured for jets in " << m_TruthJetContainerName.key() << " inside |y|<" << m_yMax);

    CHECK(m_rndmSvc.retrieve());

    m_alpha = log(m_prob2low / m_prob2high) / log(m_mjjlow / m_mjjhigh); // This calculation overrides anything read in from the configuration...
    ATH_MSG_INFO("m_alpha set to" << m_alpha);
    return StatusCode::SUCCESS;
}

StatusCode xAODVBFMjjIntervalFilter::filterEvent()
{
    // Get random number engine
    const EventContext& ctx = Gaudi::Hive::currentContext();
    CLHEP::HepRandomEngine* rndm = this->getRandomEngine(name(), ctx);
    if (!rndm)
    {
        ATH_MSG_ERROR("Failed to retrieve random number engine xAODVBFMjjIntervalFilter");
        setFilterPassed(false);
        return StatusCode::FAILURE;
    }

    // Retrieve jet container
    SG::ReadHandle<xAOD::JetContainer>  truthJetCollection{m_TruthJetContainerName};
    CHECK(truthJetCollection.isValid());

    // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
    // duplicated barcode ones
    SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
    CHECK(xTruthParticleContainer.isValid());


    // Find overlap objects
    std::vector<const xAOD::TruthParticle *> MCTruthPhotonList;
    std::vector<const xAOD::TruthParticle *> MCTruthElectronList;
    std::vector<TLorentzVector> MCTruthTauList;

    // Loop over all particles in the event
    for (const xAOD::TruthParticle* pitr : *xTruthParticleContainer) {
      if (m_photonjetoverlap == true)
          {
         // photon - copied from VBFForwardJetsFilter.cxx
                if (MC::isPhoton(pitr) && MC::isStable(pitr) &&
                    pitr->pt() >= m_olapPt &&
                    std::abs(pitr->eta()) <= m_yMax)
                {
                    MCTruthPhotonList.push_back(pitr);
                }
            }
            if (m_electronjetoverlap == true)
            {
                // electron
                if (MC::isElectron(pitr) && MC::isStable(pitr) &&
                    pitr->pt() >= m_olapPt &&
                    std::abs(pitr->eta()) <= m_yMax)
                {
                    MCTruthElectronList.push_back(pitr);
                }
            }
            if (m_taujetoverlap == true)
            {
                // tau - copied from VBFForwardJetsFilter.cxx
                if (MC::isTau(pitr) && MC::isPhysical(pitr))
                {
                    auto tau = pitr;
                    int leptonic = 0;
                    for (size_t thisChild_id = 0; thisChild_id < tau->decayVtx()->nOutgoingParticles(); thisChild_id++)
                    {
                        auto child = tau->decayVtx()->outgoingParticle(thisChild_id);
                        if (child->prodVtx() != tau->decayVtx())
                            continue;
                        if (std::abs(child->pdgId()) == MC::NU_E)
                            leptonic = 1;
                        else if (std::abs(child->pdgId()) == MC::NU_MU)
                            leptonic = 2;
                        else if (std::abs(child->pdgId()) == MC::TAU)
                            leptonic = 11;
                    }

                    if (leptonic == 0)
                    {
                        TLorentzVector nutau = sumDaughterNeutrinos(tau);
                        TLorentzVector tauvis = TLorentzVector(tau->px() - nutau.Px(),
                                                               tau->py() - nutau.Py(),
                                                               tau->pz() - nutau.Pz(),
                                                               tau->e() - nutau.E());
                        if (tauvis.Vect().Perp() >= m_olapPt && std::abs(tauvis.Vect().PseudoRapidity()) <= m_yMax)
                        {
                            MCTruthTauList.push_back(tauvis);
                        }
                    }
                }
            }
        } //loop over truth particles
    

    // Filter based on rapidity acceptance and sort
    ConstDataVector<xAOD::JetContainer> filteredJets(SG::VIEW_ELEMENTS);
    for (const xAOD::Jet* truthJet : *truthJetCollection)
    {
        if (std::abs(truthJet->rapidity()) < m_yMax && truthJet->pt() >= m_olapPt)
        {
            bool JetOverlapsWithPhoton = false;
            bool JetOverlapsWithElectron = false;
            bool JetOverlapsWithTau = false;

            if (m_photonjetoverlap == true)
            {
                JetOverlapsWithPhoton = checkOverlap(truthJet->rapidity(), truthJet->phi(), MCTruthPhotonList);
            }
            if (m_electronjetoverlap == true)
            {
                JetOverlapsWithElectron = checkOverlap(truthJet->rapidity(), truthJet->phi(), MCTruthElectronList);
            }
            if (m_taujetoverlap == true)
            {
                JetOverlapsWithTau = checkOverlap(truthJet->rapidity(), truthJet->phi(), MCTruthTauList);
            }

            if (!JetOverlapsWithPhoton && !JetOverlapsWithElectron && !JetOverlapsWithTau)
            {
                filteredJets.push_back(truthJet);
            }
        }
    }
    filteredJets.sort(High2LowByJetClassPt());

    if (m_ApplyWeighting)
    {

        double eventWeight = 1.0;
        eventWeight = getEventWeight(&filteredJets);
        double rnd = rndm->flat();
        if (1.0 / eventWeight < rnd)
        {
            setFilterPassed(false);
            ATH_MSG_DEBUG("Event failed weighting. Weight is " << eventWeight);
            return StatusCode::SUCCESS;
        }

        // Get MC event collection for setting weight
        const McEventCollection* mecc = 0;
        if (evtStore()->retrieve(mecc).isFailure() || !mecc) // FIXME keyless retrieve
        {
            setFilterPassed(false);
            ATH_MSG_ERROR("Could not retrieve MC Event Collection - weight might not work");
            return StatusCode::SUCCESS;
        }

        ATH_MSG_INFO("Event passed.  Will weight events " << eventWeight * m_norm);
        McEventCollection *mec = const_cast<McEventCollection *>(&(*mecc));
        for (unsigned int i = 0; i < mec->size(); ++i)
        {
            if (!(*mec)[i])
                continue;
            double existingWeight = (*mec)[i]->weights().size() > 0 ? (*mec)[i]->weights()[0] : 1.;
            if ((*mec)[i]->weights().size() > 0)
            {
              for (unsigned int iw = 0; iw < (*mec)[i]->weights().size(); ++iw) {
                 double existWeight = (*mec)[i]->weights()[iw];
                 (*mec)[i]->weights()[iw] = existWeight * eventWeight * m_norm;
              }
//
//                (*mec)[i]->weights()[0] = existingWeight * eventWeight * m_norm;
            }
            else
            {
                (*mec)[i]->weights().push_back(eventWeight * m_norm * existingWeight);
            }

#ifdef HEPMC3
      (*mec)[i]->add_attribute("filterWeight", std::make_shared<HepMC3::DoubleAttribute>(eventWeight*m_norm));
#endif

        }
    } // Apply weighting
    else
    {
        // just compute mjj, dphi etc
        bool pass = ApplyMassDphi(&filteredJets);
        if (!pass)
        {
            setFilterPassed(false);
            ATH_MSG_DEBUG("Event failed filter");
            return StatusCode::SUCCESS;
        }
        if (m_ApplyNjet)
        {
            if (filteredJets.size() < m_NJetsMin)
            {
                setFilterPassed(false);
                return StatusCode::SUCCESS;
            }
            if (m_NJetsMax > 0)
            {
                if (filteredJets.size() > m_NJetsMax)
                {
                    setFilterPassed(false);
                    return StatusCode::SUCCESS;
                }
            } // Njets <
        }     // Apply Njets filter
    }
    // Made it to the end - success!
    setFilterPassed(true);
    return StatusCode::SUCCESS;
}

bool xAODVBFMjjIntervalFilter::checkOverlap(double eta, double phi, const std::vector<const xAOD::TruthParticle *> &list)
{
    for (size_t i = 0; i < list.size(); ++i)
    {
        double pt = list[i]->pt();
        if (pt > m_olapPt)
        {
            /// @todo Provide a helper function for this (and similar)
            double dphi = phi - list[i]->phi();
            double deta = eta - list[i]->eta();
            if (dphi > M_PI)
            {
                dphi -= 2. * M_PI;
            }
            if (dphi < -M_PI)
            {
                dphi += 2. * M_PI;
            }
            double dr = std::sqrt(deta * deta + dphi * dphi);
            if (dr < 0.3)
                return true;
        }
    }
    return false;
}

bool xAODVBFMjjIntervalFilter::checkOverlap(double eta, double phi, const std::vector<TLorentzVector> &list)
{
    for (size_t i = 0; i < list.size(); ++i)
    {
        double pt = list[i].Vect().Perp();
        if (pt > m_olapPt)
        {
            /// @todo Provide a helper function for this (and similar)
            double dphi = phi - list[i].Phi();
            double deta = eta - list[i].Vect().PseudoRapidity();
            if (dphi > M_PI)
            {
                dphi -= 2. * M_PI;
            }
            if (dphi < -M_PI)
            {
                dphi += 2. * M_PI;
            }
            double dr = std::sqrt(deta * deta + dphi * dphi);
            if (dr < 0.3)
                return true;
        }
    }
    return false;
}

bool xAODVBFMjjIntervalFilter::ApplyMassDphi(ConstDataVector<xAOD::JetContainer> *jets)
{
    if (jets->size() < 2)
        return false;
    double mjj = (jets->at(0)->p4() + jets->at(1)->p4()).M();
    double dphi = std::abs(jets->at(0)->p4().DeltaPhi(jets->at(1)->p4()));
    ATH_MSG_INFO("mjj " << mjj << " dphi " << dphi);
    bool pass = true;
    if (mjj < m_mjjlow)
        pass = false;
    if (mjj > m_mjjhigh)
        pass = false;
    if (m_applyDphi && dphi > m_dphijj)
        pass = false;

    return pass;
}

double xAODVBFMjjIntervalFilter::getEventWeight(ConstDataVector<xAOD::JetContainer> *jets) const
{
    double weight = 1.0;
    if (jets->size() == 0)
    {
        weight /= m_prob0;
        ATH_MSG_DEBUG("Event in 0-jet weighting. Weight is " << weight);
    }
    else if (jets->size() == 1)
    {
        weight /= m_prob1;
        ATH_MSG_DEBUG("Event in 1-jet weighting. Weight is " << weight);
    }
    else
    {
        double mjj = (jets->at(0)->p4() + jets->at(1)->p4()).M();
        if (mjj < m_mjjlow)
        {
            if (m_truncatelowmjj == false)
            {
                weight /= m_prob2low;
            }
            else
            {
                weight = -1.0;
            }
        }
        else if (mjj > m_mjjhigh)
        {
            if (m_truncatehighmjj == false)
            {
                weight /= m_prob2high;
            }
            else
            {
                weight = -1.0;
            }
        }
        else
        {
            weight = weight * std::pow(m_mjjlow / mjj, m_alpha) / m_prob2low;
            ATH_MSG_DEBUG("WEIGHTING:: " << mjj << "\t" << weight);
        }
    }
    return weight;
}

TLorentzVector xAODVBFMjjIntervalFilter::sumDaughterNeutrinos(const xAOD::TruthParticle *part)
{
    TLorentzVector nu(0, 0, 0, 0);

    if (MC::isSMNeutrino(part))
    {
        nu.SetPx(part->px());
        nu.SetPy(part->py());
        nu.SetPz(part->pz());
        nu.SetE(part->e());
        return nu;
    }

    if (!part->decayVtx())
        return nu;

    for (size_t thisChild_id = 0; thisChild_id < part->decayVtx()->nOutgoingParticles(); thisChild_id++)
    {
        auto daughterparticle = part->decayVtx()->outgoingParticle(thisChild_id);
        nu += sumDaughterNeutrinos(daughterparticle);
    }
    return nu;
}


CLHEP::HepRandomEngine* xAODVBFMjjIntervalFilter::getRandomEngine(const std::string& streamName,
                                                                const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  return rngWrapper->getEngine(ctx);
}
