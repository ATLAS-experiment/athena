#include "IPPerformance/IPhistos.h"
#include "IPPerformance/IPHistogramHelpers.h"

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
  m_cats = {"class0", "class1", "class2", "class3", "class4", "class5", "class6", "class7", "class8", "class9", "class10", "class11", "class12", "class13", "class14"};
  //cats = {"class14"}; //change to this for all tracks inclusive 
  
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

  // d0 and z0 bins
  m_nbins = 200;

  // d0 range
  m_xmind0 = -400; // More reasonable range to look at, even at low pt values
  m_xmaxd0 = 400;

  // z0 range
  m_xminz0 = -600;
  m_xmaxz0 = 600;

  // Arrange all the uniformly binned variables as vectors 
  //==> will have to refer to them later while booking the hists with vectors instead of Axes[]

  // pT bins and range
  m_nbinspt = 25;
  m_xminpt = 0;
  m_xmaxpt = 25;

  // Eta bins and range
  m_nbinseta = 50;
  m_xmineta = -2.5;
  m_xmaxeta = 2.5;

  // Pileup bins and range
  m_nbinsmu = 50;
  m_xminmu = 0;
  m_xmaxmu = 100;

  // Titles for the histograms
  m_d0Title = "d_{0} [um]";
  m_z0Title = "z_{0} [um]";
  m_ptTitle = "p_{T} [GeV]";
  m_etaTitle = "#eta";
  m_IntLumiTitle = "Integrated luminosity";
  m_muTitle = "<#mu>";
  m_deltaRTitle = "#DeltaR(trk,jet)";
  m_jetPtTitle = "jet p_{T}";

  // PT Asymmetric Axis
  std::vector<double> axis = {
      0.25, 0.5, 0.75, 1., 1.25, 1.5,
      2., 2.5, 3., 3.5, 4.,
      5., 6., 7., 8., 9., 10.,
      12., 14., 16., 18., 20,
      24., 28., 32., 36., 40,
      45., 50., 55., 60.,
      70., 80., 100., 150, 250, 400, 600, 800, 1000};
  
  m_Axes["pTas"] = axis;
  axis.clear();

  // JetPT Asymmetric Axis
  axis = {
      20, 30, 40, 60, 80, 100, 
      120, 140, 160, 180, 200, 
      220, 240, 260, 280, 300,
      340, 380, 420, 460, 500, 
      550, 600, 650, 700, 800, 
      900, 1000, 1200, 1400, 1600, 1800, 2000};

  m_Axes["jetPt_as"] = axis;
  axis.clear();

  // DeltaR Axis: Nbins, x_low, x_up
  axis.push_back(100);
  axis.push_back(0.);
  axis.push_back(0.2);

  m_Axes["DR"] = axis;
  axis.clear();

  // DeltaR asymmetric Axis
  axis = {
      0, 0.004, 0.008, 0.012, 0.016, 0.020, 0.024, 0.028, 
      0.032, 0.036, 0.040, 0.044,
      0.050, 0.056, 0.062, 0.068, 0.074, 0.080,
      0.088, 0.096, 0.104, 0.112,
      0.122, 0.132, 0.142, 0.152, 0.162,
      0.202,
      0.302, 0.402};

  m_Axes["DeltaRas"] = axis;
  m_Axes["dRtrk12as"] = axis;
  axis.clear();

  // Eta axis
  axis = {-2.5, -2.0, -1.5,
          -1.25, -1.0,
          -0.80, -0.60, -0.40,
          -0.30, -0.20, -0.10,
          0.,
          0.10, 0.20, 0.30,
          0.40, 0.60, 0.80,
          1.0, 1.25,
          1.5, 2.0, 2.5};

  m_Axes["eta_as"] = axis;
  axis.clear();

  // d0 axis (symmetric)
  axis.push_back(m_nbins);
  axis.push_back(m_xmind0);
  axis.push_back(m_xmaxd0);

  m_Axes["d0"] = axis;
  axis.clear();

  // z0 axis (symmetric)
  axis.push_back(m_nbins);
  axis.push_back(m_xminz0);
  axis.push_back(m_xmaxz0);

  m_Axes["z0"] = axis;
  axis.clear();

  // pT_2D axis
  axis = {0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.2, 1.5, 2., 2.5, 3., 4., 5., 7., 9., 11., 13., 15., 20., 25., 30., 35., 40., 45., 50., 55., 60., 80., 100., 120., 140., 160.};

  m_Axes["pt_2d"] = axis;
  axis.clear();        

  // Eta_2D axis
  axis = {-2.5, -2.25, -2., -1.75, -1.5, -1.25, -1., -0.75, -0.5, -0.25, 0., 0.25, 0.5, 0.75, 1., 1.25, 1.5, 1.75, 2., 2.25, 2.5};

  m_Axes["eta_2d"] = axis;
  axis.clear();         

  // no_of_bins, xmin and xmax should be assigned in their respective orders
  std::vector<std::string> axis_variable = {"eta", "pt", "phi", "mu", "bsWidth", "dRtrk12", "d0_s", "z0_s"};
  std::vector<int> nbin_value = {m_nbinseta, m_nbinspt, 10, m_nbinsmu, 150, 100, m_nbins, m_nbins};
  std::vector<double> xmin_value = {m_xmineta, m_xminpt, -3.14, m_xminmu, 0, 0, m_xmind0, m_xminz0};
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

void IPhistos::Define3DHistos()
{
  
  std::string h_name = "";

  for (unsigned int i_cat = 0; i_cat < m_cats.size(); ++i_cat) {
    std::string & catName = m_cats[i_cat];
    for (unsigned int i_ip = 0; i_ip < m_ips.size(); ++i_ip) {
      
      h_name = m_name + "_" + catName + "_" + m_ips[i_ip] +"_vs_" + "trkpTas_vs_eta_as"; 
      m_histos3d[h_name] = plot3D(h_name,
                                "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                                "Track #eta", m_Axes["eta_as"].size() - 1, m_Axes["eta_as"].data(),
                                m_ips[i_ip], m_Axes[m_ips[i_ip] + "_s"].size() - 1, m_Axes[m_ips[i_ip] + "_s"].data());
    
      if (m_saveAdditionalHistos) {
        h_name = m_name + "_" + catName + "_" + m_ips[i_ip] +"_vs_" + "trkpTas_vs_deltaRas_trk12";
        m_histos3d[h_name] = plot3D(h_name,
                                  "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                                  "#DeltaR(trk1,trk2)", m_Axes["dRtrk12as"].size() - 1, m_Axes["dRtrk12as"].data(),
                                  m_ips[i_ip], m_Axes[m_ips[i_ip] + "_s"].size() - 1, m_Axes[m_ips[i_ip] + "_s"].data());
        
        h_name = m_name + "_" + catName + "_" + m_ips[i_ip] +"_vs_" + "trkpTas_vs_mu";
        m_histos3d[h_name] = plot3D(h_name,
                                  "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                                  "#mu", m_Axes["mu"].size() - 1, m_Axes["mu"].data(),
                                  m_ips[i_ip], m_Axes[m_ips[i_ip] + "_s"].size() - 1, m_Axes[m_ips[i_ip] + "_s"].data());
      }
  
    }
  }

} // End of Define3DHistos()

void IPhistos::Define2DHistos()
{

  std::string h_name = "";

  for (unsigned int i_cat = 0; i_cat < m_cats.size(); ++i_cat){
    std::string & catName = m_cats[i_cat];

    h_name = m_name + "_" + catName + "_jetpT_vs_Eta";
    m_histos2d[h_name] = plot2D(h_name,
                              "jetPt [GeV]", m_Axes["jetPt_as"].size() - 1, m_Axes["jetPt_as"].data(),
                              "Track #eta", m_Axes["eta"].size() - 1, m_Axes["eta"].data());
    
    h_name = m_name + "_" + catName + "_trackpT_vs_Eta";
    m_histos2d[h_name] = plot2D(h_name,
                              "Track p_{T} [GeV]", m_Axes["pTas"].size() - 1, m_Axes["pTas"].data(),
                              "Track #eta", m_Axes["eta_as"].size() - 1, m_Axes["eta_as"].data());
    
    if (m_saveAdditionalHistos) {  
      for (unsigned int i_ip = 0; i_ip < m_ips.size(); ++i_ip){
        for (unsigned int i_var = 0; i_var < m_variables.size(); ++i_var){

          h_name = m_name + "_" + catName + "_" + m_ips[i_ip] + "_vs_" + m_variables[i_var];
          m_histos2d[h_name] = plot2D(h_name,
                                    m_variables[i_var], m_Axes[m_variables[i_var]].size() - 1, m_Axes[m_variables[i_var]].data(),
                                    m_ips[i_ip], m_Axes[m_ips[i_ip]][0], m_Axes[m_ips[i_ip]][1], m_Axes[m_ips[i_ip]][2]);

        } // var
      } // IPS

    } // Additional histos
  } // categories

} // End of Define2DHistos()

void IPhistos::BookHistograms()
{

  for (unsigned int i_cat = 0; i_cat < m_cats.size(); ++i_cat){
    std::string & catName = m_cats[i_cat];
      
    m_h_bsWidth.push_back(plot1D(m_name + "h_bsWidth_" + catName, "bsWidth (xy) [um]", 150, 0, 15));
    
    if (m_saveAdditionalHistos) {  
      m_h_d0.push_back(plot1D(m_name + "h_d0_" + catName, "d0 [um]", m_nbins, m_xmind0, m_xmaxd0));
      m_h_z0.push_back(plot1D(m_name + "h_z0_" + catName, "z0 [um]", m_nbins, m_xminz0, m_xmaxz0));
      m_h_jetPt.push_back(plot1D(m_name + "h_jetPt_" + catName, "jetPt", m_Axes["jetPt_as"].size() - 1, m_Axes["jetPt_as"].data())); 
      m_h_deltaR_trk12.push_back(plot1D(m_name + "h_deltaR_trk12_" + catName, "deltaR(trk1,trk2)", m_Axes["DR"][0], m_Axes["DR"][1], m_Axes["DR"][2]));
      m_h_count.push_back(plot1D(m_name + "h_count_" + catName, "Y-axis: Number of tracks satisfying this class", 5, 0, 5));
    }
  }
  m_1D.push_back(m_h_bsWidth);
  m_1D.push_back(m_h_d0);
  m_1D.push_back(m_h_z0);
  m_1D.push_back(m_h_jetPt);
  m_1D.push_back(m_h_deltaR_trk12);
  m_1D.push_back(m_h_count);
} // End of BookHistograms()

void IPhistos::FillHistograms(double d0, double z0, double pt, double eta, double phi, int runN, double mu, double jetPt, std::vector<float> weights, double bsWidth, double deltaR_trk12, std::vector<int> class_satisfied, float intLumi)
{
    
  for (int class_index = 0; class_index < int(class_satisfied.size()); class_index++){ // class_satisfied loop ends in the end
    
    if( !class_satisfied[class_index] ) continue;
    
    std::string class_ = std::to_string(class_index);

    float w = weights[class_index];
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

      if (it->first.find("class" + class_) != std::string::npos){
        if (it->first.find("trkpTas_vs_eta") != std::string::npos)
          it->second->Fill(pt, eta, ipValue, w);
        if (it->first.find("trkpTas_vs_deltaRas_trk12") != std::string::npos)
          it->second->Fill(pt, deltaR_trk12, ipValue, w);
        if (it->first.find("trkpTas_vs_mu") != std::string::npos)
          it->second->Fill(pt, mu, ipValue, w);
      } // cat/class satisfied
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

      if (it->first.find("class" + class_) != std::string::npos){
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
    }
      //} // cat/class satisfied
    //} // histo2d
    
    // Fill 1D histograms
    m_h_bsWidth[class_index]->Fill(bsWidth * 1000, w); 
    if (m_saveAdditionalHistos){
      m_h_d0[class_index]->Fill(d0, w);
      m_h_z0[class_index]->Fill(z0, w);
      m_h_jetPt[class_index]->Fill(jetPt, w); 
      m_h_deltaR_trk12[class_index]->Fill(deltaR_trk12, w);
      m_h_count[class_index]->Fill(1, w);
    }
  
  } // End of class_satisfied loop

} // End of FillHistograms()
