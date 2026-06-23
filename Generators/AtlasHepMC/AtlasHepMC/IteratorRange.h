/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_ITERATORRANGE_H
#define ATLASHEPMC_ITERATORRANGE_H
namespace HepMC {
enum IteratorRange { parents, children, family,
                     ancestors, descendants, relatives
                   };
}
#endif
