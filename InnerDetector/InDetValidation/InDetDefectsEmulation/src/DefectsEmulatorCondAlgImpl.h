// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_DEFECTSEMULATORCONDALGIMPL_H
#define INDET_DEFECTSEMULATORCONDALGIMPL_H

#include "DefectsEmulatorCondAlgBase.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

namespace InDet {

   namespace detail {
      template <class T_Derived>
      struct DetectorEmulatorCondAlgTraits {
         using T_ID = T_Derived::T_ID;
         using T_ModuleHelper = T_Derived::T_ModuleHelper;
         using T_EmulatedDefects = T_Derived::T_EmulatedDefects;
         using T_DetectorElementCollection = InDetDD::SiDetectorElementCollection;
         using T_ModuleDesign = T_Derived::T_ModuleDesign;
      };
   }

   /** Conditions algorithms for emulating ITK pixel or strip defects.
    * The algorithm mask random modules, pixels or strips or groups thereof as
    * defect. This data can be used to reject hits which overlap with these defects.
    */
   template <class T_Derived>
   class DefectsEmulatorCondAlgImpl : public DefectsEmulatorCondAlgBase
   {
   public:
      using T_ID = detail::DetectorEmulatorCondAlgTraits<T_Derived>::T_ID;
      using T_ModuleHelper = detail::DetectorEmulatorCondAlgTraits<T_Derived>::T_ModuleHelper;
      using T_EmulatedDefects = detail::DetectorEmulatorCondAlgTraits<T_Derived>::T_EmulatedDefects;
      using T_DetectorElementCollection = detail::DetectorEmulatorCondAlgTraits<T_Derived>::T_DetectorElementCollection;
      using T_ModuleDesign = detail::DetectorEmulatorCondAlgTraits<T_Derived>::T_ModuleDesign;

      using DefectsEmulatorCondAlgBase::DefectsEmulatorCondAlgBase;

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

   protected:
      const T_Derived &derived() const { return *static_cast<const T_Derived *>(this); }

      SG::ReadCondHandleKey<T_DetectorElementCollection> m_detEleCollKey
         {this, "DetEleCollKey", "", "Key of SiDetectorElementCollection"};

      SG::WriteCondHandleKey<T_EmulatedDefects> m_writeKey
         {this, "WriteKey", "", "Key of output emulated defect conditions data"};

      Gaudi::Property<std::string> m_idName
         {this, "IDName", "", "Name of the ID tool to decode identifiers"};

      const T_ID* m_idHelper = nullptr;

   };
}

#include "DefectsEmulatorCondAlgImpl.icc"
#endif
