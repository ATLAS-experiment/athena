/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARREADOUTGEOMETRY_HECCELLCONSTLINK_H
#define LARREADOUTGEOMETRY_HECCELLCONSTLINK_H

#include "LArReadoutGeometry/HECCell.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

/**
 * @Class: HECCellConstLink 
 * 
 *	Smart Pointer to HEC Cells.  This reference counted link
 *	allocates on demand. It audits the total pointer count
 *	and collects the garbage when nobody's looking.
 */

using HECCellConstLink=GeoIntrusivePtr<const HECCell>;

#endif
