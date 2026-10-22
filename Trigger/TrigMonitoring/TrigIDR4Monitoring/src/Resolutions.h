/** emacs: this is -*- c++ -*-
 **
 **  @file    Resolutions.h
 **  @author  sutt@cern.ch
 **
 **           based on a class by Thomas Strebler <thomas.strebler@cern.ch>
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef IDTPM2_RESOLUTIONS_H
#define IDTPM2_RESOLUTIONS_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "AsgMessaging/StatusCode.h"
#include "AthenaBaseComps/AthMessaging.h"

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "TrigInDetAnalysisExample/TIDAHistogram.h"

#include "IPlotBase.h"

#include <array>

namespace IDTPM2 {

class Resolutions : public IPlotBase { 

public:

  /// Constructor
  /// this method should be an enum of "allowed" methods
  Resolutions( const std::string& name, ToolHandle<GenericMonitoringTool>* m=0 );    
  
  /// Destructor
  virtual ~Resolutions() = default;
  
  /// Book the histograms
  void initialize(); // needed to override IPlotBase
  
  /// Finalize resolution and bias histograms
  /// should be called from the calling montools
  /// stop() method
  void finalize();
  
  void execute( const TrackAnalysisStore& collections ) const;

  
  void      method( unsigned m=0 ) { m_method = m; }
  
  unsigned  method() const         { return m_method; }
  
private:
  
  //// one might well ask, why this enumeration ?
  enum Param { 
      INCLUSIVE,
      PT,
      ETA,
      D0,
      Z0,
      QOVERP,
      QOVERPT,
      THETA,
      PHI,
      Z0SIN,
      NPARAMS,
      NPARAMSOUT = 3 // Plot only vs pt and eta inclusive ????
  };
  
  
  /// get the track parameters
  void getTrackParameters( const TrackView& p,
			   std::array<float,NPARAMS>& params,
			   std::array<float,NPARAMS>& errors ) const;
  
  std::string m_paramName[NPARAMS] = {
    "inclusive", "pt", "eta", "d0", "z0", "qoverp", "ptqopt", "theta", "phi", "z0sin"
  };


private:

  /// probably will eventually no longer need these
  //  const std::string& m_dirName;
  //  const std::string& m_anaTag;

  unsigned int m_method;
      
  //// lazy and not very maintainable ...
  
  TIDA::Histogram<float>   m_pull[NPARAMS];
  TIDA::Histogram<float>   m_res[NPARAMS];
  TIDA::Histogram<float>   m_sigma[NPARAMS];
  TIDA::Histogram<float>   m_significance[NPARAMS];
  TIDA::Histogram2D<float> m_corr[NPARAMS]; // 2D correlation plots

  TIDA::Histogram<float>   m_resHelper[NPARAMS][NPARAMSOUT]; // 2D res helper ??
  TIDA::Histogram<float>   m_reswidth[NPARAMS][NPARAMSOUT];
  TIDA::Histogram<float>   m_resmean[NPARAMS][NPARAMSOUT];
  
  TIDA::Histogram<float>   m_pullHelper[NPARAMS][NPARAMSOUT]; //2D helper ??
  TIDA::Histogram<float>   m_pullwidth[NPARAMS][NPARAMSOUT]; 
  TIDA::Histogram<float>   m_pullmean[NPARAMS][NPARAMSOUT];

  /// like the res{Helper,width,mean} plots above but actually
  /// make some sense
  TIDA::Histogram2D<float> m_resolution_vs_pt[NPARAMS];  // 2D correlation plots
  TIDA::Histogram2D<float> m_resolution_vs_eta[NPARAMS]; // 2D correlation plots
  TIDA::Histogram2D<float> m_resolution_vs_z0[NPARAMS];  // 2D correlation plots
  
};
  
}

#endif // IDTPM2_RESOLUTIONS_H
