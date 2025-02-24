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
                                                    4,   // bits for flags
                                                    3    // number of masks: single-pixel, core-column, circuit
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
         : ModuleKeyHelper( std::array<unsigned int,MASKS_SIZE>{ PixelModuleHelper::getPixelMask(),
                                                                 PixelModuleHelper::getColGroup8Mask(),
                                                                 PixelModuleHelper::getChipMask()} )
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
         }
         else {
            m_swapOfflineRowsColumns=false;
            m_rows = pixelModuleDesign->rows();
            m_columns = pixelModuleDesign->columns();
            m_rowsPerCircuit = pixelModuleDesign->rowsPerCircuit();
            m_columnsPerCircuit = pixelModuleDesign->columnsPerCircuit();
            m_circuitsPerRow = pixelModuleDesign->numberOfCircuitsPerRow();
            m_circuitsPerColumn = pixelModuleDesign->numberOfCircuitsPerColumn();
         }
         m_rectangularPixels = (m_columns==200);
         }
         if (m_rectangularPixels) {
            m_masks[1]= PixelModuleHelper::getColGroup4Mask();
         }
      }
      operator bool () const { return m_columns>0; }

      unsigned int columns() const { return m_columns; }
      unsigned int rows() const { return m_rows; }
      unsigned int columnsPerCircuit() const { return m_columnsPerCircuit; }
      unsigned int rowsPerCircuit() const { return m_rowsPerCircuit; }
      unsigned int circuitsPerColumn() const { return m_circuitsPerColumn; }
      unsigned int circuitsPerRow() const { return m_circuitsPerRow; }

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

      /** Test whether the given packed hardware coordinates match the given defect
       * @param key_ref the packed "coordinates" of the defect
       * @param key_test the packed coordinates of a pixel.
       */
      bool isMatchingDefect( unsigned int key_ref, unsigned int key_test) const {
         return isOverlapping(key_ref, key_test);
      }

      /** Function to return offline column and row ranges matching the defect-area of the given key (used for histogramming)
       * @param key packed hardware coordinates addressing a single pixel, a column group or circuit defect
       * @return offline start column, end column, start row, end row, where the end is meant to be exclusive i.e. [start, end)
       */
      std::array<unsigned int,4> offlineRange(unsigned int key) const {
         unsigned int mask_index = getMaskIdx(key);
         if (mask_index !=0) {
            if (getRow(key) !=0) {
               throw std::runtime_error("invalid key");
            };

            unsigned int chip=getChip(key);
            unsigned int row=getRow(key);
            unsigned int row_end=row + rowsPerCircuit()-1;
            unsigned int column=getColumn(key);
            unsigned int column_end= column + (mask_index == 1 ? N_COLS_PER_GROUP[m_rectangularPixels]-1 : columnsPerCircuit());

            unsigned int chip_row = chip / circuitsPerRow();
            unsigned int chip_column = chip % circuitsPerRow();

            column += chip_column * columnsPerCircuit();
            column_end += chip_column * columnsPerCircuit();
            if (chip_row>=1) {
               column = columns() - column -1;
               column_end = columns() - column_end -1;

               row = rowsPerCircuit() - row -1 + chip_row * rowsPerCircuit();
               row_end = rowsPerCircuit() - row_end -1 + chip_row * rowsPerCircuit();
            }
            if (swapOfflineRowsColumns()) {
               return std::array<unsigned int,4>{ std::min(column, column_end), std::max(column,column_end)+1,
                                                  std::min(row, row_end),       std::max(row, row_end)+1 };
            }
            else {
               return std::array<unsigned int,4>{ std::min(row, row_end),       std::max(row, row_end)+1,
                                                  std::min(column, column_end), std::max(column,column_end)+1 };
            }
         }
         else {
            unsigned int chip=getChip(key);
            unsigned int row=getRow(key);
            unsigned int column=getColumn(key);

            unsigned int chip_row = chip / circuitsPerRow();
            unsigned int chip_column = chip % circuitsPerRow();

            column += chip_column * columnsPerCircuit();
            if (chip_row>=1) {
               column = columns() - column -1;

               row = rowsPerCircuit() - row -1 + chip_row * rowsPerCircuit();
            }
            if (swapOfflineRowsColumns()) {
               return std::array<unsigned int,4 >{ column, column + 1,
                                                   row, row +1 };
            }
            else {
               return std::array<unsigned int,4>{ row, row + 1,
                                                  column, column +1 };
            }
         }
      }

   private:
      bool swapOfflineRowsColumns() const { return m_swapOfflineRowsColumns; }

      unsigned short m_sensorRows=0;
      unsigned short m_sensorColumns=0;
      unsigned short m_rows = 0;
      unsigned short m_columns = 0;
      unsigned short m_rowsPerCircuit = 0;
      unsigned short m_columnsPerCircuit = 0;
      unsigned char m_circuitsPerRow = 0;
      unsigned char m_circuitsPerColumn = 0;
      bool m_rectangularPixels = false;
      bool m_swapOfflineRowsColumns=false;
   };
}
#endif
