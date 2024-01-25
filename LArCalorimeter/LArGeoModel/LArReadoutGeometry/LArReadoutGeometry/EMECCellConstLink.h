/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARREADOUTGEOMETRY_EMECCELLCONSTLINK_H
#define LARREADOUTGEOMETRY_EMECCELLCONSTLINK_H

#include "LArReadoutGeometry/EMECCell.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

/**
 * @class EMECCellConstLink
 *
 * @brief Smart Pointer to EMEC Cells.  This reference counted
 *	link allocates on demand. It audits the total pointer
 *	count and collects the garbage when nobody's looking.
 */

using EMECCellConstLink=GeoIntrusivePtr<const EMECCell>;

#endif
