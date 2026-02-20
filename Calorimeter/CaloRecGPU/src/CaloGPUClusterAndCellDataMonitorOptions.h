//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOGPUCLUSTERANDCELLDATAMONITOROPTIONS_H
#define CALORECGPU_CALOGPUCLUSTERANDCELLDATAMONITOROPTIONS_H

//This sets up some classes to be usable as options within Gaudi (and thus Athena),
//so that we can more ergonomically specify our plotter's options.
//
//For reference, see Gaudi/GaudiExamples/src/Properties/CustomPropertiesAlg.cpp


#include <tuple>
#include <iostream>
#include <string>
#include <type_traits>



#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/StatusCode.h"

#include <Gaudi/Parsers/Factory.h>

//Suppress the warning that comes up from simply including Factory.h.

//First, generic handling of tuples.
//This does set us up for handling more than we strictly need,
//but since this is only a local include
//and shouldn't bleed over to other parts of the code,
//we'll behave and (hopefully) not do anything too crazy.


//Since we're at C++17 level,
//we'll use SFINAE and enable_if
//to provide the *actual* parse and grammar and output
//that are necessary for custom types for Gaudi options.
//In C++20, of course we could use concepts
//for a more elegant implementation...

namespace impl
{
  template <class Check> struct class_can_be_tuple
  {
    template<class T> inline static constexpr
    auto tuple_type_checker(T *) -> decltype(typename T::TupleType {});
    template<class T> inline static constexpr
    void tuple_type_checker(...);
    template <class T> using tuple_type_checker_type = decltype( tuple_type_checker<T>(nullptr) );
    template <class T> inline static constexpr
    bool tuple_type_exists = !std::is_same_v<tuple_type_checker_type<T>, void>;

    template<class T> inline static constexpr
    auto to_tuple_checker(T *) -> decltype(&T::to_tuple);
    template<class T> inline static constexpr
    void to_tuple_checker(...);
    template <class T> inline static constexpr
    bool to_tuple_exists = std::is_pointer_v<decltype( to_tuple_checker<T>(nullptr) )>;

    template<class T> inline static constexpr
    auto from_tuple_checker(T *) -> decltype(&T::from_tuple);
    template<class T> inline static constexpr
    void from_tuple_checker(...);
    template <class T> inline static constexpr
    bool from_tuple_exists = std::is_pointer_v<decltype( from_tuple_checker<T>(nullptr) )>;

    template<class T> inline static constexpr
    auto tuple_size_checker(T *) -> decltype(T::TupleSize);
    template<class T> inline static constexpr
    void tuple_size_checker(...);
    template <class T> inline static constexpr
    bool tuple_size_exists = !std::is_void_v<decltype(tuple_size_checker<T>(nullptr))>;

    inline constexpr static bool value = tuple_type_exists<Check> && to_tuple_exists<Check> && from_tuple_exists<Check> && tuple_size_exists<Check>;
  };

  template <class T>
  inline static constexpr bool class_can_be_tuple_v = class_can_be_tuple<T>::value;

  template <class T>
  void tuple_safe_copy(T & dest, const T & source)
  {
    dest = source;
  }
}

//We use our macros to write a relatively limited form of variadic structured bindings
//for the tuple <-> struct conversions.

#include "MacroHelpers.h"

#define CALORECGPU_VARSB_APPENDER(THIS, PREFIX, IGNORE) , CRGPU_CONCAT(PREFIX, THIS)
#define CALORECGPU_VARSB_ASSIGNER(THIS, IGNORE1, IGNORE2) tuple_safe_copy(CRGPU_CONCAT(b, THIS), CRGPU_CONCAT(a, THIS));

#define CALORECGPU_VARSB_ENCAPSULATOR_A(...) CRGPU_MACRO_EXPANSION(CALORECGPU_VARSB_APPENDER, a, __VA_ARGS__ )
#define CALORECGPU_VARSB_ENCAPSULATOR_B(...) CRGPU_MACRO_EXPANSION(CALORECGPU_VARSB_APPENDER, b, __VA_ARGS__ )

#define CALORECGPU_VARSB_ENCAPSULATOR_ASSIGN(...) CRGPU_MACRO_EXPANSION(CALORECGPU_VARSB_ASSIGNER, _ , __VA_ARGS__,  )


#define CALORECGPU_VARSB_EXPANDER(THIS_NUM, IGNORE, SMALLER_NUMS)                                  \
  template <> struct struct_tuple_conversion<THIS_NUM>                                             \
  {                                                                                                \
    template <class T, class TupleT>                                                               \
    static void s2t(const T & t, TupleT & tup)                                                     \
    {                                                                                              \
      const auto & [CRGPU_CONCAT(a, THIS_NUM) CALORECGPU_VARSB_ENCAPSULATOR_A SMALLER_NUMS] = t;   \
      auto & [CRGPU_CONCAT(b, THIS_NUM) CALORECGPU_VARSB_ENCAPSULATOR_B SMALLER_NUMS] = tup;       \
      tuple_safe_copy(CRGPU_CONCAT(b, THIS_NUM), CRGPU_CONCAT(a, THIS_NUM));                       \
      CALORECGPU_VARSB_ENCAPSULATOR_ASSIGN SMALLER_NUMS                                            \
    }                                                                                              \
    template <class TupleT, class T>                                                               \
    static void t2s(const TupleT & tup, T & t)                                                     \
    {                                                                                              \
      const auto & [CRGPU_CONCAT(a, THIS_NUM) CALORECGPU_VARSB_ENCAPSULATOR_A SMALLER_NUMS] = tup; \
      auto & [CRGPU_CONCAT(b, THIS_NUM) CALORECGPU_VARSB_ENCAPSULATOR_B SMALLER_NUMS] = t;         \
      tuple_safe_copy(CRGPU_CONCAT(b, THIS_NUM), CRGPU_CONCAT(a, THIS_NUM));                       \
      CALORECGPU_VARSB_ENCAPSULATOR_ASSIGN SMALLER_NUMS                                            \
    }                                                                                              \
  };                                                                                               \

//If only there were truly variadic structured bindings...

namespace impl
{

  template <size_t n> struct struct_tuple_conversion
  {
    template <class T, class TupleT>
    static void s2t(const T &, TupleT &) { }
    template <class T, class TupleT>
    static T t2s(const TupleT &, T &) { }
  };

  CRGPU_RECURSIVE_MACRO(CRGPU_MACRO_EXPANSION(CALORECGPU_VARSB_EXPANDER, _, 9, 8, 7, 6, 5, 4, 3, 2, 1))
  //If we needed structs with more than 9 elements, just add more numbers to the left in descending order...

  template <class T,
            std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  void tuple_safe_copy(T & s, const typename T::TupleType & tuple)
  {
    struct_tuple_conversion<T::TupleSize>::t2s(tuple, s);
  }

  template <class T,
            std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  void tuple_safe_copy(typename T::TupleType & tuple, const T & s)
  {
    struct_tuple_conversion<T::TupleSize>::s2t(s, tuple);
  }

  template <class T,
            std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  void tuple_safe_copy(std::vector<typename T::TupleType> & t_v, const std::vector<T> & s_v)
  {
    t_v.resize(s_v.size());
    for (size_t i = 0; i < s_v.size(); ++i)
      {
        tuple_safe_copy(t_v[i], s_v[i]);
      }
  }

  template <class T,
            std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  void tuple_safe_copy(std::vector<T> & s_v, const std::vector<typename T::TupleType> & t_v)
  {
    s_v.resize(t_v.size());
    for (size_t i = 0; i < t_v.size(); ++i)
      {
        tuple_safe_copy(s_v[i], t_v[i]);
      }
  }


  template <class T>
  auto to_tuple_type_helper(const T &)
  {
    return T{};
  }
  /*
  template <class T,
            std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  auto to_tuple_type_helper(const T&)
  {
    return typename T::TupleType{};
  }
  */
  template <class T,
            std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  auto to_tuple_type_helper(const std::vector<T> &)
  {
    return std::vector<typename T::TupleType> {};
  }

  /*
  template <class ... Elems>
  auto to_tuple_type_helper(const std::tuple<Elems...> &)
  {
    return std::tuple<decltype(to_tuple_type_helper(std::declval<Elems>))...>{};
  }
  */
  //More specializations if needed?


  template <class T>
  using to_tuple_type = decltype(to_tuple_type_helper(std::declval<T>()));

  template <class T, class ... Elems> struct simple_tuple_conversion
  {
    using TupleType = std::tuple<to_tuple_type<Elems>...>;

    inline static constexpr size_t TupleSize = sizeof...(Elems);

    static TupleType to_tuple (const T & s)
    {
      TupleType ret;
      struct_tuple_conversion<TupleSize>::s2t(s, ret);
      return ret;
    }
    static T from_tuple(const TupleType & tup)
    {
      T ret;
      struct_tuple_conversion<TupleSize>::t2s(tup, ret);
      return ret;
    }
    /*
    friend
    std::ostream & operator<< (std::ostream & s, const T & t)
    {
      return s << to_tuple(t);
    }
    */
  };

  //If we had reflection, this would be so much easier...

  template <class Stream, class T>
  void output_helper(Stream & s, const T & t, const std::string & after)
  {
    Gaudi::Utils::toStream(t, s);
    s << after;
  }

  template <class Stream, class T>
  void output_helper(Stream & s, const std::vector<T> & v, const std::string & after)
  {
    s << "[";
    for (size_t i = 0; i < v.size(); ++i)
      {
        if (i > 0)
          {
            s << ", ";
          }
        impl::output_helper(s, v[i], "");
      }
    s << "]" << after;
  }
  /*
  template <class Stream, class T,
             std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
  void output_helper(Stream & s, const T & t, const std::string & after)
  {
    s << t << after;
  }
  */
}

namespace Gaudi
{
  namespace Parsers
  {

    template <typename Iterator, typename Skipper, class T>
    struct Grammar_< Iterator, T, Skipper, typename std::enable_if_t < impl::class_can_be_tuple_v<T> > >
    {
      using Grammar = typename Grammar_<Iterator, typename T::TupleType, Skipper>::Grammar;
    };

    template <class ... Tup>
    StatusCode parse(std::tuple<Tup...> & tup, const Gaudi::Parsers::InputData & input)
    {
      return parse_(tup, input);
    }

    template <class T>
    StatusCode parse(std::vector<T> & v, const Gaudi::Parsers::InputData & input)
    {
      return parse_(v, input);
    }

  }
}

template <class ... Tup>
std::ostream & operator<<(std::ostream & s, const std::tuple<Tup...> & tup )
{
  std::apply( [&] (const Tup & ... arg)
  {
    s << "(";

    size_t num = 0;

    (void) ( (void) impl::output_helper(s, arg, (++num < sizeof...(Tup) ? ", " : "")), ... );

    s << ")";

  }, tup );

  return s;
}

template < class T,
           std::enable_if_t< impl::class_can_be_tuple_v<T>>* = nullptr>
std::ostream & operator<<(std::ostream & s, const T & t )
{
  return s << T::to_tuple(t);
}

//And now for the classes proper.
//
//In general:
//
//struct ClassName: impl::simple_tuple_conversion<ClassName, T1, T2, ... , TN>
//{
//  T1 name1;
//  T2 name2;
//  ...
//  TN nameN;
//
//  ClassName() = default;
//  ClassName(const TupleType & t)
//  {
//    (*this) = from_tuple(t);
//  }
//};
//
//One could use the helper macro below.
//
//If there are mismatches in the types on the simple_tuple_conversion list,
//ugly compilation issues may ensue!

#define CALORECGPU_OPTIONCLASS_CONSTRUCTORS(CLASSNAME)  \
  CLASSNAME() = default;                                \
  CLASSNAME(const TupleType & t)                        \
  {                                                     \
    (*this) = from_tuple(t);                            \
  }                                                     \

struct MatchingOptions:
  impl::simple_tuple_conversion<MatchingOptions, double, double, double, double>
{
  double min_similarity = 0.50, term_w = 250., grow_w = 500., seed_w = 1000.;

  CALORECGPU_OPTIONCLASS_CONSTRUCTORS(MatchingOptions)
};

struct SimpleSingleTool:
  impl::simple_tuple_conversion<SimpleSingleTool, std::string, std::string>
{
  std::string tool, plot_id;
  
  CALORECGPU_OPTIONCLASS_CONSTRUCTORS(SimpleSingleTool)
};

struct SimpleToolPair:
  impl::simple_tuple_conversion<SimpleToolPair, std::string, std::string, std::string, bool, bool, bool>
{
  std::string tool_ref, tool_test, plot_id;
  bool match_in_energy = false;
  bool match_without_shared = false;
  bool match_perfectly = false;
  
  CALORECGPU_OPTIONCLASS_CONSTRUCTORS(SimpleToolPair)
};

//This was used more heavily when we had the standalone plotter,
//but I would like to keep it here in case more changes are necessary...

#endif //CALORECGPU_CALOGPUCLUSTERANDCELLDATAMONITOROPTIONS_H
