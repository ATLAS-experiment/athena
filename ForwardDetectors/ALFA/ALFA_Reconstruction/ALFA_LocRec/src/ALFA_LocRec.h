/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_LOCREC_h
#define ALFA_LOCREC_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "ALFA_LocRec/ALFA_UserObjects.h" //for MDHIT etc
#include "ALFA_Geometry/ALFA_GeometryReader.h" //for GEOMETRYCONFIGURATION
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "RtypesCore.h"
#include "Gaudi/Property.h"

#include <string>
#include <list>
#include <vector>

class ALFA_LocRecEvCollection;
class ALFA_LocRecODEvCollection;
class ALFA_LocRecEvent;
class ALFA_LocRecODEvent;

typedef struct _USERTRANSFORM
{
	Int_t iRPot;
	Float_t fAngle;
        CLHEP::Hep3Vector vecRotation;
        CLHEP::Hep3Vector vecTranslation;

} USERTRANSFORM, *PUSERTRANSFORM;


#define NSIDE  3
#define NTRACK 10
#define NTRIG  4
#define NBPM   4
//#define MAXNUMTRACKS 100

class StoreGateSvc;

class ALFA_LocRec : public AthAlgorithm
{
	public:
		ALFA_LocRec(const std::string& name, ISvcLocator* pSvcLocator);
		~ALFA_LocRec();

	private:
		GEOMETRYCONFIGURATION m_Config;
		ALFA_GeometryReader* m_pGeometryReader{new ALFA_GeometryReader()};

		ALFA_LocRecEvCollection*	m_pLocRecEvCollection{nullptr};
		ALFA_LocRecEvent*			m_pLocRecEvent{nullptr};

		ALFA_LocRecODEvCollection*	m_pLocRecODEvCollection{nullptr};
		ALFA_LocRecODEvent*			m_pLocRecODEvent{nullptr};

		// Geometry properties, copied into m_Config in initialize()
		Gaudi::Property<int> m_metrologyType{this, "MetrologyType", EMT_NOMINAL};//EGST_FILE
		Gaudi::Property<std::string> m_metrologySource{this, "MetrologySource", ""};//"rpmetrology.dat"
		Gaudi::Property<bool> m_shiftToX97Pos{this, "ShiftToX97Pos", false};

		// Per Roman Pot properties, indexed as CfgRPosParams
		Gaudi::Property<int> m_rpPosType[RPOTSCNT]{
			{this, "B7L1U_PosType", ERPPT_ACTIVE},
			{this, "B7L1L_PosType", ERPPT_ACTIVE},
			{this, "A7L1U_PosType", ERPPT_ACTIVE},
			{this, "A7L1L_PosType", ERPPT_ACTIVE},
			{this, "A7R1U_PosType", ERPPT_ACTIVE},
			{this, "A7R1L_PosType", ERPPT_ACTIVE},
			{this, "B7R1U_PosType", ERPPT_ACTIVE},
			{this, "B7R1L_PosType", ERPPT_ACTIVE}
		};
		Gaudi::Property<int> m_mdGeoType[RPOTSCNT]{
			{this, "B7L1U_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "B7L1L_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7L1U_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7L1L_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7R1U_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7R1L_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "B7R1U_MDGeometryType", EGST_IDEALGEOMETRY},
			{this, "B7R1L_MDGeometryType", EGST_IDEALGEOMETRY}
		};
		Gaudi::Property<int> m_odGeoType[RPOTSCNT]{
			{this, "B7L1U_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "B7L1L_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7L1U_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7L1L_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7R1U_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "A7R1L_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "B7R1U_ODGeometryType", EGST_IDEALGEOMETRY},
			{this, "B7R1L_ODGeometryType", EGST_IDEALGEOMETRY}
		};
		Gaudi::Property<std::string> m_mdConnString[RPOTSCNT]{
			{this, "B7L1U_MDSource", ""},
			{this, "B7L1L_MDSource", ""},
			{this, "A7L1U_MDSource", ""},
			{this, "A7L1L_MDSource", ""},
			{this, "A7R1U_MDSource", ""},
			{this, "A7R1L_MDSource", ""},
			{this, "B7R1U_MDSource", ""},
			{this, "B7R1L_MDSource", ""}
		};
		Gaudi::Property<std::string> m_odConnString[RPOTSCNT]{
			{this, "B7L1U_ODSource", ""},
			{this, "B7L1L_ODSource", ""},
			{this, "A7L1U_ODSource", ""},
			{this, "A7L1L_ODSource", ""},
			{this, "A7R1U_ODSource", ""},
			{this, "A7R1L_ODSource", ""},
			{this, "B7R1U_ODSource", ""},
			{this, "B7R1L_ODSource", ""}
		};
		
		SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EvtInfo", "EventInfo", "EventInfo name"};

	private:
		std::list<eRPotName> m_ListExistingRPots;

		UInt_t m_eventNum{0};					// real event number
		Gaudi::Property<Int_t> m_iDataType{this, "DataType", 1, "data type using in the local reconstruction"};					// data type (simulation or real data) using in the local reconstruction
		Int_t m_iEvent{0};						// event number from zero value
		Gaudi::Property<Int_t> m_iMultiplicityCutMD{this, "MultiplicityCutMD", 5};			// for Main Detector
		Gaudi::Property<Int_t> m_iNumLayerCutMD{this, "NumLayerCutMD", 3};				// for Main Detector
		Gaudi::Property<Int_t> m_iUVCutMD{this, "UVCutMD", 3};					// for Main Detector
		Gaudi::Property<Int_t> m_iUVCutMDHalfReco{this, "UVCutMDHalfReco", 3};			// for Main Detector HalfReco algorithm
		Gaudi::Property<Int_t> m_iMultiplicityCutOD{this, "MultiplicityCutOD", 30};			// for Overlap Detector
		Gaudi::Property<Float_t> m_fOverlapCutMD{this, "OverlapCutMD", 0.5};			// for Main Detector
		Gaudi::Property<Float_t> m_fDistanceCutOD{this, "DistanceCutOD", 0.5};			// for Overlap Detector
		Gaudi::Property<Float_t> m_iLayerCutOD{this, "LayerCutOD", 3};				// for Overlap Detector
		Gaudi::Property<Bool_t> m_bEdgeMethod_Opt_Sisters{this, "EdgeMethod_Opt_Sisters", false};	// for EdgeMethod
		Gaudi::Property<Bool_t> m_bEdgeMethod_Opt_UseGaps{this, "EdgeMethod_Opt_UseGaps", false};	// for EdgeMethod

		//slope, offset and Z-pos for MD fibers [8][2*10][64]
		Float_t m_faMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];
		Float_t m_fbMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];
		Float_t m_fzMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];

		//slope, offset and Z-pos for OD fibers [8][3][2][2*15], side 0 = right; side 1 = left (in +z direction)
		Float_t m_faOD[RPOTSCNT][ODPLATESCNT][ODSIDESCNT][ODLAYERSCNT*ODFIBERSCNT];
		Float_t m_fbOD[RPOTSCNT][ODPLATESCNT][ODSIDESCNT][ODLAYERSCNT*ODFIBERSCNT];
		Float_t m_fzOD[RPOTSCNT][ODPLATESCNT][ODSIDESCNT][ODLAYERSCNT*ODFIBERSCNT];

		std::string m_strKeyGeometryForReco{"ALFA_GeometryForReco"};
		Gaudi::Property<std::vector<std::string>> m_vecListAlgoMD{this, "ListAlgoMD", {}};
		Gaudi::Property<std::vector<std::string>> m_vecListAlgoOD{this, "ListAlgoOD", {}};
		Gaudi::Property<std::string> m_strAlgoOD{this, "AlgoOD", "ODTracking"};
		Gaudi::Property<std::string> m_strAlgoMD{this, "AlgoMD", "MDTracking"};
		std::string m_strKeyLocRecEvCollection{"ALFA_LocRecEvCollection"};
		std::string m_strKeyLocRecODEvCollection{"ALFA_LocRecODEvCollection"};
		std::string m_strCollectionName{"ALFA_DigitCollection"};
		std::string m_strODCollectionName{"ALFA_ODDigitCollection"};

	public:
		StatusCode initialize();
		StatusCode execute(const EventContext& ctx);
		StatusCode finalize();
		
	private:
		bool ReadGeometryDetCS();
		bool StoreReconstructionGeometry(const eRPotName eRPName, const eFiberType eFType, const char* szDataDestination);
		void SaveGeometry();
		void ClearGeometry();

		StatusCode ALFACollectionReading(std::list<MDHIT> &ListMDHits, std::list<ODHIT> &ListODHits);	

		StatusCode RecordCollection();
		StatusCode RecordODCollection();
		StatusCode ExecuteRecoMethod(const std::string& strAlgo, const eRPotName eRPName, const std::list<MDHIT> &ListMDHits, const std::list<ODHIT> &ListODHits);
};

#endif	//ALFA_LOCREC_h
