/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_CONDITIONSSERVICES_TRT_STRAWALIGNDBSVC_H
#define TRT_CONDITIONSSERVICES_TRT_STRAWALIGNDBSVC_H

/** @file TRT_StrawAlignDbSvc.h
 * @brief interface to TRT straw alignment constants
 * @author Peter Hansen <phansen@nbi.dk>
 */

#include <vector>
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "InDetIdentifier/TRT_ID.h"
#include "ITRT_StrawAlignDbSvc.h"
#include "TRT_ConditionsData/StrawDxContainer.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY; // This class uses const_cast, regFcn (callback) and DataHandle. Legacy code

namespace InDetDD {
  class TRT_DetectorManager ;
}

/** @class TRT_StrawAlignDbSvc
  interface to TRT straw alignment constants
*/
class TRT_StrawAlignDbSvc: public extends<AthService, ITRT_StrawAlignDbSvc>
{
 public:
  
  /// constructor
  using base_class::base_class;
  
  /// destructor
  virtual ~TRT_StrawAlignDbSvc();
  
  /// tool initialize
  virtual StatusCode initialize();

  /// tool finalize
  virtual StatusCode finalize();

  // methods to access straw alignment data

  /// create an TRTCond::ExpandedIdentifier from a TRTID identifier
  TRTCond::ExpandedIdentifier trtcondid( const Identifier& id, int level = TRTCond::ExpandedIdentifier::STRAW) const;

  // methods to set straw alignment data

  /// set endpoints for a TRTCond::ExpandedIdentifier
  void setDx( const TRTCond::ExpandedIdentifier& id, float dx1, float dx2, float dxerr ) ;
  
  // methods for persistence
  
  /// write calibration constants to flat text file 
  StatusCode writeTextFile(const std::string& file) const;
  
  /// read calibration from text file into TDS
  virtual StatusCode readTextFile(const std::string& file);
  StatusCode readTextFile(StrawDxContainer* dxcontainer,
                          const std::string& file);

  /// write the calibration objects to output, after cleaning
  StatusCode streamOutObjects () const;
  
  /// register calibration objects with the IoV service
  StatusCode registerObjects(std::string tag, int run1, int event1, int run2, int event2) const;
    

  /// access to containers
  StrawDxContainer* getDxContainer() const ;
  const StrawDxContainer* getConstDxContainer() const ;

  /// IOV call back for dx objects. normally this doesn't do anything.
  StatusCode IOVCallBack();

  /** get shift near electronics */
  float getDx1( const Identifier& id ) const ;
  /** get shift far from electronics */
  float getDx2( const Identifier& id ) const ;
  /** get error on shift */
  float getDxErr( const Identifier& id ) const ;
  /** set shifts */
  void setDx( const Identifier& id, float dx1, float dx2, float dxerr ) ;

 private:

  ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "DetectorStore"};
  std::string m_par_dxcontainerkey{"/TRT/Calib/DX"};   //!> folder name
  StringProperty m_par_strawtextfile{this, "StrawTextFile", ""};    //!> input text file
  BooleanProperty m_par_forcecallback{this, "ForceCallback", false};           //!> force callback to manager
  const TRT_ID* m_trtid{};              //!> id helper
  const InDetDD::TRT_DetectorManager* m_trtman{}; //!> detector manager
  
  PublicToolHandle<IAthenaOutputStreamTool> m_streamer{this, "StreamTool", "AthenaOutputStreamTool/CondStream1"}; //!> OutputStreamTool

};

////////////////////////////////////////////////////////////////////////////////////////////
//  inline methods
////////////////////////////////////////////////////////////////////////////////////////////

inline TRT_StrawAlignDbSvc::StrawDxContainer* TRT_StrawAlignDbSvc::getDxContainer() const {
  const StrawDxContainer* rc = getConstDxContainer();
  return const_cast<StrawDxContainer*>(rc) ; 
}

inline const TRT_StrawAlignDbSvc::StrawDxContainer*
TRT_StrawAlignDbSvc::getConstDxContainer() const
{
  const StrawDxContainer* rc = nullptr;
  if (m_detStore->retrieve (rc, m_par_dxcontainerkey).isFailure()) {
    ATH_MSG_ERROR("Cannot retrieve " << m_par_dxcontainerkey);
  }
  return rc;
}

inline TRTCond::ExpandedIdentifier 
TRT_StrawAlignDbSvc::trtcondid( const Identifier& id, int level) const
{
  return TRTCond::ExpandedIdentifier( m_trtid->barrel_ec(id),m_trtid->layer_or_wheel(id),
				      m_trtid->phi_module(id),m_trtid->straw_layer(id),
				      m_trtid->straw(id),level ) ;
}


inline void
TRT_StrawAlignDbSvc::setDx( const TRTCond::ExpandedIdentifier& id,float dx1, float dx2, float dxerr ) 
{
  //std::cout << " Setting the dx1 to " << dx1 << " dx2 to " << dx2 << std::endl;
  getDxContainer()->setDx( id, dx1, dx2, dxerr); 
}

inline float
TRT_StrawAlignDbSvc::getDx1( const Identifier& id ) const
{
  return getDxContainer()->getDx1( trtcondid(id,TRTCond::ExpandedIdentifier::STRAW) ) ;
}

inline float
TRT_StrawAlignDbSvc::getDx2( const Identifier& id ) const
{
  return getDxContainer()->getDx2( trtcondid(id,TRTCond::ExpandedIdentifier::STRAW) ) ;
}

inline float
TRT_StrawAlignDbSvc::getDxErr( const Identifier& id ) const
{
  return getDxContainer()->getDxErr( trtcondid(id,TRTCond::ExpandedIdentifier::STRAW) ) ;
}

inline void
TRT_StrawAlignDbSvc::setDx( const Identifier& id,float dx1, float dx2, float dxerr ) 
{
  setDx( trtcondid(id,TRTCond::ExpandedIdentifier::STRAW), dx1, dx2, dxerr) ;
}

#endif //  TRT_STRAWALIGNDBSVC_H
