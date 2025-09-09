/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_CORE_COLUMNAR_DEF_H
#define COLUMNAR_CORE_COLUMNAR_DEF_H

#include <cstdint>
#include <stdexcept>

namespace columnar
{
  // This checks that COLUMNAR_DEFAULT_ACCESS_MODE is indeed defined, plus makes it
  // available for use with `if constexpr`.
  constexpr unsigned columnarAccessMode = COLUMNAR_DEFAULT_ACCESS_MODE;

  struct ColumnarModeXAOD
  {
    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = true;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = true;
  };



  struct ColumnarModeArray
  {
    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = false;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = false;


    /// Element Link Definition
    /// =======================
    ///
    /// In general links are just represented by an integer index into
    /// the target container. However, for variant links (i.e. links
    /// pointing into one of multiple containers) we need to be able to
    /// identify the target container. This happens by using the top
    /// couple of bits of the index to identify the target container.
    ///
    /// For variant links there is also an additional column that
    /// contains the keys for the target containers for each link, in
    /// the order the tool defined. If there are multiple accessors for
    /// the same variant link column, they will share the same key
    /// column and as such need to define the variant in the same way.
    ///
    /// There are a couple of configurable aspects to this, like the
    /// exact types involved, as well as the number of bits used for the
    /// key, all of which can be configured through the columnar mode.
    /// Beyond that there are also a couple of helper functions and
    /// convenience definitions to avoid bit packing/unpacking code
    /// being repeated in multiple places.

    /// the type used for columns that represent element links
    using LinkIndexType = std::size_t;

    /// the value used for an invalid link (a.k.a. empty/null link)
    static constexpr LinkIndexType invalidLinkValue = static_cast<LinkIndexType>(-1);

    /// the number of bits used for the key inside the link
    static constexpr unsigned linkKeyBits = 8;

    /// the type used for the key column
    using LinkKeyType = std::uint8_t;

    /// various helper definitions
    static_assert (linkKeyBits <= 8 * sizeof(LinkKeyType), "Link key bits exceed key type size");
    static_assert (linkKeyBits < 8 * sizeof(LinkIndexType), "Link key bits exceed link type size");
    static constexpr unsigned linkIndexBits = 8 * sizeof(LinkIndexType) - linkKeyBits;
    static constexpr LinkIndexType linkIndexMask = (static_cast<LinkIndexType>(1) << linkIndexBits) - 1;

    /// get the key value from a link value
    static inline LinkKeyType getLinkKey (LinkIndexType link) {
      return link >> linkIndexBits;
    }

    /// get the index value from a link value
    static inline LinkIndexType getLinkIndex (LinkIndexType link) {
      return link & linkIndexMask;
    }

    /// merge a key and index value into a link value
    static inline LinkIndexType mergeLinkKeyIndex (LinkIndexType key, LinkIndexType index) {
      if (key >= (static_cast<LinkIndexType>(1) << linkKeyBits))
        throw std::runtime_error ("link key too large to fit in link: " + std::to_string(key));
      if (index & ~linkIndexMask)
        throw std::runtime_error ("index too large to fit in link: " + std::to_string(index));
      return index | (key << linkIndexBits);
    }
  };



#if COLUMNAR_DEFAULT_ACCESS_MODE == 0
  using ColumnarModeDefault = ColumnarModeXAOD;
#elif COLUMNAR_DEFAULT_ACCESS_MODE == 2
  using ColumnarModeDefault = ColumnarModeArray;
#else
  #error "COLUMNAR_DEFAULT_ACCESS_MODE must be 0 or 2"
#endif
}

#endif
