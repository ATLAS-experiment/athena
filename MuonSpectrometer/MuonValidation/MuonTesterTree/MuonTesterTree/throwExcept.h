/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonTesterTree_THROWEXCEPT_H
#define MuonTesterTree_THROWEXCEPT_H

#include <exception>
#include <sstream>

#define THROW_EXCEPTION(MESSAGE)                      \
    {                                                 \
        std::stringstream except_str{};               \
        except_str<<__FILE__<<":"<<__LINE__<<" --- "; \
        except_str<<MESSAGE;                          \
        throw std::runtime_error(except_str.str());   \
    }
#endif