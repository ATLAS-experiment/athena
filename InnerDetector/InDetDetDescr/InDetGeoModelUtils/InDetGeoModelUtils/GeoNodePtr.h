/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _GeoNodePtr_H_
#define _GeoNodePtr_H_

#include <iostream>

#include "GeoModelKernel/GeoIntrusivePtr.h"

template <class T>
using GeoNodePtr = GeoIntrusivePtr<T>;
#endif
