/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_TAUSELECTIONCUTS_H
#define TAUANALYSISTOOLS_TAUSELECTIONCUTS_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  documentation in: https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-TauSelectionTool.rst
*/

// Framework include(s):
#include "xAODTau/TauJet.h"
#include "PATCore/AcceptData.h"
#include "PATCore/AcceptInfo.h"

// ROOT include(s):
#include "TH1F.h"

namespace TauAnalysisTools
{

class TauSelectionTool;

class TauSelectionCut
{

public:
  TauSelectionCut(const std::string& sName, TauAnalysisTools::TauSelectionTool* tTST);
  virtual ~TauSelectionCut();

  void writeControlHistograms();
  void fillHistogramCutPre(const xAOD::TauJet& xTau);
  void fillHistogramCut(const xAOD::TauJet& xTau);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const = 0;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) = 0;
  TH1F* CreateControlPlot(const char* sName, const char* sTitle, int iBins, double dXLow, double dXUp);

  const std::string& getName()
  {
    return m_sName;
  };

  void setProperty(const std::string& name, const std::string& value);

protected:
  std::string m_sName;

  TH1F* m_hHistCutPre;
  TH1F* m_hHistCut;

  TauSelectionTool* m_tTST;

  void declareProperty(const std::string& name, std::string& loc);
  std::map<std::string, std::string&> m_mProperties;
  std::string getProperty(const std::string& name);

private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const = 0;
};

class TauSelectionCutPt
  : public TauSelectionCut
{
public:
  TauSelectionCutPt(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutAbsEta
  : public TauSelectionCut
{
public:
  TauSelectionCutAbsEta(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutAbsCharge
  : public TauSelectionCut
{
public:
  TauSelectionCutAbsCharge(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutNTracks
  : public TauSelectionCut
{
public:
  TauSelectionCutNTracks(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutRNNJetScoreSigTrans
  : public TauSelectionCut
{
public:
  TauSelectionCutRNNJetScoreSigTrans(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutGNTauScoreSigTrans
  : public TauSelectionCut
{
public:
  TauSelectionCutGNTauScoreSigTrans(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutJetIDWP
  : public TauSelectionCut
{
public:
  TauSelectionCutJetIDWP(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutRNNEleScore
  : public TauSelectionCut
{
public:
  TauSelectionCutRNNEleScore(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

class TauSelectionCutEleIDWP
  : public TauSelectionCut
{
public:
  TauSelectionCutEleIDWP(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};

//added by Li-Gang Xia < ligang.xia@cern.ch >
//to remove taus overlapping with muons satisfying pt > 2 GeV and not calo-tagged
class TauSelectionCutMuonOLR
  : public TauSelectionCut
{
public:
  TauSelectionCutMuonOLR(TauSelectionTool* tTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::TauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  bool m_bTauMuonOLR; //False: overlapped, the tau is not kept. True: not overlapped, the tau is kept.)
  virtual void fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const override;
};


}

#endif // TAUANALYSISTOOLS_TAUSELECTIONCUTS_H
