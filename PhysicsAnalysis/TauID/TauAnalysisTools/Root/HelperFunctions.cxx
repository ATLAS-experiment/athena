/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>

// local include(s)
#include "TauAnalysisTools/HelperFunctions.h"
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/ParticleConstants.h"
#include "TF1.h"

#ifdef ASGTOOL_ATHENA
#include "CLHEP/Units/SystemOfUnits.h"
using CLHEP::GeV;
#else
#define GeV 1000
#endif

using namespace TauAnalysisTools;

//______________________________________________________________________________
void TauAnalysisTools::split(const std::string& sInput, const char cDelim, std::vector<std::string>& vOut)
{
  std::stringstream sSS(sInput);
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(sItem);
}

//______________________________________________________________________________
void TauAnalysisTools::split(TEnv& rEnv, const std::string& sKey, const char cDelim, std::vector<std::string>& vOut)
{
  std::stringstream sSS(rEnv.GetValue(sKey.c_str(),""));
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(sItem);
}

//______________________________________________________________________________
void TauAnalysisTools::split(TEnv& rEnv, const std::string& sKey, const char cDelim, std::vector<size_t>& vOut)
{
  std::stringstream sSS(rEnv.GetValue(sKey.c_str(),""));
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(size_t(stoi(sItem)));
}

//______________________________________________________________________________
void TauAnalysisTools::split(TEnv& rEnv, const std::string& sKey, const char cDelim, std::vector<int>& vOut)
{
  std::stringstream sSS(rEnv.GetValue(sKey.c_str(),""));
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(stoi(sItem));
}

//______________________________________________________________________________
void TauAnalysisTools::split(TEnv& rEnv, const std::string& sKey, const char cDelim, std::vector<unsigned>& vOut)
{
  std::stringstream sSS(rEnv.GetValue(sKey.c_str(),""));
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(stoi(sItem));
}

//______________________________________________________________________________
void TauAnalysisTools::split(TEnv& rEnv, const std::string& sKey, const char cDelim, std::vector<float>& vOut)
{
  std::stringstream sSS(rEnv.GetValue(sKey.c_str(),""));
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(stof(sItem));
}

//______________________________________________________________________________
void TauAnalysisTools::split(TEnv& rEnv, const std::string& sKey, const char cDelim, std::vector<double>& vOut)
{
  std::stringstream sSS(rEnv.GetValue(sKey.c_str(),""));
  std::string sItem;
  while (std::getline(sSS, sItem, cDelim))
    vOut.push_back(stod(sItem));
}
//______________________________________________________________________________
double TauAnalysisTools::tauPt(const xAOD::TauJet& xTau)
{
  // return tau pt in GeV
  return xTau.pt()/1000.;
}

//______________________________________________________________________________
double TauAnalysisTools::tauP(const xAOD::TauJet& xTau)
{
  // return tau P in GeV
  return xTau.p4().P()/1000.;
}

//______________________________________________________________________________
double TauAnalysisTools::tauEta(const xAOD::TauJet& xTau)
{
  // return tau eta
  return xTau.eta();
}

//______________________________________________________________________________
double TauAnalysisTools::tauAbsEta(const xAOD::TauJet& xTau)
{
  // return absolute tau eta
  return std::abs(xTau.eta());
}

//______________________________________________________________________________
double TauAnalysisTools::finalTauPt(const xAOD::TauJet& xTau)
{
  // return MVA based tau pt in GeV
  return xTau.ptFinalCalib()/GeV;
}

//______________________________________________________________________________
double TauAnalysisTools::finalTauEta(const xAOD::TauJet& xTau)
{
  // return MVA based tau eta
  return xTau.etaFinalCalib();
}

//______________________________________________________________________________
double TauAnalysisTools::finalTauAbsEta(const xAOD::TauJet& xTau)
{
  // return MVA based absolute tau eta
  return std::abs(xTau.etaFinalCalib());
}

//______________________________________________________________________________
double TauAnalysisTools::finalTauP(const xAOD::TauJet& xTau)
{
  // return tau P in GeV
  return xTau.p4(xAOD::TauJetParameters::FinalCalib).P()/GeV;
}

//______________________________________________________________________________
double TauAnalysisTools::tauLeadTrackEta(const xAOD::TauJet& xTau)
{
  // return leading charge tau track eta
  double dTrackEta = 0.;
  double dTrackMaxPt = 0.;
  for( unsigned int iNumTrack = 0; iNumTrack < xTau.nTracks(); iNumTrack++)
  {
    if (xTau.track(iNumTrack)->pt() > dTrackMaxPt)
    {
      dTrackMaxPt = xTau.track(iNumTrack)->pt();
      dTrackEta = xTau.track(iNumTrack)->eta();
    }
  }
  return dTrackEta;
}

//______________________________________________________________________________
double TauAnalysisTools::truthTauPt(const xAOD::TauJet& xTau)
{
  // return truth tau Pt in GeV
  const xAOD::TruthParticle* xTruthTau = getTruth(xTau);

  // if there is a truth tau return pT, otherwise return 0 (getTruth will print an error)
  static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
  if (xTruthTau!=nullptr && accIsHadronicTau (*xTruthTau))
    return xTruthTau->pt()/GeV;
  else
    return 0.;
}
//______________________________________________________________________________
double TauAnalysisTools::truthVisTauPt(const xAOD::TauJet& xTau)
{
  // return truth visible tau Pt in GeV
  const xAOD::TruthParticle* xTruthTau = xAOD::TauHelpers::getTruthParticle(&xTau);

  // if there is a truth tau return visible pT, otherwise return 0 (getTruth will print an error)
  static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
  static const SG::ConstAccessor<double> accPtVis("pt_vis");
  if (xTruthTau!=nullptr && accIsHadronicTau (*xTruthTau))
    return (accPtVis(*xTruthTau)/GeV);
  else
    return 0.; 
}
//______________________________________________________________________________
double TauAnalysisTools::truthTauAbsEta(const xAOD::TauJet& xTau)
{
  // return truth tau absolute eta
  const xAOD::TruthParticle* xTruthTau = getTruth(xTau);

  // if there is a truth tau return absolute eta, otherwise return -5 (getTruth will print an error)
  static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
  if (xTruthTau!=nullptr && accIsHadronicTau (*xTruthTau))
    return std::abs(xTruthTau->eta());
  else
    return -5.;
}

//______________________________________________________________________________
double TauAnalysisTools::truthDecayMode(const xAOD::TauJet& xTau)
{
  // return truth tau decay mode.
  int iDecayMode = getTruthDecayMode(xTau);
  return static_cast<double>(iDecayMode);
}

//______________________________________________________________________________
const xAOD::TruthParticle* TauAnalysisTools::getTruth(const xAOD::TauJet& xTau)
{
  typedef ElementLink< xAOD::TruthParticleContainer > Link_t;
  static const SG::ConstAccessor<Link_t> accTruthParticleLink("truthParticleLink");
  if (!accTruthParticleLink(xTau))
  {
    Error("TauAnalysisTools::getTruth", "No truth match information available. Please run TauTruthMatchingTool first");
  }

  const Link_t xTruthTauLink = accTruthParticleLink(xTau);
  const xAOD::TruthParticle* xTruthTau = xTruthTauLink.cachedElement();

  return xTruthTau;
}


//______________________________________________________________________________
xAOD::TauJetParameters::DecayMode TauAnalysisTools::getTruthDecayMode(const xAOD::TauJet& xTau)
{
  const xAOD::TruthParticle* xTruthTau = getTruth(xTau);

  static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
  if (xTruthTau!=nullptr && accIsHadronicTau(*xTruthTau))
    return getTruthDecayMode(*xTruthTau);
  else
    return xAOD::TauJetParameters::Mode_Error;
}

//______________________________________________________________________________
xAOD::TauJetParameters::DecayMode TauAnalysisTools::getTruthDecayMode(const xAOD::TruthParticle& xTruthTau)
{
  static const SG::ConstAccessor<size_t> accNumCharged ("numCharged");
  if (!(accNumCharged.isAvailable(xTruthTau)))
  {
    // passed truth particle is not a truth tau
    return xAOD::TauJetParameters::Mode_Error;
  }

  int iCharged = getNTauDecayParticles(xTruthTau,MC::PIPLUS, true) + getNTauDecayParticles(xTruthTau,MC::KPLUS, true);
  int iNeutral = getNTauDecayParticles(xTruthTau,MC::PI0, true);
  if (iCharged == 1)
  {
    if (iNeutral == 0) return xAOD::TauJetParameters::DecayMode::Mode_1p0n;
    if (iNeutral == 1) return xAOD::TauJetParameters::DecayMode::Mode_1p1n;
    if (iNeutral >= 2) return xAOD::TauJetParameters::DecayMode::Mode_1pXn;
  }
  else if (iCharged == 3)
  {
    if (iNeutral == 0) return xAOD::TauJetParameters::DecayMode::Mode_3p0n;
    if (iNeutral >= 1) return xAOD::TauJetParameters::DecayMode::Mode_3pXn;
  }

  if (iCharged == 2 or iCharged == 4 or iCharged == 5)
    return xAOD::TauJetParameters::DecayMode::Mode_Other;
  if (iCharged == 0 or iCharged >=6)
    return xAOD::TauJetParameters::DecayMode::Mode_NotSet;

  // if you got here, something should have gone wrong
  return xAOD::TauJetParameters::DecayMode::Mode_Error;
}

//______________________________________________________________________________
int TauAnalysisTools::getNTauDecayParticles(const xAOD::TruthParticle& xTruthTau, int iPdgId, bool bCompareAbsoluteValues)
{
  int iNum = 0;
  static const SG::ConstAccessor<std::vector<int> > accDecayModeVector("DecayModeVector");
  if (!accDecayModeVector.isAvailable(xTruthTau))
  {
    Warning("TauAnalysisTools::getNTauDecayParticles", "passed truth particle is not a truth tau, return 0");
    return 0;
  }

  for(auto iPdgId2 : accDecayModeVector(xTruthTau))
    if (!bCompareAbsoluteValues)
    {
      if (iPdgId2 == iPdgId) iNum++;
    }
    else
    {
      if (std::abs(iPdgId2) == std::abs(iPdgId)) iNum++;
    }
  return iNum;
}

//______________________________________________________________________________
bool TauAnalysisTools::testFileForEOFContainsCharacters(const std::string& sFileName)
{
  // returns true if last line in file is empty or the line starts with the
  // number sign #

  std::ifstream fInputFile;
  fInputFile.open(sFileName);
  if(!fInputFile.is_open())
    return true;

  fInputFile.seekg(-1,fInputFile.end);

  bool bKeepLooping = true;
  while(bKeepLooping)
  {
    char ch;
    fInputFile.get(ch);

    if(static_cast<int>(fInputFile.tellg()) <= 1)
    {
      fInputFile.seekg(0);
      bKeepLooping = false;
    }
    else if(ch == '\n')
      bKeepLooping = false;
    else
      fInputFile.seekg(-2,fInputFile.cur);
  }

  std::string sLastLine;
  getline(fInputFile,sLastLine);
  fInputFile.close();

  return (sLastLine.size() == 0 or sLastLine[0] == '#');
}

//______________________________________________________________________________
void TauAnalysisTools::createPi0Vectors(const xAOD::TauJet* xTau, std::vector<TLorentzVector>& vPi0s)
{
  // reset the pi0s
  vPi0s.clear();

  // Since the PFO links as they come out of reconstruction, only correspond to
  // calorimeter clusters, whereas we want the consts_pi0 vectors to correspond
  // to real pi0s, we need to be careful to collect the PFOs correctly to pi0s
  // for the cases where number of pi0s does not match to the decay mode:
  size_t iNumPi0PFO = xTau->nPi0PFOs();

  int iDecayMode = -1;

  if (!(xTau->panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_DecayMode, iDecayMode)))
  {
    Error("TauAnalysisTools::createPi0Vectors", "Failed to retrieve panTauDetail decay mode.");
    return;
  }

  if (iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_1p1n && iNumPi0PFO > 1)
  {
    // float fMassPi0 = ParticleConstants::piZeroMassInMeV;
    float fMassPi0Squared = ParticleConstants::piZeroMassInMeV*ParticleConstants::piZeroMassInMeV;

    // combine both photons (with 0 mass from Pantau) to one pi0 vector:
    const xAOD::PFO* xPfo1 = xTau->pi0PFO(0);
    const xAOD::PFO* xPfo2 = xTau->pi0PFO(1);
    vPi0s.push_back(xPfo1->p4() + xPfo2->p4());

    // re-set the mass to one pi0:
    double dNewMomentum = std::sqrt(vPi0s[0].E() * vPi0s[0].E() - fMassPi0Squared);
    vPi0s[0].SetPxPyPzE(vPi0s[0].Vect().Unit().Px() * dNewMomentum,
                        vPi0s[0].Vect().Unit().Py() * dNewMomentum,
                        vPi0s[0].Vect().Unit().Pz() * dNewMomentum,
                        vPi0s[0].E());
  }
  else if (iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_1pXn && iNumPi0PFO == 1)
  {
    // make a single pi0 from a PFO that contains two pi0s:
    const xAOD::PFO* xPfo = xTau->pi0PFO(0);
    // add the 2-pi0 vector preliminarily to the pi0vector:
    vPi0s.push_back(xPfo->p4());

    // re-set the mass back to one pi0:
    double dNewMomentum = std::sqrt(vPi0s[0].E() / 2 * vPi0s[0].E() / 2 - vPi0s[0].M() / 2. * vPi0s[0].M() / 2.);
    vPi0s[0].SetVectM(vPi0s[0].Vect() * (dNewMomentum / vPi0s[0].P()), vPi0s[0].M() / 2.);

    // create another pi0 from the same vector:
    vPi0s.push_back(vPi0s[0]);
  }
  else
  {
    // if it's not any of the special cases above then just collect the PFOs:
    for (size_t iPFO = 0; iPFO < iNumPi0PFO; iPFO++)
    {
      vPi0s.push_back(xTau->pi0PFO(iPFO)->p4());
    }
  }
}


//______________________________________________________________________________
void TauAnalysisTools::correctedPi0Vectors(const xAOD::TauJet* xTau, std::vector<TLorentzVector>& correctedPi0s, TLorentzVector& TauP4){
  //reset the pi0s
  correctedPi0s.clear();

  int iDecayMode = -1;  

  if (!(xTau->panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_DecayMode, iDecayMode)))
  {
    Error("TauAnalysisTools::correctedPi0Vectors", "Failed to retrieve panTauDetail decay mode.");
    return;
  }

  //Reading in the pi0 vector from createPi0Vectors
  std::vector<TLorentzVector> vPi0s;
  createPi0Vectors(xTau,vPi0s);

  if(iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_1p1n || iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_1pXn || iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_3pXn){  
    //Adding up Pi0 P4s from createPi0Vectors
    TLorentzVector Sum_vPi0s;
    for(unsigned int i = 0; i < vPi0s.size() ; i++){
      Sum_vPi0s += vPi0s[i];
    }
    
    //Get sum of the chargedPFO (i.e. tau track) p4
    TLorentzVector Sum_ChrgPFOP4;
    for(const xAOD::TauTrack* track : xTau->tracks()) {
      Sum_ChrgPFOP4 += track->p4();
    }
    
    //Get tau FinalCalib P4 (explicitly requiring p4(xAOD::TauJetParameters::TauCalibType::FinalCalib) should be superfluous, as FinalCalib is the default p4)
    TLorentzVector FinalCalibP4 = xTau->p4();
    
    //Calculate the difference 3-vector between FinalCalib and Sum of chargedPFOP4
    double px = FinalCalibP4.Px() - Sum_ChrgPFOP4.Px();
    double py = FinalCalibP4.Py() - Sum_ChrgPFOP4.Py();
    double pz = FinalCalibP4.Pz() - Sum_ChrgPFOP4.Pz();

    double p_correctedPi0s = std::sqrt( std::pow(px,2.) + std::pow(py,2.) + std::pow(pz,2.) );
    double p_vPi0s = Sum_vPi0s.P();

    //Calucate scale factor for the pi0 3-vector momentum
    double X = p_correctedPi0s/p_vPi0s;

    //Scale the pi0s with X and recalculate the new pi0 energy
    double px_scaled, py_scaled, pz_scaled, e;
    double mPi0 = ParticleConstants::piZeroMassInMeV;
    for(unsigned int i = 0; i < vPi0s.size() ; i++){
      px_scaled = vPi0s[i].Px() * X;
      py_scaled = vPi0s[i].Py() * X;
      pz_scaled = vPi0s[i].Pz() * X;
      e = std::sqrt( std::pow(px_scaled,2.) + std::pow(py_scaled,2.) + std::pow(pz_scaled,2.) + std::pow(mPi0,2.) );

      //Append the corrected pi0P4 to correctedPi0s
      TLorentzVector P4_correctedPi0s;
      P4_correctedPi0s.SetPxPyPzE(px_scaled,py_scaled,pz_scaled,e);
      correctedPi0s.push_back(P4_correctedPi0s);
    }
  }else{
    correctedPi0s = vPi0s;
  }

  //Correct angles between pi0s for 1pXn decays with 1 cluster
  if(iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_1pXn && xTau->nPi0PFOs() == 1){

    //Get Function of Delta R between the two Pi0s
    TF1 DeltaRdist("DeltaRdist", "pol3", 0, 67500);
    DeltaRdist.SetParameter(0, 0.07924);
    DeltaRdist.SetParameter(1, -2.078/1000000.);
    DeltaRdist.SetParameter(2,  2.619/100000000000.);
    DeltaRdist.SetParameter(3, -1.238/10000000000000000.);
  
    //Get Sum of pi0 P4.Pt()
    TLorentzVector SumPi0_P4;
    for( unsigned int i = 0 ; i < correctedPi0s.size() ; i++){
      SumPi0_P4 += correctedPi0s[i];
    }

    float SumPi0_pt = SumPi0_P4.Pt();

    //Get delta R value (mean of true DeltaR distribution)
    float deltaR;
    if(SumPi0_pt >= 67500){
      deltaR = 0.020; // = DeltaRdist.Eval(67500);
    } else{
      deltaR = DeltaRdist.Eval(SumPi0_pt);
    }

    TLorentzVector correctedPi0_0, correctedPi0_1;
    correctedPi0_0.SetPtEtaPhiM( correctedPi0s[0].Pt()/cos(0.5*deltaR/std::sqrt(2.0)), correctedPi0s[0].Eta()+0.5*deltaR/std::sqrt(2.0), correctedPi0s[0].Phi()+0.5*deltaR/std::sqrt(2.0), correctedPi0s[0].M() );
    correctedPi0_1.SetPtEtaPhiM( correctedPi0s[1].Pt()/cos(0.5*deltaR/std::sqrt(2.0)), correctedPi0s[1].Eta()-0.5*deltaR/std::sqrt(2.0), correctedPi0s[1].Phi()-0.5*deltaR/std::sqrt(2.0), correctedPi0s[1].M() );

    std::vector<TLorentzVector> AngleCorrectedPi0s;
    AngleCorrectedPi0s.push_back(correctedPi0_0);
    AngleCorrectedPi0s.push_back(correctedPi0_1);

    //Reparametrise: Delta R -> mass of pi0 Cluster
    TLorentzVector PionCluster_angleCorrected = AngleCorrectedPi0s[0]+AngleCorrectedPi0s[1];
    
    double dNewMomentum = std::sqrt(PionCluster_angleCorrected.E()/2. * PionCluster_angleCorrected.E()/2. - PionCluster_angleCorrected.M() / 2. * PionCluster_angleCorrected.M() / 2.);
    correctedPi0s[0].SetVectM(PionCluster_angleCorrected.Vect() * (dNewMomentum / PionCluster_angleCorrected.P()), PionCluster_angleCorrected.M() / 2.);
    correctedPi0s[1] = correctedPi0s[0];
  }

  //Calculate the new tau P4
  for(const xAOD::TauTrack* track : xTau->tracks()) {
    TauP4 += track->p4();
  }

  for(unsigned int iPi0=0; iPi0 < correctedPi0s.size(); iPi0++) {
    TauP4 += correctedPi0s[iPi0];
  }

}

//______________________________________________________________________________
TruthMatchedParticleType TauAnalysisTools::getTruthParticleType(const xAOD::TauJet& xTau)
{
  typedef ElementLink< xAOD::TruthParticleContainer > Link_t;
  static const SG::ConstAccessor< Link_t > accTruthParticleLink("truthParticleLink");
  if (!accTruthParticleLink.isAvailable(xTau))
    Error("TauAnalysisTools::getTruthParticleType", "No truth match information available. Please run TauTruthMatchingTool first.");

  const xAOD::TruthParticle* xTruthParticle = xAOD::TauHelpers::getTruthParticle(&xTau);
  if (xTruthParticle)
  {
    if (xTruthParticle->isTau())
      {
      static const SG::ConstAccessor<char> accIsHadronicTau("IsHadronicTau");
      if (static_cast<bool>(accIsHadronicTau(*xTruthParticle)))
        return TruthHadronicTau;
      else
        return TruthLeptonicTau;
    }
    if (xTruthParticle->isMuon())
      return TruthMuon;
    if (xTruthParticle->isElectron())
      return TruthElectron;
  }
  
  // TODO: use const xAOD::Jet* xTruthJet = xAOD::TauHelpers::getLink<xAOD::Jet>(&xTau, "truthJetLink");
  // currently it is unavailable as templated class is not in icc file
  static const SG::ConstAccessor< ElementLink< xAOD::JetContainer > > accTruthJetLink("truthJetLink");
  const ElementLink< xAOD::JetContainer > lTruthParticleLink = accTruthJetLink(xTau);
  if (lTruthParticleLink.isValid())
    return TruthJet;

  return Unknown;
}

TruthMatchedParticleType TauAnalysisTools::getTruthParticleType(const xAOD::DiTauJet& xDiTau)
{
  static const SG::ConstAccessor<char> accIsTruthHadronic("IsTruthHadronic");
  if (!accIsTruthHadronic.isAvailable(xDiTau))
    Error("TauAnalysisTools::getTruthParticleType", "No truth match information available. Please run DiTauTruthMatchingTool first");

  TruthMatchedParticleType eTruthMatchedParticleType = Unknown;

  if (accIsTruthHadronic(xDiTau))
    eTruthMatchedParticleType = TruthHadronicDiTau;

  return eTruthMatchedParticleType;
}

std::vector<const xAOD::TauJet*> TauAnalysisTools::combineTauJetsWithMuonRM(const xAOD::TauJetContainer* taus_std, const xAOD::TauJetContainer* taus_muonRM){
  //! Special function to combine the muon-removed taus with the standard taus. 
  //! This can impact your MET calculation and OLR.
  //! You will know if you need this, otherwise please consult TauCP group.
  // std::string message = "found " + std::to_string(taus_muonRM->size()) + " muon-removal taus";
  // Info("TauAnalysisTools::getTauJetsWithMuonRM", message.c_str());
  std::vector<const xAOD::TauJet*> taus_murm_vec(taus_muonRM->begin(), taus_muonRM->end());
  std::vector<const xAOD::TauJet*> taus_combined;
  static const SG::ConstAccessor<ElementLink<xAOD::TauJetContainer> >
    originalTauJetAcc ("originalTauJet");
  for(const xAOD::TauJet* tau_std : *taus_std){
    auto replacement_itr = std::find_if(taus_murm_vec.begin(), taus_murm_vec.end(), 
      [&](const xAOD::TauJet* tau_murm){
        auto link_to_ori_tau = originalTauJetAcc (*tau_murm);
        if (!link_to_ori_tau.isValid()) { return false; }
        if (*link_to_ori_tau == tau_std){ return true;  }
        return false;
      }
    );
    if (replacement_itr == taus_murm_vec.end()) { taus_combined.push_back(tau_std); }
    else { 
      // message = "replacement found at TauJets_MuonRM index " + std::to_string((*replacement_itr)->index()) + " for TauJets index " +  std::to_string(tau_std->index());
      // Info("TauAnalysisTools::getTauJetsWithMuonRM", message.c_str());
      taus_combined.push_back(*replacement_itr); 
      taus_murm_vec.erase(replacement_itr);
      // message = std::to_string(taus_murm_vec.size()) + " muon-removal taus left";
      // Info("TauAnalysisTools::getTauJetsWithMuonRM", message.c_str());
    }
  }
  // Every muon-removal tau should have been used, otherwise there is a problem.
  assert(taus_murm_vec.empty());
  return taus_combined;
}
