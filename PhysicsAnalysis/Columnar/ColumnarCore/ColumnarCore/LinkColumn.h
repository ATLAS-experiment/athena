/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_LINK_COLUMN_H
#define COLUMNAR_CORE_LINK_COLUMN_H

#include <AthLinks/ElementLink.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/OptObjectId.h>

namespace columnar
{
  /// a special column type that behaves like an @ref OptObjectId, but
  /// applies an internal cast in xAOD mode
  ///
  /// Normally the `ElementLink` type should just be the type of the
  /// object linked to, but apparently in some cases it is not.  In
  /// those cases you have to use this variant and specify the type
  /// parameter for the `ElementLink`.
  template<ContainerIdConcept LT,typename ELT>
  struct LinkCastColumn {};



  // in xAOD mode we can do a straightforward conversion from
  // ElementLink to OptObjectId, as ElementLink contains all the
  // information about the object
  template<ContainerIdConcept LT>
  struct ColumnTypeTraits<OptObjectId<LT>,ColumnarModeXAOD> final
  {
    using CM = ColumnarModeXAOD;
    using ColumnType = NativeColumn<ElementLink<typename LT::xAODElementLinkType>>;
    using UserType = OptObjectId<LT>;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnarTool*/, ColumnInfo& info) {return info;}

    static OptObjectId<LT> convertInput (const ElementLink<typename LT::xAODElementLinkType>& link)
    {
      if (link.isValid())
      {
        typename LT::xAODObjectIdType *ptr = *link.cptr();
        return OptObjectId<LT,CM> (ptr);
      } else
      {
        return OptObjectId<LT,CM> ();
      }
    }
  };


  template<ContainerIdConcept LT,typename ELT>
  struct ColumnTypeTraits<LinkCastColumn<LT,ELT>,ColumnarModeXAOD> final
  {
    using CM = ColumnarModeXAOD;
    using ColumnType = NativeColumn<ElementLink<ELT>>;
    using UserType = OptObjectId<LT>;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnarTool*/, ColumnInfo& info) {return info;}

    static OptObjectId<LT> convertInput (const ElementLink<ELT>& link)
    {
      if (link.isValid())
      {
        auto *ptr = *link.cptr();
        if (!ptr) return OptObjectId<LT,CM> ();
        auto *ptr2 = dynamic_cast<typename LT::xAODObjectIdType*>(ptr);
        if (!ptr2) throw std::runtime_error ("link not of expected type");
        return OptObjectId<LT,CM> (ptr2);
      } else
      {
        return OptObjectId<LT,CM> ();
      }
    }
  };





  // in Array mode we take an index from the underlying column and
  // combine it with the data vector from the input to get the new
  // OptObjectId
  template<ContainerIdConcept LT>
  struct ColumnTypeTraits<OptObjectId<LT>,ColumnarModeArray>
  {
    using CM = ColumnarModeArray;
    using ColumnType = typename CM::LinkIndexType;
    using UserType = OptObjectId<LT>;
    using DataType = void **;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = false;
    static constexpr bool useConvertWithDataInput = true;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnarTool*/, ColumnInfo& info)
    {
      info.linkTargetNames = {std::string{LT::idName}};
      return info;
    }

    static OptObjectId<LT> convertInput (void **data, typename CM::LinkIndexType link)
    {
      if (link == invalidObjectIndex)
        return OptObjectId<LT,CM> ();
      return OptObjectId<LT,CM> (data, link);
    }
  };

  // I'm just inheriting the ColumnTypeTraits from OptObjectId, as the
  // behavior is exactly the same. Note that this is only for regular
  // container IDs, as e.g. VariantContainerId needs special handling.
  template<RegularContainerIdConcept LT,typename ELT>
  struct ColumnTypeTraits<LinkCastColumn<LT,ELT>,ColumnarModeArray> : ColumnTypeTraits<OptObjectId<LT>,ColumnarModeArray> {};
}

#endif
