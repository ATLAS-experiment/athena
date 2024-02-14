/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <regex>
#include <vector>
#include <map>
#include <set>

namespace FlavorTagDiscriminants {
namespace str{
  typedef std::vector<std::pair<std::regex, std::string> > StringRegexes;

  const std::string remapName(const std::string& name,
                              std::map<std::string, std::string>& remap,
                              std::set<std::string>& usedRemap);

  std::string sub_first(const StringRegexes& res,
                        const std::string& var_name,
                        const std::string& context);

  template <typename T>
  T match_first(const std::vector<std::pair<std::regex, T> >& regexes,
                const std::string& var_name,
                const std::string& context) {
    for (const auto& pair: regexes) {
      if (std::regex_match(var_name, pair.first)) {
        return pair.second;
      }
    }
    throw std::logic_error(
      "no regex match found for input variable '" + var_name + "' in "
      + context);
  }

} // end namespace str
} // end namespace FlavorTagDiscriminants
#endif // STRINGUTILS_H