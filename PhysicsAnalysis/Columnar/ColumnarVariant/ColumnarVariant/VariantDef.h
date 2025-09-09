/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_VARIANT_VARIANT_DEF_H
#define COLUMNAR_VARIANT_VARIANT_DEF_H

#include <ColumnarCore/ContainerId.h>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <array>
#include <tuple>

namespace columnar
{
  /// @brief a "variant" ContainerId
  ///
  /// A "variant" ContainerId (CI) in this context means that an object
  /// id or link can refer to an object in one of several containers.
  /// This is contrasted with regular object ids or links, which are
  /// only ever associated with a single container. If you are wondering
  /// about the name, I named it after `std::variant`, as it seems like
  /// a similar concept.
  ///
  /// While it may seem very convenient to be able to refer to more than
  /// a single container, it is actually a non-trivial thing to achieve
  /// in columnar mode, and should only be used when that capability is
  /// needed and can't be reasonably avoided. In practice that means use
  /// it when otherwise you would have to implement similar functionality
  /// in your own code.
  ///
  /// This also only works with a fixed list of containers, not an
  /// open-ended list. As such you have to pass in a `CIList...` of
  /// container ids you intend to use with it. Further you need to pass
  /// a `CIBase` that isn't a possible variant container, but is used to
  /// define a common base class in xAOD mode. If you also want to use
  /// it as a possible "variant" container id, you need to list it
  /// twice.
  ///
  /// Note that it is generally possible for variant links to refer to
  /// containers not in the list, but variant (non-optional) object ids
  /// are guaranteed to refer to a valid object in one of the
  /// containers.
  ///
  /// It is possible to go from a variant id/link to a regular id, but
  /// due to the way it is implemented there are slight differences
  /// between modes: In xAOD mode it is based on `dynamic_cast` of
  /// underlying objects, whereas in columnar mode it is based on unique
  /// ids for each container. In practice that means that if you have
  /// e.g. multiple track containers in your variant you can (and have
  /// to) distinguish them in columnar mode, whereas in xAOD mode they
  /// will show up as belonging to each of the containers.
  ///
  /// Note that in columnar mode if you are only interested in a single
  /// variant for a variant link, it is probably more efficient to query
  /// the link directly for the specific variant (as opposed to
  /// converting it to a variant object id first). However, the impact
  /// of this hasn't been benchmarked, so it is unclear whether it
  /// provides a significant performance benefit.
  ///
  ///
  /// Implementation Details
  /// ----------------------
  ///
  /// In xAOD mode the object ids are represented as pointers to the
  /// base class (as identified via `CIBase`), and links are represented
  /// by a pointer to the proper `ElementLink`. Implementation-wise this
  /// is mostly pass through in xAOD mode, with no overhead compared to
  /// "regular" containers.
  ///
  /// In columnar mode the object ids are represented as a pair of
  /// integers (and a data pointer), as opposed to the single integer
  /// for the regular columnar object ids. That second integer
  /// identifies the container the object belongs to, and is essentially
  /// just the index in the variant list.
  ///
  /// Object links in columnar mode are a bit more complex, but at the
  /// heart they are still a pair of integers. However, in the
  /// persistent format the integer identifying the container can not be
  /// expected to correspond to the index in the variant list. Instead
  /// it is assumed to be a key that is unique for each container, and
  /// then there is another data-vector that holds the unique key for
  /// each "variant" container in order.
  ///
  /// Variant accessors in columnar mode essentially just create N
  /// accessors internally, one for each variant container. This is
  /// necessary because the accessors are linked to the specific
  /// container they operate on. However that also means that the number
  /// of accessors can quickly balloon if a variant container has a
  /// large number of variants and accessors.

  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList>
  struct VariantContainerId
  {
    static_assert (sizeof...(CIList) > 0, "need to have at least one variant container type for VariantContainerId");

    /// identify this as a container id definition
    static constexpr bool isContainerId = true;

    /// whether this is a non-const container
    static constexpr bool isMutable = CIBase::isMutable;

    /// whether this can be retrieved as a range per event
    static constexpr bool perEventRange = false;

    /// whether this can be retrieved as a single object per event
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = typename CIBase::xAODObjectIdType;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = typename CIBase::xAODObjectRangeType;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = typename CIBase::xAODElementLinkType;


    /// Variant Specific Definitions
    /// ----------------------------

    using baseId = CIBase;

    static constexpr std::size_t numVariants = sizeof...(CIList);

    static constexpr std::array<std::string_view,numVariants> idNameArray = {CIList::idName...};

    template<ContainerIdConcept CI>
    static constexpr bool isValidContainer ()
    {
      return (... || std::is_same_v<CI,CIList>);
    }

    /// get the index of the given container in the list
    template<ContainerIdConcept CI>
    static constexpr unsigned getVariantIndex ()
    {
      constexpr unsigned index = boost::mp11::mp_find<std::tuple<CIList...>,CI>::value;
      return index;
    }

    using mpCIList = boost::mp11::mp_list<CIList...>;
  };
}

#endif
