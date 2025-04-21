
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_BINSTRTOHEXSTR_H
#define GLOBALSIM_BINSTRTOHEXSTR_H

#include <algorithm>
#include <string>

namespace GlobalSim {


  char hex_char(std::string::size_type begin,
		std::string::size_type end,
		const std::string& s) {
    auto place_val{1U};
    for (auto i{begin+1}; i != end; ++i) {
      place_val *= 2;
    }

    auto val{0U};
    for (std::size_t i{begin}; i != end; ++i) {
      if (s[i] ==  '1') {val += place_val;}
      place_val /= 2;
    }
    
    if (val < 10) {

      return char('0' + val);
    }

    return char('a' + val-10);
  }
  
  std::string binStrToHexStr(std::string s) {

    if (s.size() == 0) {return "";}


    bool header{false};

    if ((s.starts_with("0b") or s.starts_with("0B")) and  s.size() >=2){
      if (s.size() == 2) {return "";};
      header = true;
   }

    const std::string::size_type offset = header ? 2:0;
    const auto sz = s.size()-offset;


    auto is_bin_char = [](const auto& c) {return c == '0' or c == '1';};

    if (!std::all_of(std::cbegin(s)+offset, std::cend(s), is_bin_char)) {
      throw std::out_of_range("ss contains non-binary char");
    }
      
    const auto n_ragged = sz - 4*(sz/4);

    std::string result;
    result.reserve (n_ragged  ? 1+(sz)/4 : (sz)/4);

    
    std::string::size_type start{offset};
    std::string::size_type stop{offset+n_ragged};

    if (n_ragged) {result.push_back(hex_char(start, stop, s));}

    start = offset+n_ragged;
    stop = start + 4;

    if (stop > s.size()) {return result;}

    
    for (auto i = start; i != s.size(); i += 4) {
      result.push_back(hex_char(i, i+4, s));

      start = stop;
      stop += 4;
    }

    return result;

  }
}  
#endif
