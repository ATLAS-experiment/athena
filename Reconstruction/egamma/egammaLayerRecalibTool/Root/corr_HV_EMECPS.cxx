#include "egammaLayerRecalibTool/corr_HV_EMECPS.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string>
#include <utility>

#include <TH2D.h>
#include <TFile.h>
#include <TSystem.h>

#include "PathResolver/PathResolver.h"


corr_HV_EMECPS::corr_HV_EMECPS(){
  std::string filename = PathResolverFindCalibFile("egammaLayerRecalibTool/v2/emecps_hv_corr_2015-2016_207X.root");

  auto file = std::unique_ptr<TFile>(TFile::Open(filename.c_str()));
  if (not file or file->IsZombie()) {
    std::cerr << "FATAL: cannot open " << filename << std::endl;
  } else {
    auto obj = dynamic_cast<TH2D*>(file->Get("hc"));
    if (!obj) {
      std::cerr << " cannot read histogram hc " << std::endl;
    } else{
      obj->SetDirectory(nullptr);
      m_HV[0].reset(obj);
    }

    obj = dynamic_cast<TH2D*>(file->Get("ha"));
    if (!obj) {
      std::cerr << " cannot read histogram ha " << std::endl;
    } else{
      obj->SetDirectory(nullptr);
      m_HV[1].reset(obj);
    }
  }
}
//===============================================================================
corr_HV_EMECPS::~corr_HV_EMECPS() = default;

//===============================================================================
float corr_HV_EMECPS::getCorr(int run, float eta,float phi) const
{
   float newCorr=1.;
   if (eta>-1.5 && eta<1.5) return newCorr;
   if (eta<-1.85 || eta > 1.85) return newCorr;

   int iside=0;
   if (eta>0.) iside=1;

   if (!m_HV[iside]) {
      std::cerr << " cannot find histogram to apply correction " << std::endl;
   } else {
     int ibin = std::as_const(*m_HV[iside]).GetXaxis()->FindFixBin(((Double_t)(run+0.1)));
      if (ibin<1 || ibin>7) return newCorr;
      if (phi>3.14159) phi=phi-6.283185;
      int jbin = std::as_const(*m_HV[iside]).GetYaxis()->FindFixBin(phi);
      if (jbin<1 || jbin>2048) return newCorr;
      newCorr =  m_HV[iside]->GetBinContent(ibin,jbin);
   }
   //std::cout << " in  corr_HV_EMECPS run,eta,phi,corr " << run << " " << eta << " " << phi << " " << newCorr << std::endl;
   return newCorr;

}
