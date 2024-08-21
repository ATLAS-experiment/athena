/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETCONVERSIONFINDERTOOLS_VERTEXPOINTESTIMATOR_H
#define INDETCONVERSIONFINDERTOOLS_VERTEXPOINTESTIMATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkParameters/TrackParameters.h"

namespace InDet {
  
  /**
     @class VertexPointEstimator
     Some helper tools like:
     * hits counter
     @author Tatjana Lenz , Thomas Koffas
  */
  
  class VertexPointEstimator : public AthAlgTool {
    
  public:
    VertexPointEstimator (const std::string& type,const std::string& name, const IInterface* parent);
    
    virtual ~VertexPointEstimator() = default;
    
    static const InterfaceID& interfaceID();

    virtual StatusCode initialize() override;
    
    virtual StatusCode finalize() override;

    typedef std::map<std::string, float> Values_t;
    
    /** Get intersection point of two track helices.
     * Based on pure geometric arguments.
     * Return error flag if problems.
     */
    Amg::Vector3D getCirclesIntersectionPoint(const Trk::Perigee* per1,
                                              const Trk::Perigee* per2,
                                              unsigned int flag,
                                              int& errorcode) const;


    /** Get intersection point of two track helices.
     * Based on pure geometric arguments.
     * Return error flag if problems.
     * Also return dictionary of values for decorations. 
     */
    Amg::Vector3D getCirclesIntersectionPoint(const Trk::Perigee* per1,
                                              const Trk::Perigee* per2,
                                              unsigned int flag,
                                              int& errorcode,
                                              Values_t& decors) const;

    /** Return list of keys used for decorations. */
    static std::vector<std::string> decorKeys() ;


  private:
    Amg::Vector3D intersectionImpl (const Trk::Perigee *per1,
                                    const Trk::Perigee *per2,
                                    unsigned int flag,
                                    int& errorcode,
                                    float& deltaPhi,
                                    float& deltaR) const;


    static double areaVar(double, double, double, double, double, double, double&) ;
    static double areaVar(double, double, double, double, double, double, double&, double&, double&) ;
    static bool   circleIntersection(double, double, double, 
			      double, double, double, 
			      double&, double&, 
			      double&, double&) ;
    static bool   secondDegree(double, double, double, double&, double&) ;
    static double areaTriangle(double, double, double, double, double, double) ;
    
    static const double s_bmagnt;
    DoubleArrayProperty m_maxDR
      {this, "MaxTrkXYDiffAtVtx", {10000., 10000., 10000.},
       "maximum XY separation, non-intersecting circles"};
    DoubleArrayProperty m_maxDZ
      {this, "MaxTrkZDiffAtVtx", {10000., 10000., 10000.},
       "maximum allowed track Z separation at the vertex"};
    DoubleArrayProperty m_maxR
      {this, "MaxTrkXYValue", {10000., 10000., 10000.},
       "maximum allowed vertex radius"};
    DoubleArrayProperty m_minArcLength
      {this, "MinArcLength", {-10000., -10000., -10000.},
       "minimum permitted arc length, track to vertex, depends on the posion of the first measurement"};
    DoubleArrayProperty m_maxArcLength
      {this, "MaxArcLength", {10000., 10000., 10000.},
       "maximum permitted arc length, track to vertex, depends on the posion of the first measurement"};
    DoubleArrayProperty m_minDr
      {this, "MinDeltaR", {-5., -25., -50.},
       "minimum difference between helix centers"};
    DoubleArrayProperty m_maxDr
      {this, "MaxDeltaR", {5., 10., 10.},
       "maximum difference between helix centers"};
    DoubleArrayProperty m_maxHl
      {this, "MaxHl", {10000., 10000., 10000.}, "maximum ratio H/l"};
    DoubleArrayProperty m_maxPhi
      {this, "MaxPhi", {0.05, 0.1, 0.1},
       "maximum DPhi at the estimated vertex position"};
    BooleanProperty m_returnOnError{this, "ReturnOnError", true};
  };
  
}
#endif // INDETCONVERSIONFINDERTOOLS_VERTEXPOINTESTIMATOR_H 

 
