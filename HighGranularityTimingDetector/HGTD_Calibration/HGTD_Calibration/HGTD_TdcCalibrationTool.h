/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/HGTD_Calibration/HGTD_TdcCalibrationTool.h
 *
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch> (CREST integration)
 *
 * @date May, 2025
 *
 * @brief Simulation of ALTIROC Phase Shifter.
 *        Modified to read TDC calibration from conditions database.
 */

#ifndef HGTD_TDCCALIBRATIONTOOL_H
#define HGTD_TDCCALIBRATIONTOOL_H

#include <array>
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/Units.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "CLHEP/Random/RandomEngine.h"

#include "HGTD_RawData/HGTD_ALTIROC_RDO_Collection.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetSimEvent/SiHit.h"
#include "SiDigitization/SiSurfaceCharge.h"

#include "StoreGate/ReadCondHandleKey.h"

namespace HGTD {
  constexpr unsigned int TOA_OVERLFLOW_MASK = 0x80;
}

class HGTD_ID;

namespace InDetDD {
class SolidStateDetectorElementBase;
}

namespace CLHEP {
class HepRandomEngine;
}

class HGTD_TdcCalibrationTool : public AthAlgTool {
public:
HGTD_TdcCalibrationTool(const std::string& type, const std::string& name,
                        const IInterface* parent);

  /** AlgTool initialize */
  virtual StatusCode initialize() override final;

  /**
   * @brief Retrieves the TDC measurment window upper bound based on the sensor placement.
   *
   * @param [in] element Detector element (module) of the activated sensor.
   *
   * @return Upper bound (in ns) of the TDC measurment window.
   */

  float activeWindowUpperBound(const InDetDD::SolidStateDetectorElementBase* element) const;

  /**
   * @brief Simulate the TDC digitization.
   *
   * @param [in] element  Detector element (module) of the activated sensor.
   * @param [in] hit_time Time as measured by the LGAD sensor
   *
   * @return 8-bit TOA ALTIROC code which is stored in the ALTIROC RDO.
   */
   uint8_t Time2TOA(const InDetDD::SolidStateDetectorElementBase* element, float   hit_time) const;

  /**
   * @brief Retrieve time of flight from 7-bit TOA code applying the TDC calibration parameters.
   *
   * @param [in] element Detector element (module) the RDOs were found on.
   * @param [in] toa     7-bit TOA code as stored in the RDO
   *
   * @return Absolute time to be used on the HGTD_Clusters.
   */
  float   TOA2Time(const InDetDD::SolidStateDetectorElementBase* element, uint8_t toa) const;

  /**
  * @brief Check for the overflow flag within TOA code, which indicates an out of range measurement.
  *
  * @param [in] toa     8-bit TOA code as produced by ALTIROC
  *
  * @return True if overflow bit is set, false otherwise.
  */
  bool   checkTOAoverflow(uint8_t toa) const {
    return toa & HGTD::TOA_OVERLFLOW_MASK;
  }


  private:

  using ToaBinSizes = std::vector<float>;

  struct CalibrationCache {
    const CondAttrListCollection* source{nullptr};
    std::shared_ptr<const ToaBinSizes> toaBinSizes;
  };

  /** @brief Get the effective TOA bin sizes for the current conditions IOV. */
  std::shared_ptr<const ToaBinSizes> getToaBinSizes() const;

  /** @brief Read one TOA bin width per consecutive conditions channel. */
  ToaBinSizes readToaBinSizes(const CondAttrListCollection& attrListColl) const;

  /** @brief Convert a TOA code using an already-loaded calibration vector. */
  float toa2Time(const InDetDD::SolidStateDetectorElementBase* element,
                 uint8_t toa, const ToaBinSizes& toaBinSizes) const;

  FloatProperty m_active_window{this, "PS_ActiveRange", 2.5 * Athena::Units::nanosecond,
    "ALTIROC PS active range" };

  FloatProperty m_lhc_rise_edge{this, "LHC_RiseEdge",12.5 * Athena::Units::nanosecond,
    "LHC clock rise edge time" };

  FloatProperty m_ps_large_step{this, "PS_LargeStep", 1.562 * Athena::Units::nanosecond,
    "ALTIROC PS large step"};

  FloatProperty m_ps_small_step{this, "PS_SmallStep", 9.7 * Athena::Units::picosecond,
    "ALTIROC PS small step"};

  FloatProperty m_toa_bin_size {this, "TOABinSize", 20 * Athena::Units::picosecond,
    "Nominal TDC TOA bin size (fallback when conditions DB not available)"};

  SG::ReadCondHandleKey<CondAttrListCollection> m_calibDataKey{
      this, "TdcCalibKey", "/HGTD/Calibration/TdcBinSize",
      "Key of the TDC calibration conditions folder"};

  BooleanProperty m_useCondDB{this, "UseCondDB", false,
      "If true, read toa_bin_size from conditions DB instead of property"};

  std::shared_ptr<const ToaBinSizes> m_fallbackBinSizes;
  mutable SG::SlotSpecificObj<std::mutex> m_cacheMutex ATLAS_THREAD_SAFE;
  mutable SG::SlotSpecificObj<CalibrationCache> m_cache ATLAS_THREAD_SAFE;

};

#endif // HGTD_TDCCALIBRATIONTOOL_H
