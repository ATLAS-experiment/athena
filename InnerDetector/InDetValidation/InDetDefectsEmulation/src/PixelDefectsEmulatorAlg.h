/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef InDet_PixelDefectsEmulatorAlg_H
#define InDet_PixelDefectsEmulatorAlg_H

#include "DefectsEmulatorAlg.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "PixelEmulatedDefects.h"
#include "InDetIdentifier/PixelID.h"

namespace InDet {
   template <>
   struct DefectsEmulatorTraits<PixelRDO_Container> {
      using ID_Helper =   PixelID;
      using DefectsData = PixelEmulatedDefects;
      using RDORawData =  PixelRDORawData;
      using  RDORawDataConcreteType = Pixel1RawData;
      using ModuleHelper = PixelModuleHelper;
   };

   /** Algorithm which selectively copies hits from an input PixelRDO_Container.
    *
    * Hits will be copied unless they are marked as defects in the "defects"
    * conditions data. This is a specialization of DefectsEmulatorAlg for PixelRDOs
    */
   class PixelDefectsEmulatorAlg :public DefectsEmulatorAlg<PixelRDO_Container>
   {
   public:
      using DefectsEmulatorAlg<PixelRDO_Container>::DefectsEmulatorAlg;
   };
}
#endif
