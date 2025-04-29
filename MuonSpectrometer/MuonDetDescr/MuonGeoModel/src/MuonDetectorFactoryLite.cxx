/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MuonDetectorFactoryLite.h"
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPerfUtils.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "MuonGeoModel/MYSQL.h"
#include "MuonGeoModel/MuonChamberLite.h"
#include "MuonGeoModel/RDBReaderAtlas.h"
#include "MuonGeoModel/Station.h"

#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "StoreGate/StoreGateSvc.h"

namespace MuonGM {

  // Helper function to retrieve ID Helpers:
  template <class T> 
  const T  *getIdHelper (StoreGateSvc *pDetStore, const std::string & helperName) {
    const T *helper;
    StatusCode sc = pDetStore->retrieve(helper, helperName);
    if (sc.isFailure()) {
      return nullptr;
    }	    
    else {
      return helper;
    }
  };
  



  MuonDetectorFactoryLite::MuonDetectorFactoryLite(StoreGateSvc *pDetStore, GeoModelIO::ReadGeoModel *sqliteReader):
    m_pDetStore{pDetStore},
    m_sqliteReader(sqliteReader) {
    }

  MuonDetectorFactoryLite::~MuonDetectorFactoryLite() =default;

  const MuonDetectorManager *MuonDetectorFactoryLite::getDetectorManager() const { return m_manager; }
  MuonDetectorManager *MuonDetectorFactoryLite::getDetectorManager() { return m_manager; }

  void MuonDetectorFactoryLite::create(GeoPhysVol *) {

    std::map<std::string, GeoAlignableTransform*> mapAXF = m_sqliteReader->getPublishedNodes<std::string, GeoAlignableTransform*>("MuonSys");                     
    std::map<std::string, GeoFullPhysVol*>        mapFPV = m_sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("MuonSys");                                            

    MsgStream log(Athena::getMessageSvc(), "MuGM:MuonFactory");

    if (!m_manager)  m_manager = new  MuonDetectorManager();





    // Iterate using iterator in for loop
    for (const auto& [key, pV] : mapFPV) {
      if (key.starts_with ("sMD") || key.starts_with ("sTG")) {
        int /*index1=key[3]-'0',*/ eta=key[5]-'0', ml=key[7]-'0', phi=key[12]-'0';
      
        char AC=key[13];
        int ec = AC=='C' ? -1 : 1;
        std::string vName = pV->getLogVol()->getName();
        if (key.starts_with("sMD")) {
          std::string sName = vName.substr(4,4);
          std::unique_ptr<MuonGM::MMReadoutElement> re = std::make_unique<MuonGM::MMReadoutElement>(pV, sName, ec*eta,phi,ml,m_manager,nullptr);
          re->initDesign();
          m_manager->addMMReadoutElement(std::move(re));
        }
        else if (key.starts_with("sTG")) {
          std::string sName = vName.substr(7,4);
          std::unique_ptr<sTgcReadoutElement> re = std::make_unique<sTgcReadoutElement>(pV, sName, ec*eta, phi, ml, m_manager);
          re->initDesign(2.6);
          m_manager->addsTgcReadoutElement(std::move(re));
        }
      }
    }



    // here create the MYSQL singleton
    MYSQL::LockedMYSQL mysql = MYSQL::GetPointer();
    mysql->set_amdb_from_RDB(true);

    std::unique_ptr<RDBReaderAtlas> dbr;
    {        
      // We never alter these in the "Lite" workflow
      std::string oracleTag="";
      std::string oracleNode="";
      std::map<std::string, std::string> ascii{};
      dbr = std::make_unique<RDBReaderAtlas>(m_pDetStore, m_pRDBAccess, oracleTag, oracleNode,  ascii);
	  

    }



    // set here the flag deciding whether to include cutouts:
    // m_includeCutouts = 1 => include cutouts
    // m_includeCutouts = 0 => no cutouts
    m_manager->setCutoutsFlag(true);
    m_manager->setCutoutsBogFlag(true);


    StatusCode sc = StatusCode::SUCCESS;

    dbr->setManager(getDetectorManager());
    sc = dbr->ProcessDB(*mysql);
    if (sc != StatusCode::SUCCESS) {
      log << MSG::ERROR << " FAILURE in DB access; Muon node will not be built" << endmsg;
      return;
    }

    GeoFullPhysVol *p4 = mapFPV["MuonTreeTop"];
    m_manager->addTreeTop(p4); // This is the top!
    log << MSG::INFO << " TreeTop added to the Manager" << endmsg;

    // Look through the map explicitly for Stations
    std::set<std::string> mappedStations;
    for (const auto& i: mapFPV) {
      size_t pos=i.first.find("_Station");
      if (pos!=std::string::npos) mappedStations.insert(i.first.substr(0,pos));
    }

    // Boudreau Jan 2024 Keep track of instantiated readout geometries. This is
    // needed because the muon geometry is rebuild multiple times in current 
    // implementation. 
    std::set<std::string> keyset; 

    for (const auto& i: mappedStations) {
      auto it= mysql->stationMap().find(i);
      if (it==mysql->stationMap().end()) {
	        throw std::runtime_error("Raw/readout geometry mismatch");
      }

      Station *station = (*it).second.get();
      std::string stname(station->GetName(), 0, 3);
  
      bool isAssembly = false;
      if (station->GetNrOfCutouts() > 0 && stname.substr(0, 1) != "T")
	isAssembly = true;

      // BIR have multilayers of diff. length and overall station volume clashes with toroids
      if (stname == "BIR")
	isAssembly = true;

      MuonChamberLite l(*mysql, station,&mapFPV,&mapAXF); // here is where we start to create a MuonChamber with all readoutelements
      l.setKeyset(&keyset);

      PositionIterator pit;
      AlignPos ap;
      for ( auto const& [key, position] : *station) {
	int zi = position.zindex;
	int fi = position.phiindex;

	// here build the physical volume (tree) associated to the chamber
	if (stname == "CSL")  isAssembly = true;

	// CSL because coffin shape of the station mother volume
	l.addReadoutLayers(*mysql, getDetectorManager(), zi, fi, position.isMirrored, isAssembly);

	// here define the GeoAlignableTransform associated to the chamber
	// nominal transform first
	GeoAlignableTransform *xf = mapAXF[station->GetName()+"_Station_"+std::to_string(zi)+"_"+std::to_string(fi)];
	// alignment issues and readout geometry for station
	MuonStation *mst = m_manager->getMuonStation(station->GetName(), zi, fi + 1);
	if (!mst) {
	  log << MSG::WARNING << "For Station with nameTag=<" << station->GetName() << "> at zi/fi = " << zi << "/" << fi
	      << " no MuonStation found => no possibility to align" << endmsg;
	  continue;
	}
	mst->setTransform(xf);
	GeoTrf::Transform3D tsz_to_szt = GeoTrf::RotateZ3D(-90 * Gaudi::Units::degree) * GeoTrf::RotateY3D(-90 * Gaudi::Units::degree);
	GeoTrf::Transform3D   nativeToAmdbLRS=tsz_to_szt * station->native_to_tsz_frame(*mysql, position); 
	mst->setNativeToAmdbLRS(nativeToAmdbLRS);
	mst->setNominalAmdbLRSToGlobal(station->tsz_to_global_frame(*mysql, position) * tsz_to_szt.inverse());

	int nAlines = station->CountAlignPos(zi, fi);
	if (nAlines == 0) {
	  ap.tras = ap.traz = ap.trat = ap.rots = ap.rotz = ap.rott = 0.;
	  ap.jobindex = 0;
	  mst->setDelta_fromAline(ap.tras, ap.traz, ap.trat, ap.rots, ap.rotz, ap.rott);
	} else if (nAlines > 0) {
	  AlignPosIterator alast;
	  AlignPosIterator afirst = station->getFirstAlignPosInRange(zi, fi, alast);

	  for (AlignPosIterator acurrent = afirst; acurrent != alast; ++acurrent) {
	    ap = acurrent->second;
	    if (ap.phiindex != fi || ap.zindex != zi) {
	      log << MSG::ERROR << "Inconsistent AlignedPosition found in the static Geometry DB: aligPos.fi, zi = " << ap.phiindex << ", " << ap.zindex
		  << " for station " << station->GetName() << " at fi/zi = " << fi << "/" << zi << " AlignPos indices fi/zi " << fi << "/" << zi << endmsg;
	    }
	    GeoTrf::Transform3D tsz_to_szt = GeoTrf::RotateZ3D(-90 * Gaudi::Units::degree) * GeoTrf::RotateY3D(-90 * Gaudi::Units::degree);
	    // Keep, for later developments.	    GeoTrf::Transform3D nominalTransform=station->getNominalTransform(*mysql, position);
	    // Keep, for later developments.        GeoTrf::Transform3D native_to_amdbl=tsz_to_szt * station->native_to_tsz_frame(*mysql, position);
	    mst->setTransform(xf);
	    mst->setNativeToAmdbLRS(tsz_to_szt * station->native_to_tsz_frame(*mysql, position)); 
	    mst->setNominalAmdbLRSToGlobal(station->tsz_to_global_frame(*mysql, position) * tsz_to_szt.inverse()); 
	    mst->setDeltaAmdbLRS(GeoTrf::Transform3D::Identity());
	  }
	}
      } // end loop on positions
    } // for ( it = sel.begin(); it != sel.end(); it++ ) {
    delete mysql.get();
  } // MuonDetectorFactoryLite::create

} // namespace MuonGM
