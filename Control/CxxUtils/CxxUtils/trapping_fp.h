// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/trapping_fp.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2023
 * @brief Tell the compiler to optimize assuming that FP may trap.
 */


#ifndef CXXUTILS_TRAPPING_FP_H
#define CXXUTILS_TRAPPING_FP_H


// Tell the compiler to optimize the containing block assuming that
// FP may trap.  This is sometimes needed with clang to avoid spurious FPEs
// resulting from auto-vectorization.
#if defined(__clang__) && defined(__x86_64__) && !defined(__HIP_DEVICE_COMPILE__)
# define CXXUTILS_TRAPPING_FP _Pragma("float_control(except, on)") \
         class CxxUtilsTrappingFPDummy
#else
# define CXXUTILS_TRAPPING_FP class CxxUtilsTrappingFPDummy
#endif


#endif // not CXXUTILS_TRAPPING_FP_H
