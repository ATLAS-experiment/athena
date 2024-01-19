/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeConverter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRGEOMODELCNV_VOLUMECONVERTER_H
#define TRKDETDESCRGEOMODELCNV_VOLUMECONVERTER_H

#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"
#include "TrkDetDescrGeoModelCnv/VolumeIntersection.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/DetachedTrackingVolume.h"
#include "TrkVolumes/VolumeBounds.h"

namespace Trk {

  typedef std::pair< Material, double > MaterialComponent; 
    
   struct VolumeSpan{
     double phiMin{0.};
     double phiMax{2*M_PI};  
     double rMin{0.};   
     double rMax{1.e5}; 
     double xMin{-1.e5};
     double xMax{1.e5}; 
     double yMin{-1.e5};
     double yMax{1.e5}; 
     double zMin{-1.e5}; 
     double zMax{1.e5}; 
   };

   
   struct VolumePart{

     std::vector<const Volume*>  parts;
     float    sign;
     //VolumePart() {};
     //VolumePart( const Volume* vol, float asign ): sign(asign) { parts.push_back(vol); } ;
     //VolumePart( VolumePart& vp) : parts(vp.parts), sign(vp.sign) {} ;
     //VolumePart( VolumePart& vp, const Volume* vol,  float asign, bool add) : parts(vp.parts), sign(vp.sign*asign)
     //{ if (!add) parts[parts.size()-1]=vol;
     //  else  parts.push_back(vol); } 
   };
   

  /**
    @class VolumeConverter
    
    A Simple Helper Class that collects methods for material simplification.
        
    @author sarka.todorova@cern.ch
    */
    
     class VolumeConverter {

       public:
       /** Default constructor*/
       VolumeConverter() = default;

       /** Destructor*/
       ~VolumeConverter() = default;
    
	/** translation of GeoVPhysVol to Trk::TrackingVolume */
	TrackingVolume* translate( const GeoVPhysVol* gv, bool simplify, bool blend, double blendMassLimit) const;

        /** Simplification of tracking volume : default :  envelope creation : envelope has an analytically calculable volume
       //                                                                     simplify:  material is diluted to fill the envelope 
       //                                                                     blendInfo:    material content is stored to allow material blending  */
	//	TrackingVolume*  simplifyVolume( TrackingVolume* trvol, bool simplify,  bool blendInfo,
	//		     std::vector<std::vector<std::pair<std::unique_ptr<const Trk::Volume>, double> > >* constituentsVector) const;

        /** Decomposition of volume into set of non-overlapping subtractions from analytically calculable volume */ 
	std::vector<std::pair<const Trk::Volume*, const Trk::Volume* > > 
	splitComposedVolume(const Trk::Volume* trVol, std::vector<const Trk::Volume* >& vv) const;

       double resolveBooleanVolume(const Trk::Volume* trVol, double tolerance) const;
 
        /** Estimation of the geometrical volume span */
       const Trk::VolumeSpan* findVolumeSpan(const Trk::VolumeBounds* volBounds,
					      const Amg::Transform3D& transform, double zTol, double phiTol) const; 

        /** Volume calculation : by default return analytical solution only */
        double calculateVolume(const Trk::Volume* vol, bool nonBooleanOnly = false, double precision = 1.e-3 ) const;

        /**  the tricky part of volume calculation */
        double estimateFraction(std::pair<const Trk::Volume*, const Trk::Volume* > sub, double precision) const;

        /** material collection for layers */
	void collectMaterial(const GeoVPhysVol* pv, Trk::MaterialProperties& layMat, double sf) const; 

       /** material collection for volumes */
	void collectMaterialContent(const GeoVPhysVol* gv,  std::vector<Trk::MaterialComponent>&  materialContent) const;

      private:

       double leadingVolume(const GeoShape* sh) const;

	Trk::GeoShapeConverter m_geoShapeConverter;               //!< shape converter
	Trk::GeoMaterialConverter m_materialConverter;               //!< material converter
	Trk::VolumeIntersection m_intersectionHelper;                   //!< overlaps
      
        static constexpr double  s_precisionInX0 = 1.e-3;     // tentative required precision of the material thickness estimate  
    };
 
} // end of namespace Trk

#endif

