/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//
//      2014-06-28 Author: Remi Lafaye (Annecy) 
//
/////////////////////////////////////////////////////////////

#ifndef egammaMonToolBase_H
#define egammaMonToolBase_H

#include <vector>
#include "TProfile.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TString.h"

class TH1;
class TH2;
class TString;
class TProfile;
class StoreGateSvc;

struct egammaBaseHist {
  std::string m_nameOfEgammaType {};

  // Global panel histograms
  TH1 *m_hN {};                // Histogram for number of egammas
  TH1 *m_hEt {};               // Histogram for egamma transverse energies
  TH1 *m_hEta {};              // Histogram for egamma eta
  TH1 *m_hPhi {};              // Histogram for egamma phi
  TH2 *m_hEtaPhi {};           // Histogram for egamma eta,phi
  TH2 *m_hEtaPhi4GeV {};       // Histogram for egamma eta,phi (only candidates with a pt greater than 4 GeV)
  TH2 *m_hEtaPhi20GeV {};      // Histogram for egamma eta,phi (only candidates with a pt greater than 20 GeV)
  TH1 *m_hTopoEtCone40 {};     // Histogram for egamma isolation energy TopoEtcone40 
  TH1 *m_hPtCone20 {};         // Histogram for egamma isolation energy PtCone20 
  TH1 *m_hTime {};             // Histogram for egamma cluster time
  
  // egamma ID per region histograms
  std::vector<TH1*> m_hvEhad1 {};         
  std::vector<TH1*> m_hvCoreEM {};        
  std::vector<TH1*> m_hvF0 {};            
  std::vector<TH1*> m_hvF1 {};            
  std::vector<TH1*> m_hvF2 {};            
  std::vector<TH1*> m_hvF3 {};            
  std::vector<TH1*> m_hvRe233e237 {};     
  std::vector<TH1*> m_hvRe237e277 {};     
  
  // egamma per region histograms
  std::vector<TH1*> m_hvN {};           // Histograms for number of egammas
  std::vector<TH1*> m_hvEt {};          // Histograms for egamma transverse energies
  std::vector<TH1*> m_hvEta {};         // Histograms for egamma transverse eta
  std::vector<TH1*> m_hvPhi {};         // Histograms for egamma transverse phi
  std::vector<TH1*> m_hvTopoEtCone40 {};  // Histograms for egamma calo-based isolation transverse energies
  std::vector<TH1*> m_hvPtCone20 {};      // Histograms for egamma track-based isolation transverse energies
  std::vector<TH1*> m_hvTime {};          // Histograms for egamma times
  
  // Monitoring per lumiblock
  unsigned int m_lumiBlockNumber {};
  
  TH1 *m_hLB_N {}; // Histogram for number of egamma vs LB

  egammaBaseHist(std::string name)
    : m_nameOfEgammaType (std::move(name))
  {
  }
};

class egammaMonToolBase : public ManagedMonitorToolBase
{
 public:
  
  egammaMonToolBase(const std::string& type, const std::string& name, const IInterface* parent); 
  
  virtual ~egammaMonToolBase();
  
  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms() override;
  virtual StatusCode fillHistograms() override;
  virtual StatusCode procHistograms() override;

 protected:

  enum { BARREL=0, CRACK, ENDCAP, FORWARD, NREGION };

  bool hasBadLar(const EventContext& ctx);
  static int  GetRegion(float eta);
  static int  GetForwardRegion(float eta);
  void bookTH1F(TH1* &h, MonGroup& mygroup, const std::string& hname, const std::string& htitle, int nbins, float low, float high);
  void bookTH1F(TH1* &h, MonGroup& mygroup, const std::string& hname_prefix, const std::string& htitle_prefix, int nbins, float low, float high, std::string &nameOfEgammaType);
  void bookTH2F(TH2* &h, MonGroup& mygroup, const std::string& hname, const std::string& htitle, int nbinsx, float xlow, float xhigh, int nbinsy, float ylow, float yhigh);
  void bookTH2F(TH2* &h, MonGroup& mygroup, const std::string& hname_prefix, const std::string& htitle_prefix, int nbinsx, float xlow, float xhigh, int nbinsy, float ylow, float yhigh, std::string &nameOfEgammaType);
  void bookTProfile(TProfile* &h, MonGroup& mygroup, const std::string& hname, const std::string& htitle, int nbins, float xlow, float xhigh, float ylow, float yhigh);
  void bookTH1FperRegion(std::vector<TH1*> &vhist, MonGroup& mygroup, const std::string& hname, const std::string& htitle, int nbins, float low, float high, unsigned int min_region, unsigned int max_region);
  void bookTH1FperRegion(std::vector<TH1*> &vhist, MonGroup& mygroup, const std::string& hname_prefix, const std::string& htitle, int nbins, float low, float high, unsigned int min_region, unsigned int max_region, std::string &nameOfEgammaType);
  void bookTH2FperRegion(std::vector<TH2*> &vhist, MonGroup& mygroup, const std::string& hname, const std::string& htitle, int nbinsx, float xlow, float xhigh, int nbinsy, float ylow, float yhigh, unsigned int min_region, unsigned int max_region);
  void bookTH2FperRegion(std::vector<TH2*> &vhist, MonGroup& mygroup, const std::string& hname_prefix, const std::string& htitle_prefix, int nbinsx, float xlow, float xhigh, int nbinsy, float ylow, float yhigh, unsigned int min_region, unsigned int max_region, std::string &nameOfEgammaType);
  static void fillTH1FperRegion(std::vector<TH1*> &vhist, unsigned int ir, float x);
  static void fillTH2FperRegion(std::vector<TH2*> &vhist, unsigned int ir, float x, float y);
  void fillEfficiencies(TH1* h, TH1* href);
  bool hasGoodTrigger(const std::string& comment);
  unsigned int getCurrentLB(const EventContext& ctx);

  std::vector<std::string> m_Trigger; // generic Trigger Name
  ToolHandle<Trig::TrigDecisionTool> m_trigdec; // Trigger Decision Tool Handle
  bool m_UseTrigger; // Use Trigger ?

  std::string m_GroupExtension;

  unsigned int  m_currentLB;
  SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{this, "EventInfoKey", "EventInfo"};

 private:
  std::vector<std::string> m_region;
};

#endif
