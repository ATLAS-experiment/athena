/**
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
**/
#ifndef CaloRecGPU_CaloRecUtilities_h
#define CaloRecGPU_CaloRecUtilities_h 

#include <cstring> //std::memcpy
#include <utility> //std::forward

namespace CaloRecGPU{

  ///@class multi_class_holder
  ///A convenient way to handle a compile-time list of types,
  ///useful for several metaprogramming techniques...
  template <class ... Args>
  struct multi_class_holder{
    static constexpr size_t size(){
      return sizeof...(Args);
    }
  };

// Apply: f(Types{}, index, args...)
template<class F, class... Types, class... Args>
void apply_to_multi_class(F&& f, multi_class_holder<Types...>, Args&&... args){
  auto&& func = std::forward<F>(f);  // avoid forwarding F multiple times
  [&]<std::size_t... I>(std::index_sequence<I...>) {
    (func(Types{}, I, std::forward<Args>(args)...), ...);
  }(std::make_index_sequence<sizeof...(Types)>{});
}

  inline float float_unhack(const unsigned int bits){
    float res;
    std::memcpy(&res, &bits, sizeof(float));
    //In C++20, we should bit-cast. For now, for our platform, works.
    return res;
  }

  inline double protect_from_zero(const double x){
    return x == 0 ? 1e-15 : x;
  }

  inline float protect_from_zero(const float x){
    return x == 0 ? 1e-7 : x;
  }
}
#endif
