/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_EMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALG_H
#define INDET_EMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALG_H

#include "EmulatedDefectsToDetectorElementStatusCondAlgBase.h"


namespace InDet {

   namespace details {
      template <class T_Derived>
      struct EmulatedDefectsToDetectorElementStatusTraits;
   }

   template <class T_Derived>
   class EmulatedDefectsToDetectorElementStatusCondAlg : public EmulatedDefectsToDetectorElementStatusCondAlgBase
   {
   public:
      using T_ConcreteDetectorElementStatusType  = details::EmulatedDefectsToDetectorElementStatusTraits<T_Derived>::T_ConcreteDetectorElementStatusType;
      using T_EmulatedDefects  = details::EmulatedDefectsToDetectorElementStatusTraits<T_Derived>::T_EmulatedDefects;
      using T_ModuleHelper  = details::EmulatedDefectsToDetectorElementStatusTraits<T_Derived>::T_ModuleHelper;
      using T_EmulatedDefectsKey = typename T_ModuleHelper::KEY_TYPE;
      static constexpr unsigned int CHIP_MASK_IDX = details::EmulatedDefectsToDetectorElementStatusTraits<T_Derived>::CHIP_MASK_IDX;

      using EmulatedDefectsToDetectorElementStatusCondAlgBase::EmulatedDefectsToDetectorElementStatusCondAlgBase;

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

   protected:
      static ChipFlags_t makeChipStatus(const InDetDD::SiDetectorElement &detector_element,
                                        const std::vector<T_EmulatedDefectsKey> &emulated_module_defects) {
         T_ModuleHelper module_helper( detector_element.design() );
         ChipFlags_t chip_status=(1u<<module_helper.circuitsPerColumn()*module_helper.circuitsPerRow())-1;
         if constexpr(CHIP_MASK_IDX < T_ModuleHelper::N_MASKS) {
            for (T_EmulatedDefectsKey key : emulated_module_defects) {
               if (module_helper.getMaskIdx(key) == CHIP_MASK_IDX) {
                  assert( module_helper.getChip(key) < 31 && (1u<<module_helper.getChip(key) ) < std::numeric_limits<ChipFlags_t>::max());
                  chip_status &= ~(1<<(module_helper.getChip(key)));
               }
            }
         }
         return chip_status;
      };


      SG::ReadCondHandleKey<T_EmulatedDefects> m_emulatedDefectsKey
         {this, "EmulatedDefectsKey", "", "Key of the emulated defects input collection"};

   };
}

#include "EmulatedDefectsToDetectorElementStatusCondAlg.icc"
#endif
