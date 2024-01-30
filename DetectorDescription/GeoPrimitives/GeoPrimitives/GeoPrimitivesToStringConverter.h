/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeoPrimitvesToStringConverter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef GEOPRIMITIVESTOSTRINGCONVERTER_H_
#define GEOPRIMITIVESTOSTRINGCONVERTER_H_

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#ifndef XAOD_STANDALONE
#   include "CLHEP/Geometry/Transform3D.h"
#   include "CLHEP/Geometry/Point3D.h"
#   include "CLHEP/Vector/TwoVector.h"
#endif // not XAOD_STANDALONE

namespace Amg {

  /** GeoPrimitvesToStringConverter

      static methods for conversion of GeoPrimitives and will call the EventPrimitives converter (Matrix)
      to std::string.

      This is to enhance formatted screen ouput and for ASCII based
      testing.

      The offset can be used to offset the lines (starting from line 2) wrt to the
      zero position for formatting reasons.

      @author Niels.Van.Eldik@cern.ch 

  */

  inline std::string toString( const Translation3D& translation, int precision = 4 ){
    Vector3D trans{translation.x(), translation.y(), translation.z()};
    return toString( trans, precision );
  }


  inline std::string toString( const Transform3D& transform, int precision = 4, const std::string& rotOffSet = "") {
    std::stringstream sstr{};
    bool printed{false};
    if (transform.translation().mag() > std::numeric_limits<float>::epsilon()) {
        sstr<<"translation: "<<toString(transform.translation(), precision);
        printed = true;
    }
    if (!Amg::doesNotDeform(transform)) {
        if (printed) sstr<<rotOffSet<<", ";
        sstr<<"rotation: {"<<toString(transform.linear()*Vector3D::UnitX(), precision)<<",";
        sstr<<toString(transform.linear()*Vector3D::UnitY(), precision)<<",";
        sstr<<toString(transform.linear()*Vector3D::UnitZ(), precision)<<"}";
        printed = true;
    }
    if (!printed) sstr<<"Identity matrix ";
    return sstr.str();
}

#ifndef XAOD_STANDALONE

  inline std::string toString( const CLHEP::HepRotation& rot, int precision = 4, const std::string& offset="" ){
    std::ostringstream sout;

    sout << std::setiosflags(std::ios::fixed) << std::setprecision(precision);
    for( int i=0;i<3;++i ){
      for( int j=0;j<3;++j ){
	if( j == 0 ) sout << "(";
	double val = roundWithPrecision(rot(i,j),precision);
	sout << val;
	if( j == 2 ) sout << ")";
	else         sout << ", ";
      }
      if( i != 2 ) {
	sout << std::endl;
	sout << offset;    
      }       
    }    
    return sout.str();
  }


  inline std::string toString( const CLHEP::Hep3Vector& translation, int precision = 4){
    std::ostringstream sout;
    sout << std::setiosflags(std::ios::fixed) << std::setprecision(precision);
    for( int j=0;j<3;++j ){
      if( j == 0 ) sout << "(";
      double val = roundWithPrecision( translation[j],precision);
      sout << val;
      if( j == 2 ) sout << ")";
      else         sout << ", ";
    }
    return sout.str();
  }

  inline std::string toString( const CLHEP::Hep2Vector& translation, int precision = 4){
    std::ostringstream sout;
    sout << std::setiosflags(std::ios::fixed) << std::setprecision(precision);
    for( int j=0;j<2;++j ){
      if( j == 0 ) sout << "(";
      double val = roundWithPrecision( translation[j],precision);
      sout << val;
      if( j == 1 ) sout << ")";
      else         sout << ", ";
    }
    return sout.str();
  }

  inline std::string toString( const HepGeom::Transform3D& transf, int precision = 4, const std::string& offset=""){
    std::ostringstream sout;
    sout << "Translation : " << toString( transf.getTranslation(), precision ) << std::endl;
    std::string rotationOffset = offset + "              ";
    sout << offset << "Rotation    : " << toString( transf.getRotation(), precision+2, rotationOffset );
    return sout.str();
  }

#endif // not XAOD_STANDALONE

}

#endif /* GEOPRIMITIVESTOSTRINGCONVERTER_H_ */
