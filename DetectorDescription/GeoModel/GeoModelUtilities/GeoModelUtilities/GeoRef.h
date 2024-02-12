// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file GeoModelUtilities/GeoRef.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2022
 * @brief Simple smart-pointer class for GeoModel objects.
 *
 * GeoModel itself has a Link class, but it only handles the volume base class.
 */


#ifndef GEOMODELUTILITIES_GEOREF_H
#define GEOMODELUTILITIES_GEOREF_H

#include "GeoModelKernel/GeoIntrusivePtr.h"

template <class T> using GeoRef = GeoIntrusivePtr<T>;

#endif // not GEOMODELUTILITIES_GEOREF_H
