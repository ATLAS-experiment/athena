// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_STRIPDEFECTSEMULATORCONDALG_H
#define INDET_STRIPDEFECTSEMULATORCONDALG_H

#include "DefectsEmulatorCondAlgImpl.h"

#include "StripEmulatedDefects.h"
#include "StripModuleHelper.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "SCT_ConditionsTools/ISCT_ConditionsTool.h"
#include "InDetIdentifier/SCT_ID.h"

#include "ConnectedModulesUtil.h"

namespace InDet {

   class StripDefectsEmulatorCondAlg;
   namespace detail {
      template <>
      struct DetectorEmulatorCondAlgTraits<StripDefectsEmulatorCondAlg> {
         using T_ID = SCT_ID;
         using T_ModuleHelper = StripModuleHelper;
         using T_EmulatedDefects = InDet::StripEmulatedDefects;
         using T_DetectorElementCollection = InDetDD::SiDetectorElementCollection;
         using T_ModuleDesign = InDetDD::SCT_ModuleSideDesign;
      };
   }

   /** Conditions algorithms for emulating ITK strip defects.
    * The algorithm mask random strips and modules as defect. This data can be used to reject hits overlapping with
    * these defects.
    */
   class StripDefectsEmulatorCondAlg : public DefectsEmulatorCondAlgImpl<StripDefectsEmulatorCondAlg>
   {
   public:
      friend class DefectsEmulatorCondAlgImpl<StripDefectsEmulatorCondAlg>;

      using DefectsEmulatorCondAlgImpl<StripDefectsEmulatorCondAlg>::DefectsEmulatorCondAlgImpl;

      virtual StatusCode initialize() override final;

   protected:
      ToolHandle<ISCT_ConditionsTool> m_moduleVetoTool
         {this,"SCTConditionsTool","","Optional SCT conditions tool to mark modules as defect (alternative to ModulePattern)"};

      /** Name of the SCT_ID helper.
       */
      static std::string IDName() { return std::string("SCT_ID"); }

      /** Return the map which defines which modules are connected to the same sensor as a reference module.
       */
      std::unordered_multimap<unsigned int, unsigned int> getModuleConnectionMap(const InDetDD::SiDetectorElementCollection &det_ele_coll) const {
         return ConnectedModulesUtil::createModuleConnectionMap(*m_idHelper, det_ele_coll);
      }
      /** Alternative method to declare strip modules as defect.
       * If the module veto tool is enabled, the tool can be used alternatively to declare strip modules as defect.
       */
      bool isModuleDefect(const EventContext &ctx, unsigned int id_hash) const {
         return (m_moduleVetoTool.isEnabled() &&  !m_moduleVetoTool->isGood(id_hash, ctx));
      }
   };
}
#endif
