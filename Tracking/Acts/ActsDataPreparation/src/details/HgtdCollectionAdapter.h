#ifndef ACTSTRK_HGTDCOLLECTIONADAPTER_H
#define ACTSTRK_HGTDCOLLECTIONADAPTER_H

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"

namespace ActsTrk {
   struct HgtdCollectionAdapterBase {
      HgtdCollectionAdapterBase(const HGTD_DetectorManager &hgtd_det_mgr,
                            const HGTD_TdcCalibrationTool &hgtd_tdc_calib_tool,
                            Identifier id)
         : m_element(hgtd_det_mgr.getDetectorElement(id)),
           m_hgtd_tdc_calib_tool(&hgtd_tdc_calib_tool)
      { assert(m_element); }

      Identifier identify() const {return m_element->identify(); }
      const InDetDD::HGTD_DetectorElement &element() const { return *m_element; }
   protected:
      const InDetDD::HGTD_DetectorElement *m_element;
      const HGTD_TdcCalibrationTool *m_hgtd_tdc_calib_tool;
   };

   
   template <typename T_RDOCollection>
   struct HgtdCollectionAdapter;

   template <>
   struct HgtdCollectionAdapter<HGTD_RDO_Collection>
      : HgtdCollectionAdapterBase {
      using HgtdCollectionAdapterBase::HgtdCollectionAdapterBase;
      static float calibratedTime(const HGTD_RDO &rdo) {
         return rdo.getTOA();
      }
      static float calibratedTime(const HGTD_RDO &rdo, [[maybe_unused]] std::uint8_t raw_time) {
         // @TODO or rather convert digitized time ? 
         return rdo.getTOA();
      }
      uint8_t rawTime(const HGTD_RDO &rdo) const {
         return std::min(m_hgtd_tdc_calib_tool->Time2TOA(m_element, rdo.getTOA()), static_cast<std::uint8_t>(127));
      }
      static uint16_t ToT(const HGTD_RDO &rdo) {
         return rdo.getTOT();
      }
   };
   template <>
   struct HgtdCollectionAdapter<HGTD_ALTIROC_RDO_Collection>
      : HgtdCollectionAdapterBase
   {
      using HgtdCollectionAdapterBase::HgtdCollectionAdapterBase;
      float calibratedTime(const HGTD_ALTIROC_RDO &rdo) const {
         return m_hgtd_tdc_calib_tool->TOA2Time(m_element, rdo.getToA());
      }
      float calibratedTime([[maybe_unused]] const HGTD_ALTIROC_RDO &rdo, std::uint8_t raw_time) const {
         return m_hgtd_tdc_calib_tool->TOA2Time(m_element, raw_time);
      }
      static uint8_t rawTime(const HGTD_ALTIROC_RDO &rdo) {
         return rdo.getToA();
      }
      static uint16_t ToT(const HGTD_ALTIROC_RDO &rdo) {
         return rdo.getToT();
      }
   };
}
#endif
