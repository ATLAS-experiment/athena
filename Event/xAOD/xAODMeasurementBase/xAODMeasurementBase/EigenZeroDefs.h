// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODMeasurementBase/EigenZeroDefs.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2025
 * @brief Ensure that eigen aux variables get properly zeroed.
 */


#ifndef XAODMEASUREMENTBASE_EIGENZERODEFS_H
#define XAODMEASUREMENTBASE_EIGENZERODEFS_H


#include "EventPrimitives/EventPrimitives.h"
#include "AthContainers/tools/AuxDataTraits.h"


namespace SG {


// The default Matrix constructor does not initialize the matrix contents.
// Specialize this for Matrix, so that when we have Matrix auxiliary variables,
// they'll be fully initialized after a resize, etc.
template <typename SCALAR, int ROWS, int COLS, int OPTIONS, int MAXROWS, int MAXCOLS>
struct Zero<Eigen::Matrix<SCALAR, ROWS, COLS, OPTIONS, MAXROWS, MAXCOLS> >
{
  typedef Eigen::Matrix<SCALAR, ROWS, COLS, OPTIONS, MAXROWS, MAXCOLS> Matrix;
  static Matrix zero()
  {
    Matrix m;
    m.setZero();
    return m;
  }
};


} // namespace SG


#endif // not XAODMEASUREMENTBASE_EIGENZERODEFS_H
