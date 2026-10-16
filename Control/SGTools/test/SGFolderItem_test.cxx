/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  SGTools/test/SGFolderItem_test.cxx
 * @author scott snyder
 * @date May, 2016
 * @brief Unit test for SGFolderItem.
 */

#undef NDEBUG


#include "SGTools/SGFolderItem.h"
#include <print>
#include <cassert>


void test1()
{
  std::println ("test1");

  SG::FolderItem i1 (123, "key");
  assert (i1.id() == 123);
  assert (i1.key() == "key");
  assert (i1.exact() == false);
  assert (i1.isFolder() == false);

  SG::FolderItem i2 (456, "ley", true);
  assert (i2.id() == 456);
  assert (i2.key() == "ley");
  assert (i2.exact() == true);
  assert (i2.isFolder() == false);

  SG::FolderItem i3 (123, "ley");

  assert (!(i1 < i1));
  assert (i1 < i2);
  assert (i1 < i3);

  assert (!(i2 < i1));
  assert (!(i2 < i2));
  assert (!(i2 < i3));

  assert (!(i3 < i1));
  assert ( (i3 < i2));
  assert (!(i3 < i3));
}


int main()
{
  test1();
  return 0;
}
