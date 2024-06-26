/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MYSQL_H
#define MYSQL_H

#include "AthenaBaseComps/AthMessaging.h"

#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "MuonGeoModel/Position.h"
#include "MuonGeoModel/Station.h"
#include "MuonGeoModel/Technology.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"


#include "CxxUtils/LockedPointer.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

#include <map>
#include <string>
#include <mutex>

/*
  This class holds an std::map of stations* (key = stationName, i.e. BMS5),
  an std::map of Technologies* (key = RPC20), and an std::map of TgcReadoutParams*.
  Stations and Technologies are used only to build the geometry (can be deleted
  after that).
  TgcReadoutParams are used by the TgcReadoutElements -> must live forever
  (they belong to the readout geometry).
  MYSQL is used only to build the geometry - can be deleted as soon as the job is
  done (in the Factory).
  It is responsible for releasing the memory allocated by these objects.

  This is not thread safe!
*/

namespace MuonGM {
    
    typedef std::map<int, std::string>::const_iterator AllocposIterator;
    typedef std::map<std::string, int>::const_iterator allocPosIterator;

    class MYSQL: public AthMessaging {
      public:
        enum TgcReadoutRange { NTgcReadouts = 30 };

        using LockedMYSQL = CxxUtils::LockedPointer<MYSQL>;
        using StationMap = std::map<std::string, std::unique_ptr<Station> >;
        using StationIterator = StationMap::const_iterator;

        using TgcReadParsMap = std::map<std::string, GeoModel::TransientConstSharedPtr<TgcReadoutParams>>;
        using TgcReadParsIterator = TgcReadParsMap::const_iterator;

        using TechnologyMap = std::map<std::string, std::unique_ptr<Technology>>;
        using TechnologyIterator = TechnologyMap::const_iterator;
  
        ~MYSQL();
         bool amdb_from_RDB() const;
         void set_amdb_from_RDB(bool);
         void setGeometryVersion(const std::string& s);
         std::string getGeometryVersion() const;
         void setLayoutName(const std::string& s);
         std::string getLayoutName() const;
         void setNovaVersion(int i);
         int getNovaVersion() const;
         void setNovaReadVersion(int i);
         int getNovaReadVersion() const;

        const StationMap& stationMap() const;
        const TgcReadParsMap& tgcReadParsMap() const;

        AllocposIterator AllocposBegin() const;
        AllocposIterator AllocposEnd() const;
        AllocposIterator AllocposFind(int) const;
        std::string AllocposFindName(int) const;
        void addAllocpos(int i, const std::string& str);
        // the new ones
        std::string allocPosBuildKey(const std::string& statType, int fi, int zi) const;
        int allocPosBuildValue(int subtype, int cutout) const;
        allocPosIterator allocPosBegin() const;
        allocPosIterator allocPosEnd() const;
        allocPosIterator allocPosFind(const std::string& key) const;
        allocPosIterator allocPosFind(const std::string& statType, int fi, int zi) const;
        int allocPosFindSubtype(const std::string& statType, int fi, int zi) const;
        int allocPosFindSubtype(const std::string& key) const;
        int allocPosFindSubtype(allocPosIterator it) const;
        int allocPosFindCutout(const std::string& statType, int fi, int zi) const;
        int allocPosFindCutout(const std::string& key) const;
        int allocPosFindCutout(allocPosIterator it) const;
        
        void addallocPos(const std::string& key, int value);
        void addallocPos(const std::string& statType, int fi, int zi, int subtyp, int cutout);
        void addallocPos(const std::string& key, int subtype, int cutout);

        int NStations() const;
        int NTgcReadTypes() const;

        static LockedMYSQL GetPointer();
        const Station* GetStation(const std::string& name) const;
        Station *GetStation(const std::string& name);
        Position GetStationPosition(const std::string& nameType, int fi, int zi) const;
        GeoModel::TransientConstSharedPtr<TgcReadoutParams> GetTgcRPars(const std::string& name) const;
        GeoModel::TransientConstSharedPtr<TgcReadoutParams> GetTgcRPars(int i) const;
        
        void StoreStation(Station* s);
        void PrintAllStations() const;
        void StoreTechnology(Technology* t);
        void StoreTgcRPars(GeoModel::TransientConstSharedPtr<TgcReadoutParams> t);
        Technology *GetTechnology(const std::string& name);
        const Technology *GetTechnology(const std::string& name) const;
        const Technology *GetATechnology(const std::string& name) const;
        void PrintTechnologies();

        // singleton
      private:
        // Protects s_thePointer.
        struct MYSQLPtr
        {
          std::recursive_mutex m_mutex;
          MYSQL* m_ptr{nullptr};
        };
        static MYSQLPtr& GetMYSQLPtr();

        MYSQL();
        std::map<int, std::string> m_allocatedpos;
        std::map<std::string, int> m_allocPos;
        StationMap m_stations{};
        TechnologyMap m_technologies{};
        TgcReadParsMap m_tgcReadouts{};
        std::array<GeoModel::TransientConstSharedPtr<TgcReadoutParams>, NTgcReadouts> m_tgcReadout{};
       

        std::string m_geometry_version{"unknown"}; // from our job-option
        std::string m_layout_name{"unknown"};      // from nova
        std::string m_DBMuonVersion{"unknown"};    // name of the MuonVersion table-collection in Oracle
        int m_nova_version{0};
        int m_amdb_version{0};
        bool m_amdb_from_rdb{false};
    };



} // namespace MuonGM

#endif
