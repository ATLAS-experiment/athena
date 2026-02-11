#ifndef IPHISTOS_H
#define IPHISTOS_H

#include "TFile.h"
#include "IPPerformance/BaseHistos.h"

#include <string>

class IPhistos : public BaseHistos
{  
 public:
  
  IPhistos(const std::string& inputName);  

  void Define3DHistos() override;
  void Define2DHistos() override;
  void Define1DHistos() override {};
  std::vector<std::vector<TH1D*>> get1Dvector() {
	  return m_1D;
  };
  void BookHistograms();
  void FillHistograms(double d0, double z0, double pt, double eta, double phi, int runN, double mu, double jetPt, 
                      std::vector<float> weights, double bsWidth, double deltaR_trk12, std::vector<int> class_satisfied, float intLumi);
  void BuildAxesMap();

  void SaveAdditionalHistos(bool saveAdditionalHistos) { m_saveAdditionalHistos = saveAdditionalHistos; };

 private:

  bool m_saveAdditionalHistos;

  std::vector<TH1D*> m_h_d0;
  std::vector<TH1D*> m_h_z0;
  std::vector<TH1D*> m_h_jetPt;
  std::vector<TH1D*> m_h_bsWidth;
  std::vector<TH1D*> m_h_deltaR_trk12;
  std::vector<TH1D*> m_h_count;
  
  std::vector<std::vector<TH1D*>> m_1D; 
  int m_nbins;
  double m_xmind0; 
  double m_xmaxd0;
  double m_xminz0;
  double m_xmaxz0;
  int m_nbinspt;
  double m_xminpt;
  double m_xmaxpt;
  int m_nbinseta;
  double m_xmineta;
  double m_xmaxeta;
  int m_nbinsmu;
  double m_xminmu;
  double m_xmaxmu;

  std::string m_d0Title;
  std::string m_z0Title;
  std::string m_ptTitle;
  std::string m_etaTitle;
  std::string m_IntLumiTitle;
  std::string m_muTitle;
  std::string m_deltaRTitle;
  std::string m_jetPtTitle;

  // The name of the categories
  std::vector<std::string> m_cats; 

  // The name of the IPs (d0, z0)
  std::vector<std::string> m_ips;     
  
  // The name of the var needs to match the name of the axes. TODO: ADD CHECK
  std::vector<std::string> m_variables;
  
  // The name of the slices
  std::vector<std::string> m_PTSlices;
  std::vector<std::string> m_EtaSlices;

  std::vector<double> m_pt_min; // Lower boundaries of the PTSlices
  std::vector<double> m_pt_max; // Upper boundaries of the PTSlices
  std::vector<double> m_abseta_min; // Lower boundaries of the EtaSlices
  std::vector<double> m_abseta_max; // Upper boundaries of the EtaSlices

};

#endif
