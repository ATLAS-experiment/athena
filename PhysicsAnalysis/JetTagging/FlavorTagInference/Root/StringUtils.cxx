#include "FlavorTagInference/StringUtils.h"


namespace FlavorTagInference {
namespace str {

  // remap a variable name while keeping track of which remaps were used
  const std::string remapName(const std::string& name,
                              std::map<std::string, std::string>& remap,
                              std::set<std::string>& usedRemap) {
    if (auto h = remap.extract(name)) {
      usedRemap.insert(h.key());
      return h.mapped();
    }
    return name;
}

  // regex substitution for first match in a variable name
  std::string sub_first(const StringRegexes& res,
                        const std::string& var_name,
                        const std::string& context) {
    for (const auto& pair: res) {
      const std::regex& re = pair.first;
      const std::string& fmt = pair.second;
      if (std::regex_match(var_name, re)) {
        return std::regex_replace(var_name, re, fmt);
      }
    }
    throw std::logic_error(
      "no regex match found for variable '" + var_name + "' while " + context);
  }
}
}
