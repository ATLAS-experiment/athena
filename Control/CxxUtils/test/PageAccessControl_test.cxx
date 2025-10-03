/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include <cassert>
#include <iostream>
#include <unistd.h>
#include "CxxUtils/procmaps.h"
#include "CxxUtils/page_access.h"
#include "CxxUtils/PageAccessControl.h"
// #define DEBUGIT 1
using namespace std;
int main(void) {
  cout << "*** PageAccessControl_test starts ***" <<endl;
  long pagesize_ret = sysconf(_SC_PAGE_SIZE);
  if (pagesize_ret < 0 || pagesize_ret >= 1024*1024*1024) return 1;
  size_t pagesize = pagesize_ret;
#ifdef DEBUGIT
  const bool DUMPMAPS(true);
#else
  const bool DUMPMAPS(false);
#endif
  procmaps pmaps(DUMPMAPS);
  PageAccessControl pac(pmaps);
  //protect a heap object
  void* p = malloc (3*pagesize);
  int* pi = static_cast<int*> (athena::next_page_address (p));
  *pi = 2;
  void* pv = malloc(10);
  //assert(pac.forbidPage(pi));
  if (!pac.forbidPage(pi)) perror("forbidPage fails");
  //FIXME assert(pac.forbidPage(pv, 10));
  //assert(pac.protectPage(pv, 10, PROT_READ));
  assert(pac.restorePageProt(pi));
  //assert(pac.restorePageProt(pv));
  cout << "accessing restored pointer " << *pi << endl;

  

//make valgrind happy
  free(p);
  free(pv);

  cout << "*** PageAccessControl_test OK ***" <<endl;
  return 0;
}
