/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBPLANETRACKINGALGO_H
#define TBREC_TBPLANETRACKINGALGO_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "TBEvent/TBHitPlaneCont.h"

#include <utility>
#include <vector>

/** @class TBHitPlane TBHitPlane.h "TBEvent/TBHitPlane.h"
This algorithm is used to fit a collection of TBHitPlane (TBHitPlaneCont) 
and create a TBTrack data object.
*/

class TBPlaneTrackingAlgo : public AthAlgorithm {
 public:
  
  /////////////////////////////
  // Contructor & Destructor //
  /////////////////////////////
  TBPlaneTrackingAlgo(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBPlaneTrackingAlgo(){};

  // public methods //
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

 private:
  void FillRandomHit();
  StatusCode getnewcalib();

  
  // Private function members //

  ////////////////////////////////////////////////////////////////////////
  /// \brief Fit data to the function u = a1 + a2*w. and determines
  ///        intercept, slope, residual for each BPC, and chi2 on fit
  /////////////////////////////////////////////////////////////////////////
  bool fitPlane(const TBHitPlaneCont * hitPlaneCont, double &a1, double &a2,
		double &chi2, std::vector<double> &residual);

  ////////////////////////////////////////////////////////////////////////
  /// \brief Fit data to the function u = a1 + a2*w.
  /////////////////////////////////////////////////////////////////////////
  bool fitHits(const std::vector<double> & u, const std::vector<double> & w,
	       const std::vector<double> & eu, // const std::vector<double> & ew,
	       double &a1, double &a2);

  ////////////////////////////////////////////////////////////////////////
  /// \brief Calculates the residual-> r = (u_i - a1 - a2*w_i)
  /////////////////////////////////////////////////////////////////////////
  double getResidual(const double & u, const double & w,const double &a1,
		     const double &a2);

  ////////////////////////////////////////////////////////////////////////
  /// \brief Calculates the chi2 += ((u_i - a1 - a2*w_i)/eu)^2
  /////////////////////////////////////////////////////////////////////////
  double getChi2(const std::vector<double> &v_u, 
		 const std::vector<double> &v_w,
		 const std::vector<double> &v_eu,
//		 const std::vector<double> &v_ew,
		 const double &a1, const double &a2);

  ////////////////////////////////////////////////////////////////////////
  /// \brief Build HitPlaneCont from BPC
  /////////////////////////////////////////////////////////////////////////
  StatusCode buildHits();
    
  //Used to test this algo set to true if you want to debug
  Gaudi::Property<bool> m_testAlgo{this, "TestAlgo", false, "Use FillRandomHit to generate test data"};
  TBHitPlaneCont m_hitPlaneCont_u;
  TBHitPlaneCont m_hitPlaneCont_v;

  // Positions of BPC needed to construct hits  
  Gaudi::Property<std::vector<float>> m_bpc_posZX{this, "BPCposZX", {}, "BPC Z positions (X plane)"};
  Gaudi::Property<std::vector<float>> m_bpc_posZY{this, "BPCposZY", {}, "BPC Z positions (Y plane)"};
  Gaudi::Property<std::vector<float>> m_bpc_posX{this, "BPCposX", {}, "BPC X positions"};
  Gaudi::Property<std::vector<float>> m_bpc_posY{this, "BPCposY", {}, "BPC Y positions"};
  Gaudi::Property<std::vector<float>> m_bpc_errposZX{this, "BPCerrposZX", {}, "Errors on BPC Z positions (X plane)"};
  Gaudi::Property<std::vector<float>> m_bpc_errposZY{this, "BPCerrposZY", {}, "Errors on BPC Z positions (Y plane)"};
  Gaudi::Property<std::vector<float>> m_bpc_errposX{this, "BPCerrposX", {}, "Errors on BPC X positions"};
  Gaudi::Property<std::vector<float>> m_bpc_errposY{this, "BPCerrposY", {}, "Errors on BPC Y positions"};
  Gaudi::Property<std::vector<float>> m_bpc_errmeasX{this, "BPCerrmeasX", {}, "BPC X measurement errors"};
  Gaudi::Property<std::vector<float>> m_bpc_errmeasY{this, "BPCerrmeasY", {}, "BPC Y measurement errors"};

  Gaudi::Property<std::vector<std::string>> m_bpc_names{this, "BPCnames", {}, "BPC names"};

  // filename containing positions constants
  Gaudi::Property<std::string> m_calib_filename{this, "CalibFileName", "H6HitCalib.txt", "File name containing positions constants"};
  unsigned int m_runnumber{0};

};

#endif	
