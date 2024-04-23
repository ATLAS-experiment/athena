/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrkVertexFitterUtils_KalmanVertexTrackUpdator_H
#define TrkVertexFitterUtils_KalmanVertexTrackUpdator_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVertexFitterInterfaces/IVertexUpdator.h"
#include "TrkVertexFitterInterfaces/IVertexTrackUpdator.h"

/**
 * @class Trk::KalmanVertexTrackUpdator
 *
 * A concrete implementation of the VertexTrackUpdator 
 * using the Kalman filter algorithm. The algorithm
 * refits a single track with the knowledge of the
 * production vertex.
 * 
 * Based on R. Fruhwirth et al. Comp. Phys. Comm 96(1996) 189 
 *
 *@author Kirill Prokofiev, November 2005
 * 
 * ----------------------------------------------------------
 * Changes:
 *
 * David Shope <david.richard.shope@cern.ch> (2016-04-19)
 * 
 * EDM Migration to xAOD - from Trk::VxCandidate to xAOD::Vertex, 
 *                         from Trk::RecVertex   to xAOD::Vertex,
 *                         from Trk::Vertex      to Amg::Vector3D
 */


namespace Trk
{
 class VxTrackAtVertex;
 
 class KalmanVertexTrackUpdator : public AthAlgTool, virtual public IVertexTrackUpdator
 {
  public: 
  
   virtual StatusCode initialize() override;

/**
 * Constructor
 */
   KalmanVertexTrackUpdator(const std::string& t, const std::string& n, const IInterface*  p);

/**
 * Destructor
 */
   ~KalmanVertexTrackUpdator() = default;

/**
 * Update method  
 */ 
   virtual void  update(VxTrackAtVertex& trk, const xAOD::Vertex& vtx) const override;
    
  private:
  
   ToolHandle< IVertexUpdator > m_Updator {
     this, "VertexUpdator", "Trk::KalmanVertexUpdator"};
  
   DoubleProperty m_maxWeight {
     this, "MaximalWeight", 0.001, "Max weight of a given track in the vertex reconstruction"};
   BooleanProperty m_skipInvertibleCheck {
     this, "SkipInvertibleCheck", false, "skip the invertible check for the track CovMat and the vertex CovMat"};

 }; //end of class definitions
} //end of the namespace definitions

#endif
