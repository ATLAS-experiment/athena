/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SEMILEPTONICCORR_SEMILCORR_H
#define SEMILEPTONICCORR_SEMILCORR_H

#include "TH2F.h"
#include "TFile.h"
#include <iostream>
#include "TLorentzVector.h"
#include <vector>
#include <memory>


class semilCorr{

public:
  enum Systematics{
    TAGGINGWEIGHT = 1,
    FRAGMENTATION = 2,
    DECAY = 3,
    MSRESO = 4,
    IDRESO = 5,
    DECAYREW = 6,
    MUONSPECTRUM = 7,
    ALL = 8
  };
  
private:
  std::vector<std::vector<std::unique_ptr<TH1F> > > m_histos;
  std::vector<float> m_etas;

  std::unique_ptr<TFile> m_f;

  bool m_Debug;

  float getResponse(float pt, float eta, const std::vector<std::unique_ptr<TH1F> >& h);
  float getSemilCorrToIncl(const TLorentzVector& jet, const TLorentzVector& mu,
			   const std::vector<std::unique_ptr<TH1F> >& histos);
  std::vector<int> getHistoIndices(semilCorr::Systematics syst);


public:
  semilCorr(const TString& fIn, const std::string& suffix = "", bool DebugIn = false);
  ~semilCorr(); 

  float getSemilCorrToIncl(const TLorentzVector& jet, const TLorentzVector& mu);
  float getBjetCorrToIncl(const TLorentzVector& jet, const TLorentzVector& mu);

  float getSemilCorrToInclSyst(const TLorentzVector& jet, const TLorentzVector& mu, 
			       semilCorr::Systematics syst = semilCorr::ALL);

};

#endif
