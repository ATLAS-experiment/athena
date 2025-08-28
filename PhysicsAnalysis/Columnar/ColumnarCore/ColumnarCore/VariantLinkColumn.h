/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_VARIANT_LINK_COLUMN_H
#define COLUMNAR_CORE_VARIANT_LINK_COLUMN_H

#include <ColumnarCore/LinkColumn.h>
#include <ColumnarCore/VectorColumn.h>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <tuple>

namespace columnar
{
  /// @brief a "variant" @ref ContainerId
  ///
  /// A "variant" @ref ContainerId (CI) in this context means that an
  /// object id or element link can refer to one of several containers.
  /// This is contrasted with regular object ids or element links, which
  /// are only ever linked to a single container.
  ///
  /// While it may seem very convenient to be able to refer to more than
  /// a single container, it is actually a non-trivial thing to achieve
  /// in columnar mode, and should be avoided if at all possible.  In
  /// fact, in the current code base, the only use case is the MET
  /// association map, which has to link to all the constituents for
  /// each of the defined `IParticleContainer`s.  That objectively needs
  /// a "variant" link, but please think carefully before adding a
  /// second user of this formalism.  Also note that since there is only
  /// one user so far, details of the implementation may change as
  /// needed.
  ///
  /// The way it is implemented is that as a user you have to define a
  /// fixed list of CIs at compile time, and those are the containers
  /// you can use this with.  A link may still refer to other
  /// containers, you just won't be able to access them through the
  /// class.  To make things work in xAOD mode, the first CI should
  /// refer to an xAOD class that is the base to other xAOD classes.
  /// Also note that in xAOD mode casts/tests are implemented as
  /// `dynamic_cast`, so if you have multiple CIs referring to the same
  /// xAOD type, those can not be distinguished at runtime in xAOD mode.
  ///
  /// This particular struct is used to contract a list of ContainerIds
  /// into a single type, so that it can then be used as a single
  /// template parameter for other "variant" types in the columnar
  /// infrastructure.  In general there will never be an actual instance
  /// of this type, it's just for passing around the list of
  /// `ContainerId`s in a more convenient way.
  ///
  /// If you are wondering about the name, I named it after
  /// `std::variant`, as it seems like a similar concept.
  template<ContainerIdConcept LT,ContainerIdConcept... LTList>
  struct VariantContainerId
  {
    using xAODObjectIdType = typename LT::xAODObjectIdType;
    using xAODElementLinkType = typename LT::xAODElementLinkType;

    static constexpr std::size_t numVariants = 1 + sizeof...(LTList);
    template<ContainerIdConcept CI>
    static constexpr bool isValidContainer ()
    {
      return (std::is_same_v<CI,LT> || ... || std::is_same_v<CI,LTList>);
    }

    /// get the index of the given container in the list
    template<ContainerIdConcept CI>
    static constexpr unsigned getVariantIndex ()
    {
      constexpr unsigned index = boost::mp11::mp_find<std::tuple<LT,LTList...>,CI>::value;
      return index;
    }
  };





  /// @brief a "variant" link to a single object
  ///
  /// The `CI` template parameter is required to be a specialization of
  /// @ref VariantContainerId.  See the documentation of that type for
  /// more information on the "variant" concept.
  ///
  /// Unlike `ObjectId` this tries to model more of a link semantic,
  /// though the differences are tiny:
  /// * internally it stores an `ElementLink`, or the columnar equivalent
  /// * it has some mechanics for just comparing the links, without having
  ///   to look at underlying objects
  /// * technically it won't have to retrieve the underlying object, though
  ///   in practice the user (or the implementation) will likely do that
  ///   regardless
  ///
  /// Not sure whether this is really better than modeling an
  /// `ObjectId`, but in the end this isn't widely used, so there is no
  /// need to overthing it.  Should this become more widely used, this
  /// design can be revisited.
  template<typename CI,typename CM=ColumnarModeDefault>
  class VariantObjectLink;



  template<ContainerIdConcept... LTList>
  class VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeXAOD> final
  {
    /// Public Members
    /// ==============
  public:

    using CI = VariantContainerId<LTList...>;
    using CM = ColumnarModeXAOD;

    using LinkType = ElementLink<typename CI::xAODElementLinkType>;

    VariantObjectLink (const LinkType* val_link)
      : m_link (val_link)
    {}

    const typename CI::xAODObjectIdType* getXAODObject () const
    {
      if (m_link->isValid())
      {
        return **m_link;
      } else
      {
        return nullptr;
      }
    }

    /// whether this is a valid link
    explicit operator bool () const noexcept
    {
      return m_link->isValid();
    }

    /// whether this is a valid link
    [[nodiscard]] bool has_value () const noexcept
    {
      return m_link->isValid();
    }

    /// whether this link is valid and points to an object of the given container
    ///
    /// Note that this is only exact in columnar mode, in xAOD mode it
    /// will employ a dynamic_cast, and may return true for multiple
    /// containers.
    template<ContainerIdConcept CI2>
    [[nodiscard]] bool isContainer () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      return m_link->isValid() && dynamic_cast<typename CI2::xAODObjectRangeType*>(m_link->getStorableObjectPointer());
    }

    /// whether this link points to the given object
    template<ContainerIdConcept CI2>
    [[nodiscard]] bool operator == (ObjectId<CI2,CM> id) const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      return m_link->isValid() && (**m_link) == &id.getXAODObject();
    }

    /// whether this link points to the given object
    ///
    /// Note that this is only exact in xAOD mode.  In columnar mode if
    /// both links are to containers not listed in CI it can in some
    /// situations return true, when it should return false.
    [[nodiscard]] bool operator == (const VariantObjectLink<CI,CM>& obj) const
    {
      return m_link == obj.m_link;
    }

    /// return the ObjectId if it is in the given container or `nullopt` otherwise
    template<ContainerIdConcept CI2>
    [[nodiscard]] OptObjectId<CI2,CM> tryGetObject () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      if (m_link->isValid())
      {
        return OptObjectId<CI2,CM> (dynamic_cast<const typename CI2::xAODObjectIdType*>(**m_link));
      } else
        return OptObjectId<CI2,CM> ();
    }

    /// Private Members
    /// ===============
  private:

    const LinkType* m_link = nullptr;
  };
  template<typename... LTList>
  std::ostream& operator<< (std::ostream& str, const VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeXAOD>& obj)
  {
    return str << obj.getXAODObject() << "/" << obj.getXAODObject()->type();
  }

  // in xAOD mode we can do a fairly straightforward conversion from
  // std::vector as the logic is inside VariantObjectLink
  template<typename... LTList>
  struct ColumnTypeTraits<std::vector<VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeXAOD>>,ColumnarModeXAOD> final
  {
    using CI = VariantContainerId<LTList...>;
    using CM = ColumnarModeXAOD;
    using LinkType = ElementLink<typename CI::xAODElementLinkType>;
    using ColumnType = NativeColumn<std::vector<LinkType>>;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnBase*/, ColumnInfo& info) {return info;}
    static auto convertInput (const std::vector<LinkType>& value) {
      return detail::VectorConvertView ([](const LinkType& link) {return VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeXAOD> (&link);}, std::span<const LinkType>(value));};
    using UserType = decltype(convertInput(std::declval<std::vector<LinkType>>()));
  };



  template<ContainerIdConcept... LTList>
  class VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeArray> final
  {
    /// Public Members
    /// ==============
  public:

    using CI = VariantContainerId<LTList...>;
    using CM = ColumnarModeArray;

    VariantObjectLink (ColumnarOffsetType val_link, const std::uint8_t* val_keys, void** val_data)
      : m_link (val_link), m_keys (val_keys), m_data (val_data)
    {}

    const typename CI::xAODObjectIdType* getXAODObject () const
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    /// whether this is a valid link
    explicit operator bool () const noexcept
    {
      return m_link != invalidObjectIndex;
    }

    /// whether this is a valid link
    [[nodiscard]] bool has_value () const noexcept
    {
      return m_link != invalidObjectIndex;
    }

    /// whether this link is valid and points to an object of the given container
    ///
    /// Note that this is only exact in columnar mode, in xAOD mode it
    /// will employ a dynamic_cast, and may return true for multiple
    /// containers.
    template<ContainerIdConcept CI2>
    [[nodiscard]] bool isContainer () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      return getLinkKey() == m_keys[variantIndex];
    }

    /// whether this link points to the given object
    template<ContainerIdConcept CI2>
    [[nodiscard]] bool operator == (ObjectId<CI2,CM> id) const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      return getLinkIndex() == id.getIndex() && getLinkKey() == m_keys[variantIndex];
    }

    /// whether this link points to the given object
    ///
    /// Note that this is only exact in xAOD mode.  In columnar mode if
    /// both links are to containers not listed in CI it can in some
    /// situations return true, when it should return false.
    [[nodiscard]] bool operator == (const VariantObjectLink<CI,CM>& obj) const
    {
      if (m_keys == obj.m_keys)
        return m_link == obj.m_link;
      if (getLinkIndex() != obj.getLinkIndex())
        return false;
      const auto thisKey = getLinkKey();
      const auto thatKey = obj.getLinkKey();
      for (unsigned i = 0; i < CI::numVariants; ++ i)
      {
        if (m_keys[i] == thisKey)
        {
          if (obj.m_keys[i] == thatKey)
            return true;
          else
            return false;
        } else if (obj.m_keys[i] == thatKey)
          return false;
      }
      return false;
    }

    /// return the ObjectId if it is in the given container or `nullopt` otherwise
    template<ContainerIdConcept CI2>
    [[nodiscard]] OptObjectId<CI2,CM> tryGetObject () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      if (getLinkKey() == m_keys[variantIndex])
        return OptObjectId<CI2,CM> (m_data, getLinkIndex());
      else
        return OptObjectId<CI2,CM> ();
    }

    [[nodiscard]] ColumnarOffsetType getLinkIndex () const noexcept
    {
      return m_link & ~(static_cast<ColumnarOffsetType>(0xff) << (8*(sizeof(ColumnarOffsetType)-1)));
    }

    [[nodiscard]] std::uint8_t getLinkKey () const noexcept
    {
      return m_link >> (8*(sizeof(ColumnarOffsetType)-1));
    }

    /// Private Members
    /// ===============
  private:

    ColumnarOffsetType m_link = 0;
    const std::uint8_t* m_keys = nullptr;
    void** m_data = nullptr;
  };
  template<typename... LTList>
  std::ostream& operator<< (std::ostream& str, const VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeArray>& obj)
  {
    return str << obj.getLinkKey() << "/" << obj.getLinkIndex();
  }

  // in external mode we need to use a vector column, as well as an
  // extra column to contain our keys in order.
  template<ContainerIdConcept CI,typename... LTList>
  class AccessorTemplate<CI,std::vector<VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeArray>>,ColumnAccessMode::input,ColumnarModeArray> final
  {
    /// Public Members
    /// ==============
  public:

    using VariantCI = VariantContainerId<LTList...>;
    static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;
    using CM = ColumnarModeArray;
    static constexpr std::array containerIdNames = {LTList::idName...};

    AccessorTemplate () = default;

    AccessorTemplate (ColumnarTool<CM>& columnBase, const std::string& name, ColumnInfo&& info = {})
    {
      std::string offsetName = columnBase.containerStoreName (CI::idName) + "." + name + ".offset";
      std::string dataName = columnBase.containerStoreName (CI::idName) + "." + name + ".data";
      std::string keysName = columnBase.containerStoreName (CI::idName) + "." + name + ".keys";

      auto offsetInfo = info;
      offsetInfo.offsetName = columnBase.containerStoreName (CI::idName);
      offsetInfo.isOffset = true;
      auto dataInfo = info;
      dataInfo.offsetName = offsetName;
      dataInfo.variantLinkKeyColumn = keysName;
      dataInfo.variantLinkContainers.reserve (VariantCI::numVariants);
      for (unsigned i = 0; i < VariantCI::numVariants; ++ i)
        dataInfo.variantLinkContainers.push_back (columnBase.containerStoreName (containerIdNames[i]));
      auto keyInfo = info;
      keyInfo.fixedDimensions.push_back (VariantCI::numVariants);

      m_offsetData = std::make_unique<ColumnAccessorDataArray> (&m_offsetIndex, &m_offsetData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
      columnBase.addColumn (offsetName, m_offsetData.get(), std::move (offsetInfo));
      m_dataData = std::make_unique<ColumnAccessorDataArray> (&m_dataIndex, &m_dataData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
      columnBase.addColumn (dataName, m_dataData.get(), std::move (dataInfo));
      m_keysData = std::make_unique<ColumnAccessorDataArray> (&m_keysIndex, &m_keysData, &typeid (std::uint8_t), ColumnAccessMode::input);
      columnBase.addColumn (keysName, m_keysData.get(), std::move (keyInfo));
    }

    AccessorTemplate (AccessorTemplate&& that)
    {
      moveAccessor (m_offsetIndex, m_offsetData, that.m_offsetIndex, that.m_offsetData);
      moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
      moveAccessor (m_keysIndex, m_keysData, that.m_keysIndex, that.m_keysData);
    }

    AccessorTemplate& operator = (AccessorTemplate&& that)
    {
      if (this != &that)
      {
        moveAccessor (m_offsetIndex, m_offsetData, that.m_offsetIndex, that.m_offsetData);
        moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
        moveAccessor (m_keysIndex, m_keysData, that.m_keysIndex, that.m_keysData);
      }
      return *this;
    }

    auto operator () (ObjectId<CI,CM> id) const noexcept
    {
      auto *offset = static_cast<const ColumnarOffsetType*>(id.getData()[m_offsetIndex]);
      auto *data = static_cast<const ColumnarOffsetType*>(id.getData()[m_dataIndex]);
      auto *keys = static_cast<const std::uint8_t*>(id.getData()[m_keysIndex]);
      return detail::VectorConvertView ([keys,data=id.getData()](ColumnarOffsetType value) {return VariantObjectLink<VariantContainerId<LTList...>,ColumnarModeArray> (value, keys,data);}, std::span<const ColumnarOffsetType> (data + offset[id.getIndex()], offset[id.getIndex()+1]-offset[id.getIndex()]));
    }

    /// Private Members
    /// ===============
  private:

    unsigned m_offsetIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_offsetData;
    unsigned m_dataIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_dataData;
    unsigned m_keysIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_keysData;
  };
}

#endif
