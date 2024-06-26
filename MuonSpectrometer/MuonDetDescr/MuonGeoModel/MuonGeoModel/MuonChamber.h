/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonChamber_H
#define MuonChamber_H

#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/Station.h"
#include "AthenaBaseComps/AthMessaging.h"

class IMessageSvc;

namespace MuonGM {
    class MuonDetectorManager;
    class CscReadoutElement;
    class MdtReadoutElement;
    class RpcReadoutElement;
    class TgcReadoutElement;
    class CscComponent;
    class MdtComponent;
    class RpcComponent;
    class TgcComponent;
    class Position;
    class FPVMAP;
    class MYSQL;

    class MuonChamber : public DetectorElement, public AthMessaging {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers

        std::array<double, 10> rotangle{};

        void setFineClashFixingFlag(int value);

        MuonChamber(const MYSQL& mysql, Station *s);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql,
                           MuonDetectorManager *manager, int ieta, int iphi, bool is_mirrored, bool &isAssembly);
        virtual void print() const override;
        void setFPVMAP(FPVMAP *fpvmap);

      private:
        void setCscReadoutGeom(const MYSQL& mysql,
                               CscReadoutElement *re, const CscComponent *cc, const Position &p);
        void setMdtReadoutGeom(const MYSQL& mysql,
                               MdtReadoutElement *re, const MdtComponent *cc, const Position &p);
        void setRpcReadoutGeom(const MYSQL& mysql,
                               RpcReadoutElement *re, const RpcComponent *cc, const Position &p);
        void setTgcReadoutGeom(const MYSQL& mysql,
                               TgcReadoutElement *re, const TgcComponent *cc, const Position &p, const std::string& statname);

        Station *m_station{nullptr};
        int m_enableFineClashFixing{0};

        FPVMAP *m_FPVMAP = nullptr;
    };

    
} // namespace MuonGM

#endif
