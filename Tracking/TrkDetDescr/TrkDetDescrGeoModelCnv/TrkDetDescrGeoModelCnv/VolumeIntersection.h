/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeIntersection.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRGEOMODELCNV_VOLUMEINTERSECTION_H
#define TRKDETDESCRGEOMODELCNV_VOLUMEINTERSECTION_H

#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include <utility> //for std::pair
#include <vector>

namespace Trk{
  class Volume;
}

namespace Trk {

   struct PolygonCache{
     double hZ;  double minZ; double maxZ; 
     Amg::Vector3D center;
     int nVtx;
     std::vector< Amg::Vector3D > vertices;
     std::vector< std::pair<double,double> > xyVertices;    // size+1
     std::vector<bool> commonVertices;
     std::vector<std::pair<double, double> > edges;
   PolygonCache() : hZ(0.), minZ(0.), maxZ(0.), center(Amg::Vector3D(0.,0.,0.)),nVtx(0) {};  
   }; 

   struct EdgeCross{
   
     std::pair<int, int> edge_id;
     std::pair<double, double> edge_pos;
     bool used;
   EdgeCross( std::pair<int,int> ei, std::pair<double, double> ep ) : edge_id(ei), edge_pos(ep), used(false) {};
   };

  /**
    @class VolumeIntersection
    
    A Simple Helper Class that collects methods for calculation of overlap of two geometrical objects.
        
    @author sarka.todorova@cern.ch
    */
    
     class VolumeIntersection {

       public:
      
       std::pair<bool, const Trk::Volume* >  intersect(const Volume*, const Volume*) const; 
       std::pair<bool, const Trk::Volume* >  intersectApproximative(const Volume*, const Volume*) const; 
    
      private:

	GeoShapeConverter m_geoShapeConverter;               //!< shape converter

	PolygonCache polygonXY(const Volume*, int swap = 0) const; 
	Trk::PolygonCache intersectPgon(Trk::PolygonCache&, Trk::PolygonCache&) const; 
	bool inside(std::pair<double,double> vtx, std::vector<std::pair<double,double>> pgon ) const;
	double det(std::pair<double,double> a, std::pair<double,double> b, bool ) const;
   };
 
} // end of namespace Trk

#endif

