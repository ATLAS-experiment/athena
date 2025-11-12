/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkEGamma/EGElectronAmbiguityTool.h"

#include "xAODEgamma/ElectronxAODHelpers.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"

#include "TruthUtils/HepMCHelpers.h"

#include "TMath.h"

namespace {
void
helix(const xAOD::TrackParticle* trkP,
      const xAOD::Vertex* pvtx,
      std::vector<double>& he)
{
  constexpr double PTTOCURVATURE = -0.301;

  he[0] = 1. / tan(trkP->theta());
  he[1] = PTTOCURVATURE * trkP->charge() / trkP->pt();

  if (trkP->phi0() > 0.)
    he[4] = trkP->phi0();
  else
    he[4] = TMath::TwoPi() + trkP->phi0();

  double c1 = cos(trkP->phi0());
  double s1 = sin(trkP->phi0());
  he[3] = trkP->d0() + c1 * pvtx->y() - s1 * pvtx->x();

  c1 *= he[0];
  s1 *= he[0];
  he[2] = trkP->z0() - c1 * pvtx->x() - s1 * pvtx->y() + pvtx->z();
}
}//end anonymous


namespace DerivationFramework {

StatusCode
EGElectronAmbiguityTool::initialize()
{

  ATH_CHECK(m_containerName.initialize());
  ATH_CHECK(m_VtxContainerName.initialize());
  ATH_CHECK(m_tpContainerName.initialize());
  ATH_CHECK(m_tpCName.initialize());

  ATH_CHECK(m_drv.initialize());
  ATH_CHECK(m_dphiv.initialize());
  ATH_CHECK(m_dmee.initialize());
  ATH_CHECK(m_dmeeVtx.initialize());
  ATH_CHECK(m_dsep.initialize());
  ATH_CHECK(m_dambi.initialize());
  ATH_CHECK(m_dtrv.initialize());
  ATH_CHECK(m_dtpv.initialize());
  ATH_CHECK(m_dtzv.initialize());

  return StatusCode::SUCCESS;
}

EGElectronAmbiguityTool::DecorHandles::DecorHandles
  (const EGElectronAmbiguityTool& tool, const EventContext& ctx)
    : drv     (tool.m_drv, ctx),
      dphiv   (tool.m_dphiv, ctx),
      dmee    (tool.m_dmee, ctx),
      dmeeVtx (tool.m_dmeeVtx, ctx),
      dsep    (tool.m_dsep, ctx),
      dambi   (tool.m_dambi, ctx),
      dtrv    (tool.m_dtrv, ctx),
      dtpv    (tool.m_dtpv, ctx),
      dtzv    (tool.m_dtzv, ctx)
{
}

StatusCode
EGElectronAmbiguityTool::addBranches(const EventContext& ctx) const
{

  DecorHandles dh (*this, ctx);

  static const SG::AuxElement::ConstAccessor<char> aidCut(m_idCut);

  // retrieve primary vertex
  const xAOD::Vertex* pvtx(nullptr);
  SG::ReadHandle<xAOD::VertexContainer> inputVtx{ m_VtxContainerName, ctx };
  if (inputVtx.ptr()){
    const xAOD::VertexContainer* vtxC = inputVtx.ptr();
    for (const auto* vertex : *vtxC) {
      if (vertex->vertexType() == xAOD::VxType::VertexType::PriVtx) {
        pvtx = vertex;
        break;
      }
    }
  }

  // retrieve electron container
  SG::ReadHandle<xAOD::ElectronContainer> inputElectrons{ m_containerName,
                                                          ctx };
  const xAOD::ElectronContainer* eleC = inputElectrons.ptr();

  if (!eleC) {
    ATH_MSG_ERROR(
      "Couldn't retrieve Electron container with key: " << m_containerName);
    return StatusCode::FAILURE;
  }
  if (!pvtx) {
    ATH_MSG_DEBUG("No primary vertex found. Setting default values.");
    for (const xAOD::Electron* iele : *eleC) {
      dh.drv(*iele) = -1;
      dh.dphiv(*iele) = -1;
      dh.dmee(*iele) = -1;
      dh.dmeeVtx(*iele) = -1;
      dh.dsep(*iele) = -1;
      dh.dambi(*iele) = -1;
      dh.dtrv(*iele) = -1;
      dh.dtpv(*iele) = -1;
      dh.dtzv(*iele) = -1;
    }
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Pvx z = " << pvtx->z() << ", number of electrons "
                           << eleC->size());

  // Make a container of selected tracks : with Si hits, close to electron track
  SG::ReadHandle<xAOD::TrackParticleContainer> inputTrackParticles{
    m_tpContainerName, ctx
  };
  const xAOD::TrackParticleContainer* idtpC = inputTrackParticles.ptr();

  std::set<const xAOD::TrackParticle*> alreadyStored;
  std::set<const xAOD::TrackParticle*> eleIDtpStored, eleGSFtpStored;
  auto closeByTracks =
    std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(
      SG::VIEW_ELEMENTS);

  for (const auto* ele : *eleC) {

    dh.dambi(*ele) = -1;

    // Electron preselection
    if (ele->pt() < m_elepTCut ||
        m_idCut.empty() || !aidCut.isAvailable(*ele) || !aidCut(*ele))
      continue;

    // Just for debug
    const xAOD::TrackParticle* eleGSFtp = ele->trackParticle();
    eleGSFtpStored.insert(eleGSFtp);

    const xAOD::TrackParticle* eleIDtp =
      xAOD::EgammaHelpers::getOriginalTrackParticle(ele);
    eleIDtpStored.insert(eleIDtp);

    // The loop on track
    for (const auto* tp : *idtpC) {

      // Keep the electron track (I build a container to run vertexing on it...)
      if (tp == eleIDtp) {
        closeByTracks->push_back(tp);
        alreadyStored.insert(tp);
        continue;
      }

      // potential candidate to store if not already there
      if (alreadyStored.find(tp) != alreadyStored.end())
        continue;

      //// Keep only opposite charge
      // if (tp->charge() * ele->charge() > 0)
      //  continue;

      // Close-by
      double dR = eleIDtp->p4().DeltaR(tp->p4());
      double dz = std::abs(eleIDtp->z0() - tp->z0()) * sin(eleIDtp->theta());
      if (dR >= 0.3 || dz >= m_dzCut)
        continue;

      // With minimum number of Si hits
      if (xAOD::EgammaHelpers::numberOfSiHits(tp) < m_nSiCut)
        continue;

      alreadyStored.insert(tp);

      closeByTracks->push_back(tp);
    }
  }

  if (closeByTracks->empty())
    return StatusCode::SUCCESS;

  if (msgLvl(MSG::DEBUG)) {
    SG::ReadHandle<xAOD::TrackParticleContainer> tpCReadHandle{ m_tpCName,
                                                                ctx };
    const xAOD::TrackParticleContainer* tpC = tpCReadHandle.ptr();

    ATH_MSG_DEBUG("Number of input tracks "
                  << idtpC->size() << " , number of selected close-by tracks "
                  << closeByTracks->size() << " , number of GSF tracks "
                  << tpC->size());
    for (const auto* trk : eleIDtpStored)
      ATH_MSG_DEBUG("ele  ID trk "
                    << trk << " pt = " << trk->pt() * 1e-3
                    << " eta = " << trk->eta() << " phi = " << trk->phi()
                    << " nSi = " << xAOD::EgammaHelpers::numberOfSiHits(trk));
    for (const auto* trk : eleGSFtpStored)
      ATH_MSG_DEBUG("ele GSF trk "
                    << trk << " pt = " << trk->pt() * 1e-3
                    << " eta = " << trk->eta() << " phi = " << trk->phi()
                    << " nSi = " << xAOD::EgammaHelpers::numberOfSiHits(trk));
    for (const xAOD::TrackParticle* trk : *closeByTracks)
      ATH_MSG_DEBUG("closeby trk "
                    << trk << " pt = " << trk->pt() * 1e-3
                    << " eta = " << trk->eta() << " phi = " << trk->phi()
                    << " nSi = " << xAOD::EgammaHelpers::numberOfSiHits(trk));
  }

  for (const auto* ele : *eleC) {

    // Electron preselection
    if (ele->pt() < m_elepTCut ||
        m_idCut.empty() || !aidCut.isAvailable(*ele) || !aidCut(*ele))
      continue;

    // Henri's circles
    if (decorateSimple(dh, closeByTracks, ele, pvtx).isFailure()) {
      ATH_MSG_ERROR("Cannot decorate the electron with the simple info");
      return StatusCode::FAILURE;
    }

  } // loop on electrons to decorate

  return StatusCode::SUCCESS;
}
}

StatusCode
DerivationFramework::EGElectronAmbiguityTool::decorateSimple(
  DecorHandles& dh,
  std::unique_ptr<ConstDataVector<xAOD::TrackParticleContainer>>& tpC,
  const xAOD::Electron* ele,
  const xAOD::Vertex* pvtx) const
{
  // This is the GSF electron track
  const xAOD::TrackParticle* eleGSFtrkP = ele->trackParticle();

  // And the ID one
  const xAOD::TrackParticle* eleIDtrkP =
    xAOD::EgammaHelpers::getOriginalTrackParticle(ele);

  // For the time being, use the ID one, to be consistent when we only find a
  // good ID to make a conversion and no GSF
  bool useGSF = false; // hardcoded because it seems we will not use true. Kept
                       // for the time being, as not 100% sure
  const xAOD::TrackParticle* eletrkP = useGSF ? eleGSFtrkP : eleIDtrkP;

  ATH_MSG_DEBUG("Electron pt = " << ele->pt() * 1e-3 << " eta = " << ele->eta()
                                 << " phi = " << ele->phi() << " GSF trk ptr = "
                                 << eleGSFtrkP << " ID trk ptr " << eleIDtrkP);

  if (m_isMC) {
    const xAOD::TruthParticle* truthEl =
      xAOD::TruthHelpers::getTruthParticle(*ele);
    double tpvr = -1, tpvp = 9e9, tpvz = 9e9;
    if (truthEl && MC::isElectron(truthEl) &&
        truthEl->prodVtx() != nullptr) {
      tpvr = truthEl->prodVtx()->perp();
      tpvp = truthEl->prodVtx()->phi();
      tpvz = truthEl->prodVtx()->z();
    }
    dh.dtrv(*ele) = tpvr;
    dh.dtpv(*ele) = tpvp;
    dh.dtzv(*ele) = tpvz;
  }

  // Find the closest track particle with opposite charge and a minimum nb of Si
  // hits
  const xAOD::TrackParticle* otrkP(nullptr);
  double detaMin = 9e9;
  for (const xAOD::TrackParticle* tp : *tpC) {
    // Keep only opposite charge
    if (tp->charge() * eletrkP->charge() > 0)
      continue;

    // Close-by
    double dR = eletrkP->p4().DeltaR(tp->p4());
    double dz = std::abs(eletrkP->z0() - tp->z0()) * sin(eletrkP->theta());
    if (dR >= 0.3 || dz >= m_dzCut)
      continue;

    double deta = std::abs(eletrkP->eta() - tp->eta());
    if (deta < detaMin) {
      otrkP = tp;
      detaMin = deta;
    }
  }

  double rv = -9e9;
  double pv = -9e9;
  double mee = -1.;
  double meeAtVtx = -1.;
  double sep = -9e9;
  bool goodConv = false;

  if (otrkP) {

    // To be consistent with the other, use the ID track.
    TLorentzVector ep4;
    ep4.SetPtEtaPhiM(eletrkP->pt(), eletrkP->eta(), eletrkP->phi(), ParticleConstants::electronMassInMeV);

    // Maybe could see if a GSF tp exists for this ID tp and use it if yes ?
    TLorentzVector op4;
    op4.SetPtEtaPhiM(otrkP->pt(), otrkP->eta(), otrkP->phi(), ParticleConstants::electronMassInMeV);

    // Simple masses
    mee = (ep4 + op4).M();
    op4.SetPhi(eletrkP->phi());
    meeAtVtx = (ep4 + op4).M();

    // And the conversion point
    std::vector<double> helix1, helix2;
    helix1.resize(5);
    helix2.resize(5);
    helix(eletrkP, pvtx, helix1);
    helix(otrkP, pvtx, helix2);

    double beta(0.);
    if (helix1[4] < helix2[4])
      beta = TMath::PiOver2() - helix1[4];
    else
      beta = TMath::PiOver2() - helix2[4];

    double phi1(helix1[4] + beta);
    if (phi1 > TMath::TwoPi())
      phi1 -= TMath::TwoPi();
    if (phi1 < 0.)
      phi1 += TMath::TwoPi();

    double phi2(helix2[4] + beta);
    if (phi2 > TMath::TwoPi())
      phi2 -= TMath::TwoPi();
    if (phi2 < 0.)
      phi2 += TMath::TwoPi();

    /// HelixToCircle Main Track Electron
    double r1 = 1 / (2. * std::abs(helix1[1]));

    double charge1(1.);
    if (helix1[1] < 0.)
      charge1 = -1.;
    double rcenter1(helix1[3] / charge1 + r1);
    double phicenter1(phi1 + TMath::PiOver2() * charge1);

    double x1 = rcenter1 * cos(phicenter1);
    double y1 = rcenter1 * sin(phicenter1);

    /// HelixToCircle Other Electron Conv Track
    double r2 = 1 / (2. * std::abs(helix2[1]));

    double charge2(1.);
    if (helix2[1] < 0.)
      charge2 = -1.;
    double rcenter2(helix2[3] / charge2 + r2);
    double phicenter2(phi2 + TMath::PiOver2() * charge2);

    double x2 = rcenter2 * cos(phicenter2);
    double y2 = rcenter2 * sin(phicenter2);
    //////

    double dx(x1 - x2);
    if (dx < 1e-9 && dx > 0.)
      dx = 1e-9;
    if (dx > -1e-9 && dx < 0.)
      dx = -1e-9;
    double slope((y1 - y2) / dx);
    double b(y1 - slope * x1);
    double alpha(atan(slope));
    double d(sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
    // only keeping opposite sign option
    double separation = d - r1 - r2;
    double cpx1, cpx2;
    if (x1 > x2) {
      cpx1 = x1 - r1 * cos(alpha);
      cpx2 = x2 + r2 * cos(alpha);
    } else {
      cpx1 = x1 + r1 * cos(alpha);
      cpx2 = x2 - r2 * cos(alpha);
    }

    double temp1 = (cpx1 + cpx2) / 2;
    double temp2 = slope * temp1 + b;
    double convX = cos(beta) * temp1 + sin(beta) * temp2;
    double convY = -sin(beta) * temp1 + cos(beta) * temp2;

    double dct(helix1[0] - helix2[0]);

    ///////
    if (std::abs(separation) < m_sepCut && std::abs(dct) < m_dctCut) {
      goodConv = true;
      sep = separation;
      pv = std::atan2(convY, convX);
      rv = sqrt(convX * convX + convY * convY);
      if (convX * cos(eletrkP->phi()) + convY * sin(eletrkP->phi()) < 0)
        rv *= -1.;
    }
  } else {
    dh.dambi(*ele) = -1;
  }
  dh.drv(*ele) = rv;
  dh.dphiv(*ele) = pv;
  dh.dmee(*ele) = mee;
  dh.dmeeVtx(*ele) = meeAtVtx;
  dh.dsep(*ele) = sep;
  if (goodConv && rv > m_rvECCut && meeAtVtx < m_meeAtVtxECCut)
    dh.dambi(*ele) = 2;
  else if (otrkP) {
    if (mee < m_meeICCut)
      dh.dambi(*ele) = 1;
    else
      dh.dambi(*ele) = 0;
  }
  return StatusCode::SUCCESS;
}

