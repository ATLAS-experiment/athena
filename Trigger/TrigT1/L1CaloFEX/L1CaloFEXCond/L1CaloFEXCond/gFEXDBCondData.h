/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for gFEXDBCondData - Tool to read the COOL DB for gFEX
//                              -------------------
//     begin                : 23 10 2024
//     email                : jared.little@cern.ch
//***************************************************************************

#ifndef gFEXDBCondData_H
#define gFEXDBCondData_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include <unordered_map>

namespace LVL1 {

class gFEXDBCondData
{

    public:

        // getters
        const std::array<int,12>& get_Aslopes() const;
        const std::array<int,12>& get_Bslopes() const;
        const std::array<int,12>& get_Cslopes() const;

        const std::array<int,12>& get_AnoiseCuts() const;
        const std::array<int,12>& get_BnoiseCuts() const;
        const std::array<int,12>& get_CnoiseCuts() const;

        // setters
        void set_Aslopes(const std::array<int,12>& params);
        void set_Bslopes(const std::array<int,12>& params);
        void set_Cslopes(const std::array<int,12>& params);

        void set_AnoiseCuts(const std::array<int,12>& params);
        void set_BnoiseCuts(const std::array<int,12>& params);
        void set_CnoiseCuts(const std::array<int,12>& params);

    private:
        std::array<int,12> m_Aslopes{0};
        std::array<int,12> m_Bslopes{0};
        std::array<int,12> m_Cslopes{0};

        std::array<int,12> m_AnoiseCuts{0};
        std::array<int,12> m_BnoiseCuts{0};
        std::array<int,12> m_CnoiseCuts{0};

};

} // end of namespace LVL1

CLASS_DEF( LVL1::gFEXDBCondData , 188973968 , 1 )
CONDCONT_DEF( LVL1::gFEXDBCondData , 21542672 );

#endif