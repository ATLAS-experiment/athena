/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDET_PIXELMODULEHELPER_H
#define INDET_PIXELMODULEHELPER_H

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include <cassert>
#include <array>
#include <stdexcept>
#include "ModuleKeyHelper.h"

namespace InDet {
   /** Helper class to convert between offline column, row and hardware chip, column, row coordinates.
    */
   class PixelModuleHelper : public ModuleKeyHelper<unsigned int, // key type
                                                    12,  // bits for rows
                                                    12,  // bits for columns
                                                    4,   // bits for chip
                                                    2    // bits for defect type
                                                    > {
   public:

      static constexpr std::array<unsigned short,2> N_COLS_PER_GROUP {
         8,  //8 columns per group for square pixels
         4}; //4 columns per group for rectangular pixels

      // mask with every bit set for all chips, columns, rows, but the mask index bits.
      static constexpr unsigned int getPixelMask()     { return MaskUtils::createMask<0,ROW_BITS+COL_BITS+CHIP_BITS>(); }
      // mask with row and lowest 3 column bits set to zero i.e. 8 adjacent columns
      static constexpr unsigned int getColGroup8Mask() { return MaskUtils::createMask<ROW_BITS+3,ROW_BITS+COL_BITS+CHIP_BITS>(); }
      // mask with row and lowest 2 column bits set to zero i.e. 4 adjacent columns
      static constexpr unsigned int getColGroup4Mask() { return MaskUtils::createMask<ROW_BITS+2,ROW_BITS+COL_BITS+CHIP_BITS>(); }
      // mask with row and column bits set to zero.
      static constexpr unsigned int getChipMask()      { return MaskUtils::createMask<ROW_BITS+COL_BITS,ROW_BITS+COL_BITS+CHIP_BITS>(); }


      PixelModuleHelper(const InDetDD::SiDetectorDesign &design)
      {
         const InDetDD::PixelModuleDesign *pixelModuleDesign = dynamic_cast<const InDetDD::PixelModuleDesign *>(&design);
         if (pixelModuleDesign) {
         m_sensorColumns = pixelModuleDesign->columns();
         m_sensorRows = pixelModuleDesign->rows();
         if (pixelModuleDesign->rowsPerCircuit()==400 /* @TODO find a better way to identify when to swap columns and rows*/ ) {
            // the front-ends of ITk ring triplet modules are rotated differently
            // wrt. the offline coordinate system compared to quads and
            // barrel triplets. Once these modules are identified, the translation
            // works in exactly the same way, but columns and rows need to be swapped,
            m_swapOfflineRowsColumns=true;
            m_columns = pixelModuleDesign->rows();
            m_rows = pixelModuleDesign->columns();
            m_columnsPerCircuit = pixelModuleDesign->rowsPerCircuit();
            m_rowsPerCircuit = pixelModuleDesign->columnsPerCircuit();
            m_circuitsPerColumn = pixelModuleDesign->numberOfCircuitsPerRow();
            m_circuitsPerRow = pixelModuleDesign->numberOfCircuitsPerColumn();
            m_columnPitch = pixelModuleDesign->phiPitch();
            m_rowPitch = pixelModuleDesign->etaPitch();
         }
         else {
            m_swapOfflineRowsColumns=false;
            m_rows = pixelModuleDesign->rows();
            m_columns = pixelModuleDesign->columns();
            m_rowsPerCircuit = pixelModuleDesign->rowsPerCircuit();
            m_columnsPerCircuit = pixelModuleDesign->columnsPerCircuit();
            m_circuitsPerRow = pixelModuleDesign->numberOfCircuitsPerRow();
            m_circuitsPerColumn = pixelModuleDesign->numberOfCircuitsPerColumn();
            m_columnPitch = pixelModuleDesign->etaPitch();
            m_rowPitch = pixelModuleDesign->phiPitch();
         }
         m_rectangularPixels = (m_columns==200);
         }
      }
      static constexpr unsigned int N_MASKS=3;
      static constexpr unsigned int nMasks() { return N_MASKS; }
      std::array<unsigned int, N_MASKS> masks() const {
         return std::array<unsigned int,N_MASKS> {
            PixelModuleHelper::getPixelMask(),
            (m_rectangularPixels ? PixelModuleHelper::getColGroup4Mask() : PixelModuleHelper::getColGroup8Mask() ),
            PixelModuleHelper::getChipMask()
         };
      }
      operator bool () const { return m_columns>0; }

      unsigned int columns() const { return m_columns; }
      unsigned int rows() const { return m_rows; }
      unsigned int columnsPerCircuit() const { return m_columnsPerCircuit; }
      unsigned int rowsPerCircuit() const { return m_rowsPerCircuit; }
      unsigned int circuitsPerColumn() const { return m_circuitsPerColumn; }
      unsigned int circuitsPerRow() const { return m_circuitsPerRow; }

      float columnPitch() const { return m_columnPitch; }
      float rowPitch() const { return m_rowPitch; }

      /** compute "hardware" coordinates from offline coordinates.
       * @param row offline row aka. phi index
       * @param column offline column aka. eta index
       * @return packed triplet of chip, column, row.
      */
      unsigned int hardwareCoordinates(unsigned int row, unsigned int column) const {
         unsigned int chip =0;
         if (swapOfflineRowsColumns()) {
            unsigned int tmp=row;
            row=column;
            column=tmp;
         }
         if (circuitsPerColumn()>1) {
            assert( circuitsPerColumn() == 2);
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
         if (swapOfflineRowsColumns()) {
            std::swap(column,row);
         }
         if (row>=nSensorRows() || column>=nSensorColumns()) {
            throw std::runtime_error("Invvalid offline coordinates");
         }
         return std::make_pair(row,column);
      }

      /** Return total number of pixels per module.
       */
      unsigned int nCells() const {
         return nSensorColumns() * nSensorRows();
      }
      /** Return the number of offline columns
       */
      unsigned int nSensorColumns() const {
         return m_sensorColumns;
      }
      /** Return the number of offline rows
       */
      unsigned int nSensorRows() const {
         return m_sensorRows;
      }
      /** return the maximum number of unique mask (or group) defects per module.
       */
      unsigned int nElements(unsigned int mask_i) const {
         switch (mask_i) {
         case 1:
            return nSensorColumns() * circuitsPerRow() / (m_rectangularPixels ? 4 : 8);
         case 2:
            return circuitsPerColumn() * circuitsPerRow();
         default:
            assert( mask_i==0);
            return nCells();
         }
      }

      /** Function to return offline column and row ranges matching the defect-area of the given key (used for histogramming)
       * @param range pair of packed hardware coordinates addressing the start and end pixel of an rectangular inclusive pixel range.
       * @return offline start column, end column, start row, end row, where the end is meant to be exclusive i.e. [start, end)
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
                                               start.second, start.second+1};
         }
      }
      bool swapOfflineRowsColumns() const { return m_swapOfflineRowsColumns; }

   private:

      unsigned short m_sensorRows=0;
      unsigned short m_sensorColumns=0;
      unsigned short m_rows = 0;
      unsigned short m_columns = 0;
      unsigned short m_rowsPerCircuit = 0;
      unsigned short m_columnsPerCircuit = 0;
      unsigned char m_circuitsPerRow = 0;
      unsigned char m_circuitsPerColumn = 0;

      float m_columnPitch = 0;
      float m_rowPitch = 0;

      bool m_rectangularPixels = false;
      bool m_swapOfflineRowsColumns=false;
   };
}
#endif
