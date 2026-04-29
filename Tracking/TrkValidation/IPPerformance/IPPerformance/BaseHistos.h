#ifndef BASEHISTO_H
#define BASEHISTO_H

#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include <string>
#include <map>
#include <vector>

class BaseHistos {

 public:
  BaseHistos(std::string inputName); 
  virtual ~BaseHistos() {};
  
  virtual void define3DHistos() = 0;
  virtual void define2DHistos() = 0;
  virtual void define1DHistos() = 0;

  TH3D* get3dHisto(const std::string& str) {
    return m_histos3d[str];
  }

  TH2D* get2dHisto(const std::string& str) {
    return m_histos2d[str];
  }

  TH1D* get1dHisto(const std::string& str) {
    return m_histos1d[str];
  }

  std::map<std::string, TH3D*>& get3DHistos() {
        return m_histos3d;
    }

  std::map<std::string, TH2D*>& get2DHistos() {
        return m_histos2d;
    }

  std::map<std::string, TH1D*>& get1DHistos() {
        return m_histos1d;
    }


  TH1D*  plot1D(const std::string& name,const std::string& xtitle, int nbinsX, double xmin, double xmax);

  TH1D*  plot1D(const std::string& name,const std::string& xtitle, int nbinsX, double* axisX);
  
  TH2D*  plot2D(const std::string& name,
    const std::string& xtitle, int nbinsX, double xmin, double xmax,
    const std::string& ytitle, int nbinsY, double ymin, double ymax);

  TH2D*  plot2D(const std::string& name,
    const std::string& xtitle, int nbinsX, double* axisX,
    const std::string& ytitle, int nbinsY, double* axisY);

  TH2D*  plot2D(const std::string& name,
    const std::string& xtitle, int nbinsX, const double* axisX,
    const std::string& ytitle, int nbinsY, const double* axisY);
  
  TH2D*  plot2D(const std::string& name,
    const std::string& xtitle, int nbinsX, double* axisX,
    const std::string& ytitle, int nbinsY, double ymin, double ymax);
  
  TH3D*  plot3D(const std::string& name,
    const std::string& xtitle, int nbinsX, double xmin, double xmax,
    const std::string& ytitle, int nbinsY, double ymin, double ymax,
    const std::string& ztitle, int nbinsZ, double zmin, double zmax);

  TH3D*  plot3D( const std::string& name,
    const std::string& xtitle, int nbinsX, double* axisX,
    const std::string& ytitle, int nbinsY, double* axisY,
    const std::string& ztitle, int nbinsZ, double* axisZ);

  TH3D*  plot3D(const std::string& name,
    const std::string& xtitle, int nbinsX,  double* axisX,
    const std::string& ytitle, int nbinsY,  double* axisY,
    const std::string& ztitle, int nbinsZ, double zmin, double zmax);
    
 protected:
  
  std::string m_name;
  
  std::map<std::string, std::vector<double> > m_Axes;
    
  std::map<std::string, TH1D*> m_histos1d;
  typedef std::map<std::string, TH1D*>::iterator it1d;
  
  std::map<std::string, TH2D*> m_histos2d;
  typedef std::map<std::string, TH2D*>::iterator it2d;

  std::map<std::string, TH3D*> m_histos3d;
  typedef std::map<std::string, TH3D*>::iterator it3d;
  
};


#endif
