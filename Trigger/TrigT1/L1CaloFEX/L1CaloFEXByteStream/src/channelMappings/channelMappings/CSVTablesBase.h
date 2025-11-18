/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CSV_TABLES_H
#define CSV_TABLES_H
#include <array>
#include <memory>
#include "CSVWrapper.h"

class CSVTablesBase{
    public:
        std::shared_ptr<CSVWrapper> GetTable(int table_num);
   protected:
        CSVTablesBase(const std::string & subsystem);
    private:
        CSVTablesBase(CSVTablesBase const&) = delete;
        void operator=(CSVTablesBase const&)  = delete;
        std::array<std::shared_ptr<CSVWrapper>, 6> m_tables;
        std::array<bool, 6> m_table_opens;
        std::string m_subsystem;
};


#endif
