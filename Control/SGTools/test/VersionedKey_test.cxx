/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "SGTools/SGVersionedKey.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <print>
#include <cassert>
using namespace std;
using namespace SG;

#define SGASSERTERROR( FALSEEXPR )   \
    std::cerr << "Now we expect to see an error message:" << std::endl \
              << "----Error Message Starts--->>" << std::endl; \
    assert(!FALSEEXPR); \
    std::cerr<< "<<---Error Message Ends-------" << std::endl

void strtest (const VersionedKey& k, const std::string& exp)
{
  std::ostringstream ss1;
  ss1 << k;
  assert (ss1.str() == exp);
  std::ostringstream ss2;
  std::print (ss2, "{}", k);
  assert (ss2.str() == exp);
}

int main() {
  VersionedKey vk1("bla", 1);
  std::string key("Bleah");
  unsigned int version(99);
  VersionedKey vk2(key, version);
  VersionedKey vk3(";00;default");
  VersionedKey vk4("default");

  cout << "Versioned Key 1=" << vk1 << endl;
  strtest (vk1, ";01;bla");
  std::string k1;
  unsigned char v1(0);
  vk1.decode(k1,v1);
  assert("bla" == k1);
  assert(1 == v1);
  cout << "Versioned Key 2=" << vk2 << endl;
  strtest (vk2, ";99;Bleah");
  vk2.decode(k1,v1);
  assert("Bleah"== k1);
  assert(99 == v1);
  assert(99 == vk2.version());
  cout << "Versioned Key 3=" << vk3 << endl;
  strtest (vk3, ";00;default");
  vk3.decode(k1,v1);
  assert("default" == k1);
  assert(0 == v1);
  vk4.decode(k1,v1);
  assert(0 == v1);
  
  assert("bla" == vk1.key());
  assert("Bleah" == vk2.key());
  assert("default" == vk3.key());


  //compare vkeys
  VersionedKey ck1("bla", 1);
  VersionedKey ck2("bla", 2);
  VersionedKey ck3("blu", 0);
  VersionedKey ck4("blu", 0);
  assert(ck1 < ck2);
  assert(!(ck2 < ck1));
  assert(ck1 < ck3);
  assert(!(ck3 < ck1));
  assert(ck2 < ck3);
  assert(!(ck3 < ck1));
  assert(!(ck3 < ck4));
  assert(!(ck4 < ck3));

  assert(!ck1.sameKey(ck4));
  assert(ck3.sameKey(ck4));
  assert(ck3.sameKey("blu"));
  std::string bluString("blu");
  assert(ck3.sameKey(bluString));

  return 0;
}



