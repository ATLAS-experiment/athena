#include <stdexcept>

#include "ZdcUtils/RPDUtils.h"

namespace RPDUtils {
  unsigned int ZDCSideToSideIndex(int const ZDCSide) {
    switch (ZDCSide) {
      case -1:
        return sideC;
      case 1:
        return sideA;
      default:
        throw std::runtime_error("Invalid ZDC side: " + std::to_string(ZDCSide));
    }
    return 0;
  };
} // namespace RPDUtils
