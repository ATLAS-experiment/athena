/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Common Muon Readout Element properties
 -----------------------------------------
 ***************************************************************************/

#ifndef MUONREADOUTGEOMETRY_MUONREADOUTELEMENT_H
#define MUONREADOUTGEOMETRY_MUONREADOUTELEMENT_H

#include "CxxUtils/checker_macros.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "TrkDetElementBase/TrkDetElementBase.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
ATLAS_CHECK_FILE_THREAD_SAFETY;

namespace MuonGM {   
    

    class MuonDetectorManager;
    class MuonStation;

    /**
       Base class for the XxxReadoutElement,
       with Xxx = Mdt, Rpc, Tgc, Csc. It's a Trk::TrkDetElementBase,
       therefore it must implement the generic tracking interfaces requested to
       the geometry: center, normal, surfaceBound, transform.
       It is synched to the raw geometry via the pointer, required by the
       constructor, to its corresponding GeoVFullPhysVol.
       A MuonReadoutElement holds a pointer to its parent MuonStation.
    */

    class MuonReadoutElement : public Trk::TrkDetElementBase, public AthMessaging {
    public:
        // Destructor default       
        virtual ~MuonReadoutElement();

        
        /// Returns the ATLAS Identifier of the MuonReadOutElement. 
        /// Usually the Identifier corresponds to the first channel covered by the Element
        inline Identifier identify() const override final;
        /// Returns the IdentifierHash of the MuonStation, i.e. 2 readoutelements with same
        /// sationIndex, stationEta, stationPhi have the same identifiyHash
        inline IdentifierHash identifyHash() const override final;
        ///  Returns the IdentifierHash of the detector element. Unlike the identifyHash, this
        ///  hash is unique for each readout element
        inline IdentifierHash detectorElementHash() const;
        ///  Sets the Identifier, hashes & station names
        void setIdentifier(const Identifier& id);

       
        virtual bool containsId(const Identifier& id) const = 0;

        inline int getStationIndex() const;
        inline int getStationEta() const;
        inline int getStationPhi() const;
      

        const Amg::Vector3D globalPosition() const;
        const Amg::Transform3D& absTransform() const;
        const Amg::Transform3D& defTransform() const;

        // Amdb local (szt) to global coord
        virtual Amg::Vector3D AmdbLRSToGlobalCoords(const Amg::Vector3D& x) const;
        virtual Amg::Transform3D AmdbLRSToGlobalTransform() const;
        // Global to Amdb local (szt) coord
        virtual Amg::Vector3D GlobalToAmdbLRSCoords(const Amg::Vector3D& x) const;
        virtual Amg::Transform3D GlobalToAmdbLRSTransform() const;

        // like MDT, RPC, TGC, CSC
        inline std::string getTechnologyType() const;
        // like MDT1, RPC4, TGC1, etc...
        inline const std::string& getTechnologyName() const;
        void setTechnologyName(const std::string& str);
        // BOL, EIS, BMF, T1F, etc ...
        inline std::string getStationType() const;
        // BOL1, BEE1, etc...
        inline const std::string& getStationName() const;
        void setStationName(const std::string&);

        // size of the detector
        inline double getSsize() const;
        inline double getRsize() const;
        inline double getZsize() const;
        inline double getLongSsize() const;
        inline double getLongRsize() const;
        inline double getLongZsize() const;
        
        /// Seems to be exclusively used by the MDTs 
        /// --> Move it to MdtReadoutElement
        inline double getStationS() const;
        void setStationS(double);


        void setLongSsize(double);
        void setLongRsize(double);
        void setLongZsize(double);
        void setSsize(double);
        void setRsize(double);
        void setZsize(double);
       
        bool largeSector() const;
        bool smallSector() const;
        
        inline bool sideA() const;
        inline bool sideC() const;

        void setParentStationPV(const PVConstLink&);
        void setParentStationPV();
        PVConstLink parentStationPV() const;
        const MuonStation* parentMuonStation() const;
        void setParentMuonStation(const MuonStation*);
        Amg::Transform3D toParentStation() const;
        Amg::Vector3D parentMuonStationPos() const;
       
        int getIndexOfREinMuonStation() const;
       

        bool hasCutouts() const { return m_hasCutouts; }
        void setHasCutouts(bool flag) { m_hasCutouts = flag; }

        // Tracking related interfaces
        // Element Surface
        // This creates a new surface. The client is responsible for deleting it.
        inline int cachingFlag() const;
        void setCachingFlag(int value);
        virtual void clearCache() = 0;
        virtual void fillCache() = 0;
        virtual void refreshCache() = 0;
        
        const Muon::IMuonIdHelperSvc* idHelperSvc() const { return m_idHelperSvc.get(); }
        
        Trk::DetectorElemType detectorType() const override final { return m_type; }

    protected:
        MuonReadoutElement(GeoVFullPhysVol* pv, MuonDetectorManager* mgr, Trk::DetectorElemType detType);
        
        double m_Ssize{-9999.};
        double m_Rsize{-9999.};
        double m_Zsize{-9999.};
        double m_LongSsize{-9999.};
        double m_LongRsize{-9999.};
        double m_LongZsize{-9999.};
        //!< size in the specified direction
                
        inline const MuonDetectorManager* manager() const;
        std::string m_techname{"TTT0"};
        //!< MDT or RPC or TGC or CSC plus a two digits subtype; example RPC17
        std::string m_statname{"XXX0"};     //!< examples are BMS5, CSS1, EML1
       
        int m_caching{-1};
        //!< 0 if we want to avoid caching geometry info for tracking interface
        bool m_hasCutouts{false};            //!<  true is there are cutouts in the readdout-element

    private:
        void setIndexOfREinMuonStation();
       
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonDetectorManager"};
        Trk::DetectorElemType m_type{Trk::DetectorElemType::SolidState};
        Identifier m_id{0};                    //!< extended data-collection identifier
        IdentifierHash m_idhash{0};            //!< data-collection hash identifier
        IdentifierHash m_detectorElIdhash{0};  //!< detector element hash identifier

        int m_indexOfREinMuonStation{-999};  //!<  index of this RE in the mother MuonStation
       
        double m_stationS{0.};
        /// Identifier field of the station index
        int m_stIdx{-1};
        /// Identifier field of the station eta
        int m_eta{-1};
        /// Identifier field of the station phi
        int m_phi{-1};
             
        PVConstLink m_parentStationPV{nullptr};
        const MuonStation* m_parentMuonStation{nullptr};
        MuonDetectorManager* m_muon_mgr{nullptr};
    };

    Identifier MuonReadoutElement::identify() const { return m_id; }
    IdentifierHash MuonReadoutElement::identifyHash() const { return m_idhash; }
    IdentifierHash MuonReadoutElement::detectorElementHash() const { return m_detectorElIdhash; }
    std::string MuonReadoutElement::getTechnologyType() const { return m_techname.substr(0, 3); }
    const std::string& MuonReadoutElement::getTechnologyName() const { return m_techname; }
    std::string MuonReadoutElement::getStationType() const { return m_statname.substr(0,3); }
    const std::string& MuonReadoutElement::getStationName() const { return m_statname; }

    int MuonReadoutElement::getStationIndex() const { return m_stIdx; }
    int MuonReadoutElement::getStationEta() const { return m_eta; }
    int MuonReadoutElement::getStationPhi() const { return m_phi; }
    
    double MuonReadoutElement::getSsize() const { return m_Ssize; }
    double MuonReadoutElement::getRsize() const { return m_Rsize; }
    double MuonReadoutElement::getZsize() const { return m_Zsize; }
    double MuonReadoutElement::getLongSsize() const { return m_LongSsize; }
    double MuonReadoutElement::getLongRsize() const { return m_LongRsize; }
    double MuonReadoutElement::getLongZsize() const { return m_LongZsize; }
    double MuonReadoutElement::getStationS() const { return m_stationS; }
    
    const MuonDetectorManager* MuonReadoutElement::manager() const { return m_muon_mgr; }
    
   
    bool MuonReadoutElement::sideA() const { return (getStationEta() > 0); }
    bool MuonReadoutElement::sideC() const { return (getStationEta() < 0); }
    int MuonReadoutElement::cachingFlag() const { return m_caching; }

    inline const Amg::Transform3D& MuonReadoutElement::absTransform() const { return getMaterialGeom()->getAbsoluteTransform(); }

    inline const Amg::Transform3D& MuonReadoutElement::defTransform() const { return getMaterialGeom()->getDefAbsoluteTransform(); }

}  // namespace MuonGM
 
#endif  // MUONREADOUTGEOMETRY_MUONREADOUTELEMENT_H
