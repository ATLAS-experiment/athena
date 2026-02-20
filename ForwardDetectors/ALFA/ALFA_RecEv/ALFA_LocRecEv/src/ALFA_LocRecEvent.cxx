/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "ALFA_LocRecEv/ALFA_LocRecEvent.h"



ALFA_LocRecEvent::ALFA_LocRecEvent(int iAlgoNum, int n_pot_num, float x_pos, float y_pos, float fOverU, float fOverV, int iNumU, int iNumV, std::vector<int> iFibSel):
	m_iAlgoNum(iAlgoNum), m_pot_num(n_pot_num), m_x(x_pos), m_y(y_pos), m_fOverU(fOverU), m_fOverV(fOverV), m_iNumU(iNumU), m_iNumV(iNumV), m_iFibSel(std::move(iFibSel))
{}
