/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOUTILS_DEFS_H
#define ACTSGEOUTILS_DEFS_H
/// Header file to manage the common inlcudes
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
///
#include <ActsGeometryInterfaces/GeometryContext.h>

#include <limits>
#include <set>

/** @brief forward declarations of the classes defined in the package*/
namespace ActsTrk{

   class SurfaceCache;
   using SurfaceCacheSet = std::set<std::unique_ptr<SurfaceCache>, std::less<>>;

   
   class TransformCache;
   template <class> class TransformCacheDetEle;
}

#endif