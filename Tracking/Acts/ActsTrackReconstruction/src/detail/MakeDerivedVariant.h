/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MAKEDERIVEDVARIANT_H
#define MAKEDERIVEDVARIANT_H

#include <variant>

namespace ActsTrk::detail::MakeDerivedVariant {
  
  template <typename T>
  static T lvalue(T &&a) { return a;}
  
  // dummy method returning a variant which allows for one additional type than the given variant
  template <class T, typename...Args>
  constexpr std::variant<Args..., T> extend(const std::variant<Args...> &, const T &) {
    return std::variant<Args..., T>();
  }
  
  // helper structor to hold the declaration of the variant
  template <typename TypeHelper,
	    typename VariantType,
	    std::size_t N = std::variant_size_v<VariantType> >
  struct MakeVariant {
    using variant_type = decltype( extend( MakeVariant<TypeHelper, VariantType, N-1>::m_val,
					   typename TypeHelper::template type<decltype( lvalue( std::get<N-1>(VariantType{})))> {}  ) );
    variant_type m_val;
  };
  
  // specialisation of above helper structure for N=1 i.e. the variant just allowing T<1>
  template<typename TypeHelper,
	   typename VariantType>
  struct MakeVariant<TypeHelper, VariantType, 1> {
    using variant_type = std::variant< typename TypeHelper::template type< decltype( lvalue( std::get<0>(VariantType{}))) > > ;
    variant_type m_val;
  };
  
}

#endif
