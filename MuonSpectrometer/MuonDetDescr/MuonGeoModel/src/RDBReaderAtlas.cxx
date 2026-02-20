/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/RDBReaderAtlas.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "MuonGeoModel/MdtComponent.h"
#include "MuonGeoModel/StationSelector.h"
#include "MuonGeoModel/TGC_Technology.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "CxxUtils/ArrayHelper.h"

namespace MuonGM {

    using GasGapIntArray = TgcReadoutParams::GasGapIntArray;
    using GasGapFloatArray = TgcReadoutParams::GasGapFloatArray;
    using WiregangArray = TgcReadoutParams::WiregangArray;
    using StripArray = TgcReadoutParams::StripArray;
        
    RDBReaderAtlas::RDBReaderAtlas(StoreGateSvc *pDetStore, IRDBAccessSvc *pRDBAccess, const std::string& geoTag, const std::string& geoNode,
                                    const std::map<std::string, std::string>& asciiFileDBMap):
         DBReader(pDetStore), AthMessaging{"MuGM:RDBReadAtlas"},
            m_geoTag(geoTag), m_geoNode(geoNode), m_pRDBAccess(pRDBAccess) {
        m_SCdbaccess = StatusCode::FAILURE;

      
        ATH_MSG_INFO("Start retrieving dbObjects with tag = <" << geoTag << "> node <" << geoNode << ">");
        
        // here putting RDB data in private "objects" form       
        m_dhatyp = std::make_unique<DblQ00Atyp>(m_pRDBAccess, geoTag, geoNode);        
        m_atyp = m_dhatyp->data();

       
        m_dhasmp = std::make_unique<DblQ00Asmp>(m_pRDBAccess, geoTag, geoNode);      
        m_asmp = m_dhasmp->data();

       
        m_dhalmn = std::make_unique<DblQ00Almn>(m_pRDBAccess, geoTag, geoNode); 
        m_almn = m_dhalmn->data();

        
        m_dhaptp = std::make_unique<DblQ00Aptp>(m_pRDBAccess, geoTag, geoNode);   
        m_aptp = m_dhaptp->data();

       
        m_dhacut = std::make_unique<DblQ00Acut>(m_pRDBAccess, geoTag, geoNode);
        m_acut = m_dhacut->data();

        m_dhalin = std::make_unique<DblQ00Alin>(m_pRDBAccess, geoTag, geoNode);
        m_alin = m_dhalin->data();

        m_dhdbam = std::make_unique<DblQ00Dbam>(m_pRDBAccess, geoTag, geoNode);
        m_dbam = m_dhdbam->data();

       
        m_dhwrpc = std::make_unique<DblQ00Awln>(m_pRDBAccess, geoTag, geoNode);
        m_wrpc = m_dhwrpc->data();

        m_dhwtgc = std::make_unique<DblQ00Atln>(m_pRDBAccess, geoTag, geoNode);
        m_wtgc = m_dhwtgc->data();

        m_dhwmdt = std::make_unique<DblQ00Wmdt>(m_pRDBAccess, geoTag, geoNode);
        m_wmdt = m_dhwmdt->data();

        m_dhwcsc = std::make_unique<DblQ00Wcsc>(m_pRDBAccess, geoTag, geoNode); 
        m_wcsc = m_dhwcsc->data();

      
        m_dhwrpcall = std::make_unique<DblQ00Wrpc>(m_pRDBAccess, geoTag, geoNode);   
        m_wrpcall = m_dhwrpcall->data();

        m_dhwtgcall = std::make_unique<DblQ00Wtgc>(m_pRDBAccess, geoTag, geoNode); 
        m_wtgcall = m_dhwtgcall->data();

        m_dhwspa = std::make_unique<DblQ00Wspa>(m_pRDBAccess, geoTag, geoNode);
        m_wspa = m_dhwspa->data();

       
        m_dhwded = std::make_unique<DblQ00Wded>(m_pRDBAccess, geoTag, geoNode);
        m_wded = m_dhwded->data();

       
        m_dhwsup = std::make_unique<DblQ00Wsup>(m_pRDBAccess, geoTag, geoNode);   
        m_wsup = m_dhwsup->data();

        if (m_pRDBAccess->getRecordsetPtr("XtomoData", geoTag, geoNode)->size()!=0) {
            m_dhxtomo= std::make_unique<DblQ00Xtomo>(pRDBAccess, geoTag, geoNode);
            ATH_MSG_INFO("XtomoData table found in Oracle");
        } else {
            m_dhxtomo= std::make_unique<DblQ00Xtomo>();
            ATH_MSG_INFO("No XtomoData table in Oracle");
        }
        
        if (m_dhxtomo)
            m_xtomo = m_dhxtomo->data();

        // ASZT
        if (asciiFileDBMap.find("ASZT") != asciiFileDBMap.end()) {
            ATH_MSG_INFO( "getting aszt from ascii file - named <" << asciiFileDBMap.find("ASZT")->second << ">");
            ATH_MSG_INFO( "Ascii aszt input has priority over A-lines in ORACLE; A-lines from Oracle will not be read");
            // dbdata = 0;
            m_dhaszt = std::make_unique<DblQ00Aszt>(asciiFileDBMap.find("ASZT")->second);
            if (m_dhaszt->size() == 0) {
                ATH_MSG_ERROR("Couldn't read ASZT from ascii file!");
            } else {
                ATH_MSG_INFO("N. of lines read = " << m_dhaszt->size());
            }
        }

        if (!m_dhaszt || m_dhaszt->size() == 0) {
            ATH_MSG_INFO( "No Ascii aszt input found: looking for A-lines in ORACLE");
            if (m_pRDBAccess->getRecordsetPtr("ASZT",geoTag,geoNode)->size()==0) {
                m_dhaszt = std::make_unique<DblQ00Aszt>();
                ATH_MSG_INFO("No ASZT table in Oracle");
            } else {
                ATH_MSG_INFO("ASZT table found in Oracle");
                m_dhaszt = std::make_unique<DblQ00Aszt>(m_pRDBAccess, geoTag, geoNode);   
                ATH_MSG_INFO("ASZT size is " << m_dhaszt->size());
            }
        } else {
            ATH_MSG_INFO( "ASZT table in Oracle, if any, will not be read" );
        }
        if (m_dhaszt)
            m_aszt = m_dhaszt->data();

        // Internal CSC Alignment parameters
        if (asciiFileDBMap.find("IACSC") != asciiFileDBMap.end()) {

            ATH_MSG_INFO( "getting iacsc from ascii file - named <" << asciiFileDBMap.find("IACSC")->second << ">" );
            ATH_MSG_INFO( "Ascii iacsc input has priority over A-lines in ORACLE; A-lines from Oracle will not be read" );
            // dbdata = 0;
            m_dhiacsc = std::make_unique<DblQ00IAcsc>(asciiFileDBMap.find("IACSC")->second);
            if (m_dhiacsc->size() == 0) {
                ATH_MSG_ERROR( "Couldn't read IACSC from ascii file!" );
            } else {
                ATH_MSG_INFO( "N. of lines read = " << m_dhiacsc->size() );
            }
        }
        if (!m_dhiacsc || m_dhiacsc->size() == 0) {
            ATH_MSG_INFO( "No Ascii iacsc input found: looking for A-lines in ORACLE" );
            if (m_pRDBAccess->getRecordsetPtr("IZST", geoTag,geoNode)->size()==0) {
                m_dhiacsc = std::make_unique<DblQ00IAcsc>();
                ATH_MSG_INFO( "No ISZT table in Oracle" );
            } else {
                ATH_MSG_INFO( "ISZT table found in Oracle" );
                m_dhiacsc = std::make_unique<DblQ00IAcsc>(m_pRDBAccess, geoTag, geoNode);                   
            }            
        } else {
            ATH_MSG_INFO( "ISZT table in Oracle, if any, will not be read" );
        }
        if (m_dhiacsc)
            m_iacsc = m_dhiacsc->data();


        m_dhwchv = std::make_unique<DblQ00Wchv>(m_pRDBAccess, geoTag, geoNode);   
        m_wchv = m_dhwchv->data();

        m_dhwcro = std::make_unique<DblQ00Wcro>(m_pRDBAccess, geoTag, geoNode); 
        m_wcro = m_dhwcro->data();

        m_dhwcmi = std::make_unique<DblQ00Wcmi>(m_pRDBAccess, geoTag, geoNode);
        m_wcmi = m_dhwcmi->data();

       
        m_dhwlbi = std::make_unique<DblQ00Wlbi>(m_pRDBAccess, geoTag, geoNode);
        m_wlbi = m_dhwlbi->data();

        // everything fetched
        m_SCdbaccess = StatusCode::SUCCESS;
        ATH_MSG_INFO( "Access granted for all dbObjects needed by muon detectors" );
    }

    StatusCode RDBReaderAtlas::ProcessDB(MYSQL& mysql) {
        // Check access to the database (from the constructor)
        if (m_SCdbaccess == StatusCode::FAILURE) {
            return m_SCdbaccess;
        }

        // set GeometryVersion in MYSQL
        mysql.setGeometryVersion(getGeometryVersion());
        // set LayoutName read from amdb
        mysql.setLayoutName(m_dbam[0].amdb);
        // set NovaVersion     in MYSQL
        mysql.setNovaVersion(m_dbam[0].version);
        // set AmdbVersion     in MYSQL
        mysql.setNovaReadVersion(m_dbam[0].nvrs);

        // Process Stations and components
        MuonGM::ProcessStations(mysql, m_dhalmn.get(), m_almn, m_dhatyp.get(), m_atyp, m_dhwmdt.get(), m_wmdt);

        // Process Technologies
        ProcessTechnologies(mysql);

        // Process Positions
        MuonGM::ProcessPositions(mysql, m_dhaptp.get(), m_aptp);

        // Process Cutouts
        if (getGeometryVersion().substr(0, 1) != "P") {
            MuonGM::ProcessCutouts(mysql, m_dhacut.get(), m_acut, m_dhalin.get(), m_alin, m_dhatyp.get(), m_atyp);
        }

        // Process Alignements
        if (m_dhaszt && m_dhaszt->size() > 0) {
            MuonGM::ProcessAlignements(mysql, m_dhaszt.get(), m_aszt);
        }

        // Process TgcReadout
        RDBReaderAtlas::ProcessTGCreadout(mysql);

        //
        ATH_MSG_INFO( "Intermediate Objects built from primary numbers" );

        return m_SCdbaccess;
    }

    void RDBReaderAtlas::ProcessTechnologies(MYSQL& mysql) {
        // here loop over station-components to init technologies at each new entry
        std::vector<std::string> slist;
        slist.emplace_back("*");
        StationSelector sel(mysql, slist);
        StationSelector::StationIterator it;
        ATH_MSG_DEBUG( " from RDBReaderAtlas --- start " );

        bool have_spa_details = (getGeometryVersion().substr(0, 1) != "P");

        for (it = sel.begin(); it != sel.end(); ++it) {
            Station *station = (*it).second;
            for (int ic = 0; ic < station->GetNrOfComponents(); ic++) {
                Component *c = station->GetComponent(ic);
                if (c == nullptr)
                    continue;
                const std::string &cname = c->name;

                if (cname.compare(0, 3, "CSC") == 0)
                    MuonGM::ProcessCSC(mysql, m_dhwcsc.get(), m_wcsc, cname);
                else if (cname.compare(0, 3, "MDT") == 0)
                    MuonGM::ProcessMDT(mysql, m_dhwmdt.get(), m_wmdt, cname);
                else if (cname.compare(0, 3, "RPC") == 0)
                    MuonGM::ProcessRPC(mysql, m_dhwrpc.get(), m_wrpc, m_dhwrpcall.get(), m_wrpcall, cname);
                else if (cname.compare(0, 3, "TGC") == 0)
                    MuonGM::ProcessTGC(mysql, m_dhwtgc.get(), m_wtgc, m_dhwtgcall.get(), m_wtgcall, cname);
                else if (cname.compare(0, 3, "SPA") == 0)
                    MuonGM::ProcessSPA(mysql, m_dhwspa.get(), m_wspa, cname);
                else if (cname.compare(0, 3, "DED") == 0)
                    MuonGM::ProcessDED(mysql, m_dhwded.get(), m_wded, cname);
                else if (cname.compare(0, 3, "SUP") == 0)
                    MuonGM::ProcessSUP(mysql, m_dhwsup.get(), m_wsup, cname);
                else if (cname.compare(0, 3, "CHV") == 0 && have_spa_details)
                    MuonGM::ProcessCHV(mysql, m_dhwchv.get(), m_wchv, cname);
                else if (cname.compare(0, 3, "CRO") == 0 && have_spa_details)
                    MuonGM::ProcessCRO(mysql, m_dhwcro.get(), m_wcro, cname);
                else if (cname.compare(0, 3, "CMI") == 0 && have_spa_details)
                    MuonGM::ProcessCMI(mysql, m_dhwcmi.get(), m_wcmi, cname);
                else if (cname.compare(0, 2, "LB") == 0 && have_spa_details)
                    MuonGM::ProcessLBI(mysql, m_dhwlbi.get(), m_wlbi, cname);
            }
        }

        ATH_MSG_INFO( "nMDT " << nmdt << " nCSC " << ncsc << " nTGC " << ntgc << " nRPC " << nrpc );
        ATH_MSG_INFO( "nDED " << nded << " nSUP " << nsup << " nSPA " << nspa );
        ATH_MSG_INFO( "nCHV " << nchv << " nCRO " << ncro << " nCMI " << ncmi << " nLBI " << nlbi );
    }

    void RDBReaderAtlas::ProcessTGCreadout(MYSQL& mysql) {
        //
        // in case of layout Q and following
        //
        IRDBRecordset_ptr ggln = m_pRDBAccess->getRecordsetPtr("GGLN", m_geoTag, m_geoNode);

        int version(0);
        float wirespacing(0);
        unsigned int gglnSize(0);
        if (ggln)
            gglnSize = ggln->size();
        else {
            ATH_MSG_WARNING(" ProcessTGCreadout - IRDBRecordset_ptr GGLN is nullptr" );
        }
        if (gglnSize) {
            version = (int)(*ggln)[0]->getInt("VERS");
            wirespacing = (*ggln)[0]->getFloat("WIRESP") * Gaudi::Units::mm;
        }

        ATH_MSG_INFO( " ProcessTGCreadout - version " << version << " wirespacing " << wirespacing );

        // loop over the banks of station components: ALMN
        for (unsigned int ich = 0; ich < gglnSize; ++ich) {
            int type = (int)(*ggln)[ich]->getInt("JSTA");
            std::string name = "TGCReadout" + MuonGM::buildString(type, 2);

            // NCHRNG missing in GGLN, HARD-CODED !!!
            int nchrng;
            if (type == 1 || type == 6 || type == 12 || type >= 18) {
                nchrng = 24;
            } else {
                nchrng = 48;
            }
            using enum TgcReadoutParams::TgcReadoutArraySizes;
            GasGapIntArray nwgs{make_array<int, MaxNGaps>(0)};
            GasGapIntArray nsps{make_array<int, MaxNGaps>(0)};
            WiregangArray iwgs1{make_array<int, MaxNGangs>(0)};
            WiregangArray iwgs2{make_array<int, MaxNGangs>(0)};
            WiregangArray iwgs3{make_array<int, MaxNGangs>(0)};
            std::vector<StripArray> slarge{make_array<double, MaxNStrips>(0)}; 
            std::vector<StripArray> sshort{make_array<double, MaxNStrips>(0)};

            for (int i = 0; i < 3; i++) {
                nwgs[i] = (*ggln)[ich]->getInt("NWGS", i );
                nsps[i] = (*ggln)[ich]->getInt("NSPS", i);
            }
            for (int i = 0; i < nwgs[0]; i++) {
                iwgs1[i] = (*ggln)[ich]->getInt("IWGS1", i);
            }

            for (int i = 0; i < nwgs[1]; i++) {
                iwgs2[i] = (*ggln)[ich]->getInt("IWGS2", i);
            }
            for (int i = 0; i < nwgs[2]; i++) {
                iwgs3[i] = (*ggln)[ich]->getInt("IWGS3", i);
            }

            // read and store parameters for strips
            float pdist = (*ggln)[ich]->getFloat("PDIST");

            for (int i = 0; i < nsps[0] + 1; i++) {
                slarge[0][i] = (*ggln)[ich]->getFloat("SLARGE", i);
                sshort[0][i] = (*ggln)[ich]->getFloat("SHORT", i);
            }
            GeoModel::TransientConstSharedPtr<TgcReadoutParams> rpar = 
                    std::make_unique<TgcReadoutParams>(name, type, wirespacing, nchrng, 
                                                      std::move(nwgs), 
                                                      std::move(iwgs1), 
                                                      std::move(iwgs2), 
                                                      std::move(iwgs3), 
                                                      pdist, 
                                                      std::move(slarge), 
                                                      std::move(sshort), 
                                                      std::move(nsps));
            mysql.StoreTgcRPars(rpar);
            // parameters for TGC inactive inner structure

            std::ostringstream Astr;
            if (ich < 9) {
                Astr << "0" << ich + 1;
            } else {
                Astr << ich + 1;
            }
            std::string A = Astr.str();
            TGC *tgc = dynamic_cast<TGC*>(mysql.GetTechnology("TGC" + A));
            tgc->widthWireSupport = (*ggln)[ich]->getFloat("S1PP");
            tgc->widthGasChannel = (*ggln)[ich]->getFloat("S2PP");
            tgc->distanceWireSupport = (*ggln)[ich]->getFloat("WSEP");
            tgc->offsetWireSupport[0] = (*ggln)[ich]->getFloat("SP1WI");
            tgc->offsetWireSupport[1] = (*ggln)[ich]->getFloat("SP2WI");
            tgc->offsetWireSupport[2] = (*ggln)[ich]->getFloat("SP3WI");
            tgc->angleTilt = (*ggln)[ich]->getFloat("TILT") * Gaudi::Units::deg;
            tgc->radiusButton = (*ggln)[ich]->getFloat("SP1BU");
            tgc->pitchButton[0] = (*ggln)[ich]->getFloat("SP2BU");
            tgc->pitchButton[1] = (*ggln)[ich]->getFloat("SP3BU");
            tgc->angleButton = (*ggln)[ich]->getFloat("SP4BU") * Gaudi::Units::deg;
        }
        
    }

    std::string RDBReaderAtlas::TGCreadoutName(int ichtyp) {
       
        if (getGeometryVersion().substr(0, 1) == "P") {

            if (m_tgcReadoutMapping.empty()) {
                // first time fill the vector
                m_tgcReadoutMapping.emplace_back("T1F1"); // 1

                m_tgcReadoutMapping.emplace_back("T1E1"); // 2
                m_tgcReadoutMapping.emplace_back("T1E2");
                m_tgcReadoutMapping.emplace_back("T1E3");
                m_tgcReadoutMapping.emplace_back("T1E4");

                m_tgcReadoutMapping.emplace_back("T2F1"); // 6

                m_tgcReadoutMapping.emplace_back("T2E1"); // 7
                m_tgcReadoutMapping.emplace_back("T2E2");
                m_tgcReadoutMapping.emplace_back("T2E3");
                m_tgcReadoutMapping.emplace_back("T2E4");
                m_tgcReadoutMapping.emplace_back("T2E5"); // 11

                m_tgcReadoutMapping.emplace_back("T3F1"); // 12

                m_tgcReadoutMapping.emplace_back("T3E1"); // 13
                m_tgcReadoutMapping.emplace_back("T3E2");
                m_tgcReadoutMapping.emplace_back("T3E3");
                m_tgcReadoutMapping.emplace_back("T3E4");
                m_tgcReadoutMapping.emplace_back("T3E5"); // 17

                m_tgcReadoutMapping.emplace_back("T4F1"); // 18

                m_tgcReadoutMapping.emplace_back("T4E1"); // 19
            }

            if (ichtyp < 1 || ichtyp > 19) {
                ATH_MSG_ERROR( " DBReader::TGCreadoutName  - ichtype " << ichtyp << " out of range 1-19" );
                return "XXXY";
            }
        } else { // if (getGeometryVersion().substr(0,1) == "Q")

            // Layout Q and following
            //
            if (m_tgcReadoutMapping.empty()) {
                // first time fill the vector

                m_tgcReadoutMapping.emplace_back("T1F1_1"); // 1

                m_tgcReadoutMapping.emplace_back("T1E1_1"); // 2
                m_tgcReadoutMapping.emplace_back("T1E1_2");
                m_tgcReadoutMapping.emplace_back("T1E1_3");
                m_tgcReadoutMapping.emplace_back("T1E1_4");

                m_tgcReadoutMapping.emplace_back("T2F1_1"); // 6

                m_tgcReadoutMapping.emplace_back("T2E1_1"); // 7
                m_tgcReadoutMapping.emplace_back("T2E1_2");
                m_tgcReadoutMapping.emplace_back("T2E1_3");
                m_tgcReadoutMapping.emplace_back("T2E1_4");
                m_tgcReadoutMapping.emplace_back("T2E1_5"); // 11

                m_tgcReadoutMapping.emplace_back("T3F1_1"); // 12

                m_tgcReadoutMapping.emplace_back("T3E1_1"); // 13
                m_tgcReadoutMapping.emplace_back("T3E1_2");
                m_tgcReadoutMapping.emplace_back("T3E1_3");
                m_tgcReadoutMapping.emplace_back("T3E1_4");
                m_tgcReadoutMapping.emplace_back("T3E1_5"); // 17

                m_tgcReadoutMapping.emplace_back("T4F1_1"); // 18
                m_tgcReadoutMapping.emplace_back("T4F2_1"); // 19

                m_tgcReadoutMapping.emplace_back("T4E1_1"); // 20
                m_tgcReadoutMapping.emplace_back("T4E2_1"); // 21
            }

            if (ichtyp < 1 || ichtyp > 21) {
                ATH_MSG_ERROR( " DBReader::TGCreadoutName  - ichtype " << ichtyp << " out of range 1-21" );
                return "XXXY";
            }
        }

        return m_tgcReadoutMapping[ichtyp - 1];
    }
} // namespace MuonGM
