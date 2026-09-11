/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>

#include "TrigConfHLTUtils/HLTUtils.h"

using namespace TrigConf;


HashStore::~HashStore() {
}

HashMap::~HashMap() {
}


HLTHash HLTUtils::string2hash( const std::string& s, const std::string& category )
{
  // Try to find existing hash in category
  const auto icat = s_hashStore.hashCat.find(category);
  if (icat != s_hashStore.hashCat.end()) { // category found
    const HashMap& cat = icat->second;
    const auto ihash = cat.name2hash.find(s);
    if (ihash != cat.name2hash.end()) {   // hash found
      return ihash->second;
    }
  }
  else {
    s_hashStore.hashCat.emplace(category, std::make_unique<HashMap>());
  }

  /*********************************************************************
   This hash function is derived from the ELF hashing function and
   unchanged since Run-1. For Phase-II we should really switch to a
   64 bit hash like CxxUtils::crc64 which is safer (and faster).
   But we need to take care of backwards compatibility with old data.
   Original author: Tomasz Bold
  *********************************************************************/
  HLTHash hash = 0xd2d84a61;
  for ( int i = (int)s.size()-1; i >= 0; --i )
  //coverity[INTEGER_OVERFLOW]
    hash ^= ( hash >> 5) + s[i] + ( hash << 7 );

  for ( int i = 0; i < (int)s.size(); ++i )
    hash ^= ( hash >> 5) + s[i] + ( hash << 7 );
  /********************************************************************/

  // Try to insert new hash
  HashMap& cat = s_hashStore.hashCat.at(category);
  const auto& [itr, inserted] = cat.hash2name.emplace(hash, s);

  if ( inserted ) {
    // also update reverse map
    cat.name2hash.emplace(s, hash);
  }
  else {
    // There are two cases where insertion into the hash->name map would fail:
    // 1) another thread entered the same hash/name pair already
    // 2) there is a hash collision
    if ( s != itr->second ) {
      throw std::domain_error("Hash collision in category " + category +
                              " for elements " +  itr->second + " and " + s);
    }
  }

  return hash;
}

const std::string HLTUtils::hash2string( HLTHash hash, const std::string& category ) {

  const auto& icat = s_hashStore.hashCat.find(category);
  if (icat == s_hashStore.hashCat.end()) {
    return "UNKNOWN CATEGORY";
  }

  const HashMap& cat = icat->second;
  const auto& h = cat.hash2name.find(hash);
  if (h == cat.hash2name.end()) {
    return "UNKNOWN HASH ID";
  }

  return h->second;
}

void HLTUtils::hashes2file( const std::string& fileName) {
  std::ofstream fout(fileName);

  for (const auto [category, hashes] : s_hashStore.hashCat) {
    fout << s_newCategory << '\n' << category << '\n';
    for (const auto [hash, nameptr] : hashes.hash2name) {
      std::string name(nameptr);
      std::erase(name, '\n'); // Remove any line breaks
      fout << hash << '\n' << name << '\n';
    }
  }
}

void HLTUtils::file2hashes( const std::string& fileName) {
  std::ifstream fin(fileName);
  if (!fin.is_open()) {
    return;
  }
  std::string line;
  std::string category;
  // Note: this method is a to aid with development/debugging. 
  // It won't be used in production code, hence it is light on error checking.
  while(std::getline(fin, line)) {
    if (line == s_newCategory) {
      std::getline(fin, category);
      continue;
    }
    HLTHash hash = std::stoul(line);
    std::string name;
    std::getline(fin, name);
    HLTHash check = string2hash(name, category);
    if (check != hash) {
      std::cerr << "Inconsistency in file2hashes(" << fileName << ") function,"
                   " item " << name << " has hash " << hash << " not " << check << std::endl;
    }
  }
}
