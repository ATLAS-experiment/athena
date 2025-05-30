/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//-------------------------------------------------------------------------------------------//
//                                                                                           //
// This class (GeoModelExperiment) is a stored manager of managers. It provides access to    //
// the world physical volume and all of the managers; so, indirectly, all of the detector    //
// specific top level physical volumes.                                                      //
//                                                                                           //
// Joe Boudreau October 2003                                                                 //
//                                                                                           //
//-------------------------------------------------------------------------------------------//

#ifndef GEOMODELUTILITIES_GEOMODELEXPERIMENT_H
#define GEOMODELUTILITIES_GEOMODELEXPERIMENT_H

/// Ensure that the extensions for the Vector3D are properly loaded
#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelKernel/GeoPhysVol.h"
#ifndef BUILDVP1LIGHT
    #include "AthenaKernel/CLASS_DEF.h"
    #include "AthenaKernel/CondCont.h"
#endif
#include <set>
#include <string>
#include <vector>

class GeoVDetectorManager;

class GeoModelExperiment {

public:


  //-----------------------------Internal----------------------------------------------------//
  //                                                                                         //
  typedef const GeoVDetectorManager *                            value_type;                 //
                                                                                             //
  class NameEquals{                                                                          //
  public:                                                                                    //
                                                                                             //
    NameEquals(const std::string & name);                                                    //
    bool operator () (const value_type & m) const;                                           //
                                                                                             //
  private:                                                                                   //
                                                                                             //
    std::string m_name;                                                                      //
                                                                                             //
  };                                                                                         //
                                                                                             //
  class LexigraphicalOrder{                                                                  //
  public:                                                                                    //
    bool operator () (const value_type & a, const value_type & b) const;                     //
  };                                                                                         //
                                                                                             //
  typedef LexigraphicalOrder                                     order_type;                 //
  typedef std::set<value_type, order_type>                       collection_type;            //
  typedef collection_type::iterator                              iterator_type;              //
  typedef collection_type::const_iterator                        const_iterator_type;        //
                                                                                             //
                                                                                             //
  typedef const_iterator_type                                    ConstIterator;              //
  //                                                                                         //
  //-----------------------------------------------------------------------------------------//


  // ---------------------------For you: ----------------------------------------------------//
  //                                                                                         //
  // Standard Constructor                                                                    //
  GeoModelExperiment(PVLink physVol);                                                   //
  //                                                                                         //
  // Standard Destructor                                                                     //
  virtual ~GeoModelExperiment() = default;                                                             //
  //                                                                                         //
  // Return the World physical volume:                                                       //
  GeoPhysVol *getPhysVol();                                                                  //
  const GeoPhysVol *getPhysVol() const;                                                      //
  //
  // Print a list of all the managers:
  std::vector<std::string> getListOfManagers() const;
  //                                                                                         //
  // Manage the managers:                                                                    //
  void addManager(const GeoVDetectorManager *);                                              //
  //                                                                                         //
  // Access those managers:                                                                  //
  const GeoVDetectorManager *getManager(const std::string & name) const;                     //
  ConstIterator beginManager() const;                                                        //
  ConstIterator endManager()   const;                                                        //
  // Add temporary volumes created during Geo2G4 translation                                 //
  void addTmpVolume(PVConstLink volume);                                                     //
  //                                                                                         //
  //-----------------------------------------------------------------------------------------//
  
 private:
  
  PVLink                                                m_physVol{};
  collection_type                                       m_managers;
  std::vector<PVConstLink>                              m_tmpVolumes;

};

#ifndef BUILDVP1LIGHT
//using the macros below we can assign an identifier (and a version)
//This is required and checked at compile time when you try to record/retrieve
CLASS_DEF(GeoModelExperiment, 9875, 1);
CONDCONT_MIXED_DEF( GeoModelExperiment , 67974987 );
#endif

#endif // GEOMODELSVC_GEOMODELEXPERIMENT_H



