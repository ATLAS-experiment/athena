/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ALFA_LocRec/ALFA_MDMultiple.h"
#include <algorithm>    // std::copy

ALFA_MDMultiple::ALFA_MDMultiple() :
    AthMessaging("ALFA_MDMultiple")
{
	memset(&m_iNumHitsLayer, 0.0, sizeof(m_iNumHitsLayer));

	m_fOverlapCut      = 0.0;
	m_iMultiplicityCut = 0;
	m_iNumLayerCut     = 0;
	m_iRPot            = 0;
	m_iUVCut           = 0;
}

StatusCode ALFA_MDMultiple::Initialize(Int_t iRPot, Float_t faMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT], Float_t fbMD[RPOTSCNT][ALFALAYERSCNT*ALFAPLATESCNT][ALFAFIBERSCNT], Int_t iMultiplicityCut, Int_t iNumLayerCut, Int_t iUVCut, Float_t fOverlapCut)
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Initialize()");

	m_iRPot            = iRPot;
	m_iUVCut           = iUVCut;
	m_iNumLayerCut     = iNumLayerCut;
	m_iMultiplicityCut = iMultiplicityCut;
	m_fOverlapCut      = fOverlapCut;

	for (Int_t iPot=0; iPot<RPOTSCNT; iPot++)
	{
		for (Int_t iLayer=0; iLayer<ALFALAYERSCNT*ALFAPLATESCNT; iLayer++)
		{
			for (Int_t iFiber=0; iFiber<ALFAFIBERSCNT; iFiber++)
			{
				m_faMD[iPot][iLayer][iFiber] = faMD[iPot][iLayer][iFiber];
				m_fbMD[iPot][iLayer][iFiber] = fbMD[iPot][iLayer][iFiber];
			}
		}
	}

//	std::cout << "m_iMultiplicityCut, m_iUVCut = " << m_iMultiplicityCut << ", " << m_iUVCut << std::endl;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_MDMultiple::Execute(const std::list<MDHIT> &ListMDHits)
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Execute()");

	LayerMap_t mapLayers;

	std::list<MDHIT>::const_iterator iter;
	for (iter=ListMDHits.begin(); iter!=ListMDHits.end(); ++iter)
	{
		if (m_iRPot == (*iter).iRPot)
		{
			mapLayers[(*iter).iPlate].ListFibers.push_back((*iter).iFiber);
		}
	}

	// RECONSTRUCTION
	std::vector<double> b_p, b_n;
	std::vector<double> Ov_p, Ov_n;
	std::vector<int> Num_p, Num_n;
	std::vector<int> FSel_n[ALFAPLATESCNT], FSel_p[ALFAPLATESCNT];
	std::vector<int> iTrackMatch[2];


	//Checking that the multiplicity cut conditions are satisfied
	//At least more than UV_cut layers have a multiplicity lower than multi_cut

	{
		Reco_Track(mapLayers, b_p, b_n, Ov_p, Ov_n, Num_p, Num_n, FSel_n, FSel_p, iTrackMatch);

		//Now sorting the tracks using NumU+NumV criteria -------------------------------
		Int_t iCntSort=0;
		Int_t iMaxSum=0;
		std::vector<Int_t> MaxTrackID;
		Int_t iTmpMaxTrackID = -1;

//		std::cout << "b_p.size = " << b_p.size() << std::endl;
		while (iCntSort<(Int_t)b_p.size())
		{
			iMaxSum=0;
			for (Int_t i=0; i<(Int_t)b_p.size(); i++)
			{
				//Checking that the maximum was not already used
				Bool_t MaxUsed=false;

				for (int j : MaxTrackID)
				{
					if (i==j) {MaxUsed = true; break;}
				}

				if (((Num_p[i]+Num_n[i])>iMaxSum) && (!MaxUsed))
				{
					iMaxSum = Num_p[i]+Num_n[i];
					iTmpMaxTrackID=i;
				}
			}
			MaxTrackID.push_back(iTmpMaxTrackID);
			iCntSort++;
		}


		for (int i : MaxTrackID)
		{
                  m_fRecXPos.push_back((b_p[i]-b_n[i])/2.0);
                  m_fRecYPos.push_back((b_p[i]+b_n[i])/2.0);

                  m_fOvU.push_back(Ov_p[i]);
                  m_fOvV.push_back(Ov_n[i]);
                  m_iNU.push_back(Num_p[i]);
                  m_iNV.push_back(Num_n[i]);

                  m_iTrackMatch[0].push_back(iTrackMatch[0][i]);
                  m_iTrackMatch[1].push_back(iTrackMatch[1][i]);

                  for (Int_t iPlate=0; iPlate<ALFAPLATESCNT; iPlate++)
                  {
                    m_iFibSel[2*iPlate].push_back(FSel_p[iPlate][i]);
                    m_iFibSel[2*iPlate+1].push_back(FSel_n[iPlate][i]);
                  }
		}
	}


	return StatusCode::SUCCESS;
}

StatusCode ALFA_MDMultiple::Finalize(Float_t (&fRecXPos)[MAXTRACKNUM], Float_t (&fRecYPos)[MAXTRACKNUM])
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Finalize()");

	size_t iTrackNum=0, iSize=0;
	std::fill_n(&fRecXPos[0], sizeof(fRecXPos)/sizeof(Float_t), -9999.0);
	std::fill_n(&fRecYPos[0], sizeof(fRecYPos)/sizeof(Float_t), -9999.0);

	iSize = std::min(m_fRecXPos.size(), m_fRecYPos.size());
	iTrackNum = (iSize < MAXTRACKNUM)? iSize : MAXTRACKNUM;

	for (size_t i=0; i<iTrackNum; i++)
	{
		fRecXPos[i] = m_fRecXPos.at(i);
		fRecYPos[i] = m_fRecYPos.at(i);
	}

	return StatusCode::SUCCESS;
}

/************************************************/
/*  Making projection and storing in an array   */
/************************************************/
void ALFA_MDMultiple::Proj_Store(LayerMap_t& mapLayers,
                                 Int_t iFiberSide, std::span<Int_t> iOver, Float_t fbRef, Int_t iSideFlag)
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Pro_Store()");

	Float_t fSign;
	Float_t fXInter, fYInter;
	Float_t FibCen;

	if (m_faMD[m_iRPot][iSideFlag][0]>0) fSign=1.0;
	else fSign=-1.0;

	for (int & iBin : iOver)
	{
		iBin=0;
	}

	for (UInt_t iLayer=0; iLayer!=ALFAPLATESCNT; ++iLayer)
	{
		const unsigned int thisSideLayer = iLayer*2+iSideFlag;
		const unsigned int thisLayer = 2*iLayer+iFiberSide;
		const std::list<int> & thisFiberContainer = mapLayers[thisLayer].ListFibers;
		for (const auto & thisFiber:thisFiberContainer)
		{
			if (thisFiber!=9999)
			{
				//Depending on layer orientation, computing the projection of the hit fiber
				fXInter= fSign*(fbRef-m_fbMD[m_iRPot][thisSideLayer][thisFiber])/(1+fSign*m_faMD[m_iRPot][thisSideLayer][thisFiber]);
				fYInter= (fSign*m_faMD[m_iRPot][thisSideLayer][thisFiber]*fbRef+m_fbMD[m_iRPot][thisSideLayer][thisFiber])/(1+fSign*m_faMD[m_iRPot][thisSideLayer][thisFiber])-fbRef;
				FibCen = (fYInter-fSign*fXInter)/sqrt(2.0);

				//Filling the table with the hit fiber
				for (Int_t iBin=0; iBin<480; iBin++)
				{
					iOver[(int)((FibCen-0.24)*1000)+iBin+36000]++;
				}
			}
		}
	}
}

/************************************************/
/*  Making projection and storing in an array   */
/************************************************/
void ALFA_MDMultiple::Proj_Store(const std::vector<Int_t> (&FiberHit)[ALFAPLATESCNT], std::span<Int_t> iOver, Float_t fbRef, Int_t iSideFlag)
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Pro_Store()");

	Float_t fSign;
	Float_t fXInter, fYInter;
	Float_t FibCen;

	if (m_faMD[m_iRPot][iSideFlag][0]>0) fSign=1.0;
	else fSign=-1.0;

	for (int & iBin : iOver)
	{
		iBin=0;
	}

	int iHit = 9999;
	for (Int_t iLayer=0; iLayer<ALFAPLATESCNT; iLayer++)
	{
		for (UInt_t j=0; j<FiberHit[iLayer].size(); j++)
		{
			iHit = FiberHit[iLayer][j];
			if (iHit!=9999)
			{
				//Depending on layer orientation, computing the projection of the hit fiber
				fXInter= fSign*(fbRef-m_fbMD[m_iRPot][iLayer*2+iSideFlag][iHit])/(1+fSign*m_faMD[m_iRPot][iLayer*2+iSideFlag][iHit]);
				fYInter= (fSign*m_faMD[m_iRPot][iLayer*2+iSideFlag][iHit]*fbRef+m_fbMD[m_iRPot][iLayer*2+iSideFlag][iHit])/(1+fSign*m_faMD[m_iRPot][iLayer*2+iSideFlag][iHit])-fbRef;
				FibCen = (fYInter-fSign*fXInter)/sqrt(2.0);

				//Filling the table with the hit fiber
				for (Int_t iBin=0; iBin<480; iBin++)
				{
					iOver[(int)((FibCen-0.24)*1000)+iBin+36000]++;
				}
			}
		}
	}
}

/************************************************/
/*   Identifying plateau in projection array	*/
/************************************************/
void ALFA_MDMultiple::Find_Proj(const std::span<const Int_t>& iOver, Float_t fbRef, Float_t &fb, Float_t &fOv, Int_t &iNum)
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Find_Proj()");

	std::vector<int> iSizePlateau;
	Int_t iNumFib=0;
	Int_t p_tmp_min;
//	Int_t p_tmp_max;
	Float_t p_min;
	Float_t p_max;

	//Determine the maximum number of overlapping fibers in both directions
	for (Int_t i=0;i<72000;i++)
	{
		if (iOver[i]>iNumFib) iNumFib=iOver[i];
	}

	//	Filling array for all values equal to the maximum
	if (iNumFib>=m_iUVCut)
	{
		for (Int_t i=0;i<72000;i++)
		{
			if (iOver[i]==iNumFib)
			{
				iSizePlateau.push_back(i);
			}
		}

//		Finding first and last position where the maximum is found
		p_min = -36.0 + double(iSizePlateau.front())*1e-3;
		p_tmp_min = iSizePlateau.front();

		p_max = -36.0 + double(iSizePlateau.back())*1e-3;
//		p_tmp_max = iSizePlateau.back();

//		Making sure that the plateau belongs to the same track
		Int_t full_width = iSizePlateau.size();

		for (Int_t i=0; i<full_width; i++)
		{
			if (iSizePlateau[full_width-i-1]-p_tmp_min < 500)
			{
//				p_tmp_max = iSizePlateau[full_width-i-1];
				p_max = -36.0 + double(iSizePlateau[full_width-i-1])*1e-3;

				break;
			}
		}

		if ((p_max-p_min)<m_fOverlapCut)
		{
//			Storing the coordinate of the maximum
			fb = fbRef+(p_min+p_max)/sqrt(2.0);
			fOv = p_max-p_min;
		}
	}

	iNum = iNumFib;
}

/************************************************/
/************************************************/
/*       Identifying fibers on the track        */
/************************************************/
/************************************************/

void ALFA_MDMultiple::Finding_Fib(LayerMap_t& mapLayers,
                                  Int_t iFiberSide, Float_t fbRef, Float_t fbRec, Int_t (&iFSel)[10], Int_t iSideFlag)
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Finding_Fib()");

	Float_t b_pos, b_neg;
	Float_t x, y, x_tmp, y_tmp;
	Float_t min_dist;
	Float_t dist_x, dist_y;
	Float_t dist_full;
	Float_t fib_dist;

	if (iSideFlag==0)
	{
		b_neg= fbRef;
		b_pos= fbRec;
	}
	else
	{
		b_neg= fbRec;
		b_pos= fbRef;
	}

	x= (b_pos-b_neg)/2.0;
	y= (b_neg+b_pos)/2.0;

	for (int & iLayer : iFSel) iLayer = 9999;

	//For each layer, we determine the hit fiber which is closest to the track
	std::list<int>::iterator intIter;
	for (Int_t iLayer = 0; iLayer<ALFAPLATESCNT; iLayer++)
	{
		min_dist=0.24;

		const unsigned int thisSideLayer = iLayer*2+iSideFlag;
		const unsigned int thisLayer = 2*iLayer+iFiberSide;
		const std::list<int> & thisFiberContainer = mapLayers[thisLayer].ListFibers;
		for (const auto & thisFiber:thisFiberContainer)
		{
			if (thisFiber != 9999)
			{
				x_tmp = (y-m_fbMD[m_iRPot][thisSideLayer][thisFiber])/m_faMD[m_iRPot][thisSideLayer][thisFiber];
				y_tmp = m_faMD[m_iRPot][thisSideLayer][thisFiber]*x+m_fbMD[m_iRPot][thisSideLayer][thisFiber];

				dist_x = TMath::Abs(x-x_tmp);
				dist_y = TMath::Abs(y-y_tmp);

				dist_full = sqrt(dist_x*dist_x+dist_y*dist_y);
				fib_dist = sqrt(TMath::Power((dist_x+dist_y)/2.0,2)-TMath::Power(dist_full/2.0,2));

				if (fib_dist <= min_dist)
				{
					min_dist = fib_dist;
					iFSel[iLayer] = thisFiber;
				}
			}
		}

	}
}

/************************************************/
/*				Finding all tracks				*/
/************************************************/

void ALFA_MDMultiple::Reco_Track(LayerMap_t& mapLayers,
                                 std::vector<double> &b_p, std::vector<double> &b_n,
								std::vector<double> &Ov_p, std::vector<double> &Ov_n,
								std::vector<int> &Num_p, std::vector<int> &Num_n,
								std::vector<int> (&FSel_n)[ALFAPLATESCNT], std::vector<int> (&FSel_p)[ALFAPLATESCNT],
								std::vector<int> (&iTrackMatch)[2])
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::Reco_Track()");

	Int_t FSel_neg[ALFAPLATESCNT];
	Int_t FSel_pos_tmp[ALFAPLATESCNT];
	std::vector<Int_t> Over_p(72000);
	std::vector<Int_t> Over_n(72000);
	Int_t cnt_step_U=0;
	Int_t cnt_step_V=0;
	Int_t NumU=0;
	Int_t NumV=0;
	Float_t b_ref_p;
	Float_t b_ref_n;
	Float_t OvU;
	Float_t OvV;
	Float_t b_pos;
	Float_t b_neg;

 	std::list<int>::iterator intIter;


	//clear
	for (Int_t i=0; i<ALFAPLATESCNT; i++)
	{
		FSel_n[i].clear();
		FSel_p[i].clear();
	}
	b_n.clear();
	b_p.clear();
	Ov_n.clear();
	Ov_p.clear();
	Num_n.clear();
	Num_p.clear();

	int iNumUFiberHits = 0;
	int iNumVFiberHits = 0;

	std::vector<Int_t> Fiber_MB_tmp[ALFAPLATESCNT];
	std::vector<Int_t> Fiber_MB_n[ALFAPLATESCNT];

	for (UInt_t iLayer=0; iLayer!=ALFAPLATESCNT; ++iLayer)
	{
		Fiber_MB_n[iLayer].clear();
		const unsigned int thisLayer=2*iLayer+1;
		const std::list<int> & thisFiberContainer = mapLayers[thisLayer].ListFibers;
		for (const auto & thisFiber:thisFiberContainer)
		{
//			std::cout << "thisFiber: " << thisFiber << std::endl;
			Fiber_MB_n[iLayer].push_back(thisFiber);
		}
	}
	do
	{
		b_ref_n=-127.0;
		b_ref_p=-127.0;

//		First projection step on U side
//		-------------------------------
//		filling the array for U side with reference value
		Proj_Store(mapLayers, 0, Over_p, b_ref_n, 0);

//		Find first maxium
		Find_Proj(Over_p, b_ref_n, b_pos, OvU, NumU);

		Finding_Fib(mapLayers, 0, b_ref_n, b_pos, FSel_pos_tmp, 0);
		for (int i=0; i<ALFAPLATESCNT; i++)
		{
			Fiber_MB_tmp[i].clear();
			Fiber_MB_tmp[i].push_back(FSel_pos_tmp[i]);
		}

		//Then reconstruction all tracks possible using the second side
		for (UInt_t iLayer=0; iLayer<ALFAPLATESCNT; iLayer++)
		{
                        FIBERS& fibers = mapLayers[2*iLayer+1];
                        fibers.ListFibers.clear();
			for (unsigned int i=0; i<Fiber_MB_n[iLayer].size(); i++)
			{
				fibers.ListFibers.push_back(Fiber_MB_n[iLayer][i]);
			}
		}

//		Then reconstruct all tracks possible using the second side
		if (NumU>=m_iUVCut)
		{
			cnt_step_V=0;
			do
			{
				// New Part to apply the multiplicity cut at each iteration
				iNumUFiberHits=0;
				iNumVFiberHits=0;
				for (UInt_t iLayer=0;iLayer<ALFAPLATESCNT;iLayer++)
				{
                                        Int_t sz0 = mapLayers[2*iLayer].ListFibers.size();
					if (sz0 > 0 && sz0<=m_iMultiplicityCut) iNumUFiberHits++;
                                        Int_t sz1 = mapLayers[2*iLayer].ListFibers.size();
					if (sz1 > 0 && sz1<=m_iMultiplicityCut) iNumVFiberHits++;
				}

				if (iNumUFiberHits>=m_iNumLayerCut && iNumVFiberHits>=m_iNumLayerCut)
				{
					//First projection on V side
					//-------------------------------
					//filling the array for V side with reference value
					Proj_Store(mapLayers, 1, Over_n, b_ref_p, 1);
					Find_Proj(Over_n, b_ref_p, b_neg, OvV, NumV);

					if (NumV>=m_iUVCut)
					{
						//Now make the second projection step
						//-----------------------------------
						//U side
						Proj_Store(Fiber_MB_tmp, Over_p, b_neg, 0);
						Find_Proj(Over_p, b_neg, b_pos, OvU, NumU);

						//V side
						Proj_Store(mapLayers, 1, Over_n, b_pos, 1);
						Find_Proj(Over_n, b_pos, b_neg, OvV, NumV);

						//Third projection steps
						//----------------------
						//U side
						Proj_Store(Fiber_MB_tmp, Over_p, b_neg, 0);
						Find_Proj(Over_p, b_neg, b_pos, OvU, NumU);

						//V side
						Proj_Store(mapLayers, 1, Over_n, b_pos, 1);
						Find_Proj(Over_n, b_pos, b_neg, OvV, NumV);

	//					//We store the information in the vector
						b_p.push_back(b_pos);
						Ov_p.push_back(OvU);
						Num_p.push_back(NumU);

						b_n.push_back(b_neg);
						Ov_n.push_back(OvV);
						Num_n.push_back(NumV);

						iTrackMatch[0].push_back(cnt_step_U);
						iTrackMatch[1].push_back(cnt_step_V);

						//Once done we want to remove the hit belonging to the first track on side V
						//We first find the corresponding fibers
						Finding_Fib(mapLayers, 1,b_pos, b_neg, FSel_neg, 1);

						for (Int_t iLayer=0; iLayer<ALFAPLATESCNT; iLayer++)
						{
							FSel_n[iLayer].push_back(FSel_neg[iLayer]);
//							FSel_p[iLayer].push_back(FSel_pos[iLayer]);
							FSel_p[iLayer].push_back(FSel_pos_tmp[iLayer]);
						}

						//Removing fibers used for the first track for V Side
						for (Int_t iLayer=0; iLayer<ALFAPLATESCNT; ++iLayer)
						{
							const unsigned int thisLayer=2*iLayer+1;
							const std::list<int> & thisFiberContainer = mapLayers[thisLayer].ListFibers;
							for (const auto & thisFiber:thisFiberContainer)
							{
								if (thisFiber == (int)FSel_neg[iLayer])
								{
									auto it = std::find(begin(thisFiberContainer), end(thisFiberContainer), thisFiber);
									mapLayers[2*iLayer+1].ListFibers.erase(it);
									break;
								}
							}


						}
						cnt_step_V++;
					}
				}
				else NumV = 0;
			}
			while (NumV>=m_iUVCut);

//			When we cannot find tracks anymore for the V side, and that all combinations with the U side has been done, we start again with the U side
//			But first we remove the fibers belonging to this track

			if (cnt_step_V>0)
			{
				for (Int_t iLayer=0; iLayer<ALFAPLATESCNT; iLayer++)
				{
                                        FIBERS& fiber = mapLayers[2*iLayer];
					std::list<int>::iterator itBeg = fiber.ListFibers.begin();
					std::list<int>::iterator itEnd = fiber.ListFibers.end();
					for (; itBeg != itEnd; ++itBeg)
					{
						if (*itBeg == (int)FSel_pos_tmp[iLayer])
						{
							fiber.ListFibers.erase(itBeg);
							break;
						}
					}



				}
			}
			else
			{
				if (NumV>0)
				{
					for (Int_t iLayer=0; iLayer<ALFAPLATESCNT; iLayer++)
					{
                                                FIBERS& fiber = mapLayers[2*iLayer];
						std::list<int>::iterator itBeg = fiber.ListFibers.begin();
						std::list<int>::iterator itEnd = fiber.ListFibers.end();
						for (; itBeg != itEnd; ++itBeg)
						{
							if (*itBeg == (int)FSel_pos_tmp[iLayer])
							{
								fiber.ListFibers.erase(itBeg);
								break;
							}
						}


					}
				}
				else break;
			}
			cnt_step_U++;
		}
	}
	while (NumU>=m_iUVCut);
}

void ALFA_MDMultiple::GetData(Int_t (&iNumU)[MAXTRACKNUM], Int_t (&iNumV)[MAXTRACKNUM], Float_t (&fOvU)[MAXTRACKNUM], Float_t (&fOvV)[MAXTRACKNUM], Int_t (&iFibSel)[MAXTRACKNUM][ALFALAYERSCNT*ALFAPLATESCNT])
{
	ATH_MSG_DEBUG("ALFA_MDMultiple::GetData()");

	Int_t iTrackNum;
	size_t iSize;
	std::fill_n(&fOvU[0], sizeof(fOvU)/sizeof(Float_t), -9999.0);
	std::fill_n(&fOvV[0], sizeof(fOvV)/sizeof(Float_t), -9999.0);
	std::fill_n(&iNumU[0], sizeof(iNumU)/sizeof(Int_t), -9999);
	std::fill_n(&iNumV[0], sizeof(iNumV)/sizeof(Int_t), -9999);
	std::fill_n(&iFibSel[0][0], sizeof(iFibSel)/sizeof(Int_t), -9999);

	iTrackNum=0;
	iSize=0;
	for (auto & iLayer : m_iFibSel)
	{
		iSize = std::max(iLayer.size(), iSize);
	}
	iTrackNum = (iSize < MAXTRACKNUM)? iSize : MAXTRACKNUM;
	for (Int_t iTrack=0; iTrack<iTrackNum; iTrack++)
	{
		for (Int_t iLayer=0; iLayer<ALFALAYERSCNT*ALFAPLATESCNT; iLayer++)
		{
			iFibSel[iTrack][iLayer] = m_iFibSel[iLayer].at(iTrack);
		}
	}

	iTrackNum=0;
	iSize=0;
	iSize = std::min(m_fOvU.size(), m_fOvV.size());
	iTrackNum = (iSize < MAXTRACKNUM)? iSize : MAXTRACKNUM;
	for (Int_t iTrack=0; iTrack<iTrackNum; iTrack++)
	{
		fOvU[iTrack] = m_fOvU.at(iTrack);
		fOvV[iTrack] = m_fOvV.at(iTrack);
	}

	iTrackNum=0;
	iSize=0;
	iSize = std::min(m_iNU.size(),  m_iNV.size());
	iTrackNum = (iSize < MAXTRACKNUM)? iSize : MAXTRACKNUM;
	for (Int_t iTrack=0; iTrack<iTrackNum; iTrack++)
	{
		iNumU[iTrack] = m_iNU.at(iTrack);
		iNumV[iTrack] = m_iNV.at(iTrack);
	}
}
