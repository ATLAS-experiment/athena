/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VKalVrt_IGNNVertexFitterInterface_H
#define VKalVrt_IGNNVertexFitterInterface_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace Rec {

static const InterfaceID IID_IGNNVertexFitterInterface("IGNNVertexFitterInterface", 1, 0);

class IGNNVertexFitterInterface : virtual public IAlgTool {
public:
  static const InterfaceID &interfaceID() { return IID_IGNNVertexFitterInterface; }

  virtual StatusCode fitAllVertices(const xAOD::JetContainer *jetCont, xAOD::VertexContainer *vertexCont,
                                      const xAOD::Vertex &primaryVertex, const EventContext &ctx) const = 0;
};

} // namespace Rec

#endif
