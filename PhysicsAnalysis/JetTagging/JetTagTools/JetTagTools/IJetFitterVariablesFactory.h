/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// -*-c++-*- header for IJetFitterVariablesFactory
#ifndef BTAGTOOL_IJETFITTERVARIABLESFACTORY_C
#define BTAGTOOL_IJETFITTERVARIABLESFACTORY_C

/******************************************************
    @class  JetFitterVariableFactory
    Package : JetTagTools 
    Created : March 2007
    DESCRIPTION:
    This class is a "variable factory". It generates, starting from the VxJetCandidate, all 
    the variables actually used by the JetFitterTag.
    @author Giacinto Piacquadio (giacinto.piacquadio AT physik.uni-freiburg.de)
    @author2 Christian Weiser (christian.weiser AT physik.uni-freiburg.de) (University of Freiburg)
********************************************************/

#include "GaudiKernel/IAlgTool.h"
#include "xAODJet/Jet.h"
#include "xAODBTagging/BTagging.h"
#include <string> 

namespace Trk {
  class VxJetFitterVertexInfo;
}

namespace Analysis {

  struct JetFitterVariables {
    float massUncorr = 0.;
    float chi2 = 0.;
    int ndof = 0;
    float dRFlightDir = 0.;
    int nVTX = 0;
    int nSingleTracks = 0;
    int nTracksAtVtx = 0;
    float mass = 0.;
    float energyFraction = 0.;
    float significance3d = 0.;
    float deltaeta = 0.;
    float deltaphi = 0.;
  };

  static const InterfaceID IID_IJetFitterVariablesFactory
  ("Analysis::IJetFitterVariablesFactory", 1, 0);

  
  class IJetFitterVariablesFactory : virtual public IAlgTool  {
    
  public:
    
    virtual ~IJetFitterVariablesFactory(){};
    
    virtual StatusCode initialize() = 0;
    virtual StatusCode finalize() = 0;
    
    virtual StatusCode fillJetFitterVariables(const xAOD::Jet &, xAOD::BTagging* BTag, const Trk::VxJetFitterVertexInfo* myJetFitterInfo, std::string basename) const = 0;
    virtual StatusCode computeJetFitterVariables(const xAOD::Jet &, const Trk::VxJetFitterVertexInfo* myJetFitterInfo, const std::string& basename, JetFitterVariables &vars) const = 0;

    static const InterfaceID& interfaceID() { return IID_IJetFitterVariablesFactory; };

  }; 

 
  
}//end Analysis namespace

#endif
