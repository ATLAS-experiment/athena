/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITRT_STRAWALIGNDBSVC_H
#define ITRT_STRAWALIGNDBSVC_H
/** @file ITRT_StrawAlignDbSvc.h
 * @brief abstract interface to TRT straw alignment constants
 * @author Peter Hansen <phansen@nbi.dk>
 */

#include <string>
#include "GaudiKernel/IService.h"
#include "AthenaKernel/IOVSvcDefs.h"

class Identifier;
namespace TRTCond {
  class ExpandedIdentifier;
  class RtRelation;
  class StrawDxContainer;
}
class  IAthenaOutputStreamTool ;

/** @class ITRT_StrawAlignDbSvc
  abstract interface to TRT straw alignment constants
*/
class ITRT_StrawAlignDbSvc: virtual public IService
{
 public:
  typedef TRTCond::StrawDxContainer StrawDxContainer ;
  DeclareInterfaceID(ITRT_StrawAlignDbSvc,1,0);

  virtual TRTCond::ExpandedIdentifier trtcondid( const Identifier&, int) const =0;
  virtual void setDx( const TRTCond::ExpandedIdentifier&, float, float, float ) =0;
  virtual StatusCode writeTextFile(const std::string&) const =0;
  virtual StatusCode readTextFile(const std::string&) =0;
  virtual StatusCode streamOutObjects () const =0;
  virtual StatusCode registerObjects (std::string, int, int, int, int) const=0;
  virtual StrawDxContainer* getDxContainer() const =0;
  virtual StatusCode IOVCallBack(IOVSVC_CALLBACK_ARGS) =0;
  virtual float getDx1( const Identifier&) const =0;
  virtual float getDx2( const Identifier&) const =0;
  virtual float getDxErr( const Identifier& ) const =0 ;
  virtual void setDx( const Identifier&, float, float, float) =0;
  
};

#endif //  ITRT_STRAWALIGNDBTOOL_H
