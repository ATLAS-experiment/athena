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


  /// @brief a trait class to provide information about the column type
  ///
  /// This sort of steers the behavior of the accessors and the various
  /// template specializations.  It provides both information the
  /// various specializatons can use in their `requires` clauses, as
  /// well as helpers for the accessors to use in their implementation.
  ///
  /// @warn Users are neither meant to use this class nor to specialize
  /// it.  This is an internal class that is used for defining the
  /// accessors and may change without notice.
  template<typename CT,typename CM>
  struct ColumnTypeTraits final
  {
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = false;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnBase*/, ColumnInfo& info) {return info;}
  };

  template<typename CT,typename CM>
    requires ((std::is_integral_v<CT> || std::is_floating_point_v<CT>) && !std::is_same_v<CT,bool>)
  struct ColumnTypeTraits<CT,CM> final
  {
    using ColumnType = CT;
    static constexpr bool isNativeType = true;
    static constexpr bool useConvertInput = false;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnBase*/, ColumnInfo& info) {return info;}
  };

  /// @brief a type wrapper to force @ref AccessorTemplate to treat the
  /// type as native
  ///
  /// This is mostly meant for internal use to allow types in accessors
  /// that we don't want users to use because they are not portable.
  /// The main example being ElementLink which only exists in xAOD mode.
  template<typename CT> struct NativeColumn final {};
  template<typename CT,typename CM>
  struct ColumnTypeTraits<NativeColumn<CT>,CM> final
  {
    using ColumnType = CT;
    static constexpr bool isNativeType = true;
    static constexpr bool useConvertInput = false;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnBase*/, ColumnInfo& info) {return info;}
  };


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

  template<typename UT,typename CT,typename CM>
  struct ColumnTypeTraits<RetypeColumn<UT,CT>,CM> final
  {
    using ColumnType = CT;
    using UserType = UT;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnBase*/, ColumnInfo& info) {return info;}
    static UT convertInput (const CT& value) {return UT (value);}
  };



  // the accessor specialization for type conversions
  template<ContainerIdConcept CI,typename CT,typename CM>
    requires (ColumnTypeTraits<CT,CM>::useConvertInput || ColumnTypeTraits<CT,CM>::useConvertWithDataInput)
  class AccessorTemplate<CI,CT,ColumnAccessMode::input,CM> final
  {
  public:

    using ColumnType = typename ColumnTypeTraits<CT,CM>::ColumnType;
    using UserType = typename ColumnTypeTraits<CT,CM>::UserType;

    AccessorTemplate () = default;

    AccessorTemplate (ColumnarTool<CM>& columnBase, const std::string& name, ColumnInfo&& info = {})
      : m_base (columnBase, name, std::move (ColumnTypeTraits<CT,CM>::updateColumnInfo(columnBase, info)))
    {}

    [[nodiscard]] decltype(auto) operator () (ObjectId<CI,CM> id) const noexcept
    {
      if constexpr (ColumnTypeTraits<CT,CM>::useConvertWithDataInput)
        return ColumnTypeTraits<CT,CM>::convertInput (id.getData(), m_base(id));
      else
        return ColumnTypeTraits<CT,CM>::convertInput (m_base(id));
    }

    [[nodiscard]] bool isAvailable (ObjectId<CI,CM> id) const noexcept
    {
      return m_base.isAvailable (id);
    }

    [[nodiscard]] std::optional<UserType> getOptional (ObjectId<CI,CM> id) const
    {
      if (m_base.isAvailable (id))
        return operator()(id);
      else
        return std::nullopt;
    }

  private:
    AccessorTemplate<CI,ColumnType,ColumnAccessMode::input,CM> m_base;
  };




  /// @brief reset a column accessor to point to a new column
  ///
  /// This allows users to have blank column accessors that only get
  /// initialized if they are actually used.  This avoids the need to
  /// have accessors wrapped inside `std::optional` or similar
  /// constructs.  Besides making accessor use slightly more consistent
  /// it should also make the code a little more efficient.

  template<ContainerIdConcept CI,typename CT,ColumnAccessMode CAM,typename CM>
  void resetAccessor (AccessorTemplate<CI,CT,CAM,CM>& accessor, ColumnarTool<CM>& columnBase, const std::string& name, ColumnInfo&& info = {})
  {
    accessor = AccessorTemplate<CI,CT,CAM,CM> (columnBase, name, std::move (info));
  }





  template<ContainerIdConcept CI,typename CT,typename CM=ColumnarModeDefault> using ColumnAccessor = AccessorTemplate<CI,CT,ColumnAccessMode::input,CM>;
  template<ContainerIdConcept CI,typename CT,typename CM=ColumnarModeDefault> using ColumnDecorator = AccessorTemplate<CI,CT,ColumnAccessMode::output,CM>;
  template<ContainerIdConcept CI,typename CT,typename CM=ColumnarModeDefault> using ColumnUpdater = AccessorTemplate<CI,CT,ColumnAccessMode::update,CM>;
}

#include "ColumnAccessorXAOD.icc"
#include "ColumnAccessorArray.icc"

#endif
