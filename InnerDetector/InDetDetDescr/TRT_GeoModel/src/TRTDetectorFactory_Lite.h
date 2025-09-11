/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class TRTDetectorFactory_Lite
 *
 * @author Joe Boudreau
 */

#ifndef TRT_GEOMODEL_TRTDETECTORFACTORY_LITE_H
#define TRT_GEOMODEL_TRTDETECTORFACTORY_LITE_H

#include "TRTParameterInterface.h"

#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

#include "GeoModelKernel/GeoDefinitions.h"

#include <memory>

namespace GeoModelIO {
  class ReadGeoModel;
}

class TRTDetectorFactory_Lite : public InDetDD::DetectorFactoryBase  {

 public:

  TRTDetectorFactory_Lite(GeoModelIO::ReadGeoModel *sqliteReader,
			  InDetDD::AthenaComps * athenaComps,
			  bool useOldActiveGasMixture,
			  bool DC2CompatibleBarrelCoordinates,
			  int overridedigversion,
			  bool alignable,
			  bool useDynamicAlignmentFolders);

  ~TRTDetectorFactory_Lite();

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;

  // Access to the results:
  virtual const InDetDD::TRT_DetectorManager * getDetectorManager() const override;

  const std::string& name() const { 
    static const std::string n("TRT_GeoModel::TRTDetectorFactory"); 
    return n;
  }

  //---------------------------Illegal operations:---------------------------------
  const TRTDetectorFactory_Lite & operator=(const TRTDetectorFactory_Lite &right) = delete;
  TRTDetectorFactory_Lite(const TRTDetectorFactory_Lite &right) = delete;
  //-------------------------------------------------------------------------------
  
 private:  

  double activeGasZPosition(bool hasLargeDeadRegion=false) const;

  void setEndcapTransformField(size_t w);

  // private member data:
  GeoModelIO::ReadGeoModel                      *m_sqliteReader{};
  InDetDD::TRT_DetectorManager                  *m_detectorManager = nullptr; // ownership handed to calleer.
  std::unique_ptr<TRTParameterInterface>        m_data;

  bool m_useOldActiveGasMixture{};
  bool m_DC2CompatibleBarrelCoordinates{};
  int m_overridedigversion{0};
  bool m_alignable{};
  bool m_useDynamicAlignFolders{};

};

#endif // TRTDetectorFactory_Lite_h
