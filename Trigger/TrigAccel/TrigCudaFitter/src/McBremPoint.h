// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __MCBREMPOINT_H__
#define __MCBREMPOINT_H__


class McBremPoint
{
public:
	McBremPoint(double, double, double, double);
public:
	virtual ~McBremPoint(void);
	double m_x, m_y, m_z, m_loss;
};

#endif
