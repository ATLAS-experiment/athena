/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFEX_HARDWARE_INFO_H
#define EFEX_HARDWARE_INFO_H

#include <string>

class EfexHardwareInfo {
    public:
        //Blank Invalid Constructor
        EfexHardwareInfo() = default;
        //Constructor
        EfexHardwareInfo(const std::string & efexlabel,
                        int fibre,
                        int inputconnector,
                        const std::string & mpod
                        );
        // Get methods
        std::string     getEFEXLabel() const;
        int             getFibreNumber() const;
        int             getRibbonFibreNumber() const;
        int             getInputConnector() const;
        int             getMpodNumber() const;
        std::string     getMpodLabel() const;
        bool            getValidity() const;
        void            setOverlap(int overlap);
        int             getOverlap() const;
        //Prints
        void            printInfo() const;
    private:
        bool m_valid{};
        std::string m_efexlabel{"invalid"}; 
        int m_fibre{-1};
        int m_inputconnector{-1};
        std::string m_mpodlabel{"invalid"};
        int m_overlap{};

};
#endif
