/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
		ALFA_GeometryReader* m_pGeometryReader;

		ALFA_LocRecEvCollection*	m_pLocRecEvCollection;
		ALFA_LocRecEvent*			m_pLocRecEvent;

		ALFA_LocRecODEvCollection*	m_pLocRecODEvCollection;
		ALFA_LocRecODEvent*			m_pLocRecODEvent;
		
		SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EvtInfo", "EventInfo", "EventInfo name"};

	private:
		std::list<eRPotName> m_ListExistingRPots;

		UInt_t m_eventNum;					// real event number
		Int_t m_iDataType;					// data type (simulation or real data) using in the local reconstruction
		Int_t m_iEvent;						// event number from zero value
		Int_t m_iMultiplicityCutMD;			// for Main Detector
		Int_t m_iNumLayerCutMD;				// for Main Detector
		Int_t m_iUVCutMD;					// for Main Detector
		Int_t m_iUVCutMDHalfReco;			// for Main Detector HalfReco algorithm
		Int_t m_iMultiplicityCutOD;			// for Overlap Detector
		Float_t m_fOverlapCutMD;			// for Main Detector
		Float_t m_fDistanceCutOD;			// for Overlap Detector
		Float_t m_iLayerCutOD;				// for Overlap Detector
		Bool_t m_bEdgeMethod_Opt_Sisters;	// for EdgeMethod
		Bool_t m_bEdgeMethod_Opt_UseGaps;	// for EdgeMethod

		//slope, offset and Z-pos for MD fibers [8][2*10][64]
		Float_t m_faMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];
		Float_t m_fbMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];
		Float_t m_fzMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];

		//slope, offset and Z-pos for OD fibers [8][3][2][2*15], side 0 = right; side 1 = left (in +z direction)
		Float_t m_faOD[RPOTSCNT][ODPLATESCNT][ODSIDESCNT][ODLAYERSCNT*ODFIBERSCNT];
		Float_t m_fbOD[RPOTSCNT][ODPLATESCNT][ODSIDESCNT][ODLAYERSCNT*ODFIBERSCNT];
		Float_t m_fzOD[RPOTSCNT][ODPLATESCNT][ODSIDESCNT][ODLAYERSCNT*ODFIBERSCNT];

		std::string m_strKeyGeometryForReco;
		std::vector<std::string> m_vecListAlgoMD;
		std::vector<std::string> m_vecListAlgoOD;
		std::string m_strAlgoOD;
		std::string m_strAlgoMD;
		std::string m_strKeyLocRecEvCollection;
		std::string m_strKeyLocRecODEvCollection;
		std::string m_strCollectionName;
		std::string m_strODCollectionName;

	public:
		StatusCode initialize();
		StatusCode execute();
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
