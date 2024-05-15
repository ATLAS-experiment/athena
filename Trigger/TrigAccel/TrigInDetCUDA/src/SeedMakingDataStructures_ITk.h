/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_SEEDMAKINGDATASTRUCTURES_ITk_H
#define TRIGINDETCUDA_SEEDMAKINGDATASTRUCTURES_ITk_H

#include "CommonStructures.h"

static constexpr unsigned int MAX_MIDDLE_SP_ITk         = 300000;
static constexpr unsigned int MAX_DOUBLET_ITk           = 5000000;
static constexpr unsigned int NUM_MIDDLE_THREADS_ITk        = 32;
static constexpr unsigned int OUTER_THREADS_MULTIPLIER_ITk  = 4; // i.e thread block is 32 x 4*192/32
static constexpr unsigned int MAX_NUMBER_DOUBLETS_ITk       = 1500;
static constexpr unsigned int NUM_TRIPLET_BLOCKS_ITk        = 1024;
static constexpr unsigned int NUM_TRIPLET_THREADS_ITk       = 1024;
static constexpr unsigned int NUM_DOUBLET_THREADS_ITk       = 16;
static constexpr unsigned int MAX_TRIPLETS_ITk              = 300;
static constexpr unsigned int TRIPLET_BUFFER_DEPTH_ITk      = 3;

typedef struct doubletInfoITk {
public:
  int m_nInner[MAX_MIDDLE_SP_ITk];
  int m_nOuter[MAX_MIDDLE_SP_ITk];
  char m_good[MAX_MIDDLE_SP_ITk];
} DOUBLET_INFO_ITk;

typedef struct doubletStorageITk {
public:
  int m_nItems;
  int m_nI;
  int m_nO;
  int m_spmIdx[MAX_MIDDLE_SP_ITk];  
  int m_innerStart[MAX_MIDDLE_SP_ITk];
  int m_outerStart[MAX_MIDDLE_SP_ITk];
  int m_inner[MAX_DOUBLET_ITk];
  int m_outer[MAX_DOUBLET_ITk];
} DOUBLET_STORAGE_ITk;

#endif
