// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#pragma once

struct TrackInfoStruct
{
public:
	TrackInfoStruct(void);
	~TrackInfoStruct(void);
	float m_params[5]{};
	int m_nHits{0};
	long m_nOffsetG{0};
	long m_nOffsetH{0};
};
