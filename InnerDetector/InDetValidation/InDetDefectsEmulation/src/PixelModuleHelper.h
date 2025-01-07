/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDET_PIXELMODULEHELPER_H
#define INDET_PIXELMODULEHELPER_H

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include <cassert>
#include <array>
#include <stdexcept>

namespace InDet {

   namespace MaskUtils {
      template<unsigned int bit, unsigned int end>
      static consteval unsigned int createMask() {
         // assert(bit<end);
         // assert(end<31u);
         if constexpr(bit>31u) {
               return 0u;
            }
         else if constexpr(bit>=end) {
               return 0u;
            }
         else {
            return (1u<<bit) | createMask<bit+1u,end>();
         }
      }
   }

   /** Helper class to convert between offline column, row and hardware chip, column, row coordinates.
    */
   class PixelModuleHelper {
   public:

      // hierarchy chosen such that the row has the least impact on sorting order
      // such that core column addresses of two adjacent core columns  enclose all pixel addresses (column, row) of the
      // first core column of this pair.
      static constexpr unsigned int COLGROUP_DEFECT_BIT_MASK = (1u<<31);
      static constexpr unsigned int CHIP_MASK = MaskUtils::createMask<24,28>(); // 16 chips max
      static constexpr unsigned int COL_MASK  = MaskUtils::createMask<12,24>(); // 4096 cols max
      static constexpr unsigned int ROW_MASK  = MaskUtils::createMask<0,12>();  // 4096 rows max
      static constexpr unsigned int CHIP_SHIFT = 24;
      static constexpr unsigned int COL_SHIFT  = 12;
      static constexpr unsigned int ROW_SHIFT  = 0;

      static constexpr std::array<unsigned short,2> N_COLS_PER_GROUP {
         8,  //8 columns per group for square pixels
         4}; //4 columns per group for rectangular pixels
      static constexpr std::array<unsigned int,2> COL_MASK_FOR_GROUP {
         MaskUtils::createMask<12,3+12>(),  //8 columns per group for square pixels
         MaskUtils::createMask<12,2+12>()}; //4 columns per group for rectangular pixels

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
         m_columnGroupLeadPixelMask = (~COL_MASK_FOR_GROUP[m_rectangularPixels]) & (~ROW_MASK) ;
         m_columnGroupRowColumnMask = CHIP_MASK|(COL_MASK & (~COL_MASK_FOR_GROUP[m_rectangularPixels] ));
      }
      operator bool () const { return m_columns>0; }

      unsigned int columns() const { return m_columns; }
      unsigned int rows() const { return m_rows; }
      unsigned int columnsPerCircuit() const { return m_columnsPerCircuit; }
      unsigned int rowsPerCircuit() const { return m_rowsPerCircuit; }
      unsigned int circuitsPerColumn() const { return m_circuitsPerColumn; }
      unsigned int circuitsPerRow() const { return m_circuitsPerRow; }
      bool swapOfflineRowsColumns() const { return m_swapOfflineRowsColumns; }
      bool rectangularPixels() const { return m_rectangularPixels; }
      unsigned int columnGroupRowColumnMask() const { return m_columnGroupRowColumnMask; }

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
         assert( (chip   & (CHIP_MASK >> CHIP_SHIFT)) == chip);
         assert( (column & (COL_MASK  >> COL_SHIFT))  == column);
         assert( (row    & (ROW_MASK  >> ROW_SHIFT))  == row);

         return (chip << CHIP_SHIFT)  | (column << COL_SHIFT) | (row << ROW_SHIFT);
      }

      /** Create a "column group" defect for the given offline coordinates.
       * @param row offline row aka. phi index of a pixel of the column group
       * @param column offline column aka. eta index of a pixel of the column group
       * @return packed triplet chip, column, row of the first pixel in the column group and column group flag set
       * Creates packed hardware coordinates for column group aka. ITk pixel core column
       * which is the coordinate of the first pixel of that group and sets the column group
       * flag.
       */
      unsigned int columnGroupDefect(unsigned int row, unsigned int column) const {
         return (hardwareCoordinates(row,column) & m_columnGroupLeadPixelMask) | COLGROUP_DEFECT_BIT_MASK;
      }

      unsigned int nPixels() const {
         return m_columns * m_rows;
      }
      unsigned int nSensorColumns() const {
         return m_sensorColumns;
      }
      unsigned int nSensorRows() const {
         return m_sensorRows;
      }
      static constexpr unsigned int getChip(unsigned int hardware_coordinates) {
         return (hardware_coordinates & CHIP_MASK) >> CHIP_SHIFT;
      }
      static constexpr unsigned int getRow(unsigned int hardware_coordinates) {
         return (hardware_coordinates & ROW_MASK) >> ROW_SHIFT;
      }
      static constexpr unsigned int getColumn(unsigned int hardware_coordinates) {
         return (hardware_coordinates & COL_MASK) >> COL_SHIFT;
      }

      /** Test whether the column group flag is set in the packed hardware coordinate tripplet
       */
      static constexpr bool isColumnGroupDefect(unsigned int key) {
         return key & COLGROUP_DEFECT_BIT_MASK;
      }

      /** Test whether the two packed hardware coorindates refer to the same column group.
       */
      static constexpr bool inSameColumnGroup(unsigned int key_ref, unsigned int key_test, unsigned int mask_out_rows_and_cols_per_group) {
         return     (key_ref  & mask_out_rows_and_cols_per_group)
            ==  (key_test & mask_out_rows_and_cols_per_group);
      }

      /** Test whether the given packed hardware coordinates refer to the same pixel
       */
      constexpr static bool isSameDefect( unsigned int key_ref, unsigned int key_test) {
         return (key_ref & (CHIP_MASK | ROW_MASK | COL_MASK)) == key_test;
      }

      /** Convenience function to test whether the given packed hardware coordinates refer to overlapping defects.
       * @param key_ref packed hardware coordinates referring either to a single pixel defect or a column group defect
       * @param key_test packed hardware coordinates referring to a single pixel defect
       * @return true if the defects overlap
       * If key_ref refers to a single pixel defect and key_test refers to a column group defect
       * the defects are only considered as overlapping if the pixel addressed by the two coordinates
       * are identical, not if key_ref only is contained in the column group addressed by key_test.
       * If key_ref and key test both could address exclusively a column group defect use :
       * <verb>
       * isSameDefect(key_ref, key_test)
       *   || ( (isColumnGroupDefect(key_ref)|| isColumnGroupDefect(key_test)) && inSameColumnGroup(key_ref, key_test, helper.columnGroupRowColumnMask()));
       * </verb>
       * instead.
       */
      constexpr static bool isSameDefectWithGroups( unsigned int key_ref, unsigned int key_test, unsigned int mask_out_rows_and_cols_per_group) {
         return isSameDefect(key_ref, key_test) || (isColumnGroupDefect(key_ref) && inSameColumnGroup(key_ref, key_test, mask_out_rows_and_cols_per_group));
      }

      /** Convenience function to return oflline column and row ranges matching the defect-area of the given key
       * @param key packed hardware coordinates addressing a single pixel or a column group defect
       * @return offline start column, end column, start row, end row, where the end is meant not to be inclusive i.e. [start, end)
       */
      std::array<unsigned int,4> offlineRange(unsigned int key) const {
         if (isColumnGroupDefect(key)) {
            if (getRow(key) !=0) {
               throw std::runtime_error("invalid key");
            };

            unsigned int chip=getChip(key);
            unsigned int row=getRow(key);
            unsigned int row_end=row + rowsPerCircuit()-1;
            unsigned int column=getColumn(key);
            unsigned int column_end= column + N_COLS_PER_GROUP[m_rectangularPixels]-1;

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
      unsigned int m_columnGroupLeadPixelMask = 0;
      unsigned int m_columnGroupRowColumnMask = 0;
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
