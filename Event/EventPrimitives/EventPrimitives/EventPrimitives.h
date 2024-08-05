/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EventPrimitives.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef EVENT_EVENTPRIMITIVES_H
#define EVENT_EVENTPRIMITIVES_H

#define EIGEN_MATRIXBASE_PLUGIN "EventPrimitives/AmgMatrixBasePlugin.h"
#define EIGEN_TRANSFORM_PLUGIN "EventPrimitives/AmgTransformPlugin.h"


#include <Eigen/Core>
#include <Eigen/Dense>

// These are the typedefs from Eigen to AMG ( Atlas Math and Geometry )
// some of those can be refined when switching to C++11
// @author: Eigen
// @responsible: Andreas.Salzburger -at- cern.ch

namespace Amg {

/** Dynamic Matrix - dynamic allocation */
using MatrixX = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
using SymMatrixX = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
/** Dynamic Vector - dynamic allocation */
using VectorX = Eigen::Matrix<double, Eigen::Dynamic, 1>;

/** Fixed capacity dynamic size types. Avoid dynamic allocations.
 * Helpfull if we  do not know the exact but a  max size.
 * But we potentially waste some space */
template <int MaxRows, int MaxCols>
using MatrixMaxX =
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, 0, MaxRows, MaxCols>;

template <int MaxDim>
using SymMatrixMaxX =
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, 0, MaxDim, MaxDim>;

template <int MaxRows>
using VectorMaxX = Eigen::Matrix<double, Eigen::Dynamic, 1, 0, MaxRows, 1>;

/** Macros for fixed size - no dynamic allocations, no waste of space */
#ifndef AmgMatrixDef
#define AmgMatrixDef
#define AmgMatrix(rows, cols) Eigen::Matrix<double, rows, cols, 0, rows, cols>
#define AmgSymMatrix(dim) Eigen::Matrix<double, dim, dim, 0, dim, dim>
#endif

#ifndef AmgVectorDef
#define AmgVectorDef
#define AmgVector(rows) Eigen::Matrix<double, rows, 1, 0, rows, 1>
#define AmgRowVector(cols) Eigen::Matrix<double, 1, cols, Eigen::RowMajor, 1, cols>
#endif

}  // namespace Amg

#endif /* EVENT_EVENTPRIMITIVES_H */
