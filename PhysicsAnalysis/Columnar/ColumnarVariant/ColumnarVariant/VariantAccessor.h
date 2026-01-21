/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_VARIANT_VARIANT_ACCESSOR_H
#define COLUMNAR_VARIANT_VARIANT_ACCESSOR_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarVariant/VariantDef.h>
#include <boost/mp11/tuple.hpp>

namespace columnar
{
  // the column accessor for variant objects in ColumnarModeXAOD
  //
  // This just wraps a regular accessor, as in xAOD mode variants aren't
  // really a thing.
  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList,typename CT,ColumnAccessMode CAM>
    requires requires { AccessorTemplate<CIBase,CT,CAM,ColumnarModeXAOD>{}; }
  class AccessorTemplate<VariantContainerId<CIBase,CIList...>,CT,CAM,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (!std::is_const_v<CT>, "CT must not be const");

    using CI = VariantContainerId<CIBase,CIList...>;
    using CM = ColumnarModeXAOD;
    using AccessorTuple = std::tuple<AccessorTemplate<CIList,CT,CAM,CM>...>;

    AccessorTemplate () noexcept = default;

    AccessorTemplate (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnInfo&& info = {})
    {
      resetAccessor (m_accessor, columnarTool, name, ColumnInfo(info));
    }

    AccessorTemplate (AccessorTemplate&& that)
    {
      m_accessor = std::move (that.m_accessor);
    }

    AccessorTemplate& operator = (AccessorTemplate&& that)
    {
      if (this != &that)
        m_accessor = std::move (that.m_accessor);
      return *this;
    }

    AccessorTemplate (const AccessorTemplate&) = delete;
    AccessorTemplate& operator = (const AccessorTemplate&) = delete;

    void reset (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnInfo&& info = {})
    {
      resetAccessor (m_accessor, columnarTool, name, ColumnInfo(info));
    }

    [[nodiscard]] decltype(auto) operator () (ObjectId<CI,CM> id) const noexcept
    {
      return m_accessor (id.getBaseObject());
    }

    [[nodiscard]] bool isAvailable (ObjectId<CI,CM> id) const noexcept
    {
      return m_accessor.isAvailable (id.getBaseObject());
    }

    /// Private Members
    /// ===============
  private:

    AccessorTemplate<CIBase,CT,CAM,CM> m_accessor;
  };



  // the column accessor for variant objects in array columnar modes
  //
  // This internally contains a tuple of accessors, one for each
  // variant. This is probably not the best way to implement it, but it
  // fits best with the current accessor infrastructure.
  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList,typename CT,ColumnAccessMode CAM,ColumnarArrayMode CM>
    requires requires { AccessorTemplate<CIBase,CT,CAM,CM>{}; }
  class AccessorTemplate<VariantContainerId<CIBase,CIList...>,CT,CAM,CM> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (!std::is_const_v<CT>, "CT must not be const");

    using CI = VariantContainerId<CIBase,CIList...>;
    using AccessorTuple = std::tuple<AccessorTemplate<CIList,CT,CAM,CM>...>;

    AccessorTemplate () noexcept = default;

    AccessorTemplate (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnInfo&& info = {})
    {
      boost::mp11::tuple_for_each (m_accessors, [&columnarTool,&name,&info] (auto& accessor)
      {
        resetAccessor (accessor, columnarTool, name, ColumnInfo(info));
      });
    }

    AccessorTemplate (AccessorTemplate&& that)
    {
      m_accessors = std::move (that.m_accessors);
    }

    AccessorTemplate& operator = (AccessorTemplate&& that)
    {
      if (this != &that)
        m_accessors = std::move (that.m_accessors);
      return *this;
    }

    AccessorTemplate (const AccessorTemplate&) = delete;
    AccessorTemplate& operator = (const AccessorTemplate&) = delete;

    void reset (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnInfo&& info = {})
    {
      boost::mp11::tuple_for_each (m_accessors, [&columnarTool,&name,&info] (auto& accessor)
      {
        resetAccessor (accessor, columnarTool, name, ColumnInfo(info));
      });
    }

    [[nodiscard]] decltype(auto) operator () (ObjectId<CI,CM> id) const noexcept
    {
      return internalGet<0> (id);
    }

    [[nodiscard]] bool isAvailable (ObjectId<CI,CM> id) const noexcept
    {
      return internalIsAvailable<0> (id);
    }

    /// Private Members
    /// ===============
  private:

    AccessorTuple m_accessors;

    template<unsigned Index>
    void internalInit (ColumnarTool<CM>& columnarTool, const std::string& name, const ColumnInfo& info)
    {
      resetAccessor (std::get<Index>(m_accessors), columnarTool, name, ColumnInfo(info));
      if constexpr (Index + 1 < CI::numVariants)
        internalInit<Index + 1>(columnarTool, name, info);
    }

    template<unsigned Index>
    decltype(auto) internalGet (const ObjectId<CI,CM>& id) const noexcept
    {
      if (id.getVariantIndex() == Index)
      {
        using CI2 = std::tuple_element_t<Index,std::tuple<CIList...>>;
        ObjectId<CI2,CM> objId {id.getData(), id.getObjectIndex()};
        return std::get<Index>(m_accessors)(objId);
      } else if constexpr (Index+1 < CI::numVariants)
        return internalGet<Index + 1>(id);
      else
      {
        std::cerr << "Invalid variant index: " << id.getVariantIndex() << std::endl;
        std::abort ();
      }
    }

    template<unsigned Index>
    bool internalIsAvailable (const ObjectId<CI,CM>& id) const noexcept
    {
      if (id.getVariantIndex() == Index)
      {
        using CI2 = std::tuple_element_t<Index,std::tuple<CIList...>>;
        ObjectId<CI2,CM> objId {id.getData(), id.getObjectIndex()};
        return std::get<Index>(m_accessors).isAvailable(objId);
      } else if constexpr (Index+1 < CI::numVariants)
        return internalIsAvailable<Index + 1>(id);
      else
      {
        std::cerr << "Invalid variant index: " << id.getVariantIndex() << std::endl;
        std::abort ();
      }
    }
  };
}

#endif
