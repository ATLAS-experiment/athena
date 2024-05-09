/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class TRTDetectorFactory_Lite
 *
 * @author Joe Boudreau
 */

#ifndef TRT_GEOMODEL_TRTDETECTORFACTORY_LITE_H
#define TRT_GEOMODEL_TRTDETECTORFACTORY_LITE_H

#include "TRTParameterInterface.h"

#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h" //for Argon

#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include <string>

class GeoPhysVol;
class GeoFullPhysVol;

namespace GeoModelIO {
  class ReadGeoModel;
}

class TRTDetectorFactory_Lite : public InDetDD::DetectorFactoryBase  {

 public:
  
  //--------------------------Public Interface:----------------------------
  // Constructor:
  TRTDetectorFactory_Lite(GeoModelIO::ReadGeoModel *sqliteReader,
			  InDetDD::AthenaComps * athenaComps,
			  const ITRT_StrawStatusSummaryTool * sumTool,
			  bool useOldActiveGasMixture,
			  bool DC2CompatibleBarrelCoordinates,
			  int overridedigversion,
			  bool alignable,
			  bool useDynamicAlignmentFolders);

  // Destructor:
  ~TRTDetectorFactory_Lite() = default;

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;

  // Access to the results:
  virtual const InDetDD::TRT_DetectorManager * getDetectorManager() const override;
  //------------------------------------------------------------------------

  //---------------------------Illegal operations:--------------------------
  const TRTDetectorFactory_Lite & operator=(const TRTDetectorFactory_Lite &right) = delete;
  TRTDetectorFactory_Lite(const TRTDetectorFactory_Lite &right) = delete;
  //------------------------------------------------------------------------

private:

  double activeGasZPosition(bool hasLargeDeadRegion=false) const;

  void setEndcapTransformField(size_t w);

  // private member data:
  GeoModelIO::ReadGeoModel                      *m_sqliteReader;
  InDetDD::TRT_DetectorManager                  *m_detectorManager = nullptr; // ownership handed to calleer.
  std::unique_ptr<TRTParameterInterface>        m_data;

  bool m_useOldActiveGasMixture;
  bool m_DC2CompatibleBarrelCoordinates;
  int m_overridedigversion;
  bool m_alignable;
//  const ITRT_StrawStatusSummaryTool* m_sumTool; // added for Argon
  bool m_useDynamicAlignFolders;

};

#endif // TRTDetectorFactory_Lite_h
