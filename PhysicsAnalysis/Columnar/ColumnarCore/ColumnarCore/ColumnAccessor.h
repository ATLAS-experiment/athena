/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_COLUMN_ACCESSOR_H
#define COLUMNAR_CORE_COLUMN_ACCESSOR_H

#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/Decorator.h"
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectId.h>
#include <ColumnarCore/ObjectRange.h>
#include <ColumnarCore/ContainerId.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <span>
#include <type_traits>

namespace columnar
{
  struct ColumnAccessorOptions final
  {
    /// @brief whether this replaces another column
    ///
    /// For corrections it is quite typical that a column is meant to
    /// replace another column.  In columnar land we will create
    /// genuinely new columns, in xAOD land we will usually overwrite
    /// the content of those columns.  This member is used to indicate
    /// which column gets replaced.
    std::string replacesColumn {};


    /// @brief whether this column is optional
    ///
    /// Essentially this indicates that the column can be skipped, and
    /// the tool will check whether the column is present before trying
    /// to use it.  This allows to adapt the tool somewhat to different
    /// environments.
    ///
    /// The downside here is that overall this is still a bit ambiguous,
    /// i.e. whoever links up the columns needs to decide whether it is
    /// needed.  For columns that are not in the input file that's easy,
    /// there is no choice but omitting them.  However, some columns
    /// only exist as a backup option for other columns, and ideally we
    /// don't want to load the backup columns when the main columns are
    /// missing.  So either that needs to be set during configuration,
    /// or we need to add more meta-information for that case, or the
    /// user needs to do something smart (i.e. manual) in their code.
    bool isOptional = false;


    /// @brief whether to add data dependencies in AthenaMT
    ///
    /// In AthenaMT we need to track data dependencies between
    /// algorithms, but we usually only want to use a subset of the data
    /// dependencies used in columnar code. This flag indicates that
    /// this accessor should be added as a data dependency in AthenaMT.
    bool addMTDependency = false;

    ColumnInfo makeColumnInfo () const
    {
      ColumnInfo info;
      info.replacesColumn = replacesColumn;
      info.isOptional = isOptional;
      return info;
    }
  };



  template<ColumnAccessMode CAM> struct ColumnAccessModeTraits;

  template<> struct ColumnAccessModeTraits<ColumnAccessMode::input>
  {
    template<typename T> using XAODAccessor = SG::ConstAccessor<T>;
    template<typename T> using ColumnType = const T;
  };

  template<> struct ColumnAccessModeTraits<ColumnAccessMode::output>
  {
    template<typename T> using XAODAccessor = SG::Decorator<T>;
    template<typename T> using ColumnType = T;
  };

  template<> struct ColumnAccessModeTraits<ColumnAccessMode::update>
  {
    template<typename T> using XAODAccessor = SG::Accessor<T>;
    template<typename T> using ColumnType = T;
  };



  namespace detail
  {
    /// @brief the backend implementation for @ref AccessorTemplate
    ///
    /// The main difference here is that this class doesn't have a
    /// `ContainerId` template parameter, as that is (normally) only
    /// needed in the constructor. So the needed information can just be
    /// passed in there.
    ///
    /// The @ref AccessorTemplate class will mostly wrap this class and
    /// adds the `ContainerId` template parameter and do all the necessary
    /// static type checking to ensure that each accessor is only used
    /// with the right objects. There are still some more specialized
    /// accessor implementations that integrate more closely with the
    /// `ContainerId`.
    ///
    /// @par CT the column type
    /// @par CAM the column access mode
    /// @par CM the columnar mode

    template<typename CT,ColumnAccessMode CAM,ColumnarMode CM> class ContainerFreeAccessor final
    {
    public:
      static constexpr bool isDefined = false;
    };



    /// @brief a help implementation of @ref AccessorTemplate that
    /// handles type conversions
    ///
    /// The idea is that some column accessors are best defined in terms
    /// of e.g. the underlying column is an `int`, but we need to
    /// represent it as a different type (e.g. an `enum` or an
    /// `ObjectId`) to the user.
    ///
    /// The main reason for having this is separate from
    /// `ContainerFreeAccessor` is that this allows to handle modes with
    /// nested `std::vector` columns correctly. Essentially I need to be
    /// able to define the column as `std::vector<MemoryType>` and then
    /// convert it to something that behaves like `std::span<const CT>>`.

    template<typename CT,ColumnarMode CM> class MemoryAccessor final
    {
    public:
      static constexpr bool isDefined = false;
    };



    template<typename CT,ColumnarMode CM>
      requires ((std::is_integral_v<CT> || std::is_floating_point_v<CT>) && !std::is_same_v<CT,bool>)
    class MemoryAccessor<CT,CM> final
    {
    public:
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = true;
      static constexpr bool hasSetter = false;
      using MemoryType = CT;

      static void updateColumnInfo (ColumnInfo& /*info*/) {}
    };
  }



  /// @brief the raw column accessor template class
  ///
  /// The idea is that various different kinds of accessors can
  /// specialize this template to provide support for the specific types
  /// and modes they support.  The primary way in which users specify
  /// alternate accessors is by wrapping the column type, e.g.
  /// `std::vector<float>` for a column that contains a vector of
  /// floats per object.
  ///
  /// Originally all accessor classes were separate and unique
  /// templates, but I switched to this overloaded approach for several
  /// reasons:
  /// * I actually need a lot fewer specializations than I had template
  ///   classes before.  Often instead of a full accessor class I can
  ///   just define a trait class, or conversely make a specialization
  ///   that covers what before where multiple classes.
  /// * There are some accessor configuration that are supported now and
  ///   would have required a separate template class before, e.g.
  ///   something like `std::vector<RetypeColumn<...>>` wasn't
  ///   supported before, but now it is.
  /// * Overall I find it easier to ensure consistency across
  ///   specializations with this approach, as well as making it easier
  ///   to add some new behavior.
  /// * With a single template class for accessors I can use the same
  ///   set of `typedef`/`using` statements for all accessors, which
  ///   makes it a lot easier to maintain.  Before I had a full set for
  ///   the main accessor, but for the other accessors the list was
  ///   fairly incomplete.
  /// * The way users instantiate "non-standard" accessors seems a
  ///   little more clear with this approach.
  ///
  /// @par CI the container id
  /// @par CT the column type
  /// @par CAM the column access mode
  /// @par CM the columnar mode
  template<ContainerIdConcept CI,typename CT,ColumnAccessMode CAM,typename CM> class AccessorTemplate;


  /// @brief a type wrapper to force @ref AccessorTemplate to treat the
  /// type as native
  ///
  /// This is mostly meant for internal use to allow types in accessors
  /// that we don't want users to use because they are not portable.
  /// The main example being ElementLink which only exists in xAOD mode.
  template<typename CT> struct NativeColumn final {};
  namespace detail
  {
    template<typename CT,ColumnarMode CM>
    class MemoryAccessor<NativeColumn<CT>,CM> final
    {
    public:
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = true;
      static constexpr bool hasSetter = false;
      using MemoryType = CT;

      static void updateColumnInfo (ColumnInfo& /*info*/) {}
    };
  }


  /// @brief a type wrapper to make @ref AccessorTemplate convert the
  /// underlying column type to a different type
  ///
  /// This is mostly meant for handling enums correctly, but there are a
  /// couple of other situations in which this can be helpful.
  template<typename UT,typename CT> struct RetypeColumn final
  {
    static_assert (!std::is_const_v<CT>, "CT must not be const");
    static_assert (!std::is_const_v<UT>, "UT must not be const");
  };

  namespace detail
  {
    template<typename UT,typename CT,ColumnarMode CM>
    class MemoryAccessor<RetypeColumn<UT,CT>,CM> final
    {
    public:
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = CT;

      static void updateColumnInfo (ColumnInfo& /*info*/) {}
      static auto makeViewer (auto&&)
      {
        return [] (const auto& value) {return static_cast<UT>(value);};
      }
    };
  }




  /// @brief reset a column accessor to point to a new column
  ///
  /// This allows users to have blank column accessors that only get
  /// initialized if they are actually used.  This avoids the need to
  /// have accessors wrapped inside `std::optional` or similar
  /// constructs.  Besides making accessor use slightly more consistent
  /// it should also make the code a little more efficient.

  template<ContainerIdConcept CI,typename CT,ColumnAccessMode CAM,typename CM>
  void resetAccessor (AccessorTemplate<CI,CT,CAM,CM>& accessor, ColumnarTool<CM>& columnBase, const std::string& name, ColumnAccessorOptions&& options = {})
  {
    accessor = AccessorTemplate<CI,CT,CAM,CM> (columnBase, name, std::move (options));
  }





  template<ContainerIdConcept CI,typename CT,typename CM=ColumnarModeDefault> using ColumnAccessor = AccessorTemplate<CI,CT,ColumnAccessMode::input,CM>;
  template<ContainerIdConcept CI,typename CT,typename CM=ColumnarModeDefault> using ColumnDecorator = AccessorTemplate<CI,CT,ColumnAccessMode::output,CM>;
  template<ContainerIdConcept CI,typename CT,typename CM=ColumnarModeDefault> using ColumnUpdater = AccessorTemplate<CI,CT,ColumnAccessMode::update,CM>;
}

#include "ColumnAccessorXAOD.icc"
#include "ColumnAccessorArray.icc"

#endif
