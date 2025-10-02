/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cstring>
#include <sstream>
#include <iterator>

#include <iostream>

#include "TrigNavStructure/StringSerializer.h"

// the implementation is almost exact copy of the one in the TrigSerializeResult

namespace {
  const char* const delimiter = "\n";

  unsigned int getPadding(unsigned int sizeToReserve) {
    return (sizeof(uint32_t) - sizeToReserve%sizeof(uint32_t)) % sizeof(uint32_t);
  }
}
namespace HLT {
namespace StringSerializer {
  /*  unsigned int inquireSize(const std::vector<uint32_t>& storage) {
    if ( storage.size() <= 1 )
      return 0;
    return storage[0];
  }
  */

  void serialize (const std::vector<std::string>& strings, std::vector<uint32_t>& storage ) {
    std::ostringstream ostream;
    ostream.str(""); // ostream reset
  
    // copy strings into m_ostream with delimiter after each element (even after last element)
    copy(strings.begin(), strings.end(), std::ostream_iterator<std::string>(ostream, delimiter));
    
    unsigned sizeToReserve = ostream.str().size();
  
    // Pad with '\n'
    unsigned int sizeToPad = getPadding(sizeToReserve);
  
    if ( sizeToPad != 0 ) {
      sizeToReserve += sizeToPad;
      for ( unsigned i = 0 ; i < sizeToPad; i++ )
	ostream << delimiter;
    }
    ostream.flush();

    std::string s (ostream.str());
    const uint32_t* udata = reinterpret_cast<const uint32_t*> (s.data());
    storage.push_back(strings.size()); // put number of strings first
    storage.insert(storage.end(), udata, udata+sizeToReserve/sizeof(uint32_t));
  }

  void serialize (const std::string& str, std::vector<uint32_t>& storage ) {
    std::vector<std::string> tmp(1, str);
    return serialize(tmp, storage);
  }


  std::size_t deserialize (std::vector<uint32_t>::const_iterator first,
			   std::vector<uint32_t>::const_iterator last,
			   std::vector<std::string>& strings)
  {
    std::size_t storageSize = std::distance(first, last);
  
    if ( storageSize <= 1 ) return storageSize;
    unsigned int numOfStrings = *first;

    std::string whole(reinterpret_cast<const char*>(&*(first+1)),
                      (storageSize-1)*sizeof(uint32_t));
    std::istringstream istream;

    istream.clear();  // istream reset
    istream.str(whole);

    std::string one;
    unsigned int readInStrings=0;
    std::size_t nChars=0;
  
    while ( istream.good() && readInStrings < numOfStrings ) {
      getline(istream, one, delimiter[0]);
      strings.push_back(one);
      readInStrings++;
      nChars += one.size() + 1;
    }
  
    return 1 + (nChars+getPadding(nChars))/sizeof(uint32_t);  // # uint32_t words (incl. header)
  }

  std::size_t deserialize (std::vector<uint32_t>::const_iterator first,
			   std::vector<uint32_t>::const_iterator last,
			   std::string& str)
  {
    std::vector<std::string> tmp;
    std::size_t nWords = deserialize(first, last, tmp); 
    if (tmp.size() == 1 )
      str = tmp[0];

    return nWords;
  }
  
  
} // eof StringSerialzier
} // eof HLT
