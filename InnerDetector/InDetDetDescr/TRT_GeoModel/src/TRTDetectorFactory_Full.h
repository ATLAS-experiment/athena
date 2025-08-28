/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class TRTDetectorFactory_Full
 *
 * @brief This class creates the TRT Geometry
 *
 * @author Joe Boudreau, Andrei and Iouris Zalite, Thomas Kittelmann
 */


#ifndef TRT_GEOMODEL_TRTDETECTORFACTORY_FULL_H
#define TRT_GEOMODEL_TRTDETECTORFACTORY_FULL_H

#include "TRTParameterInterface.h"

#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h" //for Argon

#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h"
#include "InDetGeoModelUtils/InDetMaterialManager.h"

#include "GeoModelKernel/GeoDefinitions.h"

class GeoPhysVol;
class GeoFullPhysVol;

class TRTDetectorFactory_Full : public InDetDD::DetectorFactoryBase  {

 public:
  
  //--------------------------Public Interface:--------------------------------
  // Constructor:
  TRTDetectorFactory_Full(InDetDD::AthenaComps * athenaComps,
			  const ITRT_StrawStatusSummaryTool * sumTool,
			  bool useOldActiveGasMixture,
			  bool DC2CompatibleBarrelCoordinates,
			  bool alignable,
			  bool doArgon,
			  bool doKrypton,
			  bool useDynamicAlignmentFolders);

  // Destructor:
  ~TRTDetectorFactory_Full() = default;

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;

  // Access to the results:
  virtual const InDetDD::TRT_DetectorManager * getDetectorManager() const override;
  //---------------------------------------------------------------------------

  //---------------------------Illegal operations:------------------------------------------

  const TRTDetectorFactory_Full & operator=(const TRTDetectorFactory_Full &right) = delete;
  TRTDetectorFactory_Full(const TRTDetectorFactory_Full &right) = delete;
  //----------------------------------------------------------------------------------------

 private:

  // Gas mixture enumerator
  enum ActiveGasMixture
    {
    GM_XENON,
    GM_KRYPTON,
    GM_ARGON
    };

  ActiveGasMixture DecideGasMixture(int strawStatusHT);

  // private helper methods:
  const GeoShape* makeModule(double length
			     , const GeoTrf::Vector2D& corner1
			     , const GeoTrf::Vector2D& corner2
			     , const GeoTrf::Vector2D& corner3
			     , const GeoTrf::Vector2D& corner4
			     , GeoTrf::Transform3D & absolutePosition
			     , double shrinkDist=0) const;

  GeoPhysVol* makeStraw(bool hasLargeDeadRegion=false
			, ActiveGasMixture gasMixture = GM_XENON);

  GeoFullPhysVol* makeStrawPlane(size_t w
				 , ActiveGasMixture gasMixture = GM_XENON);

  // private member data:
  InDetDD::TRT_DetectorManager                  *m_detectorManager = nullptr; // ownership handed to calleer.
  std::unique_ptr<InDetMaterialManager>         m_materialManager;
  std::unique_ptr<TRTParameterInterface>        m_data;

  bool m_useOldActiveGasMixture;
  bool m_DC2CompatibleBarrelCoordinates;
  bool m_alignable;
  const ITRT_StrawStatusSummaryTool* m_sumTool; // added for Argon
  bool m_strawsvcavailable;
  bool m_doArgon;
  bool m_doKrypton;
  bool m_useDynamicAlignFolders;

  GeoFullPhysVol* m_type1Planes[3] = {nullptr, nullptr, nullptr};
  GeoFullPhysVol* m_type2Planes[3] = {nullptr, nullptr, nullptr};
};

#endif // TRTDetectorFactory_Full_h
