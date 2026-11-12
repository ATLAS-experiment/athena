/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_LINK_COLUMN_H
#define COLUMNAR_CORE_LINK_COLUMN_H

#include <AthLinks/ElementLink.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarCore/ContainerId.h>
#include <ColumnarCore/OptObjectId.h>
#include <ColumnarCore/VectorConvertView.h>
#include <SGCore/sgkey_t.h>

#include <algorithm>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

namespace columnar
{
  namespace detail
  {
    /// @brief the type of the container a link column targets, for the
    /// purpose of counting the keys a link accepts, or `void` if the
    /// container id does not provide a CLID (see @ref linkTargetClid)
    ///
    /// This only exists to calculate @ref LinkCastColumn::numLinkTypes,
    /// for both regular and variant links.
    template<typename CI>
    struct LinkClidTypeImp final { using type = void; };
    template<typename CI>
      requires requires { CI::containerClid(); }
    struct LinkClidTypeImp<CI> final { using type = typename CI::xAODElementLinkType; };
    template<typename CI>
    using LinkClidType = typename LinkClidTypeImp<CI>::type;

    /// @brief the number of distinct types in a list of types, ignoring
    /// `void`
    ///
    /// This only exists to calculate @ref LinkCastColumn::numLinkTypes,
    /// for both regular and variant links.
    template<typename... Types>
    struct LinkDistinctTypeCount final : std::integral_constant<unsigned,0u> {};
    template<typename T,typename... Rest>
    struct LinkDistinctTypeCount<T,Rest...> final : std::integral_constant<unsigned,((std::is_void_v<T> || (std::is_same_v<T,Rest> || ...)) ? 0u : 1u) + LinkDistinctTypeCount<Rest...>::value> {};
  }



  /// a special column type that behaves like an @ref OptObjectId, but
  /// applies an internal cast in xAOD mode
  ///
  /// In general there are three types involved in a link column, each
  /// serving a different role:
  /// - `LT` is the type the link is accessed with in columnar code
  /// - `ELT` the expected type of the ElementLink decoration
  /// - `ExtraELT...` any other types that may have been used to calculate
  ///   the `sgkey` with which a target container is identified (which
  ///   are based on the type `CLID` and the container name)
  ///
  /// All these types are identified via a ContainerId. Except for `LT`
  /// the ContainerId is only used to identify the xAOD type and it
  /// doesn't matter e.g. whether you pass `Particle0Def` or
  /// `Particle1Def`. Making them all ContainerId makes type handling
  /// somewhat more uniform, and means we encapsulate the concrete types
  /// a bit more in columnar code.
  ///
  /// For xAOD-mode code the only types used are the link type and the
  /// `ElementLink` type. It will retrieve the link using the
  /// `ElementLink` type and then immediately cast to the link type.
  /// This is needed because xAOD decorations have an exact type, and it
  /// is not always the type we use a link as.
  ///
  /// For array-mode the main type used is the link type, which directly
  /// indicates the type we use. The other types are however also used
  /// and important, as they are used to deduce the possible `sgkey`
  /// values in the on-disk representation.
  ///
  /// **WARNING:** For "variant" links there is no check that your list
  /// of types is complete. If a type is missing and that's what the
  /// link is saved as, it will just be ignored and assumed to belong to
  /// an unenumerated target container.
  ///
  /// IMPORTANT: The exact handling of keys and types for keys is still
  /// in flux (30 Oct 26). Should we find that this way of handling
  /// links is too inflexible it may change in the future.
  template<ContainerIdConcept LT,ContainerIdConcept ELT = LT,ContainerIdConcept... ExtraELT>
  struct LinkCastColumn
  {
    static_assert ((requires { ExtraELT::containerClid(); } && ...),
      "the extra link types of a LinkCastColumn only add keys, so they need to provide a CLID");

    /// the number of keys a key+index link accepts for its target, i.e.
    /// the number of distinct types among the target container, `ELT`
    /// and `ExtraELT...`
    ///
    /// This relies on distinct types having distinct CLIDs, which is
    /// the same assumption the keys themselves rely on. It mirrors the
    /// CLIDs collected by the link accessor (the CLID of the target
    /// container followed by the CLIDs of the link types).
    static constexpr unsigned numLinkTypes = detail::LinkDistinctTypeCount<detail::LinkClidType<LT>,detail::LinkClidType<ELT>,detail::LinkClidType<ExtraELT>...>::value;
  };



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

    // the links with the implicit types can simply redirect to the code
    // for typed links.  for key+index links this happens at the level of
    // the ContainerFreeAccessor instead (see below).
    template<ContainerIdConcept LT,ColumnarModeConcept CM>
      requires (!CM::useKeyIndexAsLink)
    class MemoryAccessor<OptObjectId<LT,CM>,CM> final
    {
    public:

      using BaseAccessor = MemoryAccessor<LinkCastColumn<LT>,CM>;
      static_assert (BaseAccessor::isDefined, "MemoryAccessor for LinkCastColumn must be defined");

      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = BaseAccessor::viewIsReference;
      static constexpr bool hasSetter = false;
      using MemoryType = typename BaseAccessor::MemoryType;

      [[nodiscard]] static auto makeViewer (void** dataArea)
      {
        return BaseAccessor::makeViewer(dataArea);
      }
    };

    template<ContainerIdConcept LT,ContainerIdConcept ELT,ContainerIdConcept... ExtraELT>
    class MemoryAccessor<LinkCastColumn<LT,ELT,ExtraELT...>,ColumnarModeXAOD> final
    {
    public:
      using CM = ColumnarModeXAOD;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = ElementLink<typename ELT::xAODElementLinkType>;
      static auto makeViewer (void**)
      {
        return [] (const ElementLink<typename ELT::xAODElementLinkType>& link)
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




    /// @brief whether the given key and index represent a null link
    template<ColumnarModeConcept CM>
    [[nodiscard]] bool isNullLink (typename CM::LinkKeyColumnType key, typename CM::LinkIndexColumnType index) noexcept
    {
      return key == CM::nullLinkKey || index == CM::nullLinkIndex;
    }

    /// @brief add a CLID to the list of CLIDs a link accepts
    ///
    /// The order matters, the first CLID is the one used when writing
    /// links.
    inline void addLinkClid (std::vector<CLID>& clids, CLID clid)
    {
      if (clid != 0 && std::find (clids.begin(), clids.end(), clid) == clids.end())
        clids.push_back (clid);
    }

    /// @brief calculate the keys a link accepts for the given container name
    inline void fillLinkKeys (std::vector<SG::sgkey_t>& keys, const std::string& name, const std::vector<CLID>& clids)
    {
      keys.clear();
      for (CLID clid : clids)
        keys.push_back (computeSgKey (name, clid));
    }

    /// @brief whether the given key is in the list of accepted keys
    [[nodiscard]] inline bool isLinkKey (const std::vector<SG::sgkey_t>& keys, SG::sgkey_t key) noexcept
    {
      return std::find (keys.begin(), keys.end(), key) != keys.end();
    }

    /// @brief throw the exception for a link index outside its event
    ///
    /// This (and @ref throwLinkKeyMismatch) is out of line and marked
    /// cold on purpose: building the message inline made the link
    /// accessors too big to inline, and gave them a large stack frame
    /// that was paid on every (non-throwing) call.
    ///
    /// Defined in `Root/ColumnInfoHelpers.cxx`.
    [[noreturn]] [[gnu::cold]] [[gnu::noinline]]
    void throwLinkIndexOutOfRange (const std::string& columnName, std::size_t index, ColumnarOffsetType event, std::size_t numTargets);

    /// @brief throw the exception for a link key that doesn't match any
    /// of the keys accepted for the link target
    ///
    /// Defined in `Root/ColumnInfoHelpers.cxx`, see
    /// @ref throwLinkIndexOutOfRange for why this is out of line.
    [[noreturn]] [[gnu::cold]] [[gnu::noinline]]
    void throwLinkKeyMismatch (const std::string& columnName, SG::sgkey_t key, ColumnarOffsetType event, const std::vector<SG::sgkey_t>& expected);

    /// @brief convert the event-relative index of a link into an index
    /// into the target column, checking it is inside the event
    template<typename CM>
    [[nodiscard]] inline std::size_t resolveLinkIndex (void** dataArea, unsigned targetOffsetsIndex, ColumnarOffsetType event, std::size_t index, const std::string& columnName)
    {
      auto *offsets = static_cast<const ColumnarOffsetType*>(dataArea[targetOffsetsIndex]);
      const std::size_t result = offsets[event] + index;
      if constexpr (!CM::skipLinkValidation)
      {
        if (result >= offsets[event+1])
          throwLinkIndexOutOfRange (columnName, index, event, offsets[event+1]-offsets[event]);
      }
      return result;
    }



    // when using key+index links the link consists of the key and index
    // columns as found in the input file, with the index being relative
    // to the start of the event. To turn that into an OptObjectId this
    // needs the event-index column (for the offset map of the link
    // column) and the offset map of the target container.
    //
    // It also calculates the keys it accepts for the target container,
    // i.e. the StoreGate keys calculated from the container name and
    // the CLIDs of the target and the link types. The keys are shared
    // with the columnar tool, which updates them whenever the link
    // target name gets set or changes.
    template<RegularContainerIdConcept LT,ContainerIdConcept ELT,ContainerIdConcept... ExtraELT,ColumnarModeConcept CM>
      requires (CM::useKeyIndexAsLink)
    class ContainerFreeAccessor<LinkCastColumn<LT,ELT,ExtraELT...>,ColumnAccessMode::input,CM> final
    {
      /// Public Members
      /// ==============
    public:

      static_assert (LT::perEventRange, "links can only target containers with per-event ranges");

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = 0;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
        : m_columnName (optionsArray.baseName)
      {
        std::vector<CLID> clids;
        addLinkClid (clids, linkTargetClid<LT>());
        addLinkClid (clids, linkTargetClid<ELT>());
        (addLinkClid (clids, linkTargetClid<ExtraELT>()), ...);
        // until the target name is known the keys are all the null key,
        // which no (non-null) link can match. this way the vector has
        // its final size from the start, and the accessor never has to
        // check for it being empty.
        m_targetKeys = std::make_shared<std::vector<typename CM::LinkKeyColumnType>> (clids.size(), CM::nullLinkKey);
        columnarTool.watchLinkTargetName (LT::idName, [keys = m_targetKeys, clids = std::move (clids)] (const std::string& name)
        {
          fillLinkKeys (*keys, name, clids);
        });

        auto keyInfo = options.makeColumnInfo();
        keyInfo.offsetName = optionsArray.offsetName;
        m_keyData = std::make_unique<ColumnAccessorDataArray> (&m_keyIndex, &m_keyData, &typeid (typename CM::LinkKeyColumnType), ColumnAccessMode::input);
        columnarTool.addColumn (optionsArray.baseName + "." + std::string (CM::linkKeyColumnName), m_keyData.get(), std::move (keyInfo));

        auto indexInfo = options.makeColumnInfo();
        indexInfo.offsetName = optionsArray.offsetName;
        m_indexData = std::make_unique<ColumnAccessorDataArray> (&m_indexIndex, &m_indexData, &typeid (typename CM::LinkIndexColumnType), ColumnAccessMode::input);
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

        ColumnInfo targetInfo;
        targetInfo.offsetName = eventContextCIName;
        targetInfo.isOffset = true;
        targetInfo.isOptional = options.isOptional;
        m_targetOffsetsData = std::make_unique<ColumnAccessorDataArray> (&m_targetOffsetsIndex, &m_targetOffsetsData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
        columnarTool.addColumn (std::string (LT::idName), m_targetOffsetsData.get(), std::move (targetInfo));
      }

      ContainerFreeAccessor (ContainerFreeAccessor&& that)
        : m_columnName (std::move (that.m_columnName)), m_targetKeys (std::move (that.m_targetKeys))
      {
        moveAccessor (m_keyIndex, m_keyData, that.m_keyIndex, that.m_keyData);
        moveAccessor (m_indexIndex, m_indexData, that.m_indexIndex, that.m_indexData);
        moveAccessor (m_eventIndex, m_eventData, that.m_eventIndex, that.m_eventData);
        moveAccessor (m_targetOffsetsIndex, m_targetOffsetsData, that.m_targetOffsetsIndex, that.m_targetOffsetsData);
      }

      ContainerFreeAccessor& operator = (ContainerFreeAccessor&& that)
      {
        if (this != &that)
        {
          moveAccessor (m_keyIndex, m_keyData, that.m_keyIndex, that.m_keyData);
          moveAccessor (m_indexIndex, m_indexData, that.m_indexIndex, that.m_indexData);
          moveAccessor (m_eventIndex, m_eventData, that.m_eventIndex, that.m_eventData);
          moveAccessor (m_targetOffsetsIndex, m_targetOffsetsData, that.m_targetOffsetsIndex, that.m_targetOffsetsData);
          m_columnName = std::move (that.m_columnName);
          m_targetKeys = std::move (that.m_targetKeys);
        }
        return *this;
      }

      [[nodiscard]] OptObjectId<LT,CM> operator () (void** dataArea, std::size_t index) const
      {
        const auto key = static_cast<const typename CM::LinkKeyColumnType*>(dataArea[m_keyIndex])[index];
        const auto linkIndex = static_cast<const typename CM::LinkIndexColumnType*>(dataArea[m_indexIndex])[index];
        if (isNullLink<CM> (key, linkIndex))
          return OptObjectId<LT,CM> ();
        const auto event = static_cast<const ColumnarOffsetType*>(dataArea[m_eventIndex])[index];
        if constexpr (!CM::skipLinkValidation)
        {
          if constexpr (LinkCastColumn<LT,ELT,ExtraELT...>::numLinkTypes == 1u)
          {
            // with a single accepted type there is exactly one key
            if (m_targetKeys->front() != key)
              throwLinkKeyMismatch (m_columnName, key, event, *m_targetKeys);
          } else
          {
            if (!isLinkKey (*m_targetKeys, key))
              throwLinkKeyMismatch (m_columnName, key, event, *m_targetKeys);
          }
        }
        return OptObjectId<LT,CM> (dataArea, resolveLinkIndex<CM> (dataArea, m_targetOffsetsIndex, event, linkIndex, m_columnName));
      }

      [[nodiscard]] auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const
      {
        return VectorConvertView ([this,dataArea] (std::size_t index) {return (*this) (dataArea, index);},
          std::views::iota (beginIndex, endIndex));
      }

      [[nodiscard]] bool isAvailable (void** dataArea) const noexcept
      {
        return dataArea[m_keyIndex] != nullptr && dataArea[m_indexIndex] != nullptr && dataArea[m_eventIndex] != nullptr && dataArea[m_targetOffsetsIndex] != nullptr;
      }

      /// the key of the target container based on its own type
      [[nodiscard]] typename CM::LinkKeyColumnType targetKey () const noexcept
      {
        return m_targetKeys->empty() ? CM::nullLinkKey : m_targetKeys->front();
      }

      /// all the keys accepted for the target container
      [[nodiscard]] const auto& allTargetKeys () const noexcept
      {
        return *m_targetKeys;
      }

      /// Private Members
      /// ===============
    private:

      std::string m_columnName;
      std::shared_ptr<std::vector<typename CM::LinkKeyColumnType>> m_targetKeys;
      unsigned m_keyIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_keyData;
      unsigned m_indexIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_indexData;
      unsigned m_eventIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_eventData;
      unsigned m_targetOffsetsIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_targetOffsetsData;
    };

    // the links with the implicit types redirect to the typed links
    template<RegularContainerIdConcept LT,ColumnarModeConcept CM>
      requires (CM::useKeyIndexAsLink)
    class ContainerFreeAccessor<OptObjectId<LT,CM>,ColumnAccessMode::input,CM> final
    {
      /// Public Members
      /// ==============
    public:

      using BaseAccessor = ContainerFreeAccessor<LinkCastColumn<LT>,ColumnAccessMode::input,CM>;

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = BaseAccessor::internalOffsetColumns;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
        : m_accessor (columnarTool, std::move (options), std::move (optionsArray))
      {}

      [[nodiscard]] auto operator () (void** dataArea, std::size_t index) const
      {
        return m_accessor (dataArea, index);
      }

      [[nodiscard]] auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const
      {
        return m_accessor (dataArea, beginIndex, endIndex);
      }

      [[nodiscard]] bool isAvailable (void** dataArea) const noexcept
      {
        return m_accessor.isAvailable (dataArea);
      }

      /// the key of the target container based on its own type
      [[nodiscard]] auto targetKey () const noexcept
      {
        return m_accessor.targetKey ();
      }

      /// all the keys accepted for the target container
      [[nodiscard]] const auto& allTargetKeys () const noexcept
      {
        return m_accessor.allTargetKeys ();
      }

      /// Private Members
      /// ===============
    private:

      BaseAccessor m_accessor;
    };



    template<ContainerIdConcept LT,ContainerIdConcept ELT,ContainerIdConcept... ExtraELT>
    class MemoryAccessor<LinkCastColumn<LT,ELT,ExtraELT...>,ColumnarModeXAODArray> final
    {
    public:
      using CM = ColumnarModeXAODArray;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = ElementLink<typename ELT::xAODElementLinkType>;

      static auto makeViewer (void** dataArea)
      {
        return [dataArea] (const ElementLink<typename ELT::xAODElementLinkType>& link)
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
