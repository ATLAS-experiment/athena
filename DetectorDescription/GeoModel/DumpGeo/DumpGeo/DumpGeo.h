/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Header file for class DumpGeo                          // 
//                                                         //
//  Initial version:                                       //
//  - 2017, Sep -- Riccardo Maria BIANCHI                  //
//                 <riccardo.maria.bianchi@cern.ch>        //
//                                                         //
//  Main updates:                                          //
//  - 2024, Feb -- Riccardo Maria BIANCHI                  //
//                 <riccardo.maria.bianchi@cern.ch>        //
//                                                         //
//  This is the Athena algorithm to dump the geometry      //
//                                                         //
/////////////////////////////////////////////////////////////

#ifndef DumpGeo_DumpGeo
#define DumpGeo_DumpGeo

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/IIncidentListener.h"

#include <string>
#include <vector>

// class GeoExporter;

// Marked not thread-safe because GeoExporter uses VP1.
class ATLAS_NOT_THREAD_SAFE DumpGeo: public AthAlgorithm
{
 public:
  DumpGeo(const std::string& name, ISvcLocator* pSvcLocator) ATLAS_CTORDTOR_NOT_THREAD_SAFE;
  ~DumpGeo()=default;

  StatusCode initialize();
  StatusCode execute() {return StatusCode::SUCCESS;};

 private:
  // Properties
  // -- Athena-related
  Gaudi::Property<std::string> m_atlasRelease{this, "AtlasRelease", "", "The current, in use Atlas release"}; 
  Gaudi::Property<std::string> m_detDescrTag{this, "AtlasVersion", "", "The current, in use Atlas Detector Description Geometry TAG"}; 
  Gaudi::Property<std::string> m_outFileName{this, "OutSQLiteFileName", "", "The name of the output SQLite file"}; 
  Gaudi::Property<std::vector<std::string>> m_user_filterDetManagersList
                                          { this, "UserFilterDetManager", {}, "Doc", "OrderedSet<T>"};
};

#endif
