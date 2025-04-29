/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STRIPSEGMENTTOOL_H
#define STRIPSEGMENTTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonDigitContainer/sTgcDigit.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"

#include "RegSelLUT/RegSelSiLUT.h"
#include "IRegionSelector/IRegSelLUTCondData.h"

#include "TrigT1NSWSimTools/IStripSegmentTool.h"
#include "TrigT1NSWSimTools/PadTrigger.h"
#include "TrigT1NSWSimTools/TriggerTypes.h"
#include "TrigT1NSWSimTools/StripOfflineData.h"
#include "TrigT1NSWSimTools/tdr_compat_enum.h"

#include <Math/Vector3D.h>
#include <functional>
#include <algorithm>
#include <map>
#include <utility>
#include <cmath>

struct Envelope_t{
  float lower_r{FLT_MIN};
  float upper_r{FLT_MAX};
  float lower_eta{FLT_MIN};
  float upper_eta{FLT_MAX};
  float lower_z{FLT_MIN};
  float upper_z{FLT_MAX};
};

// namespace for the NSW LVL1 related classes
namespace NSWL1 {

  /**
   *
   *   @short interface for the StripTDS tools
   *
   * This class implements the Strip Clustering offline simulation. It loops over the hits,
   * readout from the StripTDSOffLineTool
   *
   *  @author Jacob Searcy <jsearcy@cern.ch>
   *
   *
   */

  class StripSegmentTool: virtual public IStripSegmentTool,
                                  public AthAlgTool {

  public:
    StripSegmentTool(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);
    virtual ~StripSegmentTool()=default;
    virtual StatusCode initialize() override;
    virtual StatusCode find_segments( std::vector< std::unique_ptr<StripClusterData> >& ,const std::unique_ptr<Muon::NSW_TrigRawDataContainer>& ) const override;
    StatusCode FetchDetectorEnvelope(Envelope_t &env) const;

  private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<IRegSelLUTCondData> m_regSelTableKey{this, "RegSelLUT", "RegSelLUTCondData_sTGC", "sTGC Region Selector lookup table"};

    Gaudi::Property<std::string> m_sTgcSdoContainer{this, "sTGC_SdoContainerName",  "sTGC_SDO", "Name of the sTGC SDO digit container"};
    Gaudi::Property<int>         m_rIndexBits      {this, "rIndexBits",              8,         "Number bits in R-index calculation"};
    Gaudi::Property<int>         m_dThetaBits      {this, "dthetaBits",              5,         "Number bits in dTheta calculation"};
    Gaudi::Property<float>       m_dtheta_min      {this, "dthetaMin",             -15.,        "Minimum allowed value for dtheta in mrad"};
    Gaudi::Property<float>       m_dtheta_max      {this, "dthetaMax",              15.,        "Maximum allowed value for dtheta in mrad"};
    Gaudi::Property<int>         m_ridxScheme      {this, "rIndexScheme",            1,         "rIndex slicing scheme/ 0-->R / 1-->eta"};

    uint8_t findRIdx(const float val, const Envelope_t &env) const;
    uint8_t findDtheta(const float) const;
  };  // end of StripSegmentTool class
} // namespace NSWL1
#endif
