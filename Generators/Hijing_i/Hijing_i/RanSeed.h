/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef RanSeed_h
#define RanSeed_h

#include <cstdint>

extern "C" { uintptr_t ranseed_address_(); }

/**
@class RanSeed
@brief  Class definition for RanSeed, which is used
      to modify the Hijing RANSEED common.
*/
class RanSeed
{
public:
    RanSeed();
    ~RanSeed();
    
    int&	nseed	(void);
    
private: 

    struct RANSEED;
    friend struct RANSEED;

    struct RANSEED
    {
	int	nseed;
    };

    static RANSEED* s_ranseed;
};

// set pointer to zero at start
RanSeed::RANSEED* RanSeed::s_ranseed =0;

inline
RanSeed::RanSeed()
{}

inline
RanSeed::~RanSeed()
{}

inline int&
RanSeed::nseed	(void)
{
    if (!s_ranseed) s_ranseed = reinterpret_cast<RANSEED*>(ranseed_address_());
    return s_ranseed->nseed;
}

#endif
