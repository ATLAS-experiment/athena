/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_VARIANT_VARIANT_LINK_COLUMN_H
#define COLUMNAR_VARIANT_VARIANT_LINK_COLUMN_H

#include <ColumnarCore/LinkColumn.h>
#include <ColumnarVariant/VariantDef.h>
#include <ColumnarVariant/VariantOptObjectId.h>
#include <ColumnarCore/VectorColumn.h>

namespace columnar
{
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

  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList>
  class ObjectLink<VariantContainerId<CIBase,CIList...>,ColumnarModeXAOD> final
  {
    /// Public Members
    /// ==============
  public:

    using CI = VariantContainerId<CIBase,CIList...>;
    using CM = ColumnarModeXAOD;

    using LinkType = ElementLink<typename CI::xAODElementLinkType>;

    ObjectLink (const LinkType* val_link)
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
    explicit operator bool () const 
    {
      return m_link->isValid();
    }

    /// whether this is a valid link
    [[nodiscard]] bool has_value () const 
    {
      return m_link->isValid();
    }

    /// get an OptObjectId for this link
    [[nodiscard]] OptObjectId<CI,CM> opt_value () const
    {
      return OptObjectId<CI,CM> (getXAODObject ());
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
    [[nodiscard]] bool operator == (const ObjectLink<CI,CM>& obj) const
    {
      return getXAODObject() == obj.getXAODObject();
    }

    /// return the ObjectId if it is in the given container or `nullopt` otherwise
    template<ContainerIdConcept CI2>
    [[nodiscard]] OptObjectId<CI2,CM> tryGetVariant () const
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
  template<typename... CIList>
  std::ostream& operator<< (std::ostream& str, const ObjectLink<VariantContainerId<CIList...>,ColumnarModeXAOD>& obj)
  {
    return str << obj.getXAODObject() << "/" << obj.getXAODObject()->type();
  }

  namespace detail
  {
    // in xAOD mode we can do a fairly straightforward conversion from
    // ElementLink as the logic is inside ObjectLink
    template<ContainerIdConcept CIBase,typename... CIList>
    class MemoryAccessor<ObjectLink<VariantContainerId<CIBase,CIList...>,ColumnarModeXAOD>,ColumnarModeXAOD> final
    {
      /// Public Members
      /// ==============
    public:

      using CI = VariantContainerId<CIBase,CIList...>;
      using CM = ColumnarModeXAOD;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = true;
      using MemoryType = ElementLink<typename CI::xAODElementLinkType>;

      static void updateColumnInfo (ColumnInfo& /*info*/) {}

      [[nodiscard]] static auto makeViewer (void**)
      {
        return [] (const ElementLink<typename CI::xAODElementLinkType>& value) {
          return ObjectLink<VariantContainerId<CIBase,CIList...>,ColumnarModeXAOD> (&value);
        };
      }

      struct Setter final
      {
        template<ContainerIdConcept CI2>
          requires (CI::template isValidContainer<CI2>())
        auto operator() (MemoryType& link, const ObjectId<CI2,ColumnarModeXAOD>& obj)
        {
          auto* container = static_cast<const typename CIBase::xAODElementLinkType*>(obj.getXAODObjectNoexcept().container());
          link = MemoryType(*container, obj.getXAODObjectNoexcept().index());
        }
      };
      [[nodiscard]] static auto makeSetter (void**)
      {
        return Setter{};
      }
    };
  }    



  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList, ColumnarArrayMode CM>
  class ObjectLink<VariantContainerId<CIBase,CIList...>,CM> final
  {
    /// Public Members
    /// ==============
  public:

    using CI = VariantContainerId<CIBase,CIList...>;

    ObjectLink (typename CM::LinkIndexType val_link, const typename CM::LinkKeyType* val_keys, void** val_dataArea)
      : m_link (val_link), m_keys (val_keys), m_dataArea (val_dataArea)
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

    /// get an OptObjectId for this link
    [[nodiscard]] OptObjectId<CI,CM> opt_value () const
    {
      if (m_link == CM::invalidLinkValue)
        return OptObjectId<CI,CM> ();
      const auto key = getLinkKey();
      for (unsigned i = 0; i < CI::numVariants; ++ i)
      {
        if (m_keys[i] == key)
          return OptObjectId<CI,CM> (m_dataArea, i, getLinkIndex());
      }
      // not sure whether we should throw here or return a nullopt, but
      // throwing is probably the safer default. if it becomes an issue
      // we can revisit this.
      throw std::runtime_error ("link key does not match any known container: " + std::to_string(key));
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
    [[nodiscard]] bool operator == (const ObjectLink<CI,CM>& obj) const
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
    [[nodiscard]] OptObjectId<CI2,CM> tryGetVariant () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      if (getLinkKey() == m_keys[variantIndex])
        return OptObjectId<CI2,CM> (m_dataArea, getLinkIndex());
      else
        return OptObjectId<CI2,CM> ();
    }

    [[nodiscard]] auto getLinkIndex () const noexcept
    {
      return CM::getLinkIndex (m_link);
    }

    [[nodiscard]] auto getLinkKey () const noexcept
    {
      return CM::getLinkKey (m_link);
    }

    /// Private Members
    /// ===============
  private:

    typename CM::LinkIndexType m_link = 0;
    const CM::LinkKeyType* m_keys = nullptr;
    void** m_dataArea = nullptr;
  };
  template<typename... CIList, ColumnarArrayMode CM>
  std::ostream& operator<< (std::ostream& str, const ObjectLink<VariantContainerId<CIList...>,CM>& obj)
  {
    return str << obj.getLinkKey() << "/" << obj.getLinkIndex();
  }



  namespace detail
  {
    // in Array mode we need to use a vector column, as well as an
    // extra column to contain our keys in order.
    template<ColumnAccessMode CAM,ContainerIdConcept CIBase,ContainerIdConcept... CIList, ColumnarArrayMode CM>
    class ContainerFreeAccessor<ObjectLink<VariantContainerId<CIBase,CIList...>,CM>,CAM,CM> final
    {
      /// Public Members
      /// ==============
    public:

      using VariantCI = VariantContainerId<CIBase,CIList...>;
      static constexpr std::array containerIdNames = {CIList::idName...};

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = 0;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
      {
        std::string dataName = optionsArray.baseName + optionsArray.dataSuffix;
        std::string keysName = optionsArray.baseName + ".keys";

        auto dataInfo = options.makeColumnInfo();
        dataInfo.offsetName = optionsArray.offsetName;
        dataInfo.isVariantLink = true;

        auto keyInfo = options.makeColumnInfo();
        keyInfo.accessMode = ColumnAccessMode::input;
        keyInfo.fixedDimensions.push_back (VariantCI::numVariants);
        keyInfo.keyColumnForVariantLink = dataName;
        keyInfo.variantLinkTargetNames.reserve (VariantCI::numVariants);
        for (unsigned i = 0; i < VariantCI::numVariants; ++ i)
          keyInfo.variantLinkTargetNames.emplace_back (containerIdNames[i]);

        m_dataData = std::make_unique<ColumnAccessorDataArray> (&m_dataIndex, &m_dataData, &typeid (typename CM::LinkIndexType), CAM);
        columnarTool.addColumn (dataName, m_dataData.get(), std::move (dataInfo));
        m_keysData = std::make_unique<ColumnAccessorDataArray> (&m_keysIndex, &m_keysData, &typeid (typename CM::LinkKeyType), ColumnAccessMode::input);
        columnarTool.addColumn (keysName, m_keysData.get(), std::move (keyInfo));
      }

      ContainerFreeAccessor (ContainerFreeAccessor&& that)
      {
        moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
        moveAccessor (m_keysIndex, m_keysData, that.m_keysIndex, that.m_keysData);
      }

      ContainerFreeAccessor& operator = (ContainerFreeAccessor&& that)
      {
        if (this != &that)
        {
          moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
          moveAccessor (m_keysIndex, m_keysData, that.m_keysIndex, that.m_keysData);
        }
        return *this;
      }

      auto operator () (void** dataArea, std::size_t index) const noexcept
        requires (CAM == ColumnAccessMode::input)
      {
        auto *data = static_cast<const typename CM::LinkIndexType*>(dataArea[m_dataIndex]);
        auto *keys = static_cast<const typename CM::LinkKeyType*>(dataArea[m_keysIndex]);
        return ObjectLink<VariantContainerId<CIBase,CIList...>,CM> (data[index], keys, dataArea);
      }

      [[nodiscard]] auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const noexcept
        requires (CAM == ColumnAccessMode::input)
      {
        auto *data = static_cast<const typename CM::LinkIndexType*>(dataArea[m_dataIndex]);
        auto *keys = static_cast<const typename CM::LinkKeyType*>(dataArea[m_keysIndex]);
        return detail::VectorConvertView ([dataArea, keys](const auto& value) {return ObjectLink<VariantContainerId<CIBase,CIList...>,CM> (value, keys, dataArea);}, std::span<const typename CM::LinkIndexType>(data+beginIndex, endIndex-beginIndex));
      }

      template<ContainerIdConcept CI2>
        requires (CAM == ColumnAccessMode::output && VariantCI::template isValidContainer<CI2>())
      void set (void** dataArea, std::size_t index, ObjectId<CI2,CM> obj) const noexcept
      {
        auto *data = static_cast<typename CM::LinkIndexType*>(dataArea[m_dataIndex]);
        auto *keys = static_cast<const typename CM::LinkKeyType*>(dataArea[m_keysIndex]);
        data[index] = CM::mergeLinkKeyIndex (keys[VariantCI::template getVariantIndex<CI2>()], obj.getIndex());
      }

      [[nodiscard]] bool isAvailable (void** dataArea) const noexcept
      {
        auto *data = static_cast<const typename CM::LinkIndexType*>(dataArea[m_dataIndex]);
        return data != nullptr;
      }

  
  
      /// Private Members
      /// ===============
    private:

      unsigned m_dataIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_dataData;
      unsigned m_keysIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_keysData;
    };



    template<ContainerIdConcept CIBase,ContainerIdConcept... CIList,typename ELT>
    class ContainerFreeAccessor<LinkCastColumn<VariantContainerId<CIBase,CIList...>,ELT>,ColumnAccessMode::input,ColumnarModeArray> final
    {
      /// Public Members
      /// ==============
    public:

      using CI = VariantContainerId<CIBase,CIList...>;
      static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;
      using CM = ColumnarModeArray;
      using BaseAccessor = ContainerFreeAccessor<ObjectLink<CI,CM>,CAM,CM>;

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = BaseAccessor::internalOffsetColumns;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
        : m_accessor (columnarTool, std::move (options), std::move (optionsArray))
      {}

      auto operator () (void** dataArea, std::size_t index) const noexcept
      {
        return m_accessor(dataArea, index).opt_value();
      }

      auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const noexcept
      {
        return VectorConvertView ([] (const ObjectLink<CI,CM>& link) {return link.opt_value();},
          m_accessor(dataArea, beginIndex, endIndex));
      }

      bool isAvailable (void** dataArea) const noexcept
      {
        return m_accessor.isAvailable (dataArea);
      }

      /// Private Members
      /// ===============
    private:

      BaseAccessor m_accessor;
    };
  }
}

#endif
