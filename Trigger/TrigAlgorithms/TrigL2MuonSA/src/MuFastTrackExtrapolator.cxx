/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuFastTrackExtrapolator.h"
#include "xAODTrigMuon/L2StandAloneMuonAuxContainer.h"
#include "xAODTrigMuon/TrigMuonDefs.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigL2MuonSA::MuFastTrackExtrapolator::MuFastTrackExtrapolator(const std::string& type, 
                                                               const std::string& name,
                                                               const IInterface*  parent): 
  AthAlgTool(type,name,parent)
{
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MuFastTrackExtrapolator::extrapolateTrack(std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns,
                                                                   double winPt) const
{
  ATH_MSG_DEBUG("in extrapolateTrack");
  
  StatusCode sc = StatusCode::SUCCESS;
  
  for (TrigL2MuonSA::TrackPattern& track : v_trackPatterns) {

    const int inner = (track.s_address==-1)? xAOD::L2MuonParameters::Chamber::EndcapInner:
      xAOD::L2MuonParameters::Chamber::BarrelInner;

    xAOD::L2StandAloneMuon* muonSA = new xAOD::L2StandAloneMuon();
    muonSA->makePrivateStore();
    muonSA->setSAddress(track.s_address);
    muonSA->setPt(track.pt*track.charge);
    muonSA->setEtaMS(track.etaMap);
    muonSA->setPhiMS(track.phiMS);
    muonSA->setRMS(track.superPoints[inner].R);
    muonSA->setZMS(track.superPoints[inner].Z);

    double etaVtx = 0.;
    double phiVtx = 0.;
    double sigEta = 0.;
    double sigPhi = 0.;

    const double eptinv = getMuFastRes(track.pt*track.charge, track.s_address, track.etaMap, track.phiMS);

    if (m_backExtrapolatorTool) {

      sc = (*m_backExtrapolatorTool)->give_eta_phi_at_vertex(muonSA, etaVtx, sigEta, phiVtx, sigPhi, winPt);
    
      if (sc.isFailure()) {
        ATH_MSG_DEBUG ("BackExtrapolator problem: "
              << "Pt of Muon Feature out of BackExtrapolator range.");
        ATH_MSG_DEBUG ("Use Muon Feature position to fill the "
		       << "TrigRoiDescriptor for IDSCAN.");
        etaVtx = track.etaMap;
        phiVtx = track.phiMS;
      }

    } else {

      ATH_MSG_ERROR("Null pointer to ITrigMuonBackExtrapolator");
      delete muonSA;
	return StatusCode::FAILURE;

    }

    track.deltaPt     = (eptinv!=0.)? eptinv * track.pt * track.pt: 1.0e10;
    track.etaVtx      = etaVtx;
    track.phiVtx      = phiVtx;
    track.deltaEtaVtx = sigEta;
    track.deltaPhiVtx = sigPhi;

    delete muonSA;
  }
  return sc;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

// Calculation of error on pT from L2 Muon SA
// Original author: Stefano Giagu
// Copied from TrigmuComb/muCombUtil.cxx
double TrigL2MuonSA::MuFastTrackExtrapolator::getMuFastRes(const double pt, const int add,
                                                           const double eta, const double phi) const
{
  
  if (pt == 0) return 1.0e30;
  
  double AbsPtInv = std::abs(1./pt);
  double AbsEta   = std::abs(eta);
  
  if ( add != -1) { // Barrel
    const int N_PARAMS = 3;
    // relative resolution terms; vparBR[0]=constant, vparBR[1]=proportional to AbsPt, vparBR[2]=proportional to AbsPtInv
    const double vparBR1[N_PARAMS] = {0.0495619, 0.00180415, 0.307058}; // for AbsEta < 0.534
    const double vparBR2[N_PARAMS] = {1., 0., 0.}; // for 0.534 < AbsEta < 0.687
    const double vparBR3[N_PARAMS] = {0.0370408, 0.00142206, -0.492544}; // for 0.687 < AbsEta < 1.05
    const double* vpar;

    if     ( AbsEta < 0.534) { vpar = vparBR1; }
    else if( AbsEta < 0.687) { vpar = vparBR2; }
    else                     { vpar = vparBR3; }

    double fracRes = sqrt(pow(vpar[0],2) + pow(vpar[1]/AbsPtInv,2) + pow(vpar[2]*AbsPtInv,2));
    return std::abs(fracRes * AbsPtInv);
  }
  else {//Takuya/Kunihiro updated numbers 
    
    const int N_PARAMS = 5;
    const double vparEC1[N_PARAMS] = {0.291483, -6.11348,  65.1099, -285.664,  440.041};
    const double vparEC2[N_PARAMS] = {0.286307, -4.6759,   43.2815, -163.185,  210.786};
    const double vparEC3[N_PARAMS] = {0.330699, -6.70755,  70.4725, -291.85,   408.739};
    const double vparEC4[N_PARAMS] = {0.261738, -4.69971,  47.4762, -183.98,   236.813};
    const double vparEC5[N_PARAMS] = {0.196301, -3.57276,  38.3744, -159.808,  228.256};
    const double vparEC6[N_PARAMS] = {0.172939, -3.10788,  33.3823, -142.996,  212.957};
    const double vparEC7[N_PARAMS] = {0.233017, -4.377,    42.5691, -171.752,  245.702};
    const double vparEC8[N_PARAMS] = {0.22389,  -4.16259,  40.1369, -162.824,  236.39};
    const double vparEC9[N_PARAMS] = {0.197992, -3.52117,  33.5997, -136.014,  197.474};
    const double vparECA[N_PARAMS] = {0.417289, -0.852254,-31.9257,  308.873, -719.591};
    const double vparECB[N_PARAMS] = {0.526612, -8.04087,  82.1906, -336.87,   462.973};
        
    const double AbsPtInvMin = 5e-3;   // 200 GeV
    const double AbsPtInvMax = 0.25;   //   4 GeV
    if( AbsPtInv < AbsPtInvMin ) AbsPtInv = AbsPtInvMin;
    if( AbsPtInv > AbsPtInvMax ) AbsPtInv = AbsPtInvMax;
      
    const double* vpar;
    xAOD::L2MuonParameters::ECRegions reg = xAOD::L2MuonParameters::whichECRegion(AbsEta,phi);
    if     ( reg ==  xAOD::L2MuonParameters::ECRegions::WeakBFieldA ) { vpar = vparECA; }
      else if( reg == xAOD::L2MuonParameters::ECRegions::WeakBFieldB ) { vpar = vparECB; }
      else {
        if     ( AbsEta < 1.20) { vpar = vparEC1; }
        else if( AbsEta < 1.35) { vpar = vparEC2; }
        else if( AbsEta < 1.50) { vpar = vparEC3; }
        else if( AbsEta < 1.65) { vpar = vparEC4; }
        else if( AbsEta < 1.80) { vpar = vparEC5; }
        else if( AbsEta < 1.95) { vpar = vparEC6; }
        else if( AbsEta < 2.10) { vpar = vparEC7; }
        else if( AbsEta < 2.35) { vpar = vparEC8; }
        else                    { vpar = vparEC9; }
      }
    
    double fracRes = vpar[0] + vpar[1]*AbsPtInv
      + vpar[2]*AbsPtInv*AbsPtInv 
      + vpar[3]*AbsPtInv*AbsPtInv*AbsPtInv
      + vpar[4]*AbsPtInv*AbsPtInv*AbsPtInv*AbsPtInv;
    
    return std::abs(fracRes * AbsPtInv);
  }
}
        
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
