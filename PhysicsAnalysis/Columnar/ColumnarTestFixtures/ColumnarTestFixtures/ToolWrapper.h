/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_TOOL_WRAPPER_H
#define COLUMNAR_TEST_FIXTURES_TOOL_WRAPPER_H

#include <ColumnarToolWrapper/ToolColumnVectorMap.h>

#include <memory>
#include <span>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace columnar
{
  struct ColumnInfo;
  class IColumnarTool;


  namespace TestUtils
  {
    /// @brief a class that holds a reference to a @ref
    /// ToolColumnVectorMap and a @ref ColumnVectorData
    ///
    /// This is at this point only used for the PHYSLITE test, which is
    /// build around having this class, but it in essense just wraps two
    /// pointers and forwards to them.

    class ToolWrapperData
    {
      /// Public Members
      /// ==============

    public:

      /// @brief constructor: wrap the given tool
      explicit ToolWrapperData (ColumnVectorData *val_columnData, const ToolColumnVectorMap *val_wrapper) noexcept;


      template<typename CT>
      void setColumn (const std::string& name, std::size_t size, CT* dataPtr) {
        m_wrapper->setColumn (*m_columnData, name, size, dataPtr);
      }



      /// Private Members
      /// ===============

    private:

      const ToolColumnVectorMap *m_wrapper = nullptr;

      ColumnVectorData *m_columnData = nullptr;
    };
  }
}

#endif
