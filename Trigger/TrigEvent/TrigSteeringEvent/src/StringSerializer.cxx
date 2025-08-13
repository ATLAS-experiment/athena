/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cstring>
#include "TrigSteeringEvent/StringSerializer.h"


namespace TrigSteeringEvent {


const char* const StringSerializer::s_delimiter = "\n";


void StringSerializer::serialize (const std::vector<std::string>& strings, std::vector<uint32_t>& storage ) {

  m_ostream.str(""); // ostream reset
  
  // copy strings into m_ostream with delimiter after each element (even after last element)
  copy(strings.begin(), strings.end(), std::ostream_iterator<std::string>(m_ostream, s_delimiter));
  
  /*  
      unsigned int i;
      for ( i = 0 ; i < strings.size() ; ++i) {
      std::cerr << "ser: " << strings[i] << std::endl;
      }
  */
  
  unsigned sizeToReserve = m_ostream.str().size();
  
  // Pad with '\n'
  unsigned int sizeToPad = getPadding(sizeToReserve);
  
  if ( sizeToPad != 0 ) {
    sizeToReserve += sizeToPad;
    for ( unsigned i = 0 ; i < sizeToPad; i++ )
      m_ostream << s_delimiter;
  }
  m_ostream.flush();
  
  std::string s (m_ostream.str());
  const uint32_t * uarray = reinterpret_cast<const uint32_t*> (s.data());

  storage.push_back(strings.size()); // put number of strings first
  storage.insert(storage.end(), &uarray[0], &uarray[sizeToReserve/sizeof(uint32_t)]);

  /*
  std::cerr << "reserved = " << sizeToReserve << ", pad = " << sizeToPad
            << ", storage size = " << storage.size() << ", serialized: " << std::endl;
  
  for ( unsigned int i = 0 ; i < sizeToReserve; ++i ) {
    std::cerr << " " << std::hex << (int)carray[i] << std::dec << "(" << (char)carray[i] << ")";
  }
  std::cerr << " " << std::dec << std::endl;
  */
  
}

void StringSerializer::serialize (const std::string& str, std::vector<uint32_t>& storage ) {
  std::vector<std::string> tmp(1, str);
  return serialize(tmp, storage);
}

std::size_t StringSerializer::deserialize (std::vector<uint32_t>::const_iterator first,
                                           std::vector<uint32_t>::const_iterator last,
                                           std::vector<std::string>& strings)
{
  std::size_t storageSize = std::distance(first, last);
  
  if ( storageSize <= 1 ) return storageSize;
  unsigned int numOfStrings = *first;

  /*
    unsigned int i;
    std::cerr << "deserialized: " << std::endl;
    for ( i = 0 ; i < storage.size()-1; ++i ) {
    std::cerr << " " << std::hex << uarray[i] ;
    }
    std::cerr << " " << std::dec << std::endl;
  */
  
  std::string whole(reinterpret_cast<const char*>(&*(first+1)),
                    (storageSize-1)*sizeof(uint32_t));

  m_istream.clear();  // istream reset
  m_istream.str(whole);

  std::string one;
  unsigned int readInStrings=0;
  std::size_t nChars=0;
  
  while ( m_istream.good() && readInStrings < numOfStrings ) {
    getline(m_istream, one, s_delimiter[0]);
    strings.push_back(one);
    readInStrings++;
    nChars += one.size() + 1;
  }
  
  return 1 + (nChars+getPadding(nChars))/sizeof(uint32_t);  // # uint32_t words (incl. header)
}

std::size_t StringSerializer::deserialize (std::vector<uint32_t>::const_iterator first,
                                           std::vector<uint32_t>::const_iterator last,
                                           std::string& str)
{
  std::vector<std::string> tmp;
  std::size_t nWords = deserialize(first, last, tmp); 
  if (tmp.size() == 1 )
    str = tmp[0];

  return nWords;
}
  
unsigned int StringSerializer::inquireSize(const std::vector<uint32_t>& storage) {
  if ( storage.size() <= 1 )
    return 0;
  return storage[0];
}


} // namespace TrigSteeringEvent
