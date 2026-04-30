/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEFINITIONS_TYPEINFORMATION_H
#define TRIGSTORAGEDEFINITIONS_TYPEINFORMATION_H

#include <stddef.h>
#include <tuple>
#include <type_traits>


namespace HLT{
namespace TypeInformation {

/// Placeholder for types without aux store
struct no_aux{};

/// EDM "type"
template<typename Object, typename Features, typename Container, typename Aux = no_aux>
struct TypeInfo {
  using object = Object;
  using list_of_features = Features;
  using container = Container;
  using aux = Aux;
};


/// True if element's container type matches T
template <typename T, typename Element>
struct MatchContainer : std::is_same<T, typename Element::container> {};

/// True if element's object type matches T
template <typename T, typename Element>
struct MatchObject : std::is_same<T, typename Element::object> {};

/// True if element's features list contains T or the object matches T
template <typename T, typename Element>
struct MatchFeatures {
  static constexpr bool value = Element::list_of_features::template has<T> || MatchObject<T, Element>::value;
};


/// List of EDM types
template <typename... Elements>
struct List {
  /// Size of list
  static constexpr size_t size = sizeof...(Elements);

  /// Our own type
  using type = List<Elements...>;

  /// Get element at index
  template <size_t I>
  using at = std::tuple_element_t<I, std::tuple<Elements...>>;

  /// Check if element exists (with optional Predicate)
  template <typename T, template <typename, typename> class Predicate = std::is_same>
  static constexpr bool has = (Predicate<T, Elements>::value || ...);

  /// Return index of element matching Predicate. Return size if not found.
  template <typename T, template <typename, typename> class Predicate = std::is_same>
  static consteval size_t indexOf() {
    size_t index = 0;
    // Fold expression with comma operator to find index
    (void)((Predicate<T, Elements>::value || (index++, false)) || ...);
    return index;
  }

  /// Find element matching T using Predicate
  template <typename T, template <typename, typename> class Predicate = std::is_same>
  using find = at<indexOf<T, Predicate>()>;

  /// Add element
  template <typename T>
  using add = List<Elements..., T>;

  /// Helper for join
  template <typename OtherList>
  struct join_helper;

  template <typename... Others>
  struct join_helper<List<Others...>> {
    using type = List<Elements..., Others...>;
  };

  /// Join another list
  template <typename T>
  using join = typename join_helper<T>::type;

  /// Execute functor for each element
  template <typename Functor>
  static void for_each(Functor&& functor) {
    (functor.template operator()<Elements>(), ...);
  }

  /// Helper for merge
  template <typename T>
  struct merge_helper {

    // Update Current entry. If the container matches, merge the features.
    template <typename Current>
    using update = std::conditional_t<
      MatchContainer<typename T::container, Current>::value,
      // Merge features if container matches
      TypeInfo<typename Current::object,
               typename Current::list_of_features::template join<typename T::list_of_features>,
               typename Current::container,
               typename Current::aux>,
      // Otherwise return unchanged
      Current >;

    // If container entry exists, update it
    static constexpr bool exists = has<typename T::container, MatchContainer>;
    using type = std::conditional_t<exists, List<update<Elements>...>, add<T>>;
  };

  /// Add T and merge feature list if container type already exists
  template <typename T>
  using merge = typename merge_helper<T>::type;
};


}//end_of_namespace TypeInformation
}//end_of_namespace HLT

#endif
