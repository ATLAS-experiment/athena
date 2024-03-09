/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PhotonVertexSelection_PhotonVertexSelectionDict_H
#define PhotonVertexSelection_PhotonVertexSelectionDict_H

#include "PhotonVertexSelection/IPhotonVertexSelectionTool.h"
#include "PhotonVertexSelection/PhotonVertexSelectionTool.h"

#include "PhotonVertexSelection/IPhotonPointingTool.h"
#include "PhotonVertexSelection/PhotonPointingTool.h"
#include "PhotonVertexSelection/DecoratePhotonPointingAlg.h"
// #include "PhotonVertexSelection/BuildVertexPointingAlg.h"


#include "PhotonVertexSelection/PhotonVertexHelpers.h"

namespace {
  struct GCCXML_DUMMY_INSTANTIATION_PhotonVertexSelection {
    std::pair<const xAOD::Vertex*, float> pair1;
    std::vector<std::pair<const xAOD::Vertex*, float> > vector1;
  };
}

#ifdef XAOD_STANDALONE

// Helper macro for declaring the setProperty functions to the dictionary:
#define SETPROPERTY_INSTAN( TYPE )                                            \
   template StatusCode asg::AsgComponent::setProperty< TYPE >( const std::string&, \
                                                               const TYPE& )

// Declare all possible setProperty template instantiations to Reflex:
SETPROPERTY_INSTAN( ToolHandle<CP::IPhotonPointingTool > );

// Make the compiler forget about this macro now...
#undef SETPROPERTY_INSTAN

#endif // XAOD_STANDALONE


#endif // PhotonVertexSelection_PhotonVertexSelectionDict_H
