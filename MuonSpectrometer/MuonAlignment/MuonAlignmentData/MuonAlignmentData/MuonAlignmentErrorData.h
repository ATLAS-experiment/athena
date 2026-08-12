// Dear emacs, this is -*-c++-*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonAlignmentErrorData is condition data which is derived and recorded by MuonAlignmentErrorDbAlg
 */

#ifndef MUONALIGNMENTERRORDATA_H
#define MUONALIGNMENTERRORDATA_H

#include <regex>
#include <vector>
#include <unordered_map>
#include <Identifier/Identifier.h>

// Struct for per-Station Deviations Information

class MuonAlignmentErrorData {
    friend class MuonAlignmentErrorDbAlg;

public:
    struct MuonAlignmentErrorRule {
        std::regex stationName {""};
        std::regex multilayer {""};
        double translation {0.0};
        double rotation {0.0};
    };
    using MuonAlignmentErrorRuleIndex = size_t;
    struct MuonAlignmentErrorRuleCache {
        /**
         * For each multilayer identifier, cache the indices of the affecting rules
         */
        std::unordered_multimap<Identifier, MuonAlignmentErrorRuleIndex> id_rule_map{};
    };

    MuonAlignmentErrorData() = default;
    virtual ~MuonAlignmentErrorData() = default;

    void setAlignmentErrorRules(std::vector<MuonAlignmentErrorRule>&& vec);
    [[nodiscard]] const std::vector<MuonAlignmentErrorRule>& getAlignmentErrorRules() const;

    void setMuonAlignmentErrorRuleCache(std::vector<MuonAlignmentErrorRuleCache>&& vec_new);
    [[nodiscard]] const std::vector<MuonAlignmentErrorRuleCache>& getMuonAlignmentErrorRuleCache() const;

    void setClobVersion(std::string clobVersion);
    [[nodiscard]] const std::string& getClobVersion() const;

    void setHasNswHits(bool val);
    [[nodiscard]] bool hasNswHits() const;

private:
    std::vector<MuonAlignmentErrorRule> m_deviations {};
    std::string m_clobVersion {"0.1"};
    bool m_hasNswHits {false};
    std::vector<MuonAlignmentErrorRuleCache> m_deviations_new {};
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(MuonAlignmentErrorData, 115867308, 1)
#include "AthenaKernel/CondCont.h"
CLASS_DEF(CondCont<MuonAlignmentErrorData>, 265772564, 0)

#endif
