/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_MDMULTIPLE_H
#define ALFA_MDMULTIPLE_H

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "AthenaBaseComps/AthMessaging.h"

#include "GaudiKernel/StatusCode.h"

#include "Rtypes.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"

#include "ALFA_LocRec/ALFA_UserObjects.h"
#include "ALFA_Geometry/ALFA_constants.h"

class ALFA_MDMultiple : public AthMessaging
{
	public:
		ALFA_MDMultiple();
		ALFA_MDMultiple(const ALFA_MDMultiple &obj) = delete;
		ALFA_MDMultiple& operator=(const ALFA_MDMultiple &obj) = delete;
		~ALFA_MDMultiple() = default;

	private:
		static constexpr size_t MAXTRACKNUM = 10;
		Int_t m_iRPot;
		Int_t m_iUVCut;
		Int_t m_iMultiplicityCut;
		Int_t m_iNumLayerCut;
		Float_t m_fOverlapCut;

		//slope, offset and Z-pos for MD fibers [8][2*10][64]
		Float_t m_faMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];
		Float_t m_fbMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT];

		//number of hits in the layer [2*10]
		Int_t m_iNumHitsLayer[ALFALAYERSCNT*ALFAPLATESCNT];

		std::vector<Float_t> m_fRecXPos, m_fRecYPos;
		std::vector<Float_t> m_fOvU, m_fOvV;
		std::vector<Int_t>   m_iNU, m_iNV;
		std::vector<Int_t>   m_iFibSel[ALFALAYERSCNT*ALFAPLATESCNT];
		std::vector<Int_t>   m_iTrackMatch[2];

	private:
		using LayerMap_t = std::map<int, FIBERS>;

	public:
		StatusCode Initialize(Int_t iRPot, Float_t faMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT], Float_t fbMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT], Int_t iMultiplicityCut, Int_t iNumLayerCut, Int_t iUVCut, Float_t fOverlapCut);
		StatusCode Execute(const std::list<MDHIT> &ListMDHits);
		StatusCode Finalize(Float_t (&fRecXPos)[MAXTRACKNUM], Float_t (&fRecYPos)[MAXTRACKNUM]);

		void GetData(Int_t (&iNumU)[MAXTRACKNUM], Int_t (&iNumV)[MAXTRACKNUM], Float_t (&fOvU)[MAXTRACKNUM], Float_t (&fOvV)[MAXTRACKNUM], Int_t (&iFibSel)[MAXTRACKNUM][ALFALAYERSCNT*ALFAPLATESCNT]);

	private:
		void Proj_Store(LayerMap_t& mapLayers,
                                Int_t iFiberSide, std::span<Int_t> iOver, Float_t fbRef, Int_t iSideFlag);
  void Proj_Store(const std::vector<Int_t> (&FiberHit)[ALFAPLATESCNT], std::span<Int_t> iOver, Float_t fbRef, Int_t iSideFlag);
		void Find_Proj(const std::span<const Int_t>& iOver, Float_t fbRef, Float_t &fb, Float_t &fOv, Int_t &fNum);
		void Finding_Fib(LayerMap_t& mapLayers,
                                 Int_t iFiberSide, Float_t fbRef, Float_t fbRec, Int_t (&iFSel)[ALFAPLATESCNT], Int_t iSideFlag);
		void Reco_Track(LayerMap_t& mapLayers,
                                std::vector<double> &b_p, std::vector<double> &b_n,
						std::vector<double> &Ov_p, std::vector<double> &Ov_n,
						std::vector<int> &Num_p, std::vector<int> &Num_n,
						std::vector<int> (&FSel_n)[ALFAPLATESCNT], std::vector<int> (&FSel_p)[ALFAPLATESCNT],
						std::vector<int> (&Track_match)[2]);

};
#endif // ALFA_MDMULTIPLE_H
