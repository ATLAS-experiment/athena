#ifndef IPHISTOS_H
#define IPHISTOS_H

#include "IPPerformance/BaseHistos.h"
#include <string>

class IPhistos : public BaseHistos
{  
 public:
  
  IPhistos(const std::string& inputName);  

  void define3DHistos();
  void define2DHistos();

  std::vector<TH1D*> get1Dvector() {
	  return m_1D;
  };
  void BookHistograms();
  void FillHistograms(float d0, float z0, float pt, float eta, float phi, float mu, float jetPt, 
                      float weight, float bsWidth, float deltaR_trk12);
  void BuildAxesMap();

  void SaveAdditionalHistos() { m_saveAdditionalHistos = true; };

 private:

  bool m_saveAdditionalHistos = false;

  TH1D* m_h_d0;
  TH1D* m_h_z0;
  TH1D* m_h_jetPt;
  TH1D* m_h_bsWidth;
  TH1D* m_h_deltaR_trk12;
  TH1D* m_h_count;

  std::vector<TH1D*> m_1D;
  static constexpr int m_nbins = 200;
  static constexpr double m_xmind0 = -400.;
  static constexpr double m_xminz0 = -600.;
  static constexpr double m_xmaxd0 = 400.;
  static constexpr double m_xmaxz0 = 600.;
  static constexpr int m_nbinspt = 25;
  static constexpr double m_xminpt = 0.;
  static constexpr double m_xmaxpt = 25;
  static constexpr int m_nbinseta = 50;
  static constexpr double m_xmaxeta = 2.5;
  static constexpr int m_nbinsmu = 50;
  static constexpr double m_xminmu = 0.;
  static constexpr double m_xmaxmu = 100.;


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
