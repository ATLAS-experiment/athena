/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackParameters.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKPARAMETERS_TRACKPARAMETERS_H
#define TRKPARAMETERS_TRACKPARAMETERS_H

#include "TrkParametersBase/CurvilinearParametersT.h"
#include "TrkParametersBase/ParametersT.h"
#include "TrkSurfaces/ConeSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"

namespace Trk {
constexpr size_t TrackParametersDim = 5;

// Abstract  base for Pattern Track Parameters and Track Parameters
using BaseParameters = ParametersCommon<TrackParametersDim, Charged>;
// Abstract base for Track Parameters
using TrackParameters = ParametersBase<TrackParametersDim, Charged>;
// Concrete representations of Track Parameters
using CurvilinearParameters = CurvilinearParametersT<TrackParametersDim, Charged, PlaneSurface>;
using AtaCone = ParametersT<TrackParametersDim, Charged, ConeSurface>;
using AtaCylinder = ParametersT<TrackParametersDim, Charged, CylinderSurface>;
using AtaDisc = ParametersT<TrackParametersDim, Charged, DiscSurface>;
using Perigee = ParametersT<TrackParametersDim, Charged, PerigeeSurface>;
using AtaPlane = ParametersT<TrackParametersDim, Charged, PlaneSurface>;
using AtaStraightLine = ParametersT<TrackParametersDim, Charged, StraightLineSurface>;
}

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream&
operator<<(MsgStream& sl, const Trk::TrackParameters& pars);

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
std::ostream&
operator<<(std::ostream& sl, const Trk::TrackParameters& pars);

#endif
