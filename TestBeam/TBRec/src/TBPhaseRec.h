/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBPHASEREC
#define TBREC_TBPHASEREC

/*! \brief reconstruction of a suitable event phase
 *
 * \author Michel Lefebvre <lefebvre@uvic.ca>
 * \date August 13, 2004
 *  
 * June 30, 2004 - implementation based on a first draft from Marco Delmastro
 */  

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/Units.h"

#include <string>
#include <vector>

class TBPhaseRec : public AthAlgorithm
{
 public:
  TBPhaseRec(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBPhaseRec();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
  StatusCode getnewcalib();
  
  static const int unknown = -99999 ;

 private:
  ////////////////
  // Properties //
  ////////////////
  Gaudi::Property<std::vector<std::string>> m_tdcNames{this, "TDCNames", {}, "TDC(s) with phase information"};   // the TDC(s) with phase information
  Gaudi::Property<std::vector<float>> m_tdcToTime{this, "TDCToTime", {}, "TDC to time conversion per TDC"};         // per TDC
  Gaudi::Property<std::vector<float>> m_tdcwac{this, "TDCwac", {}, "TDC wrap around constants"};            // wrap around constants
  Gaudi::Property<std::vector<float>> m_tdcMin{this, "TDCMin", {}, "TDC minimum values (for choosing the best TDC)"};            // used only for choosing the best TDC if necessary
  Gaudi::Property<float> m_delta{this, "TTCClockPeriod", 25.0*Athena::Units::ns, "TTC clock period"};                          // ttc clock period
  Gaudi::Property<int> m_timeBins{this, "TimeBins", 25, "number of time bins in a TTC period"};                         // number of time bins in a ttc period
  Gaudi::Property<std::string> m_TBPhaseKey{this, "TBPhaseKey", "TBPhase", "key of the TBPhase object to create"};               // key of the TBPhase object that TBPhaseRec tries to create
  Gaudi::Property<bool> m_neverReturnFailure{this, "NeverReturnFailure", false, "never set filter failure in execute()"};              // controls when StatusCode::FAILURE can be issued in execute()

  Gaudi::Property<std::string> m_calib_filename{this, "CalibFileName", "", "TDC calibration constants file"};
  unsigned int m_runnumber{0};

  Gaudi::Property<float> m_guardValue{this, "GuardCutValue", -1., "guard region cut value in TDC counts"};                     // guard region cut

  ///////////////////////////////
  // member data and functions //
  ///////////////////////////////
  
  int m_nTDC{0};
  std::vector<int> m_tdcRaw;
  std::vector<float> m_phaseReco;
  float computePhase(int tdcIndex);
  
};

#endif
