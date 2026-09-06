#include <CxxUtils/StringUtils.h>

#include "CalibrationDataInterface/CalibrationDataInternals.h"

namespace Analysis {
  namespace CalibrationDataInterface {

    // local utility function: split string into a vector of substrings separated by a specified separator
    std::vector<std::string> split(const std::string& str, const char token) {
      std::vector<std::string> result = CxxUtils::tokenize(str, token);
      for (std::string& element : result) {
        element = CxxUtils::trimWhiteSpaces(element);
      }
      return result;
    }

  }
}
