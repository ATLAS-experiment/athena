// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "McBremPoint.h"

McBremPoint::McBremPoint(double x, double y, double z, double loss)
: m_x(x), m_y(y), m_z(z), m_loss(loss)
{
}

McBremPoint::~McBremPoint(void)
{
}
