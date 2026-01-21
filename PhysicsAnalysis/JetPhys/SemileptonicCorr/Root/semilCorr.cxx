/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SemileptonicCorr/semilCorr.h"

using namespace std;

semilCorr::semilCorr(const TString& fIn, const string& /*suffix*/, bool DebugIn){
  m_Debug = DebugIn;
  m_f.reset (TFile::Open(fIn));
  m_etas.push_back(0);
  m_etas.push_back(0.8);
  m_etas.push_back(1.2);
  m_etas.push_back(1.7);
  m_etas.push_back(2.1);
  m_etas.push_back(2.5);

  vector<string> etastr;
  etastr.push_back("_e0");
  etastr.push_back("_e1");
  etastr.push_back("_e2");
  etastr.push_back("_e3");
  etastr.push_back("_e4");

  vector<string> prefix;
  prefix.push_back("corr");
  prefix.push_back("tagSyst");
  prefix.push_back("fragSyst");
  prefix.push_back("decaySyst");
  prefix.push_back("msSyst");
  prefix.push_back("idSyst");
  prefix.push_back("decayRewSyst");
  prefix.push_back("muRewSyst");
  prefix.push_back("corrIncl");

  for(unsigned int j = 0; j<prefix.size(); j++){
    vector<std::unique_ptr<TH1F> > corr;
    for(unsigned int i = 0; i<etastr.size(); i++){
      corr.emplace_back((TH1F*) m_f->Get((prefix[j]+etastr[i]).c_str()));
    }
    m_histos.push_back(std::move(corr));
  }
}


semilCorr::~semilCorr(){
}

float semilCorr::getSemilCorrToIncl(const TLorentzVector& jet,
                                    const TLorentzVector& mu)
{
  return getSemilCorrToIncl(jet,mu,m_histos[0]);
}

float semilCorr::getBjetCorrToIncl(const TLorentzVector& jet,
                                   const TLorentzVector& mu)
{
  return getSemilCorrToIncl(jet,mu,m_histos[8]);
}

float semilCorr::getSemilCorrToIncl(const TLorentzVector& jet,
                                    const TLorentzVector& mu,
				    const vector<std::unique_ptr<TH1F> >& histos)
{
  TLorentzVector jetmu = jet+mu;
  //correction to get things to 1 (or to pttruth), not to reference
  float corr = getResponse(jetmu.Pt(), jetmu.Eta(), histos);  
  return corr;
}

vector<int> semilCorr::getHistoIndices(semilCorr::Systematics syst)
{
  vector<int> indices;
  if(syst == semilCorr::ALL){
    indices.push_back(((int)semilCorr::TAGGINGWEIGHT));
    indices.push_back(((int)semilCorr::FRAGMENTATION));
    indices.push_back(((int)semilCorr::DECAY));
    indices.push_back(((int)semilCorr::MSRESO));
    indices.push_back(((int)semilCorr::IDRESO));
    indices.push_back(((int)semilCorr::DECAYREW));
    indices.push_back(((int)semilCorr::MUONSPECTRUM));
  }else
    indices.push_back((int) syst);
  return indices;
}

float semilCorr::getSemilCorrToInclSyst(const TLorentzVector& jet,
                                        const TLorentzVector& mu, 
                                        semilCorr::Systematics syst)
{
  //vector<int> indices = getHistoIndices(up,syst);
  vector<int> indices1 = getHistoIndices(syst);
  float systr = 0;
  for(unsigned int i = 0; i<indices1.size(); i++){
    systr += pow(getSemilCorrToIncl(jet,mu,m_histos[indices1[i]]),2);
  }
  return sqrt(systr);
}

float semilCorr::getResponse(float pt, float eta, const vector<std::unique_ptr<TH1F> >& h)
{
  float usePt = pt;
  int histbin = -1;
  for(unsigned int i = 0; i<m_etas.size()-1; i++){
    if(fabs(eta)>=m_etas[i] && fabs(eta)<m_etas[i+1])
      histbin = i;
  }
  
  if(histbin == -1) histbin = h.size()-1;
  float min = h[histbin]->GetBinCenter(1);
  float max = h[histbin]->GetBinCenter(h[histbin]->GetNbinsX());
  if(pt>max)
    usePt = max-0.001;
  if(pt<min) usePt = min+0.001;
  float result = h[histbin]->Interpolate(usePt);
  return result;
}

