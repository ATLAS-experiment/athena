#include "IPPerformance/BaseHistos.h"
#include "TKey.h"
#include "TClass.h"
#include "TProfile.h"

BaseHistos::BaseHistos(std::string inputName) 
{
  m_name = inputName; 
}

TH1D* BaseHistos::plot1D(std::string name,std::string xtitle, int nbinsX, double xmin, double xmax) {
  TH1D* h= new TH1D(name.c_str(),name.c_str(),nbinsX,xmin,xmax);
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->Sumw2();
  return h;
}

TH1D* BaseHistos::plot1D(std::string name,std::string xtitle, int nbinsX, double* axisX) {
  TH1D* h= new TH1D(name.c_str(),name.c_str(),nbinsX,axisX);
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->Sumw2();
  return h;
}

TH2D* BaseHistos::plot2D(std::string name,
                         std::string xtitle, int nbinsX, double xmin, double xmax,
                         std::string ytitle, int nbinsY, double ymin, double ymax) {
  
  TH2D* h = new TH2D(name.c_str(),name.c_str(),
                     nbinsX,xmin,xmax,
                     nbinsY,ymin,ymax);
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->Sumw2();
  return h;
}


TH2D* BaseHistos::plot2D(std::string name,
                         std::string xtitle, int nbinsX, double* axisX,
                         std::string ytitle, int nbinsY, double* axisY) {

  TH2D * h = new TH2D(name.c_str(),name.c_str(),
                      nbinsX,axisX,
                      nbinsY,axisY);
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->Sumw2();
  return h;
}

TH2D* BaseHistos::plot2D(std::string name,
                         std::string xtitle, int nbinsX, const double* axisX,
                         std::string ytitle, int nbinsY, const double* axisY) {

  TH2D * h = new TH2D(name.c_str(),name.c_str(),
                      nbinsX,axisX,
                      nbinsY,axisY);
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->Sumw2();
  return h;
}

TH2D* BaseHistos::plot2D(std::string name,
                         std::string xtitle, int nbinsX, double* axisX,
                         std::string ytitle, int nbinsY, double  ymin, double ymax) {

  TH2D * h = new TH2D(name.c_str(),name.c_str(),
                      nbinsX,axisX,
                      nbinsY,ymin,ymax);
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->Sumw2();
  return h;
}

TH3D*  BaseHistos::plot3D(std::string name,
                          std::string xtitle, int nbinsX, double* axisX,
                          std::string ytitle, int nbinsY, double* axisY,
                          std::string ztitle, int nbinsZ, double* axisZ) {
  
  TH3D* h = new TH3D(name.c_str(),name.c_str(),
                     nbinsX,axisX,
                     nbinsY,axisY,
                     nbinsZ,axisZ);
  
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetZaxis()->SetTitle(ztitle.c_str());
  h->Sumw2();
  return h;
}

TH3D*  BaseHistos::plot3D(std::string name,
                          std::string xtitle, int nbinsX, double* axisX,
                          std::string ytitle, int nbinsY, double* axisY,
                          std::string ztitle, int nbinsZ, double zmin, double zmax) {
  
  double xlow = axisX[0];
  double xup  = axisX[nbinsX];
  double ylow = axisY[0];
  double yup  = axisY[nbinsY];
  TH3D* h = new TH3D(name.c_str(),name.c_str(),
                     nbinsX,xlow,xup,
                     nbinsY,ylow,yup,
                     nbinsZ,zmin,zmax);
  
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetZaxis()->SetTitle(ztitle.c_str());
  h->Sumw2();
  return h;
}

TH3D*  BaseHistos::plot3D(std::string name,
                          std::string xtitle, int nbinsX, double xmin, double xmax,
                          std::string ytitle, int nbinsY, double ymin, double ymax,
                          std::string ztitle, int nbinsZ, double zmin, double zmax) {
  
  TH3D* h = new TH3D(name.c_str(),name.c_str(),
         nbinsX,xmin,xmax,
         nbinsY,ymin,ymax,
         nbinsZ,zmin,zmax);
  
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetZaxis()->SetTitle(ztitle.c_str());
  h->Sumw2();
  return h;
}
