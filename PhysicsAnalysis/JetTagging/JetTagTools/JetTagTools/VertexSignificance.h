// -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGTOOLS_VERTEXSIGNIFICANCE_H
#define JETTAGTOOLS_VERTEXSIGNIFICANCE_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "xAODTracking/Vertex.h"

#include <vector>

namespace Analysis {

  /// Signed decay length significance of the weighted mean of the secondary vertices
  /// with respect to the primary vertex. The sign is taken from the projection of the
  /// decay length on the jet axis. Returns 0 if the significance cannot be calculated.
  double get3DSignificance(const xAOD::Vertex& priVertex,
                           const std::vector<const xAOD::Vertex*>& secVertex,
                           const Amg::Vector3D& jetDirection);

  /// Largest signed significance of the individual secondary vertices with respect to
  /// the primary vertex. Vertices whose covariance cannot be inverted are skipped.
  double get3DSignificanceCorr(const xAOD::Vertex& priVertex,
                               const std::vector<const xAOD::Vertex*>& secVertex,
                               const Amg::Vector3D& jetDirection);

}

#endif
