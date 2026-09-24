/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_LINK_COLUMN_H
#define COLUMNAR_CORE_LINK_COLUMN_H

#include <AthLinks/ElementLink.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ContainerId.h>
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



  namespace detail
  {
    /// the CLID of the xAOD container a link column targets, or 0 if
    /// the container id does not provide one
    template<ContainerIdConcept LT>
    CLID linkTargetClid ()
    {
      if constexpr (requires { LT::containerClid(); })
        return LT::containerClid();
      else
        return 0;
    }

    // if the columnar mode uses typed links, then the links with the
    // implicit types can simply redirect to the code for typed links
    template<ContainerIdConcept LT,ColumnarMode CM>
      requires (CM::hasTypedLinks == true)
    class MemoryAccessor<OptObjectId<LT,CM>,CM> final
    {
    public:

      using BaseAccessor = MemoryAccessor<LinkCastColumn<LT,typename LT::xAODElementLinkType>,CM>;
      static_assert (BaseAccessor::isDefined, "MemoryAccessor for LinkCastColumn must be defined");

      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = BaseAccessor::viewIsReference;
      static constexpr bool hasSetter = false;
      using MemoryType = typename BaseAccessor::MemoryType;

      static void updateColumnInfo (ColumnInfo& info)
      {
        BaseAccessor::updateColumnInfo (info);
      }

      [[nodiscard]] static auto makeViewer (void** dataArea)
      {
        return BaseAccessor::makeViewer(dataArea);
      }
    };

    // I'm just using the MemoryAccessor from OptObjectId, as the
    // behavior is exactly the same. Note that this is only for regular
    // container IDs, as e.g. VariantContainerId needs special handling.
    template<RegularContainerIdConcept LT,typename ELT,ColumnarMode CM>
      requires (CM::hasTypedLinks == false && MemoryAccessor<OptObjectId<LT,CM>,CM>::isDefined)
    class MemoryAccessor<LinkCastColumn<LT,ELT>,CM>
    {
    public:

      using BaseAccessor = MemoryAccessor<OptObjectId<LT,CM>,CM>;

      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = BaseAccessor::viewIsReference;
      static constexpr bool hasSetter = false;
      using MemoryType = typename BaseAccessor::MemoryType;

      static void updateColumnInfo (ColumnInfo& info)
      {
        BaseAccessor::updateColumnInfo (info);
        // the persistified links are typed on ELT, so that is the CLID
        // entering the stored keys
        info.soleLinkTargetClid = clidForType<ELT>();
      }

      [[nodiscard]] static auto makeViewer (void** dataArea)
      {
        return BaseAccessor::makeViewer(dataArea);
      }
    };



    template<ContainerIdConcept LT,typename ELT>
    class MemoryAccessor<LinkCastColumn<LT,ELT>,ColumnarModeXAOD> final
    {
    public:
      using CM = ColumnarModeXAOD;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = ElementLink<ELT>;
      static auto makeViewer (void**)
      {
        return [] (const ElementLink<ELT>& link)
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
        };
      }
    };




    // in Array mode we take an index from the underlying column and
    // combine it with the data vector from the input to get the new
    // OptObjectId
    template<ContainerIdConcept LT>
    class MemoryAccessor<OptObjectId<LT,ColumnarModeArray>,ColumnarModeArray> final
    {
    public:

      using CM = ColumnarModeArray;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = typename CM::LinkIndexType;

      static void updateColumnInfo (ColumnInfo& info)
      {
        info.soleLinkTargetName = LT::idName;
        info.soleLinkTargetClid = linkTargetClid<LT>();
      }

      [[nodiscard]] static auto makeViewer (void** dataArea)
      {
        return [dataArea](const MemoryType& link)
        {
          if (link == invalidObjectIndex)
            return OptObjectId<LT,CM> ();
          return OptObjectId<LT,CM> (dataArea, link);
        };
      }
    };



    template<ContainerIdConcept LT,typename ELT>
    class MemoryAccessor<LinkCastColumn<LT,ELT>,ColumnarModeXAODArray> final
    {
    public:
      using CM = ColumnarModeXAODArray;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = ElementLink<ELT>;

      static void updateColumnInfo (ColumnInfo& info)
      {
        info.soleLinkTargetName = LT::idName;
        info.soleLinkTargetClid = clidForType<ELT>();
      }

      static auto makeViewer (void** dataArea)
      {
        return [dataArea] (const ElementLink<ELT>& link)
        {
          if (link.isValid())
          {
            return OptObjectId<LT,CM> (dataArea, link.index());
          } else
          {
            return OptObjectId<LT,CM> ();
          }
        };
      }
    };
  }
}

#endif
