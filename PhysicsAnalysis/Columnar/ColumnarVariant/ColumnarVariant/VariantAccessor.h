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
  // the column accessor for variant objects in array columnar modes
  //
  // This internally contains a tuple of accessors, one for each
  // variant. This is probably not the best way to implement it, but it
  // fits best with the current accessor infrastructure.
  template<RegularContainerIdConcept CIBase,RegularContainerIdConcept... CIList,typename CT,ColumnAccessMode CAM,ColumnarArrayMode CM>
    requires requires { detail::ContainerFreeAccessor<CT,CAM,CM>::isDefined; }
  class AccessorTemplate<VariantContainerId<CIBase,CIList...>,CT,CAM,CM> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (!std::is_const_v<CT>, "CT must not be const");

    using CI = VariantContainerId<CIBase,CIList...>;

    AccessorTemplate () noexcept = default;

    AccessorTemplate (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnAccessorOptions&& options = {})
    {
      for (std::size_t index = 0u; index < CI::numVariants; ++index)
        m_accessors[index] = detail::ContainerFreeAccessor<CT,CAM,CM>(columnarTool, ColumnAccessorOptions(options), detail::ColumnAccessorOptionsArray {.offsetName = CI::idNameArray[index], .baseName = std::string (CI::idNameArray[index]) + "." + name});
    }

    [[nodiscard]] decltype(auto) operator () (ObjectId<CI,CM> id) const
    {
      if (id.getVariantIndex() >= CI::numVariants)
        throw std::out_of_range ("invalid variant index in VariantContainerId accessor");
      return m_accessors[id.getVariantIndex()](id.getDataArea(), id.getObjectIndex());
    }

    [[nodiscard]] bool isAvailable (ObjectId<CI,CM> id) const
    {
      if (id.getVariantIndex() >= CI::numVariants)
        throw std::out_of_range ("invalid variant index in VariantContainerId accessor");
      return m_accessors[id.getVariantIndex()].isAvailable(id.getDataArea());
    }

    /// Private Members
    /// ===============
  private:

    std::array<detail::ContainerFreeAccessor<CT,CAM,CM>, CI::numVariants> m_accessors;
  };
}

#endif
