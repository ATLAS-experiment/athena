/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OBJECT_TYPE_ACCESSOR_H
#define COLUMNAR_CORE_OBJECT_TYPE_ACCESSOR_H

#include <ColumnarCore/ColumnAccessor.h>
#include <xAODBase/ObjectType.h>

namespace columnar
{
  /// @brief a specialized accessor for retrieving the xAOD object type
  /// of objects
  ///
  /// Within xAODs the object type can be retrieved directly from each
  /// object, but in columnar mode that's not build in, and we don't
  /// necessarily want to do it on a per-object basis. Instead we often
  /// want to have the object type fixed for the lifetime of the tool,
  /// so that we can adjust our input columns and momentum accessors
  /// accordingly.
  ///
  /// As such this accessor has multiple means of retrieving the object
  /// type.
  template<ContainerIdConcept CI, typename CM = ColumnarModeDefault>
  class ObjectTypeAccessor final
  {
  public:
    template<typename ToolType>
    ObjectTypeAccessor (ToolType& tool, const std::string& name, const std::string& title)
    {
      tool.declareProperty (name, m_objectType, title);
    }

    [[nodiscard]] std::optional<xAODType::ObjectType> staticType () const
    {
      if (m_objectType == 0)
        return std::nullopt;
      return static_cast<xAODType::ObjectType>(m_objectType);
    }

    [[nodiscard]] xAODType::ObjectType operator () (ObjectId<CI,CM> object) const
    {
      if constexpr (CM::isXAOD)
      {
        const auto result = object->type();
        if (m_objectType != 0 && m_objectType != static_cast<unsigned>(result)) [[unlikely]]
          throw std::runtime_error("ObjectTypeAccessor: object type mismatch");
        return result;
      } else
      {
        if (m_objectType == 0) [[unlikely]]
          throw std::runtime_error("ObjectTypeAccessor: object type not set");
        return static_cast<xAODType::ObjectType>(m_objectType);
      }
    }

  private:

    unsigned m_objectType = 0;
  };
}

#endif