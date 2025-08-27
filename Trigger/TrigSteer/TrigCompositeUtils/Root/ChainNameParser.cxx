/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <string>
#include <regex>
#include <array>
#include <algorithm>

#include "TrigCompositeUtils/ChainNameParser.h"

#include <iostream>

namespace
{
    std::string join(const std::vector<std::string> &parts, const std::string &piece)
    {
        std::string result;
        if (parts.empty())
            return result;
        auto itr = parts.begin();
        result = *itr;
        for (++itr; itr != parts.end(); ++itr)
            result += piece + *itr;
        return result;
    }

    const std::regex &legHeadRegex()
    {
        const static std::regex re(ChainNameParser::legHeadPattern());
        return re;
    }
}

namespace ChainNameParser {

    std::string LegInfo::legName() const
    {
        std::string result;
        if (multiplicity != 1)
            result += std::to_string(multiplicity);
        result += signature;
        if (threshold != -1)
            result += std::to_string(threshold);
        if (legParts.size())
            result += "_" + join(legParts, "_");
        return result;
    }

    xAODType::ObjectType LegInfo::type() const
    {
        if (signature == "e")
        {
            if (std::find(legParts.begin(), legParts.end(), "etcut") != legParts.end())
                return xAODType::CaloCluster;
            else
                return xAODType::Electron;
        }
        else if (signature == "g")
        {
            if (std::find(legParts.begin(), legParts.end(), "etcut") != legParts.end())
                return xAODType::CaloCluster;
            else
                return xAODType::Photon;
        }
        else if (signature == "j" or signature == "dispjet")
            return xAODType::Jet;
        else if (signature == "mu")
            return xAODType::Muon;
        else if (signature == "tau")
            return xAODType::Tau;
        else if (signature == "isotrk")
            return xAODType::TrackParticle;
        else
            return xAODType::Other;
    }

    LegInfoIterator::LegInfoIterator(const std::string &chain) :
        m_itr(chain.begin()), m_end(chain.end())
    {
        // Move the iterator until we've found the start of a leg
        while (!advance()) {}
        // Now we have the next leg info stored in the peek variables, but not the current.
        // Advance the iterator once to store these in the current
        this->operator++();
    }

    bool LegInfoIterator::operator==(const LegInfoIterator &other) const
    {
        return m_itr == other.m_itr && m_end == other.m_end && m_peekSignature == other.m_peekSignature;
    }

    bool LegInfoIterator::operator!=(const LegInfoIterator &other) const
    {
        return !(*this == other);
    }

    LegInfoIterator::reference LegInfoIterator::operator *() const
    {
        return m_current;
    }

    LegInfoIterator::pointer LegInfoIterator::operator->() const
    {
        return &m_current;
    }

    LegInfoIterator &LegInfoIterator::operator++()
    {
        if (m_peekSignature.empty() && m_itr == m_end)
        {
            // No more signatures to find, exhaust the iterator
            m_current = {};
            m_itr = std::string::const_iterator();
            m_end = std::string::const_iterator();
            m_peekMultiplicity = 0;
            m_peekThreshold = -1;
        }
        else
        {
            // Copy the peeked information into the current info
            m_current.multiplicity = m_peekMultiplicity;
            m_current.signature = m_peekSignature;
            m_current.threshold = m_peekThreshold;
            m_current.legParts.clear();
            m_peekSignature.clear();
            // Now step through until we find the next leg
            while (!advance()) {}
        }
        return *this;
    }

    LegInfoIterator LegInfoIterator::operator++(int)
    {
        LegInfoIterator itr(*this);
        this->operator++();
        return itr;
    }

    bool LegInfoIterator::exhausted() const
    {
        return m_itr == std::string::const_iterator();
    }

    bool LegInfoIterator::advance()
    {
        std::string::const_iterator next = std::find(m_itr, m_end, '_');
        std::smatch match;
        if (std::regex_match(m_itr, next, match, legHeadRegex()))
        {
            // This means we've found the start of the next leg. Record its data and return true
            if (match.str(1).empty())
                m_peekMultiplicity = 1;
            else
                m_peekMultiplicity = std::atoi(match.str(1).c_str());
            m_peekSignature = match.str(2);
            if (match.str(3).empty())
                m_peekThreshold = -1;
            else
                m_peekThreshold = std::atoi(match.str(3).c_str());
            // Advance the iterator (skip the underscore if there is one)
            m_itr = next == m_end ? next : next + 1;
            return true;
        }
        else if (next == m_end)
        {
            m_current.legParts.emplace_back(m_itr, next);
            // Setting the iterator to the end
            m_itr = m_end;
            return true;
        }
        else
        {
            // Otherwise this is just a leg
            m_current.legParts.emplace_back(m_itr, next);
            // Advance the iterator (skip the underscore if there is one)
            m_itr = next == m_end ? next : next + 1;
            return false;
        }
    }

    std::string HLTChainInfo::l1Item() const
    {
        std::size_t pos = m_chain.rfind("_L1");
        if (pos == std::string::npos)
            return "";
        else
            return "L1_" + m_chain.substr(pos+3);
    }

    const std::vector<std::string> &allSignatures()
    {
        const static std::vector<std::string> signatures{
            "e", "g", "j", "dispjet", "mu", "tau", "xe", "xs", "te", "ht", "noalg", "mb", "hi", "eb",
            "l1calocalib", "lar", "zdc", "lumipeb", "alfacalib", "calibAFP", "afp", "distrk", 
            "hitdvjet", "isotrk", "dedxtrk", "l1topoPh1debug", "caloclustermon", "fslrt",
            "beamspot", "cosmic", "timeburner", "mistimemonj400", "larsupercellmon", "larnoiseburst",
            "acceptedevts", "larpsall", "larpsallem", "idcalib", "metcalo", "mettrk", 
        };
        return signatures;
    }

    const std::vector<std::string> &allSignaturePostfixQualifiers()
    {
        const static std::vector<std::string> postfixQualifiers{
            "noL1", "vtx", "c", "C", "f", "a"
        };
        return postfixQualifiers;
    }

    const std::set<std::string> &singleLegIdentifiers()
    {
        const static std::set<std::string> singleLegIDs{
            "noalg", "acceptedevts"
        };
        return singleLegIDs;
    }

    std::string legHeadPattern()
    {
        // Pattern looks like an expanded version of "(\d*)(e|g|j|mu|tau|xe)(\d*)(noL1|vtx|c|f|a)?";
        // i.e. between 0-inf digits, followed by a signature, followed by another 0-inf digits, optionally followed by 0-1 a postfix qualifiers
        return "(\\d*)("+join(allSignatures(), "|")+")(\\d*)"+"("+join(allSignaturePostfixQualifiers(), "|")+")?";
    }

    std::vector<int> multiplicities(const std::string &chain)
    {
        std::vector<int> multiplicities;
        for (auto itr = LegInfoIterator(chain); !itr.exhausted(); ++itr) {
            multiplicities.push_back(itr->multiplicity);
            if (singleLegIdentifiers().count(itr->signature) == 1)
                return multiplicities;
        }
        return multiplicities;
    }

    std::vector<std::string> signatures(const std::string &chain)
    {
        std::vector<std::string> signatures;
        for (auto itr = LegInfoIterator(chain); !itr.exhausted(); ++itr) {
            signatures.push_back(itr->signature);
            if (singleLegIdentifiers().count(itr->signature) == 1)
                return signatures;
        }
        return signatures;
    }

} //> end namespace ChainNameParser

