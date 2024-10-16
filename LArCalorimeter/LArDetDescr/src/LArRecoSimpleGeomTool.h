/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ***************************************************************************
//
// 3.8.2005 Creation of the tool by claire.bourdarios@cern.ch
//
// ***************************************************************************

#ifndef LARDETDESCR_LARRECOSIMPLEGEOMTOOL_H
#define LARDETDESCR_LARRECOSIMPLEGEOMTOOL_H

#include "CaloDetDescr/ICaloRecoSimpleGeomTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloGeoHelpers/CaloPhiRange.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"

/**
 *  @class LArRecoSimpleGeomTool
 *  @brief This class implements the CaloDetDescr/ICaloRecoSimpleGeomTool and provides the
 *         simplified geometry needed for the LAr part of the Calorimeter/CaloTrackingGeometry 
 *         volumes.
 **/

class LArRecoSimpleGeomTool : public extends<AthAlgTool, ICaloRecoSimpleGeomTool>
{
public:
  
  // Constructor:
  LArRecoSimpleGeomTool(const std::string& type, 
			const std::string& name,
			const IInterface* parent);

  virtual ~LArRecoSimpleGeomTool() = default;
  virtual StatusCode initialize() override;
  
  /** the enum ALIGNVOL covers all LAr pieces, including cryostats and coil */
  
  virtual
  bool get_cylinder_surface (CaloSubdetNames::ALIGNVOL alvol,
			     Amg::Transform3D& htrans,double& hphi,
			     std::vector<double>& radius,  
			     std::vector<double>& depth, 
			     std::vector<double>& hlength ) const override;
  
  virtual
  bool get_disk_surface (CaloSubdetNames::ALIGNVOL alvol,
			 Amg::Transform3D& htrans,double& hphi, 
			 std::vector<double>& z, 
			 std::vector<double>& depth,
			 std::vector<double>& rmin, std::vector<double>& rmax ) const override;
   
  virtual void print() const override;

private:
  
  bool ScanBarrelCryo(CaloSubdetNames::ALIGNVOL alvol,
		      Amg::Transform3D& htrans,double& hphi,
		      std::vector<double>& radius,  
		      std::vector<double>& depth, 
		      std::vector<double>& hlength );
  
  bool ScanEMB(CaloSubdetNames::ALIGNVOL alvol,
	       Amg::Transform3D& htrans,double& hphi,
	       std::vector<double>& radius,  
	       std::vector<double>& depth, 
	       std::vector<double>& hlength );

  // mapping between ALIGNVOL and strings (need to retrieve StoredPV-s)
  std::string map_av(CaloSubdetNames::ALIGNVOL alvol) const;

  const CaloCell_ID*        m_calo_id = nullptr;
  CaloPhiRange             m_phi_range;

  // Access to DDDb :
  std::string m_geometry;

  IRDBRecordset_ptr m_recCryoCyl;
  IRDBRecordset_ptr m_recPresGeo;
  IRDBRecordset_ptr m_recBarrGeo;
  IRDBRecordset_ptr m_recPresPos;
  IRDBRecordset_ptr m_EmecGeo;
  IRDBRecordset_ptr m_HEC;
};

#endif // LARDETDESCR_LARSIMPLEGEOMTOOL_H






