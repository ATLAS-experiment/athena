/**
* Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*
* @file HGTD_Calibration/src/HGTD_TdcCalibrationTool.h
*
* @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
*
* @date May, 2025
*
* @brief Simulation of ALTIROC Phase Shifter
*/

#ifndef HGTD_TDCCALIBRATIONTOOL_H
#define HGTD_TDCCALIBRATIONTOOL_H

#include <array>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/Units.h"

#include "CLHEP/Random/RandomEngine.h"

#include "HGTD_RawData/HGTD_ALTIROC_RDO_Collection.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetSimEvent/SiHit.h"
#include "SiDigitization/SiSurfaceCharge.h"


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
  // Not overriding for now
  // virtual StatusCode initialize() override final;

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
   * @return 7-bit TOA ALTIROC code which is stored in the ALTIROC RDO.
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

  private:

  FloatProperty m_active_window{this, "PS_ActiveRange", 2.5 * Athena::Units::ns,
    "ALTIROC PS active range (ns)" };

  FloatProperty m_lhc_rise_edge{this, "LHC_RiseEdge",12.5 * Athena::Units::ns,
    "LHC clock rise edge time (ns)" };
    
  FloatProperty m_ps_large_step{this, "PS_LargeStep", 1.562 * Athena::Units::ns,
    "ALTIROC PS large step (ns)"};
    
  FloatProperty m_ps_small_step{this, "PS_SmallStep", 0.097 * Athena::Units::ns,
    "ALTIROC PS small step (ns)"};

  FloatProperty m_toa_bin_size {this, "TOABinSize", 0.02 * Athena::Units::ns, 
    "Nominal TDC TOA bin size (ns)"};

};

#endif // HGTD_TDCCALIBRATIONTOOL_H
 
