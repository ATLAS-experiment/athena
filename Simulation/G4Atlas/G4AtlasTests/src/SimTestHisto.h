/* -*- C++ -*- */

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTESTS_SIMTESTHISTO_H
#define G4ATLASTESTS_SIMTESTHISTO_H

/** @file SimTestHisto.h
 * @author John Chapman - ATLAS Collaboration
 */

#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "TH1.h"
#include "TH2.h"
class TProfile;

/// Utility helper class for dealing with histograms in the sim tests.
/// @note Is this class really needed?
///
class SimTestHisto
{

public:

  SimTestHisto() = default;
  ~SimTestHisto() = default;

protected:
  std::string m_path{"/truth/"};
  ServiceHandle<ITHistSvc> m_histSvc{"THistSvc", "SimTestHisto"};
};

// note: var should be of type "TH1*" even if it is filled with a TProfile*
#define _TPROFILE(var,name,nbin,xmin,xmax)		      \
  {                                                 \
    const std::string histName{name};                 \
    const std::string histPath{m_path + histName};    \
    if (!m_histSvc->exists(histPath)) {		            \
      var = new TProfile(histName.c_str(), histName.c_str(),nbin,xmin,xmax);	       \
      CHECK(m_histSvc->regHist(histPath,var));		    \
    } else {						                              \
      CHECK(m_histSvc->getHist(histPath, var));	      \
    } \
  }
  

#define _TH1D(var,name,nbin,xmin,xmax)                         \
  { \
    const std::string histName{name};                           \
    const std::string histPath{m_path + histName};              \
    if (!m_histSvc->exists(histPath)) {                         \
      var = new TH1D(histName.c_str(), histName.c_str(),nbin, xmin, xmax);  \
      var->StatOverflows();                                     \
      CHECK(m_histSvc->regHist(histPath, var));                 \
    } else {                                                    \
      CHECK(m_histSvc->getHist(histPath, var));                 \
    }                                                           \
  } 
  

#define _TH1D_NOCHECK(var,name,nbin,xmin,xmax)			       \
  if (!m_histSvc->exists(m_path+name)) {		       \
    var = new TH1D(name,name,nbin,xmin,xmax);		       \
    var->StatOverflows();				       \
    if(m_histSvc->regHist(m_path+name,var).isFailure())        \
  std::cout<<"Cannot register histogram "<<name<<std::endl;    \
  } else {						       \
    if(m_histSvc->getHist(m_path+name,var).isFailure())       \
      std::cout<<"Cannot get histogram "<<name<<std::endl;     \
  }

#define _TH1D_WEIGHTED(var,name,nbin,xmin,xmax)	\
  _TH1D(var,name,nbin,xmin,xmax);		\
  var->Sumw2();

#define _TH2D_NOCHECK(var,name,nbinx,xmin,xmax,nbiny,ymin,ymax)	       \
  if (!m_histSvc->exists(m_path+name)) {		       \
    var = new TH2D(name,name,nbinx,xmin,xmax,nbiny,ymin,ymax); \
    if(m_histSvc->regHist(m_path+name,var).isFailure())	       \
      std::cout<<"Cannot register histogram "<<name<<std::endl;\
  } else {						       \
    if(m_histSvc->getHist(m_path+name,var).isFailure())       \
      std::cout<<"Cannot get histogram "<<name<<std::endl;     \
  } 

#define _TH2D(var,name,nbinx,xmin,xmax,nbiny,ymin,ymax)	       \
  { \
    const std::string histName{name};                           \
    const std::string histPath{m_path + histName};              \
    if (!m_histSvc->exists(histPath)) {		       \
      var = new TH2D(histName.c_str(),histName.c_str(),nbinx,xmin,xmax,nbiny,ymin,ymax); \
      CHECK(m_histSvc->regHist(histPath,var));	               \
    } else {						       \
      CHECK(m_histSvc->getHist(histPath,var));	       \
    } \
  } 

#define _TH2D_WEIGHTED(var,name,nbinx,xmin,xmax,nbiny,ymin,ymax)	\
  _TH2D(var,name,nbinx,xmin,xmax,nbiny,ymin,ymax);			\
  var->Sumw2();

#define _SET_TITLE(var,title,xaxis,yaxis)				\
  var->SetXTitle(xaxis);						\
  var->SetYTitle(yaxis);						\
  var->SetTitle((std::string(var->GetName())+" : "+title).c_str());

//#define _SET_LOGX(var)
//TAxis *axis = var->GetXaxis();
//int bins = axis->GetNbin//s();

// Axis_t from = axis->GetXmin();
// Axis_t to = axis->GetXmax();
// Axis_t width = (to - from) / bins;
// Axis_t *new_bins = new Axis_t[bins + 1];

// for (int i = 0; i <= bins; i++) {
//   new_bins[i] = TMath::Power(10, from + i * width);
//
// }
// axis->Set(bins, new_bins);
// delete new_bins;

#endif //G4ATLASTESTS_SIMTESTHISTO_H
