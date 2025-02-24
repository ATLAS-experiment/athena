/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef InDet_StripDefectsEmulatorAlg_H
#define InDet_StripDefectsEmulatorAlg_H

#include "DefectsEmulatorAlg.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "StripEmulatedDefects.h"
#include "InDetIdentifier/SCT_ID.h"

namespace InDet {

   template <>
   struct DefectsEmulatorTraits<SCT_RDO_Container> {

      using ID_Helper = const  SCT_ID*;

      /** Adapter to adapt DefectEmulatorAlg for strips
       */
      struct IDAdapter {
         ID_Helper m_idHelper;
         static const std::vector<int> s_dummyvector;

         IDAdapter(ID_Helper helper) : m_idHelper(helper) {}
         int row_index(const Identifier &rdoID) const { return m_idHelper->strip(rdoID); }
         int col_index(const Identifier &rdoID) const { return m_idHelper->row(rdoID); }

         /** Clone, reject or split strip RDOs depending on overlaps with defects.
          */
         unsigned int cloneOrRejectHit( const StripModuleHelper &module_helper,
                                        const StripEmulatedDefects &emulated_defects,
                                        unsigned int idHash,
                                        unsigned int row_idx_aka_phi,
                                        unsigned int col_idx_aka_eta,
                                        const SCT_RDORawData &rdo,
                                        InDetRawDataCollection<SCT_RDORawData> &dest) {
            unsigned int n_new=0u;
            assert( rdo.getGroupSize() > 0);
            // search for all defects starting from the last strip in the group (search is performed in descending order)
            StripModuleHelper::KEY_TYPE rdo_end_key = module_helper.hardwareCoordinates(row_idx_aka_phi+rdo.getGroupSize()-1, col_idx_aka_eta);
            auto [defect_iter, end_iter] =emulated_defects.lower_bound(idHash, rdo_end_key);

            //  A) 0 defect overlapping with strip group
            //      => lower_bound(strip+group_size()-1) < strip-mask+1     -> clone
            //  B) 1 defect overlapping
            //     => lower_bound(strip+group_size()-1) < strip >= strip-mask+1 -> hit[strip .. defect-1], hit[defect+mask.. strip+group_size-1]
            //        iter++ < strip-mask+1
            //  C) n defects overlapping
            //        => lower_bound(strip+group_size()-1) < strip >= strip-mask+1  -> hit[defect+mask.. strip+group_size-1],
            //                                                                         defect+mast>strip  ?  hit[defect+mask.. last_defect-1] -> loop
            //                                                                                            : hit[strip..last_defect-1] -> end

            StripModuleHelper::KEY_TYPE rdo_key = module_helper.hardwareCoordinates(row_idx_aka_phi, col_idx_aka_eta);
            unsigned int strip = module_helper.getRow(rdo_key);
            std::pair<unsigned int, unsigned int> overlap = (defect_iter != end_iter
                                                             ?  getOverlap( module_helper,
                                                                            *defect_iter,
                                                                            strip,
                                                                            rdo.getGroupSize())
                                                             : std::make_pair(0u,0u));
            if (overlap.second==0) {
               dest.push_back(std::make_unique<SCT3_RawData>(dynamic_cast<const SCT3_RawData &>(rdo)).release() );
               ++n_new;
            }
            else if (static_cast<unsigned int>(rdo.getGroupSize()) != overlap.second) {
               const SCT3_RawData &sct3_rdo = dynamic_cast<const SCT3_RawData &>(rdo);
               Identifier module_id = m_idHelper->module_id(rdo.identify());

               unsigned int last_defect_start = strip + rdo.getGroupSize();
               for (;;) {
                  unsigned int start = overlap.first + overlap.second;
                  unsigned int group_size = last_defect_start - start;
                  if (group_size==0u) break;
                  dest.push_back(std::make_unique<SCT3_RawData>( m_idHelper->strip_id(module_id,start, col_idx_aka_eta),
                                                                 makeStripWord( sct3_rdo.getTimeBin(), start, group_size, getErrorBits(rdo) ),
                                                                 &s_dummyvector));
                  ++n_new;
                  if (defect_iter == end_iter) break;

                  last_defect_start = overlap.first;
                  overlap = std::make_pair(strip, 0u);
                  ++defect_iter;
                  if (defect_iter != end_iter) {
                     overlap = getOverlap( module_helper,
                                           *defect_iter,
                                           strip,
                                           rdo.getGroupSize());
                     if (overlap.second == 0u) {
                        overlap.first= strip;
                     }
                  }
               }
            }
            return n_new;
         }

      protected:
         /** Convenience method to create the strip word from the various components
          * @param time_bin the time bin of the strip cluster (3-bits)
          * @param strip the first strip of the strip cluster
          * @param group_size the number of adjacent strips in this cluster
          * @param error_bits (6-bits)
          * @return the packed word.
          */
         static inline unsigned int makeStripWord( unsigned int time_bin, unsigned int strip, unsigned int group_size, unsigned int error_bits) {
            assert((group_size & (~0x7FFu)) == 0u);
            assert((strip & (~0x7FFu)) == 0u);
            assert((time_bin & (~0x7u)) == 0u);
            assert((error_bits & (~0x3Fu)) == 0u );
            unsigned int word  = group_size | (strip<<11u) | (time_bin<<22u) | (error_bits <<25u);
            return word;
         }
         /** Convenience method to extract the error component from the packed word of the strip RDO.
          */
         static inline unsigned int getErrorBits( const SCT_RDORawData &rdo ) {
            return (rdo.getWord() >>25u) & 0x7u;
         }
         /** Convenience function to return the defect region overlapping with the strip group.
          * @param module_helper the strip module helper
          * @param defect_key the key which defines the defect
          * @param strip the start strip of a strip group
          * @param sequence_length the number of subsequent strips in this group.
          * @return a pair containing the start strip and number of strips of the overlapping defect area
          * Will compute the region of the given defect which overlaps with the given strip group.
          */
         std::pair<unsigned int,unsigned int> getOverlap( const StripModuleHelper &module_helper,
                                                          StripModuleHelper::KEY_TYPE defect_key,
                                                          unsigned int strip,
                                                          unsigned int sequence_length) {
            static_assert( StripModuleHelper::CHIP_MASK == StripModuleHelper::KEY_TYPE{});
            static_assert( StripModuleHelper::COL_MASK == StripModuleHelper::KEY_TYPE{});
            unsigned int defect_row = module_helper.getRow(defect_key);
            unsigned int strip_row = strip;
            unsigned int mask=module_helper.getMask(defect_key);
            if (mask >1u) {
               if (defect_row+mask >= strip_row && defect_row < strip_row+sequence_length) {
                  return std::make_pair( defect_row > strip_row ? defect_row : strip_row,
                                         (defect_row+mask <= strip_row+sequence_length ? mask :  (strip_row+sequence_length) - defect_row ));
               }
            }
            else {
               if (defect_row >= strip_row && defect_row < strip_row+sequence_length) {
                  return std::make_pair(defect_row,1u);
               }
            }
            return std::make_pair(0u,0u);
         }


      };

      using DefectsData = StripEmulatedDefects;
      using RDORawData =  SCT_RDORawData;
      using ModuleHelper = StripModuleHelper;
   };

   /** Algorithm which selectively copies hits from an input SCT_RDO_Container.
    *
    * Hits will be copied unless they are marked as defects in the "defects"
    * conditions data. This is a specialization of DefectsEmulatorAlg for SCT_RDOs.
    * RDOs referring to a group of strips are split if overlapping with a
    * defect not covering the full group of strips.
    */
   class StripDefectsEmulatorAlg :public DefectsEmulatorAlg<SCT_RDO_Container>
   {
   public:
      using DefectsEmulatorAlg<SCT_RDO_Container>::DefectsEmulatorAlg;
   };
}
#endif
