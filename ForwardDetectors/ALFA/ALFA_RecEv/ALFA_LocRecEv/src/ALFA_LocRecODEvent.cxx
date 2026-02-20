/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "ALFA_LocRecEv/ALFA_LocRecODEvent.h"

ALFA_LocRecODEvent::ALFA_LocRecODEvent(int iAlgoNum, int n_pot_num, int n_side , float y_pos, float fOverY, int iNumY, std::vector<int> iFibSel):
	m_iAlgoNum(iAlgoNum), m_pot_num(n_pot_num), m_side(n_side), m_y(y_pos), m_fOverY(fOverY), m_iNumY(iNumY), m_iFibSel(std::move(iFibSel))
{}



