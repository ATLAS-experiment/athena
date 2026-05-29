#include "IPPerformance/IPhistos.h"

#include "TROOT.h"
#include "TAxis.h"
#include "TKey.h"
#include "TClass.h"
#include <TRandom.h>
#include <TProfile.h>
IPhistos::IPhistos(const std::string& inputName) : BaseHistos(inputName)   
{

  m_name = inputName;
  BuildAxesMap();   

}

void IPhistos::BuildAxesMap()
{
  
  // TODO: automatize? based on IPtree directly
  
  // Impact parameters in consideration
  m_ips.push_back("d0");
  m_ips.push_back("z0");
  
  // pT slices
  m_PTSlices.push_back("PTSlice1");
  m_PTSlices.push_back("PTSlice2");
  
  // eta slices
  m_EtaSlices.push_back("EtaSlice1");
  m_EtaSlices.push_back("EtaSlice2");

  // pT array for pT slices: 
  m_pt_min = {5.0, 30}; // Boundaries for the study of resVSmu
  m_pt_max = {7.0, 50}; 

  // |eta| arrays for Eta slices: Eg- 0-0.8, 2.1-2.5
  m_abseta_min = {0, 2.1};   // Boundaries for the study of resVSmu
  m_abseta_max = {0.8, 2.5}; 

  //------------------------------------ List of variables ---------------------------------------
  m_variables = {"pt", "pTas", "eta", "eta_as", "phi", "mu",  "jetPt_as"};
  //----------------------------------------------------------------------------------------------

  // PT Asymmetric Axis
  std::vector<double> axis;
  for (float x = 0.25; x <= 1.5; x += 0.25) axis.push_back(x);
  for (float x = 2.0; x <= 4.0; x += 0.5) axis.push_back(x);
  for (float x = 5.0; x <= 10.0; x += 1.0) axis.push_back(x);
  for (float x = 12.0; x <= 20.0; x += 2.0) axis.push_back(x);
  for (float x = 24.0; x <= 40.0; x += 4.0) axis.push_back(x);
  for (float x = 45.0; x <= 60.0; x += 5.0) axis.push_back(x);
  axis.insert(axis.end(), {70., 80., 100., 150., 250., 400., 600., 800., 1000.});

  m_Axes["pTas"] = axis;
  axis.clear();

  // JetPT Asymmetric Axis
  for (double x = 60; x <= 100; x += 20) axis.push_back(x);
  for (double x = 120; x <= 300; x += 20) axis.push_back(x);
  for (double x = 340; x <= 500; x += 40) axis.push_back(x);
  for (double x = 550; x <= 700; x += 50) axis.push_back(x);
  axis.insert(axis.end(), {800, 900, 1000, 1200, 1400, 1600, 1800, 2000});

  m_Axes["jetPt_as"] = axis;
  axis.clear();

  // DeltaR Axis: Nbins, x_low, x_up
  //m_Axes.emplace("DR", std::vector<double>{100, 0., 0.2});
  // DeltaR Axis: Nbins, x_low, x_up
  axis.push_back(100);
  axis.push_back(0.);
  axis.push_back(0.2);

  m_Axes["DR"] = axis;
  axis.clear();

 /* for (int i = 0; i <= 100; i++) {
    axis.push_back(0.2 * i / 100.0);
  }
  m_Axes["DR"] = axis;
  axis.clear();*/
  // DeltaR asymmetric Axis
  for (float x = 0.0; x <= 0.044; x += 0.004) axis.push_back(x);
  for (float x = 0.050; x <= 0.080; x += 0.006) axis.push_back(x);
  for (float x = 0.088; x <= 0.112; x += 0.008) axis.push_back(x);
  for (float x = 0.122; x <= 0.162; x += 0.010) axis.push_back(x);
  axis.insert(axis.end(), {0.202, 0.302, 0.402});

  m_Axes["DeltaRas"] = axis;
  m_Axes["dRtrk12as"] = axis;
  axis.clear();

  // Eta axis
  for (float x = -2.5; x <= -1.5; x += 0.5) axis.push_back(x);
  for (float x = -1.25; x <= -1.; x += 0.25) axis.push_back(x);
  for (float x = -0.8; x <= -0.4; x += 0.2) axis.push_back(x);
  for (float x = -0.3; x <= -0.1; x += 0.1) axis.push_back(x);
  axis.push_back(0.0);

  // --- Positive side (mirror, excluding 0) ---
  for (std::vector<double>::reverse_iterator it = axis.rbegin() + 1;
       it != axis.rend(); ++it) {
     axis.push_back(-(*it));
  }

  m_Axes["eta_as"] = axis;
  axis.clear();

  // d0 axis (symmetric)
  //m_Axes.emplace("d0", std::vector<double>{m_nbins, -m_xmaxd0, m_xmaxd0});
  // d0 axis (symmetric)
  axis.push_back(m_nbins);
  axis.push_back(m_xmind0);
  axis.push_back(m_xmaxd0);

  m_Axes["d0"] = axis;
  axis.clear();
  /*for (int i = 0; i <= m_nbins; i++) {
    double x = m_xmind0 + i * (m_xmaxd0 - m_xmind0) / m_nbins;
    axis.push_back(x);
  }
  m_Axes["d0"] = axis;
  axis.clear();*/



  // z0 axis (symmetric)
  //m_Axes.emplace("z0", std::vector<double>{m_nbins, -m_xmaxz0, m_xmaxz0});
  // z0 axis (symmetric)
  axis.push_back(m_nbins);
  axis.push_back(m_xminz0);
  axis.push_back(m_xmaxz0);

 /* for (int i = 0; i <= m_nbins; i++) {
    double x = m_xminz0 + i * (m_xmaxz0 - m_xminz0) / m_nbins;
    axis.push_back(x);
  }*/
  m_Axes["z0"] = axis;
  axis.clear();


  // pT_2D axis
  for (float x = 0.5; x <= 1.; x += 0.1) axis.push_back(x);
  for (float x = 1.2; x <= 1.5; x += 0.3) axis.push_back(x);
  for (float x = 2.; x <= 3.; x += 0.5) axis.push_back(x);
  for (float x = 4.; x <= 5.; x += 1.) axis.push_back(x);
  for (float x = 7.; x <= 15.; x += 2.) axis.push_back(x);
  for (float x = 20.; x <= 60.; x += 5.) axis.push_back(x);
  axis.insert(axis.end(), {80., 100., 120., 140., 160.});

  m_Axes["pt_2d"] = axis;
  axis.clear();        

  // Eta_2D axis
  for (float x = -2.5; x <= 2.5; x += 0.25) axis.push_back(x);
  m_Axes["eta_2d"] = axis;
  axis.clear();         

  // no_of_bins, xmin and xmax should be assigned in their respective orders
  std::vector<std::string> axis_variable = {"eta", "pt", "phi", "mu", "bsWidth", "dRtrk12", "d0_s", "z0_s"};
  std::vector<int> nbin_value = {m_nbinseta, m_nbinspt, 10, m_nbinsmu, 150, 100, m_nbins, m_nbins};
  std::vector<double> xmin_value = {-m_xmaxeta, m_xminpt, -3.14, m_xminmu, 0, 0, -m_xmaxd0, -m_xmaxz0};
  std::vector<double> xmax_value = {m_xmaxeta, m_xmaxpt, 3.14, m_xmaxmu, 15, 0.2, m_xmaxd0, m_xmaxz0};

  // Assigning each variable with their corresponding axis ranges
  for (unsigned int j = 0; j < axis_variable.size(); j++)
  {
    double binW = (xmax_value[j] - xmin_value[j]) / nbin_value[j];
    for (int i=0; i < nbin_value[j]+1; i++) {
      axis.push_back(xmin_value[j] + i*binW);
    }
    m_Axes[axis_variable[j]] = axis;
    axis.clear();
  }

} // End of BuildAxesMap()

void IPhistos::define3DHistos()
{
  
  std::string h_name = "";

    for (unsigned int i_ip = 0; i_ip < m_ips.size(); ++i_ip) {
      
      h_name = m_name + "_" + m_ips[i_ip] +"_vs_" + "trkpTas_vs_eta_as"; 
      m_histos3d[h_name] = plot3D(h_name,
                                "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                                "Track #eta", m_Axes["eta_as"].size() - 1, m_Axes["eta_as"].data(),
                                m_ips[i_ip], m_Axes[m_ips[i_ip] + "_s"].size() - 1, m_Axes[m_ips[i_ip] + "_s"].data());
    
      if (m_saveAdditionalHistos) {
        h_name = m_name + "_" + m_ips[i_ip] +"_vs_" + "trkpTas_vs_deltaRas_trk12";
        m_histos3d[h_name] = plot3D(h_name,
                                  "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                                  "#DeltaR(trk1,trk2)", m_Axes["dRtrk12as"].size() - 1, m_Axes["dRtrk12as"].data(),
                                  m_ips[i_ip], m_Axes[m_ips[i_ip] + "_s"].size() - 1, m_Axes[m_ips[i_ip] + "_s"].data());
        
        h_name = m_name + "_" + m_ips[i_ip] +"_vs_" + "trkpTas_vs_mu";
        m_histos3d[h_name] = plot3D(h_name,
                                  "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                                  "#mu", m_Axes["mu"].size() - 1, m_Axes["mu"].data(),
                                  m_ips[i_ip], m_Axes[m_ips[i_ip] + "_s"].size() - 1, m_Axes[m_ips[i_ip] + "_s"].data());
      }
  
    }

} // End of define3DHistos()

void IPhistos::define2DHistos()
{

  std::string h_name = "";

    h_name = m_name  + "_jetpT_vs_Eta";
    m_histos2d[h_name] = plot2D(h_name,
                              "jetPt [GeV]", m_Axes["jetPt_as"].size() - 1, m_Axes["jetPt_as"].data(),
                              "Track #eta", m_Axes["eta"].size() - 1, m_Axes["eta"].data());
    
    h_name = m_name + "_trackpT_vs_Eta";
    m_histos2d[h_name] = plot2D(h_name,
                              "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                              "Track #eta", m_Axes["eta_as"].size() - 1, m_Axes["eta_as"].data());

    if (m_saveAdditionalHistos) {  
      for (unsigned int i_ip = 0; i_ip < m_ips.size(); ++i_ip){
        for (unsigned int i_var = 0; i_var < m_variables.size(); ++i_var){

          h_name = m_name + "_" + m_ips[i_ip] + "_vs_" + m_variables[i_var];
          m_histos2d[h_name] = plot2D(h_name,
                                    m_variables[i_var], m_Axes[m_variables[i_var]].size() - 1, m_Axes[m_variables[i_var]].data(),
                                    m_ips[i_ip], m_Axes[m_ips[i_ip]][0], m_Axes[m_ips[i_ip]][1], m_Axes[m_ips[i_ip]][2]);
        } // var
      } // IPS

    } // Additional histos

} // End of define2DHistos()

void IPhistos::BookHistograms()
{

  m_h_bsWidth = plot1D(m_name + "h_bsWidth", "bsWidth (xy) [um]", 150, 0, 15);

  if (m_saveAdditionalHistos) {
    m_h_d0        = plot1D(m_name + "h_d0", "d0 [um]", m_nbins, m_xmind0, m_xmaxd0);
    m_h_z0        = plot1D(m_name + "h_z0", "z0 [um]", m_nbins, m_xminz0, m_xmaxz0);
    m_h_jetPt     = plot1D(m_name + "h_jetPt", "jetPt", m_Axes["jetPt_as"].size() - 1, m_Axes["jetPt_as"].data());
    m_h_deltaR_trk12 = plot1D(m_name + "h_deltaR_trk12", "deltaR(trk1,trk2)", m_Axes["DR"][0], m_Axes["DR"][1], m_Axes["DR"][2]);
    m_h_count     = plot1D(m_name + "h_count", "Y-axis: Number of tracks satisfying this class", 5, 0, 5);
  }
  m_1D.push_back(m_h_bsWidth);
  m_1D.push_back(m_h_d0);
  m_1D.push_back(m_h_z0);
  m_1D.push_back(m_h_jetPt);
  m_1D.push_back(m_h_deltaR_trk12);
  m_1D.push_back(m_h_count);
} // End of BookHistograms()

void IPhistos::FillHistograms(float d0, float z0, float pt, float eta, float phi, float mu, float jetPt, float weight, float bsWidth, float deltaR_trk12)
{
    
    float w = weight;
    std::vector<double> ip_values;
    ip_values.push_back(d0);
    ip_values.push_back(z0);

    // Fill 3D histograms - bit ugly/slow. Come up with something better.
    for (it3d it = m_histos3d.begin(); it != m_histos3d.end(); ++it){
      if (it->first.find("Slice") != std::string::npos)
        continue;
      
      double ipValue = -999;
      if (it->first.find("d0") != std::string::npos)
        ipValue = d0;
      else
        ipValue = z0;

      if (it->first.find("trkpTas_vs_eta") != std::string::npos)
        it->second->Fill(pt, eta, ipValue, w);
      if (it->first.find("trkpTas_vs_deltaRas_trk12") != std::string::npos)
        it->second->Fill(pt, deltaR_trk12, ipValue, w);
      if (it->first.find("trkpTas_vs_mu") != std::string::npos)
        it->second->Fill(pt, mu, ipValue, w);
    } // histos3d

    // Fill 2D histograms - bit ugly/slow. Come up with something better.
    for (it2d it = m_histos2d.begin(); it != m_histos2d.end(); ++it){
      if (it->first.find("Slice") != std::string::npos)
        continue;
      
      double ipValue = -999;
      if (it->first.find("d0") != std::string::npos)
        ipValue = d0;
      else
        ipValue = z0;

      if (it->first.find("jetpT_vs_Eta") != std::string::npos)
        it->second->Fill(jetPt, eta, w); 
      if (it->first.find("trackpT_vs_Eta") != std::string::npos)
        it->second->Fill(pt, eta, w); 
      if (it->first.find("0_vs_pt") != std::string::npos)
        it->second->Fill(pt, ipValue, w);
      if (it->first.find("0_vs_pTas") != std::string::npos)
        it->second->Fill(pt, ipValue, w);
      if (it->first.find("0_vs_eta") != std::string::npos)
        it->second->Fill(eta, ipValue, w);
      if (it->first.find("0_vs_phi") != std::string::npos)
        it->second->Fill(phi, ipValue, w);
      if (it->first.find("0_vs_mu") != std::string::npos)
        it->second->Fill(mu, ipValue, w);               
      if (it->first.find("0_vs_jetPt_as") != std::string::npos)
        it->second->Fill(jetPt, ipValue, w);
      if (it->first.find("Pt_vsEta") != std::string::npos)
        it->second->Fill(pt, eta, w);
    }
    
    // Fill 1D histograms
    m_h_bsWidth->Fill(bsWidth * 1000, w); 
    if (m_saveAdditionalHistos){
      m_h_d0->Fill(d0, w);
      m_h_z0->Fill(z0, w);
      m_h_jetPt->Fill(jetPt, w); 
      m_h_deltaR_trk12->Fill(deltaR_trk12, w);
      m_h_count->Fill(1, w);
    }

} // End of FillHistograms()
