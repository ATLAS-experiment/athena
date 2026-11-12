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

#include <vector>

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
    /// Note that this may return true for multiple containers: In xAOD
    /// mode it employs a dynamic_cast, and in columnar mode containers
    /// merged by renaming them accept the same keys.
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



  namespace detail
  {
    /// @brief the offset maps of the targets of a key+index variant link
    ///
    /// This is held through a pointer, so that the @ref ObjectLink can
    /// refer to it, independent of moves of the accessor.
    template<unsigned numVariants>
    struct VariantLinkTargets final
    {
      std::string columnName;
      std::array<unsigned,numVariants> offsetIndices {};
      std::array<std::unique_ptr<ColumnAccessorDataArray>,numVariants> offsetData;
    };

    /// @brief throw the exception for a link key that doesn't match any
    /// of the variants
    ///
    /// This is cold and not inlined, so that building the message
    /// doesn't bloat the link access (see @ref throwLinkIndexOutOfRange).
    /// It is defined here, as this package is header-only.
    [[noreturn]] [[gnu::cold]] [[gnu::noinline]] inline void throwVariantLinkUnknownKey (const std::string& columnName, SG::sgkey_t key, ColumnarOffsetType event)
    {
      throw std::runtime_error ("link key does not match any known container for column " + columnName + ": read key " + std::to_string (key) + " in event " + std::to_string (event));
    }

    /// @brief throw the exception for a link target that is not in the
    /// same event as the link
    ///
    /// This is cold and not inlined, see @ref throwVariantLinkUnknownKey
    /// (which also explains why it is defined here).
    [[noreturn]] [[gnu::cold]] [[gnu::noinline]] inline void throwVariantLinkOutsideEvent (const std::string& columnName, std::size_t objectIndex, ColumnarOffsetType event)
    {
      throw std::runtime_error ("link target not in the same event as the link for column " + columnName + ": object " + std::to_string (objectIndex) + " is outside the range of event " + std::to_string (event));
    }
  }



  /// @brief the number of keys a key+index variant link accepts for
  /// each of its targets
  ///
  /// Each target accepts the CLIDs of all the variants (a generic
  /// particle target can contain any of the other types), plus the
  /// link types. See the regular @ref LinkCastColumn for the details.
  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList,ContainerIdConcept ELT,ContainerIdConcept... ExtraELT>
  struct LinkCastColumn<VariantContainerId<CIBase,CIList...>,ELT,ExtraELT...>
  {
    static_assert ((requires { ExtraELT::containerClid(); } && ...),
      "the extra link types of a LinkCastColumn only add keys, so they need to provide a CLID");

    static constexpr unsigned numLinkTypes = detail::LinkDistinctTypeCount<detail::LinkClidType<CIList>...,detail::LinkClidType<ELT>,detail::LinkClidType<ExtraELT>...>::value;
  };



  /// @brief the @ref ObjectLink for variant links in key+index mode
  /// (i.e. @ref ColumnarModeArray)
  template<ContainerIdConcept CIBase,ContainerIdConcept... CIList, ColumnarArrayModeConcept CM>
    requires (CM::useKeyIndexAsLink)
  class ObjectLink<VariantContainerId<CIBase,CIList...>,CM> final
  {
    /// Public Members
    /// ==============
  public:

    using CI = VariantContainerId<CIBase,CIList...>;
    using KeyArray = std::array<std::vector<typename CM::LinkKeyColumnType>,CI::numVariants>;
    using Targets = detail::VariantLinkTargets<CI::numVariants>;

    ObjectLink (typename CM::LinkKeyColumnType val_key, typename CM::LinkIndexColumnType val_index, ColumnarOffsetType val_event, const KeyArray* val_keys, const Targets* val_targets, void** val_dataArea)
      : m_key (val_key), m_index (val_index), m_event (val_event), m_keys (val_keys), m_targets (val_targets), m_dataArea (val_dataArea)
    {}

    const typename CI::xAODObjectIdType* getXAODObject () const
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    /// whether this is a valid link
    explicit operator bool () const noexcept
    {
      return has_value();
    }

    /// whether this is a valid link
    [[nodiscard]] bool has_value () const noexcept
    {
      return !detail::isNullLink<CM> (m_key, m_index);
    }

    /// get an OptObjectId for this link
    [[nodiscard]] OptObjectId<CI,CM> opt_value () const
    {
      if (!has_value())
        return OptObjectId<CI,CM> ();
      const unsigned variant = findVariant();
      if (variant != CI::numVariants)
        return OptObjectId<CI,CM> (m_dataArea, variant, resolveIndex (variant));
      // not sure whether we should throw here or return a nullopt, but
      // throwing is probably the safer default. if it becomes an issue
      // we can revisit this.
      detail::throwVariantLinkUnknownKey (m_targets->columnName, m_key, m_event);
    }

    /// whether this link is valid and points to an object of the given container
    ///
    /// Note that this may return true for multiple containers: In xAOD
    /// mode it employs a dynamic_cast, and in columnar mode containers
    /// merged by renaming them accept the same keys.
    template<ContainerIdConcept CI2>
    [[nodiscard]] bool isContainer () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      return has_value() && detail::isLinkKey ((*m_keys)[variantIndex], m_key);
    }

    /// whether this link points to the given object
    template<ContainerIdConcept CI2>
    [[nodiscard]] bool operator == (ObjectId<CI2,CM> id) const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      return isContainer<CI2>() && resolveIndex (variantIndex) == id.getIndex();
    }

    /// whether this link points to the same object as the given link
    ///
    /// The link indices are relative to the event, so inside one event
    /// and one target container they identify the object, without
    /// having to check them against the offset map.
    ///
    /// Note that the target container is only known for containers in
    /// the variant.  For other containers this can only compare the
    /// keys, so links to the same object using different CLIDs compare
    /// as different.
    [[nodiscard]] bool operator == (const ObjectLink<CI,CM>& obj) const noexcept
    {
      if (!has_value() || !obj.has_value())
        return has_value() == obj.has_value();
      if (m_index != obj.m_index || m_event != obj.m_event)
        return false;
      if (m_key == obj.m_key)
        return true;
      // note: I'm slightly worried about the cost of this operation,
      // should it be called in a tight loop, with a lot of variants or
      // a lot of possible target containers. If that happens, we can
      // move the variant lookup into a flat hash map, preresolve it, or
      // do something else. For now the assumption is that we don't
      // really use this operation.
      //
      // The main reason for this to be this involved is that multiple
      // variants can be the same container, and at the same time, the
      // same variant can have multiple keys (corresponding to links
      // registered with different CLIDs). That is probably rare, but it
      // technically can happen.
      const unsigned variant = findVariant();
      if (variant != obj.findVariant())
        return false;
      return variant != CI::numVariants;
    }

    /// return the ObjectId if it is in the given container or `nullopt` otherwise
    template<ContainerIdConcept CI2>
    [[nodiscard]] OptObjectId<CI2,CM> tryGetVariant () const
    {
      static constexpr unsigned variantIndex = CI::template getVariantIndex<CI2>();
      static_assert (variantIndex < CI::numVariants, "invalid container id");
      if (isContainer<CI2>())
        return OptObjectId<CI2,CM> (m_dataArea, resolveIndex (variantIndex));
      else
        return OptObjectId<CI2,CM> ();
    }

    [[nodiscard]] auto getLinkIndex () const noexcept
    {
      return m_index;
    }

    [[nodiscard]] auto getLinkKey () const noexcept
    {
      return m_key;
    }

    /// Private Members
    /// ===============
  private:

    typename CM::LinkKeyColumnType m_key = CM::nullLinkKey;
    typename CM::LinkIndexColumnType m_index = CM::nullLinkIndex;
    ColumnarOffsetType m_event = 0;
    const KeyArray* m_keys = nullptr;
    const Targets* m_targets = nullptr;
    void** m_dataArea = nullptr;

    /// the first variant accepting the key, or `CI::numVariants` if
    /// there is none
    [[nodiscard]] unsigned findVariant () const noexcept
    {
      for (unsigned i = 0; i < CI::numVariants; ++ i)
      {
        if (detail::isLinkKey ((*m_keys)[i], m_key))
          return i;
      }
      return CI::numVariants;
    }

    [[nodiscard]] std::size_t resolveIndex (unsigned variantIndex) const
    {
      return detail::resolveLinkIndex<CM> (m_dataArea, m_targets->offsetIndices[variantIndex], m_event, m_index, m_targets->columnName);
    }
  };
  template<typename... CIList, ColumnarArrayModeConcept CM>
    requires (CM::useKeyIndexAsLink)
  std::ostream& operator<< (std::ostream& str, const ObjectLink<VariantContainerId<CIList...>,CM>& obj)
  {
    return str << obj.getLinkKey() << "/" << obj.getLinkIndex();
  }



  namespace detail
  {
    // in array mode the link consists of the key and index columns as
    // found in the input file, together with the event-index column
    // (for the offset map of the link column) and the offset maps of
    // all the target containers.
    //
    // As the links are typed, this is defined for LinkCastColumn
    // directly, and the ObjectLink accessor redirects to it (see
    // below), same as for regular links.
    //
    // It also calculates the keys it accepts for the target containers,
    // i.e. the StoreGate keys calculated from the container names and
    // the CLIDs of the targets and the link types. All targets accept
    // the CLIDs of all the variants, as e.g. a generic particle target
    // can contain any of the other types. The keys are shared with the
    // columnar tool, which updates them whenever the link target names
    // get set or change.
    template<ColumnAccessMode CAM,ContainerIdConcept CIBase,ContainerIdConcept... CIList,ContainerIdConcept ELT,ContainerIdConcept... ExtraELT,ColumnarArrayModeConcept CM>
      requires (CM::useKeyIndexAsLink)
    class ContainerFreeAccessor<LinkCastColumn<VariantContainerId<CIBase,CIList...>,ELT,ExtraELT...>,CAM,CM> final
    {
      /// Public Members
      /// ==============
    public:

      using VariantCI = VariantContainerId<CIBase,CIList...>;
      using Link = ObjectLink<VariantCI,CM>;
      static constexpr std::array containerIdNames = {CIList::idName...};

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = 0;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
      {
        m_targetKeys = std::make_shared<typename Link::KeyArray> ();
        m_targets = std::make_unique<typename Link::Targets> ();
        m_targets->columnName = optionsArray.baseName;
        const std::array<CLID,VariantCI::numVariants> memberClids = {linkTargetClid<CIList>()...};
        for (unsigned i = 0; i < VariantCI::numVariants; ++ i)
        {
          std::vector<CLID> clids;
          addLinkClid (clids, memberClids[i]);
          for (CLID clid : memberClids)
            addLinkClid (clids, clid);
          addLinkClid (clids, linkTargetClid<ELT>());
          (addLinkClid (clids, linkTargetClid<ExtraELT>()), ...);
          // until the target name is known the keys are all the null
          // key, which no (non-null) link can match, see the regular
          // links for details.
          (*m_targetKeys)[i].assign (clids.size(), CM::nullLinkKey);
          columnarTool.watchLinkTargetName (containerIdNames[i], [keys = m_targetKeys, i, clids = std::move (clids)] (const std::string& name)
          {
            fillLinkKeys ((*keys)[i], name, clids);
          });

          ColumnInfo targetInfo;
          targetInfo.offsetName = eventContextCIName;
          targetInfo.isOffset = true;
          targetInfo.isOptional = options.isOptional;
          m_targets->offsetData[i] = std::make_unique<ColumnAccessorDataArray> (&m_targets->offsetIndices[i], &m_targets->offsetData[i], &typeid (ColumnarOffsetType), ColumnAccessMode::input);
          columnarTool.addColumn (std::string (containerIdNames[i]), m_targets->offsetData[i].get(), std::move (targetInfo));
        }

        auto keyInfo = options.makeColumnInfo();
        keyInfo.offsetName = optionsArray.offsetName;
        m_keyData = std::make_unique<ColumnAccessorDataArray> (&m_keyIndex, &m_keyData, &typeid (typename CM::LinkKeyColumnType), CAM);
        columnarTool.addColumn (optionsArray.baseName + "." + std::string (CM::linkKeyColumnName), m_keyData.get(), std::move (keyInfo));

        auto indexInfo = options.makeColumnInfo();
        indexInfo.offsetName = optionsArray.offsetName;
        m_indexData = std::make_unique<ColumnAccessorDataArray> (&m_indexIndex, &m_indexData, &typeid (typename CM::LinkIndexColumnType), CAM);
        columnarTool.addColumn (optionsArray.baseName + "." + std::string (CM::linkIndexColumnName), m_indexData.get(), std::move (indexInfo));

        // the event-index column is shared by all columns using the
        // same offset map, i.e. it is named after the container for
        // scalar links, and after the vector for links inside vectors
        std::string eventName = optionsArray.numOffsets == 0u ? std::string (optionsArray.offsetName) : optionsArray.baseName;
        eventName += ".";
        eventName += CM::eventIndexColumnName;
        ColumnInfo eventInfo;
        eventInfo.offsetName = optionsArray.offsetName;
        eventInfo.isOptional = options.isOptional;
        eventInfo.isEventIndex = true;
        m_eventData = std::make_unique<ColumnAccessorDataArray> (&m_eventIndex, &m_eventData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
        columnarTool.addColumn (eventName, m_eventData.get(), std::move (eventInfo));
      }

      ContainerFreeAccessor (ContainerFreeAccessor&& that)
        : m_targetKeys (std::move (that.m_targetKeys)), m_targets (std::move (that.m_targets))
      {
        moveAccessor (m_keyIndex, m_keyData, that.m_keyIndex, that.m_keyData);
        moveAccessor (m_indexIndex, m_indexData, that.m_indexIndex, that.m_indexData);
        moveAccessor (m_eventIndex, m_eventData, that.m_eventIndex, that.m_eventData);
      }

      ContainerFreeAccessor& operator = (ContainerFreeAccessor&& that)
      {
        if (this != &that)
        {
          moveAccessor (m_keyIndex, m_keyData, that.m_keyIndex, that.m_keyData);
          moveAccessor (m_indexIndex, m_indexData, that.m_indexIndex, that.m_indexData);
          moveAccessor (m_eventIndex, m_eventData, that.m_eventIndex, that.m_eventData);
          m_targetKeys = std::move (that.m_targetKeys);
          m_targets = std::move (that.m_targets);
        }
        return *this;
      }

      auto operator () (void** dataArea, std::size_t index) const
        requires (CAM == ColumnAccessMode::input)
      {
        return link (dataArea, index).opt_value();
      }

      auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const
        requires (CAM == ColumnAccessMode::input)
      {
        return VectorConvertView ([] (const Link& link) {return link.opt_value();},
          links (dataArea, beginIndex, endIndex));
      }

      /// get the @ref ObjectLink for the given index
      [[nodiscard]] auto link (void** dataArea, std::size_t index) const noexcept
        requires (CAM == ColumnAccessMode::input)
      {
        auto *keys = static_cast<const typename CM::LinkKeyColumnType*>(dataArea[m_keyIndex]);
        auto *indices = static_cast<const typename CM::LinkIndexColumnType*>(dataArea[m_indexIndex]);
        auto *events = static_cast<const ColumnarOffsetType*>(dataArea[m_eventIndex]);
        return Link (keys[index], indices[index], events[index], m_targetKeys.get(), m_targets.get(), dataArea);
      }

      /// get the @ref ObjectLink objects for the given index range
      [[nodiscard]] auto links (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const noexcept
        requires (CAM == ColumnAccessMode::input)
      {
        return detail::VectorConvertView ([this,dataArea] (std::size_t index) {return link (dataArea, index);},
          std::views::iota (beginIndex, endIndex));
      }

      template<ContainerIdConcept CI2>
        requires (CAM == ColumnAccessMode::output && VariantCI::template isValidContainer<CI2>())
      void set (void** dataArea, std::size_t index, ObjectId<CI2,CM> obj) const
      {
        static constexpr unsigned variantIndex = VariantCI::template getVariantIndex<CI2>();
        const auto event = static_cast<const ColumnarOffsetType*>(dataArea[m_eventIndex])[index];
        auto *offsets = static_cast<const ColumnarOffsetType*>(dataArea[m_targets->offsetIndices[variantIndex]]);
        if constexpr (!CM::skipLinkValidation)
        {
          if (obj.getIndex() < offsets[event] || obj.getIndex() >= offsets[event+1])
            detail::throwVariantLinkOutsideEvent (m_targets->columnName, obj.getIndex(), event);
        }
        static_cast<typename CM::LinkKeyColumnType*>(dataArea[m_keyIndex])[index] = (*m_targetKeys)[variantIndex].front();
        static_cast<typename CM::LinkIndexColumnType*>(dataArea[m_indexIndex])[index] = obj.getIndex() - offsets[event];
      }

      [[nodiscard]] bool isAvailable (void** dataArea) const noexcept
      {
        if (dataArea[m_keyIndex] == nullptr || dataArea[m_indexIndex] == nullptr || dataArea[m_eventIndex] == nullptr)
          return false;
        for (unsigned offsetIndex : m_targets->offsetIndices)
        {
          if (dataArea[offsetIndex] == nullptr)
            return false;
        }
        return true;
      }

      /// the keys of the target containers based on their own types, in
      /// the order of the variants
      [[nodiscard]] auto targetKeys () const noexcept
      {
        std::array<typename CM::LinkKeyColumnType,VariantCI::numVariants> result {};
        for (unsigned i = 0; i < VariantCI::numVariants; ++ i)
          result[i] = (*m_targetKeys)[i].front();
        return result;
      }

      /// all the keys accepted for the target containers, in the order
      /// of the variants
      [[nodiscard]] const auto& allTargetKeys () const noexcept
      {
        return *m_targetKeys;
      }



      /// Private Members
      /// ===============
    private:

      std::shared_ptr<typename Link::KeyArray> m_targetKeys;
      std::unique_ptr<typename Link::Targets> m_targets;
      unsigned m_keyIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_keyData;
      unsigned m_indexIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_indexData;
      unsigned m_eventIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_eventData;
    };



    // the links with the implicit types redirect to the typed links
    template<ColumnAccessMode CAM,ContainerIdConcept CIBase,ContainerIdConcept... CIList, ColumnarArrayModeConcept CM>
      requires (CM::useKeyIndexAsLink)
    class ContainerFreeAccessor<ObjectLink<VariantContainerId<CIBase,CIList...>,CM>,CAM,CM> final
    {
      /// Public Members
      /// ==============
    public:

      using VariantCI = VariantContainerId<CIBase,CIList...>;
      using BaseAccessor = ContainerFreeAccessor<LinkCastColumn<VariantCI,CIBase>,CAM,CM>;

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = BaseAccessor::internalOffsetColumns;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
        : m_accessor (columnarTool, std::move (options), std::move (optionsArray))
      {}

      auto operator () (void** dataArea, std::size_t index) const noexcept
        requires (CAM == ColumnAccessMode::input)
      {
        return m_accessor.link (dataArea, index);
      }

      [[nodiscard]] auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const noexcept
        requires (CAM == ColumnAccessMode::input)
      {
        return m_accessor.links (dataArea, beginIndex, endIndex);
      }

      template<ContainerIdConcept CI2>
        requires (CAM == ColumnAccessMode::output && VariantCI::template isValidContainer<CI2>())
      void set (void** dataArea, std::size_t index, ObjectId<CI2,CM> obj) const
      {
        m_accessor.set (dataArea, index, obj);
      }

      [[nodiscard]] bool isAvailable (void** dataArea) const noexcept
      {
        return m_accessor.isAvailable (dataArea);
      }

      /// the keys of the target containers based on their own types, in
      /// the order of the variants
      [[nodiscard]] auto targetKeys () const noexcept
      {
        return m_accessor.targetKeys ();
      }

      /// all the keys accepted for the target containers, in the order
      /// of the variants
      [[nodiscard]] const auto& allTargetKeys () const noexcept
      {
        return m_accessor.allTargetKeys ();
      }

      /// Private Members
      /// ===============
    private:

      BaseAccessor m_accessor;
    };
  }
}

#endif
