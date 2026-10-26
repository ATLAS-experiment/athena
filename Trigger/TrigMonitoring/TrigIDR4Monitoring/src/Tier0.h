/** emacs: this is -*- c++ -*- 
 **
 **     @file    Tier0.h
 **
 **     @author  mark sutton
 **     @date    28 Sep 2025 15:29:53 CEST
 **
 **     $Id: Tier0.h, v0.0   28 Sep 2025 15:29:53 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef IDTPM2_TIER0_H
#define IDTPM2_TIER0_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "TrigInDetAnalysis/TIDDirectory.h"
#include "TrigInDetAnalysisExample/TIDAHistogram.h"

#include "TrackAnalysisStore.h"

#include "IPlotBase.h"


namespace IDTPM2 {

class Tier0 : public IPlotBase {

public:

  Tier0( const std::string& name, ToolHandle<GenericMonitoringTool>* m=0 );
  
  //  Tier0( const std::string& name ) { } 

  virtual ~Tier0() { }
  
  virtual void initialize();

  virtual void execute( const TrackAnalysisStore& collections ) const;

  virtual void finalize();

  const std::string& name() const { return m_name; }
  
private:

  std::string m_name;

  /// Monitorwd::AScalar Histogram wrapper class

  TIDA::Histogram<float> m_htotal_efficiency;
  TIDA::Histogram<float> m_hpTeff;
  TIDA::Histogram<float> m_hetaeff;
  TIDA::Histogram<float> m_hphieff;
  TIDA::Histogram<float> m_hd0eff;
  TIDA::Histogram<float> m_hz0eff;
  TIDA::Histogram<float> m_hnVtxeff;
  TIDA::Histogram<float> m_hlbeff;

  TIDA::Histogram<float> m_hpTres;
  TIDA::Histogram<float> m_hipTres;
  TIDA::Histogram<float> m_hetares;
  TIDA::Histogram<float> m_hphires;
  TIDA::Histogram<float> m_hd0res;
  TIDA::Histogram<float> m_hz0res;

  TIDA::Histogram<float> m_htrkvtx_x_lb;
  TIDA::Histogram<float> m_htrkvtx_y_lb;
  TIDA::Histogram<float> m_htrkvtx_z_lb;


  TIDA::Histogram<float> m_hnpixvseta;
  TIDA::Histogram<float> m_hnpixvsphi;
  TIDA::Histogram<float> m_hnpixvsd0;
  TIDA::Histogram<float> m_hnpixvspT;

  TIDA::Histogram<float> m_hnsctvseta;
  TIDA::Histogram<float> m_hnsctvsphi;
  TIDA::Histogram<float> m_hnsctvsd0;
  TIDA::Histogram<float> m_hnsctvspT;

  TIDA::Histogram<float> m_hntrtvseta;
  TIDA::Histogram<float> m_hntrtvsphi;

  TIDA::Histogram<float> m_hnsihits_lb;

  TIDA::Histogram<float> m_hnpixvseta_rec;
  TIDA::Histogram<float> m_hnpixvsphi_rec;
  TIDA::Histogram<float> m_hnpixvsd0_rec;
  TIDA::Histogram<float> m_hnpixvspT_rec;

  TIDA::Histogram<float> m_hnsctvseta_rec;
  TIDA::Histogram<float> m_hnsctvsphi_rec;
  TIDA::Histogram<float> m_hnsctvsd0_rec;
  TIDA::Histogram<float> m_hnsctvspT_rec;

  TIDA::Histogram<float> m_hntrtvseta_rec;
  TIDA::Histogram<float> m_hntrtvsphi_rec;

  TIDA::Histogram<float> m_hnsihits_lb_rec;

  TIDA::Histogram<float> m_hd0vsphi;
  TIDA::Histogram<float> m_hd0vsphi_rec;

  TIDA::Histogram<float> m_hchain;
  TIDA::Histogram<float> m_hroieta;
  TIDA::Histogram<float> m_hntrk;
  TIDA::Histogram<float> m_htrkpT;

  TIDA::Histogram<float> m_htrketa;
  TIDA::Histogram<float> m_htrkphi;
  TIDA::Histogram<float> m_htrkd0;
  TIDA::Histogram<float> m_htrkz0;


  TIDA::Histogram<float> m_htrkdd0;
  TIDA::Histogram<float> m_htrkdz0;

  TIDA::Histogram<float> m_htrkd0sig;

  TIDA::Histogram<float> m_hnpix;
  TIDA::Histogram<float> m_hnsct;
  TIDA::Histogram<float> m_hnsihits;
  TIDA::Histogram<float> m_hntrt;

  TIDA::Histogram<float> m_hntrk_rec;

  TIDA::Histogram<float> m_chi2dof;
  TIDA::Histogram<float> m_chi2dof_rec;


  TIDA::Histogram<float> m_hmu;

  
  TIDA::Histogram<float> m_hlayer;


  TIDA::Histogram<float> m_htrkpT_rec;
  TIDA::Histogram<float> m_htrketa_rec;
  TIDA::Histogram<float> m_htrkphi_rec;
  TIDA::Histogram<float> m_htrkd0_rec;
  TIDA::Histogram<float> m_htrkz0_rec;

  TIDA::Histogram<float> m_htrkdd0_rec;
  TIDA::Histogram<float> m_htrkdz0_rec;

  TIDA::Histogram<float> m_htrkd0sig_rec;

  TIDA::Histogram<float> m_hnpix_rec;
  TIDA::Histogram<float> m_hnsct_rec;
  TIDA::Histogram<float> m_hnsihits_rec;
  TIDA::Histogram<float> m_hntrt_rec;


  TIDA::Histogram<float> m_hlayer_rec;


  TIDA::Histogram<float> m_htrkpT_residual;
  TIDA::Histogram<float> m_htrkipT_residual;
  TIDA::Histogram<float> m_htrketa_residual;
  TIDA::Histogram<float> m_htrkphi_residual;
  TIDA::Histogram<float> m_htrkd0_residual;
  TIDA::Histogram<float> m_htrkz0_residual;

  TIDA::Histogram<float> m_htrkdd0_residual;
  TIDA::Histogram<float> m_htrkdz0_residual;

};

}

#endif // IDTPM2_TIER0_H
