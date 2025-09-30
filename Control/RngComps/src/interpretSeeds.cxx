/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <charconv>
#include <iostream>
#include <boost/tokenizer.hpp>

#include "interpretSeeds.h"

//get the luxury level, if specified. In that case moves token past it
inline
void getLuxury( boost::tokenizer<boost::char_separator<char> >::iterator& token, short& luxLevel) {
  if ((*token) == "LUXURY") {
     ++token;
     uint32_t parsedValue = 0;
     auto result = std::from_chars(token->data(), token->data() + token->size(), parsedValue);
     if (result.ec != std::errc()) {
       std::cerr << "Parsing error in function getLuxury." << std::endl;
     }
     luxLevel = parsedValue;
     ++token;
  }
}

//get the luxury level, if specified. In that case moves token past it
inline
void getOffset( boost::tokenizer<boost::char_separator<char> >::iterator& token, uint32_t& offset) {
  if ((*token) == "OFFSET") {
     ++token;
     uint32_t parsedValue = 0;
     auto result = std::from_chars(token->data(), token->data() + token->size(), parsedValue);
     if (result.ec != std::errc()) {
       std::cerr << "Parsing error in function getOffset." << std::endl;
     }
     offset = parsedValue;
     ++token;
  }
}

bool interpretSeeds(const std::string& buffer, 
		    std::string& stream, uint32_t& seed1, uint32_t& seed2, short& luxury, uint32_t& offset)
{
  //split the space-separated string in 3 or 5 or 7 words:	
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(" ");
  tokenizer tokens(buffer, sep);
  int nToks(distance(tokens.begin(), tokens.end()));
  bool status = (nToks == 3 || nToks == 5 || nToks == 7);
  if (status) {
    tokenizer::iterator token(tokens.begin());
    stream = *token++;
    //FIXME, try permutations by hand. With more than two we'd need a parser
    getOffset(token, offset);
    getLuxury(token, luxury);
    getOffset(token, offset);
    auto [ptr1, ec1] = std::from_chars(token->data(), token->data() + token->size(), seed1);
    ++token;
    auto [ptr2, ec2] = std::from_chars(token->data(), token->data() + token->size(), seed2);
    ++token;
    if (ec1 != std::errc() || ec2 != std::errc()) {
      status = false;
    }
  }
  return status;
}

bool interpretSeeds(const std::string& buffer, 
		    std::string& stream, std::vector<uint32_t>& seeds) 
{
  //split the space-separated string in 31 or 33 words:	
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(" ");
  tokenizer tokens(buffer, sep);
  int nToks(distance(tokens.begin(), tokens.end()));
  bool status = (nToks == 31 || nToks == 33 || nToks == 771);
  if (status) {
    tokenizer::iterator token(tokens.begin());
    stream = *token++;
    --nToks;
    if (nToks == 32) nToks=30; //ranlux (FIXME NEEDED?)
    for (int i=0; i<nToks; i++) {
      uint32_t value = 0;
      auto [ptr, ec] = std::from_chars(token->data(), token->data() + token->size(), value);
      if (ec != std::errc()) {
        status = false;
        break;
      }
      seeds.push_back(value);
      ++token;
    }
  }  
  return status;
}
