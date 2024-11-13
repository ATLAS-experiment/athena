/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_GEOMODEL_TRT_DETDESCRDB_PARAMETERINTERFACE_H
#define TRT_GEOMODEL_TRT_DETDESCRDB_PARAMETERINTERFACE_H

#include "TRTParameterInterface.h"
#include "InDetGeoModelUtils/InDetDDAthenaComps.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"

class TopLevelPlacements;



class TRT_DetDescrDB_ParameterInterface : public TRTParameterInterface {
public:
  // Only allowed constructor
  TRT_DetDescrDB_ParameterInterface(InDetDD::AthenaComps * athenaComps);
  ~TRT_DetDescrDB_ParameterInterface();

  TRT_DetDescrDB_ParameterInterface() = delete;
  TRT_DetDescrDB_ParameterInterface(const TRT_DetDescrDB_ParameterInterface &) = delete;
  TRT_DetDescrDB_ParameterInterface & operator=(const TRT_DetDescrDB_ParameterInterface &) = delete;

  virtual const GeoTrf::Transform3D & partTransform(const std::string & partName) const override;
  virtual bool partPresent(const std::string & partName) const override;
  virtual InDetDD::DistortedMaterialManager * distortedMatManager() override;
  virtual void SetValues() override;
  
  IRDBRecordset_ptr  scalingTable() const {return m_scalingTable;}
  MsgStream& msg (MSG::Level lvl) { return m_athenaComps->msg(lvl); }
  
private:
  InDetDD::AthenaComps * m_athenaComps{nullptr};
  InDetDD::DistortedMaterialManager * m_distortedMatManager{nullptr};
  TopLevelPlacements * m_placements{nullptr};
  IRDBRecordset_ptr m_scalingTable;
};

#endif
