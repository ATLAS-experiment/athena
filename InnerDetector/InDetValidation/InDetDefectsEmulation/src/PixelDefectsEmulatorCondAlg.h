// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_PIXELDEFECTSEMULATORCONDALG_H
#define INDET_PIXELDEFECTSEMULATORCONDALG_H

#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "PixelEmulatedDefects.h"
#include "PixelModuleHelper.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "DefectsEmulatorCondAlgImpl.h"
#include "InDetIdentifier/PixelID.h"

namespace InDet {

   class PixelDefectsEmulatorCondAlg;
   namespace detail {
      template <>
      struct DetectorEmulatorCondAlgTraits<PixelDefectsEmulatorCondAlg> {
         using T_ID = PixelID;
         using T_ModuleHelper = PixelModuleHelper;
         using T_EmulatedDefects = InDet::PixelEmulatedDefects;
         using T_DetectorElementCollection = InDetDD::SiDetectorElementCollection;
         using T_ModuleDesign = InDetDD::PixelModuleDesign;
      };
   }

   /** Conditions algorithms for emulating ITK pixel defects.
    * The algorithm mask random pixels, core columns (group of 8 or 4 consecutive columns of a chip),
    * circuits or modules as defect. This data can be used to reject RDOs overlapping with these defects.
    */
   class PixelDefectsEmulatorCondAlg : public DefectsEmulatorCondAlgImpl<PixelDefectsEmulatorCondAlg>
   {
   public:
      friend class DefectsEmulatorCondAlgImpl<PixelDefectsEmulatorCondAlg>;

      using DefectsEmulatorCondAlgImpl<PixelDefectsEmulatorCondAlg>::DefectsEmulatorCondAlgImpl;

      virtual StatusCode initialize() override final;

   protected:

      /** The name of the PixelID identifier utility.
       */
      static std::string IDName() { return std::string("PixelID"); }

      /** Get the map which defines which modules are connected to the same physical sensor.
       * For pixel each physical sensor is connected to exactly one module, so an empty
       * map is returned.
       */
      static std::unordered_multimap<unsigned int, unsigned int> getModuleConnectionMap([[maybe_unused]] const InDetDD::SiDetectorElementCollection &det_ele) {
         return std::unordered_multimap<unsigned int, unsigned int> ();
      }
      /** Provide alternative method to mark modules as defect.
       * For pixel there is no alternative method. So, no module is marked as defect by this method.
       */
      static bool isModuleDefect([[maybe_unused]]const EventContext &ctx, [[maybe_unused]]unsigned int id_hash) {
         return false;
      }
   };
}
#endif
