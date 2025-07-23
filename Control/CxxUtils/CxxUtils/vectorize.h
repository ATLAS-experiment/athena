// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/vectorize.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2019
 * @brief Helper to enable (more agressive) auto-vectorization.
 *
 * Athena is usually built with -O2.
 * From gcc 12 and onwards this results to
 * (-O2  -Q --help=optimizers)
 * -ftree-loop-vectorize       		[enabled]
 * -ftree-slp-vectorize        		[enabled]
 * -ftree-vectorize            		[disabled]
 * -fvect-cost-model=[unlimited|dynamic|cheap|very-cheap] 	very-cheap
 * (clang uses a more agressive default model in -O2)
 *
 * There are cases where we prefer to use the gcc cheap model
 * rather than the very cheap.
 * This can be achieved by enabling tree-vectorize
 * (-O2  -ftree-vectorize -Q   --help=optimizers)
 * -ftree-loop-vectorize       		[enabled]
 * -ftree-slp-vectorize        		[enabled]
 * -ftree-vectorize            		[enabled]
 * -fvect-cost-model=[unlimited|dynamic|cheap|very-cheap]  cheap
 *
 *
 * Add
 * ATH_ENABLE_VECTORIZATION;
 * at the start of a compilation unit
 * to enable it for this file.
 *
 * Add
 * ATH_ENABLE_FUNCTION_VECTORIZATION
 * before a function to enable it for just
 * this function
 */
#include "CxxUtils/features.h"

#ifndef CXXUTILS_VECTORIZE_H
#define CXXUTILS_VECTORIZE_H

#if HAVE_GCC_CLANG_EXTENSIONS && !defined(__clang__) && !defined(SIMULATIONBASE)
# define ATH_ENABLE_VECTORIZATION                     \
  _Pragma("GCC optimize (\"tree-vectorize\")") class ATH_ENABLE_VECTORIZATION_SWALLOW_SEMICOLON
#else
# define ATH_ENABLE_VECTORIZATION class ATH_ENABLE_VECTORIZATION_SWALLOW_SEMICOLON
#endif

#endif  // not CXXUTILS_VECTORIZE_H
