/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDET_STRIPMODULEHELPER_H
#define INDET_STRIPMODULEHELPER_H

#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include <cassert>
#include <array>
#include <stdexcept>
#include <algorithm>
#include "ModuleKeyHelper.h"

namespace InDet {
   /** Helper class to convert between offline column, row and hardware chip, column, row coordinates.
    */
   class StripModuleHelper : public ModuleKeyHelper<unsigned short, // key type
                                                    12,  // bits for rows
                                                    0,   // bits for columns
                                                    0   // bits for chip
                                                    > {
   public:


      // mask with every bit set for all chips, columns, rows, but the mask index bits.
      static constexpr unsigned int getStripMask()     { return MaskUtils::createMask<0,ROW_BITS+CHIP_BITS>(); }
      // mask with row and column bits set to zero.
      static constexpr unsigned int getChipMask()      { return MaskUtils::createMask<ROW_BITS,ROW_BITS+CHIP_BITS>(); }


      StripModuleHelper(const InDetDD::SiDetectorDesign &design)
      {
         const InDetDD::SCT_ModuleSideDesign *stripModuleDesign = dynamic_cast<const InDetDD::SCT_ModuleSideDesign *>(&design);
         if (stripModuleDesign) {
            m_rows = stripModuleDesign->cells(); // strips
            m_columns=1u;
         }
      }
      static constexpr unsigned int N_MASKS = 1;
      static constexpr unsigned int nMasks() { return N_MASKS; }
      std::array<unsigned int, N_MASKS> masks() const {
         return std::array<unsigned int,N_MASKS> {
            StripModuleHelper::getStripMask()
         };
      }

      operator bool () const { return m_rows>0; }

      unsigned int columns() const { return m_columns; }
      unsigned int rows() const { return m_rows; }
      static constexpr unsigned int columnsPerCircuit()  { return 1u; }
      unsigned int rowsPerCircuit() const { return m_rows; }
      static constexpr unsigned int circuitsPerColumn()  { return 1u; }
      static constexpr unsigned int circuitsPerRow()  { return 1u; }

      static constexpr unsigned int columnsPerMask([[maybe_unused]] unsigned int mask_idx) { return 1u;}

      /** Compute "hardware" coordinates from offline coordinates.
       * @param row offline row aka. phi index
       * @param column offline column aka. eta index
       * @return packed triplet of chip, column, row.
      */
      KEY_TYPE hardwareCoordinates(unsigned int row, unsigned int column) const {
         unsigned int chip =0;
         if (circuitsPerColumn()>1) {
            chip += (row/rowsPerCircuit()) * circuitsPerRow();
            row = row % rowsPerCircuit();
            if (chip>0) {
               row = rowsPerCircuit() - row -1;
               column = columns() - column -1;
            }
         }
         if (circuitsPerRow()>1) {
            chip += column/columnsPerCircuit();
            column = column%columnsPerCircuit();
         }
         return makeKey(0u, chip, column, row);
      }
      /** compute offline coordinates from "hardware" coordinates
       * @param key packed hardware coordinates
       * @return offline row, column pair
      */
      std::pair<unsigned int,unsigned int> offlineCoordinates(unsigned int key) const {
         unsigned int chip = getChip(key);
         unsigned int column = getColumn(key);
         unsigned int row = getRow(key);
         // handle special values
         // used for merging
         // should not occur for strips
         if (row == getLimitRowMax()) {
            row=rowsPerCircuit()-1;
         }
         if (row == rowsPerCircuit()) {
            column+=1u;
            row=0u;
         }

         column+= columnsPerCircuit() * (chip%circuitsPerRow());
         if (chip>=circuitsPerRow()) {
            column=columns() - column -1;
            row=rowsPerCircuit() - row -1;
            row+=rowsPerCircuit() * (chip/circuitsPerRow());
         }
         if (row>nSensorRows() || column>nSensorColumns()) {
            throw std::runtime_error("Invvalid offline coordinates");
         }
         return std::make_pair(row,column);
      }

      /** Return the total number strips of this module.
       */
      unsigned int nCells() const {
         return m_columns * m_rows;
      }
      /** Number of offline columns aka. upper bound of eta index
       */
      unsigned int nSensorColumns() const {
         return m_columns;
      }
      /** Number of offline rows aka. upper bound of phi index
       */
      unsigned int nSensorRows() const {
         return m_rows;
      }
      /** return the maximum number of unique mask (or group) defects per module.
       * For strips there is a single mask which covers all strips.
       */
      unsigned int nElements([[maybe_unused]] unsigned int mask_i) const {
         assert( mask_i==0);
         return nCells();
      }

      /** Convenience function to return offline column and row ranges matching the defect-area of the given key (for histogramming
       * @param key packed hardware coordinates addressing a single strip (or a group defect)
       * @return offline start column, end column, start row, end row, where the end is meant not to be inclusive i.e. [start, end)
       */
      std::array<unsigned int,4> offlineRange(const std::pair<unsigned int,unsigned int> &range) const {
         if (range.first != range.second) {
            // if (getRow(range.first) !=0) {
            //    throw std::runtime_error("invalid key");
            // };

            std::pair<unsigned int, unsigned int> start=offlineCoordinates(range.first);
            std::pair<unsigned int, unsigned int> end=offlineCoordinates(range.second);
            return std::array<unsigned int,4>{ std::min(start.first, end.first),   std::max(start.first, end.first)+1,
                                               std::min(start.second, end.second), std::max(start.second,end.second)+1};
         }
         else {
            std::pair<unsigned int, unsigned int> start=offlineCoordinates(range.first);
            return std::array<unsigned int,4>{ start.first,  start.first+1,
                                               start.second, start.second+1 };
         }
      }

      static constexpr bool swapOfflineRowsColumns() { return false;}
   private:

      unsigned short m_rows = 0;
      unsigned short m_columns = 0;
   };
}
#endif
