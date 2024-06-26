/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MYSQL.h"


#include "MuonGeoModel/Technology.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "CxxUtils/checker_macros.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>

namespace MuonGM {

    MYSQL::MYSQL() :
    AthMessaging{"MuonGeoModel.MYSQL"} {}

    MYSQL::~MYSQL() {
        MYSQLPtr& ptr = GetMYSQLPtr();
        std::unique_lock l (ptr.m_mutex);

        // reset the pointer so that at next initialize the MYSQL object will be re-created
        if (ptr.m_ptr == this)
          ptr.m_ptr = nullptr;
    }

    MYSQL::MYSQLPtr& MYSQL::GetMYSQLPtr() {
        static SG::SlotSpecificObj<MYSQLPtr> ptrs ATLAS_THREAD_SAFE;
        const EventContext& ctx = Gaudi::Hive::currentContext();
        if (ctx.slot() == EventContext::INVALID_CONTEXT_ID) {
          EventContext ctx2 (0, 0);
          return *ptrs.get(ctx2);
        }
        return *ptrs.get(ctx);
    }

    MYSQL::LockedMYSQL MYSQL::GetPointer() {
        MYSQLPtr& ptr = GetMYSQLPtr();
        std::unique_lock l (ptr.m_mutex);
        if (!ptr.m_ptr) {
            ptr.m_ptr = new MYSQL;
        }
        return LockedMYSQL (*ptr.m_ptr, std::move(l));
    }

    const Station *MYSQL::GetStation(const std::string& name) const {
        ATH_MSG_VERBOSE( " looking for station " << name );
        StationIterator it = m_stations.find(name);
        if (it != m_stations.end()) {
            ATH_MSG_VERBOSE( "found the station" );            
            return it->second.get();
        }   
        return nullptr;
    }

    Station *MYSQL::GetStation(const std::string& name) {
        ATH_MSG_VERBOSE( " looking for station " << name );
        StationIterator it = m_stations.find(name);
        if (it != m_stations.end()) {
            ATH_MSG_VERBOSE( "found the station" );
            return it->second.get();
        }   
        return nullptr;
    }

    Position MYSQL::GetStationPosition(const std::string& nameType, int fi, int zi) const {
        Position p;
        ATH_MSG_VERBOSE( " MYSQL::GetStationPosition for " << nameType << " fi/zi " << fi << " " << zi );
        int subtype = allocPosFindSubtype(nameType, fi, zi);
        std::string stname = nameType + MuonGM::buildString(subtype, 0);
        const Station *st = GetStation(stname);
        if (st != nullptr) {
            ATH_MSG_VERBOSE( " found in Station " << st->GetName());
            p = (*(st->FindPosition(zi, fi))).second;
                ATH_MSG_VERBOSE( " at p.fi,zi " << p.phiindex << " " << p.zindex << " shift/z " << p.shift << " " << p.z );
            
        } else {
            ATH_MSG_WARNING( "::GetStationPosition nothing found for " << nameType << " at fi/zi " << fi << " " << zi );
        }
        return p;
    }

     GeoModel::TransientConstSharedPtr<TgcReadoutParams> MYSQL::GetTgcRPars(const std::string& name) const {
       ATH_MSG_VERBOSE( "MYSQL::GetTgcRPars looking for a TgcRPars named <" << name << ">" );      
       TgcReadParsIterator it = m_tgcReadouts.find(name);
        if (it != m_tgcReadouts.end()) {
            return it->second;
        }   
        return nullptr;
    }

    GeoModel::TransientConstSharedPtr<TgcReadoutParams> MYSQL::GetTgcRPars(int jsta) const {
        if (jsta - 1 < 0 || jsta >= NTgcReadouts) {
            ATH_MSG_ERROR( "MYSQL::GetTgcRPars jsta = " << jsta << " out of range (0," << NTgcReadouts - 1 << ")" );
            return nullptr;
        }
        return m_tgcReadout[jsta - 1];
    }

    Technology *MYSQL::GetTechnology(const std::string& name) {
        TechnologyIterator it = m_technologies.find(name);
        if (it != m_technologies.end()) {
            ATH_MSG_VERBOSE( "found the station technology name " << name );
            return it->second.get();
        } 
        ATH_MSG_VERBOSE( "MYSQL:: Technology " << name << "+++++++++ not found!" );
        return nullptr;
    }

    const Technology *MYSQL::GetTechnology(const std::string& name) const {
        TechnologyIterator it = m_technologies.find(name);
        if (it != m_technologies.end()) {
            ATH_MSG_VERBOSE( "found the station technology name " << name );
            return it->second.get();
        }      
        ATH_MSG_VERBOSE( "MYSQL:: Technology " << name << "+++++++++ not found!" );
        return nullptr;
    }

    void MYSQL::StoreTechnology( Technology* t) {
        ATH_MSG_VERBOSE( "MYSQL::StoreTechnology /// techn. named " << t->GetName() );
      
        std::unique_ptr<Technology>& stored = m_technologies[t->GetName()];
        if (stored) {
            ATH_MSG_ERROR( "MYSQL::StoreTechnology ERROR /// This place is already taken !!! for " << t->GetName() );
        } else {
           stored.reset(t);
        }
    }

    void MYSQL::StoreStation(Station* s) {
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" name " << s->GetName() );
        m_stations[s->GetName()].reset(s);
    }

    void MYSQL::StoreTgcRPars(GeoModel::TransientConstSharedPtr<TgcReadoutParams> s) {
        ATH_MSG_VERBOSE( "MYSQL::StoreTgcRPars named " << s->GetName() << " located @ " << s << " jsta = " << s->chamberType() );
        if (s->chamberType() >= NTgcReadouts) {
            ATH_MSG_ERROR( "MYSQL::StoreTgcRPars ChamberType(JSTA) " << s->chamberType() << " > NTgcReadouts=" << NTgcReadouts );
            return;
        }
        m_tgcReadout[s->chamberType() - 1] =  s;
    }

    void MYSQL::PrintAllStations() const {
        for (const auto& p : m_stations) {
            ATH_MSG_INFO( "---> Station  " << p.first );
        }
    }

    void MYSQL::PrintTechnologies() {
        for (const auto& p : m_technologies) {
            ATH_MSG_INFO( "---> Technology " << p.first );
        }
    }

    const Technology *MYSQL::GetATechnology(const std::string& name) const {
        TechnologyIterator it = m_technologies.find(name);
        
        if (it != m_technologies.end()) {
            ATH_MSG_VERBOSE( "found the station technology name " << name );
            return it->second.get();
        } 
        ATH_MSG_VERBOSE( "MYSQL:: Technology " << name << "+++++++++ not found!" );
        for (unsigned int i = 1; i <= 20; i++) {
            char chindex[3];
            sprintf(chindex, "%u", i);
            // std::string newname = name.substr(0,3)+chindex;
            std::string newname = name.substr(0, 3) + MuonGM::buildString(i, 2);
            it = m_technologies.find(newname);
            if (it != m_technologies.end()) {
                ATH_MSG_VERBOSE( " Selecting a technology called <" << newname << ">" );
                return it->second.get();
            }
        } 
        return nullptr;
    }

    std::string MYSQL::allocPosBuildKey(const std::string& statType, int fi, int zi) const {
        std::ostringstream mystream;
        mystream << statType << "fi" << MuonGM::buildString(fi, 1) << "zi" << MuonGM::buildString(zi, -1);
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" from " << statType << " fi " << fi << " zi " << zi << " we get as key " << mystream.str() );
        return mystream.str();
    }

    allocPosIterator MYSQL::allocPosFind(const std::string& statType, int fi, int zi) const {
        std::string key = allocPosBuildKey(statType, fi, zi);
        return allocPosFind(key);
    }

    int MYSQL::allocPosFindSubtype(const std::string& statType, int fi, int zi) const {
        std::string key = allocPosBuildKey(statType, fi, zi);
        return allocPosFindSubtype(key);
    }

    int MYSQL::allocPosFindCutout(const std::string& statType, int fi, int zi) const {
        std::string key = allocPosBuildKey(statType, fi, zi);
        return allocPosFindCutout(key);
    }

    void MYSQL::addallocPos(const std::string& statType, int fi, int zi, int subtyp, int cutout) {
        std::string key = allocPosBuildKey(statType, fi, zi);
        addallocPos(key, subtyp, cutout);
    }
        void MYSQL::addAllocpos(int i, const std::string& str) { m_allocatedpos[i] = str; }

    AllocposIterator MYSQL::AllocposEnd() const { return m_allocatedpos.end(); }

    AllocposIterator MYSQL::AllocposBegin() const { return m_allocatedpos.cbegin(); }

    AllocposIterator MYSQL::AllocposFind(int i) const { return m_allocatedpos.find(i); }

    std::string MYSQL::AllocposFindName(int i) const {
        AllocposIterator it = m_allocatedpos.find(i);
        // imt fix in case key is wrong:
        if (it == m_allocatedpos.end()) {
           throw std::runtime_error("AllocPosFIndName() -- Bad key");
        }   
        return (*it).second;
    }
    //
    const MYSQL::StationMap& MYSQL::stationMap() const { return m_stations;}
    const MYSQL::TgcReadParsMap& MYSQL::tgcReadParsMap() const { return  m_tgcReadouts;}   

    int MYSQL::NStations() const { return m_stations.size(); }

    int MYSQL::NTgcReadTypes() const { return m_tgcReadouts.size(); }

    int MYSQL::allocPosBuildValue(int subtype, int cutout) const { return 100 * subtype + cutout; }

    allocPosIterator MYSQL::allocPosBegin() const { return m_allocPos.begin(); }

    allocPosIterator MYSQL::allocPosEnd() const { return m_allocPos.end(); }

    allocPosIterator MYSQL::allocPosFind(const std::string& key) const { return m_allocPos.find(key); }

    int MYSQL::allocPosFindSubtype(allocPosIterator it) const {
        int value = it->second;
        int subtype = static_cast<int>(value / 100);
        return subtype;
    }

    int MYSQL::allocPosFindCutout(allocPosIterator it) const {
        int value = (*it).second;
        int cutout = static_cast<int>(value % 100);
        return cutout;
    }

    void MYSQL::addallocPos(const std::string& key, int value) { m_allocPos[key] = value; }

    void MYSQL::addallocPos(const std::string& key, int subtype, int cutout) { m_allocPos[key] = allocPosBuildValue(subtype, cutout); }

    std::string MYSQL::getGeometryVersion() const { return m_geometry_version; }

    int MYSQL::getNovaReadVersion() const { return m_amdb_version; }

    std::string MYSQL::getLayoutName() const { return m_layout_name; }

    int MYSQL::getNovaVersion() const { return m_nova_version; }

    bool MYSQL::amdb_from_RDB() const { return m_amdb_from_rdb; }

    void MYSQL::set_amdb_from_RDB(bool val) { m_amdb_from_rdb = val; }


    void MYSQL::setGeometryVersion(const std::string& s) {
        

        if (m_geometry_version != "unknown") {
            if (s == m_geometry_version)
                return;
            ATH_MSG_WARNING( "GeometryVersion already set to  <" << m_geometry_version << ">"
                            << " resetting to <" << s << ">" );
        }
        m_geometry_version = s;
       ATH_MSG_INFO( "GeometryVersion set to <" << m_geometry_version << ">" );
    }

    void MYSQL::setNovaReadVersion(int i) {
        m_amdb_version = i;
       ATH_MSG_VERBOSE("setNovaReadVersion to " << m_amdb_version );
    }

    void MYSQL::setLayoutName(const std::string& s) {
        if (m_layout_name != "unknown") {
            if (s == m_layout_name)
                return;
            ATH_MSG_WARNING( "LayoutName already set to  <" << m_layout_name << ">"
                << " resetting to <" << s << ">" );
        }
        m_layout_name = s;
       ATH_MSG_INFO( "LayoutName (from DBAM) set to <" << m_layout_name << ">  -- relevant for CTB2004" );
    }

    void MYSQL::setNovaVersion(int i) {
        m_nova_version = i;
        ATH_MSG_VERBOSE("setNovaVersion to " << m_nova_version );
    }

    int MYSQL::allocPosFindCutout(const std::string& key) const {
        int cutout = 0;
        allocPosIterator it = m_allocPos.find(key);
        if (it != allocPosEnd()) {
            return allocPosFindCutout(it);
        }
        ATH_MSG_ERROR("MYSQL::allocPosFindCutout for key  " << key << " no element found" );
        return cutout;
    }

    int MYSQL::allocPosFindSubtype(const std::string& key) const {
        int subtype = 0;
        allocPosIterator it = m_allocPos.find(key);
        if (it != allocPosEnd()) {
            return allocPosFindSubtype(it);
        }
        ATH_MSG_ERROR("MYSQL::allocPosFindSubtype for key  " << key << " no element found" );
        return subtype;
    }

} // namespace MuonGM
