/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PRIMATIVE_HELPERS_H
#define PRIMATIVE_HELPERS_H

#include "JetH5Writer/Primitive.h"

#include "HDF5Utils/Writer.h"
#include "AthContainers/AuxElement.h"

#include <stdexcept>
#include <cmath>

// helper functions
namespace detail {

  bool isCustom(const Primitive&);
  bool isCustom(const Primitive::Type&);

  // build accessor
  template <typename T, typename I, typename A, typename S=T>
  std::function<T(I)> get(std::string source, A ass, T def) {
    SG::AuxElement::ConstAccessor<S> acc(source);
    using rettype = decltype(ass(std::declval<I>()));
    if constexpr (std::is_pointer<rettype>::value) {
      return [acc, ass, def](I in) -> T {
        const auto* associated = ass(in);
        if (!associated) return def;
        return acc(*associated);
      };
    } else {
      return [acc, ass](I in) -> T { return acc(ass(in)); };
    }
  }

  template <typename T>
  auto defaultAccessor = [](typename T::input_type in){ return in; };
  template <typename T>
  using defaultAccessor_t = decltype(defaultAccessor<T>);

  using uchar = unsigned char;
  using uint = unsigned int;
  using ushort = unsigned short;
  using ull = unsigned long long;
  using ul = unsigned long;
  using ll = long long;

  template <typename T, typename A=defaultAccessor_t<T>>
  void addInput(T& c, const Primitive& input, A a=defaultAccessor<T>) {
    using I = typename T::input_type;
    using Tp = Primitive::Type;
    std::string s = input.source;
    std::string t = input.target.empty() ? input.source : input.target;
    const auto h = H5Utils::Compression::HALF_PRECISION;
    const char cz = char(0);
    const char cm = char(-1);
    const char sm = short(-1);
    switch (input.type) {
      case Tp::PRECISION_CUSTOM: // intentional fall-through
      case Tp::CUSTOM: throw std::logic_error("custom type unsupported");
      case Tp::UCHAR: c.add(t, get<uchar,I,A>(s,a,cz), cz); return;
      case Tp::CHAR: c.add(t, get<char,I,A>(s,a,cm), cm); return;
      case Tp::USHORT: c.add(t, get<ushort,I,A>(s,a,cz), cz); return;
      case Tp::SHORT: c.add(t, get<short,I,A>(s,a,sm), sm); return;
      case Tp::UINT: c.add(t, get<uint,I,A>(s,a,0), 0); return;
      case Tp::INT: c.add(t, get<int,I,A>(s,a,-1), -1); return;
      case Tp::ULONG: c.add(t, get<ul,I,A>(s,a,0), 0); return;
      case Tp::LONG: c.add(t, get<long,I,A>(s,a,-1), -1); return;
      case Tp::ULL: c.add(t, get<ull,I,A>(s,a,0), 0); return;
      case Tp::LL: c.add(t, get<ll,I,A>(s,a,-1), -1); return;
      case Tp::HALF: c.add(t, get<float,I,A>(s,a,NAN), NAN, h); return;
      case Tp::FLOAT: c.add(t, get<float,I,A>(s,a,NAN), NAN); return;
      case Tp::DOUBLE: c.add(t, get<double,I,A>(s,a,NAN), NAN); return;
      case Tp::UINT2UCHAR: c.add(t, get<uchar,I,A,uint>(s,a,0), 0); return;
      case Tp::INT2CHAR: c.add(t, get<char,I,A,int>(s,a,cm), cm); return;
      case Tp::UINT2USHORT: c.add(t, get<ushort,I,A,uint>(s,a,0), 0); return;
      case Tp::INT2SHORT: c.add(t, get<short,I,A,int>(s,a,sm), sm); return;
      case Tp::UL2ULL: c.add(t, get<ull,I,A,ul>(s,a,0),0); return;
    }
  }
}

#endif
