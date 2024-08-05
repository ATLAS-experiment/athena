/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NeutralParameters.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKNEUTRALPARAMETERS_NEUTRALPARAMETERS_H
#define TRKNEUTRALPARAMETERS_NEUTRALPARAMETERS_H

#include "TrkParametersBase/CurvilinearParametersT.h"
#include "TrkParametersBase/ParametersT.h"
#include "TrkSurfaces/ConeSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"


namespace Trk {

constexpr size_t NeutralParametersDim = 5;
// Alias declarations
using NeutralParameters = ParametersBase<NeutralParametersDim, Neutral>;
using NeutralCurvilinearParameters = CurvilinearParametersT<NeutralParametersDim, Neutral, PlaneSurface>;
using NeutralAtaCone = ParametersT<NeutralParametersDim, Neutral, ConeSurface>;
using NeutralAtaCylinder = ParametersT<NeutralParametersDim, Neutral, CylinderSurface>;
using NeutralAtaDisc = ParametersT<NeutralParametersDim, Neutral, DiscSurface>;
using NeutralPerigee = ParametersT<NeutralParametersDim, Neutral, PerigeeSurface>;
using NeutralAtaPlane = ParametersT<NeutralParametersDim, Neutral, PlaneSurface>;
using NeutralAtaStraightLine = ParametersT<NeutralParametersDim, Neutral, StraightLineSurface>;
}

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream&
operator<<(MsgStream& sl, const Trk::NeutralParameters& pars);

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
std::ostream&
operator<<(std::ostream& sl, const Trk::NeutralParameters& pars);

#endif // TRKNEUTRALPARAMETERS_NEUTRALPARAMETERS_H
