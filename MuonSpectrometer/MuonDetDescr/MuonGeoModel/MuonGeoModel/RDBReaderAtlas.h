/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RDBReaderAtlas_H
#define RDBReaderAtlas_H

#include "AthenaBaseComps/AthMessaging.h"
#include "MuonGMdbObjects/DblQ00IAcsc.h"
#include "MuonGMdbObjects/DblQ00Xtomo.h"
#include "MuonGeoModel/DBReader.h"

#include <fstream>
#include <vector>

class IMessageSvc;
class IRDBAccessSvc;

namespace MuonGM {

    class RDBReaderAtlas : public DBReader, public AthMessaging {
      public:
        RDBReaderAtlas(StoreGateSvc *pDetStore, IRDBAccessSvc *m_pRDBAccess, const std::string& geoTag, const std::string& geoNode, 
                       const std::map<std::string, std::string>& asciiFileDBMap);
        virtual ~RDBReaderAtlas() = default;
        virtual StatusCode ProcessDB(MYSQL& mysql) override;



        void ProcessTGCreadout(MYSQL& mysql);
        void ProcessCscInternalAlignments();
        std::string TGCreadoutName(int ichtyp);    

      private:
        void ProcessTechnologies(MYSQL& mysql);

        std::unique_ptr<DblQ00Dbam> m_dhdbam{nullptr};
        std::unique_ptr<DblQ00Atyp> m_dhatyp{nullptr};
        std::unique_ptr<DblQ00Asmp> m_dhasmp{nullptr};
        std::unique_ptr<DblQ00Almn> m_dhalmn{nullptr};
        std::unique_ptr<DblQ00Aptp> m_dhaptp{nullptr};
        std::unique_ptr<DblQ00Awln> m_dhwrpc{nullptr};
        std::unique_ptr<DblQ00Atln> m_dhwtgc{nullptr};
        std::unique_ptr<DblQ00Acut> m_dhacut{nullptr};
        std::unique_ptr<DblQ00Alin> m_dhalin{nullptr};
        std::unique_ptr<DblQ00Wmdt> m_dhwmdt{nullptr};
        std::unique_ptr<DblQ00Wcsc> m_dhwcsc{nullptr};
        std::unique_ptr<DblQ00Wrpc> m_dhwrpcall{nullptr};
        std::unique_ptr<DblQ00Wtgc> m_dhwtgcall{nullptr};
        std::unique_ptr<DblQ00Wded> m_dhwded{nullptr};
        std::unique_ptr<DblQ00Wsup> m_dhwsup{nullptr};
        std::unique_ptr<DblQ00Wspa> m_dhwspa{nullptr};
        std::unique_ptr<DblQ00Wchv> m_dhwchv{nullptr};
        std::unique_ptr<DblQ00Wcro> m_dhwcro{nullptr};
        std::unique_ptr<DblQ00Wcmi> m_dhwcmi{nullptr};
        std::unique_ptr<DblQ00Wlbi> m_dhwlbi{nullptr};
        std::unique_ptr<DblQ00Aszt> m_dhaszt{nullptr};
        std::unique_ptr<DblQ00IAcsc> m_dhiacsc{nullptr};
        std::unique_ptr<DblQ00Xtomo>m_dhxtomo{nullptr};
        const DblQ00Dbam::DBAM *m_dbam{nullptr};
        const DblQ00Atyp::ATYP *m_atyp{nullptr};
        const DblQ00Asmp::ASMP *m_asmp{nullptr};
        const DblQ00Almn::ALMN *m_almn{nullptr};
        const DblQ00Aptp::APTP *m_aptp{nullptr};
        const DblQ00Awln::AWLN *m_wrpc{nullptr};
        const DblQ00Atln::ATLN *m_wtgc{nullptr};
        const DblQ00Acut::ACUT *m_acut{nullptr};
        const DblQ00Alin::ALIN *m_alin{nullptr};
        const DblQ00Wmdt::WMDT *m_wmdt{nullptr};
        const DblQ00Wcsc::WCSC *m_wcsc{nullptr};
        const DblQ00Wrpc::WRPC *m_wrpcall{nullptr};
        const DblQ00Wtgc::WTGC *m_wtgcall{nullptr};
        const DblQ00Wded::WDED *m_wded{nullptr};
        const DblQ00Wsup::WSUP *m_wsup{nullptr};
        const DblQ00Wspa::WSPA *m_wspa{nullptr};
        const DblQ00Wchv::WCHV *m_wchv{nullptr};
        const DblQ00Wcro::WCRO *m_wcro{nullptr};
        const DblQ00Wcmi::WCMI *m_wcmi{nullptr};
        const DblQ00Wlbi::WLBI *m_wlbi{nullptr};
        const DblQ00Aszt::ASZT *m_aszt{nullptr};
        const DblQ00IAcsc::IACSC *m_iacsc{nullptr};
        const DblQ00Xtomo::XTOMO *m_xtomo{nullptr};

        const std::string m_geoTag{};
        const std::string m_geoNode{};
        IRDBAccessSvc *m_pRDBAccess{nullptr};
        std::string m_asciiFileDB{};


    };

} // namespace MuonGM

#endif
