#include "PrimitiveHelpers.h"

#include <unordered_set>


namespace detail {
  bool isCustom(const Primitive& p) {
    return isCustom(p.type);
  }
  bool isCustom(const Primitive::Type& t) {
    using Tp = Primitive::Type;
    return std::unordered_set {
      Tp::CUSTOM, Tp::PRECISION_CUSTOM
    }.count(t);
  }
}
