/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 *  @file TRT_RIOMaker.h
 *  Header file for class TRT_RIO_Maker
 *  (c) ATLAS Detector software
 *  @author I.Gavrilenko
 **/

#ifndef TRT_RIO_Maker_H
#define TRT_RIO_Maker_H

//STL
#include <string>

//Gaudi
#include "GaudiKernel/ToolHandle.h"

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

//InDet
// no forward declare (typedef?)
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "InDetRawData/TRT_RDO_Container.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "IRegionSelector/IRegSelTool.h"
#include "TRT_DriftCircleTool/ITRT_DriftCircleTool.h"

//fwd declarations
class ISvcLocator;

namespace InDet{

  /** TRT_DriftCircleContainer production algorithm
   * Top Algorithm for InDetRawDataContainer<TRT_RDORawData> conversion
   * to TRT_DriftCircleContainer
   **/
  class TRT_RIO_Maker : public AthReentrantAlgorithm {
  public:
    ///constructor
    TRT_RIO_Maker(const std::string &name, ISvcLocator *pSvcLocator);
    ///no-op destructor
    virtual ~TRT_RIO_Maker()  ;
    /**    @name Usual algorithm methods */
    //@{
    virtual StatusCode initialize ()  override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize   ()  override;
    //@}
  private:
  
    /**    @name Disallow default instantiation, copy, assignment */
    TRT_RIO_Maker() = delete;
    TRT_RIO_Maker(const TRT_RIO_Maker&) = delete;
    TRT_RIO_Maker &operator=(const TRT_RIO_Maker&) = delete;
  
    const TRT_ID* m_pTRTHelper;
    SG::ReadHandleKey<TRT_RDO_Container> m_rdoContainerKey{
      this, "TRTRDOLocation", "TRT_RDOs"};
    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{
      this, "RoIs", "", "RoIs to read in"};

    ToolHandle<ITRT_DriftCircleTool> m_driftcircle_tool{
      this, "TRT_DriftCircleTool", "InDet::TRT_DriftCircleTool"};
    ToolHandle<IRegSelTool> m_regionSelector {
      this, "RegSelTool", "RegSelTool/RegSelTool_TRT" }; //!< region selector tool

    IntegerProperty m_mode_rio_production{this, "ModeRIOProduction", 0};
    BooleanProperty m_trtBadChannels{
      this, "TRTBadChannels", true,
      "selection of the TRT bad channels(true/false)"};
    BooleanProperty m_roiSeeded{this, "isRoI_Seeded", false, "Use RoI"};
    BooleanProperty m_useDataPoolWithCache{
      this, "useDataPoolWithCache", false, "use DataPool With Cache"};

    SG::WriteHandleKey<InDet::TRT_DriftCircleContainer> m_rioContainerKey{
      this, "TRTRIOLocation", "TRT_DriftCircles"};
    SG::UpdateHandleKey<InDet::TRT_DriftCircleContainerCache>
    m_rioContainerCacheKey{this, "TRT_DriftCircleCache", ""};
    

  };
}//end of namespace
#endif // TRT_RIO_Maker_H
