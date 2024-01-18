/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonChamberLite_H
#define MuonChamberLite_H

#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/Station.h"
#include "AthenaBaseComps/AthMessaging.h"
#include <map>
#include <string> 
#include <set>
class GeoAlignableTransform;
class GeoVFullPhysVol;

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
    class MYSQL;

    class MuonChamberLite : public DetectorElement, public AthMessaging{

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers

        std::array<double,10> rotangle{};

        MuonChamberLite(const MYSQL& mysql, Station *s, 
		        std::map<std::string, GeoFullPhysVol*> * mapFPV,
                        std::map<std::string, GeoAlignableTransform *> * mapAXF);
        GeoVPhysVol *addReadoutLayers(const MYSQL& mysql,
                           MuonDetectorManager *manager, int ieta, int iphi, bool is_mirrored, bool &isAssembly);
        virtual void print() const override;
        void setKeyset(std::set<std::string> *keySet) {m_keySet=keySet;};

      private:
        void setCscReadoutGeom(const MYSQL& mysql,
                               CscReadoutElement *re, const CscComponent *cc, const Position &p);
        void setMdtReadoutGeom(const MYSQL& mysql,
                               MdtReadoutElement *re, const MdtComponent *cc, const Position &p);
        void setRpcReadoutGeom(const MYSQL& mysql,
                               RpcReadoutElement *re, const RpcComponent *cc, const Position &p, const std::string& geomVers, MuonDetectorManager *manager);
        void setTgcReadoutGeom(const MYSQL& mysql,
                               TgcReadoutElement *re, const TgcComponent *cc, const Position &p, const std::string& statname);

        Station *m_station{nullptr};

        std::map<std::string, GeoFullPhysVol*>        * m_mapFPV{nullptr};
	std::map<std::string, GeoAlignableTransform*> * m_mapAXF{nullptr};
	std::set<std::string>                         * m_keySet{nullptr};      
    };


} // namespace MuonGM

#endif
