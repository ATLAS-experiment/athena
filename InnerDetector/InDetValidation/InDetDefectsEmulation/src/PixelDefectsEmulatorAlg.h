/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
      using ID_Helper = const PixelID *;
      struct IDAdapter {
         ID_Helper m_idHelper;

         IDAdapter(ID_Helper helper) : m_idHelper(helper) {}
         int row_index(const Identifier &rdoID) const { return m_idHelper->phi_index(rdoID); }
         int col_index(const Identifier &rdoID) const { return m_idHelper->eta_index(rdoID); }

         unsigned int cloneOrRejectHit( const PixelModuleHelper &module_helper,
                                        const PixelEmulatedDefects &emulated_defects,
                                        unsigned int idHash,
                                        unsigned int row_idx,
                                        unsigned int col_idx,
                                        const PixelRDORawData &rdo,
                                        InDetRawDataCollection<PixelRDORawData> &dest) {
            unsigned int n_new=0u;
            if (!emulated_defects.isDefect(module_helper, idHash, row_idx, col_idx)) {
               dest.push_back(std::make_unique<Pixel1RawData>(dynamic_cast<const Pixel1RawData &>(rdo)).release() );
               ++n_new;
            }
            return n_new;
         }

      };
      using DefectsData = PixelEmulatedDefects;
      using RDORawData =  PixelRDORawData;
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
