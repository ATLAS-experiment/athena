/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _TrkVKalVrtCore_PGRAPH_H
#define _TrkVKalVrtCore_PGRAPH_H

namespace Trk {
  class PGraph{

// 1. HOT SCALAR CONTROL VARIABLES (Grouped together to fit into 1-2 cache lines)
    long int m_tabnr;
    long int m_lweit;
    long int m_lvset;
    long int m_lwset;
    long int m_choice{}; //initialized because improper use could corrupt memory

    // 2. FREQUENTLY ACCESSED SMALL ARRAYS (Accessed heavily during graph traversal steps)
    long int m_vset[1000];
    long int m_wset[1000];
    long int m_lteit[1000];
    long int m_lsett[1000];

    // 3. COLD / MASSIVE BUFFERS (Pushed to the end since they are accessed via bulk indexing)
    short int m_teit[999000];
    short int m_sett[499500];

public:
    //coverity[UNINIT_CTOR]
    PGraph() { }  // cppcheck-suppress uninitMemberVar; prevent zeroing of large arrays.

    int pgraphm_(long int *weit, long int edges, long int nodes, long int *set, long int *nptr, long int nth) noexcept;
    static void trevni_(long int *from, long int length, long int *to, long int maxv, long int *newlng, long int *work) noexcept;

  };
}

#endif
