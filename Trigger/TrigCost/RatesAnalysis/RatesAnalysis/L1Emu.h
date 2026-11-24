/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Author: Teng Jian Khoo
// Class representing emulated L1 logic combining preexisting items in the menu
#ifndef RATESANALYSIS_L1EMU_H
#define RATESANALYSIS_L1EMU_H 1

#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include <regex>
#include "AthenaBaseComps/AthMessaging.h"


class LogicalElement {
    public:
    virtual ~LogicalElement() = default;
    // Receive map of threshold to multiplicity input to CTP
    virtual bool evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug=false) const = 0;
    virtual bool isGroup() const {return false;};
    virtual std::string to_string() const = 0;
};

std::ostream& operator << ( std::ostream& outs, const LogicalElement& l);


class TriggerElement: public LogicalElement {
    private:
    std::string m_threshold{""};
    size_t m_multiplicity{0};

    public:
    TriggerElement(const std::string& threshold, size_t multiplicity) :
        m_threshold(threshold), m_multiplicity(multiplicity)
    {}

    virtual bool evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug) const override final;

    const std::string& threshold() const {return m_threshold;}

    size_t multiplicity() const {return m_multiplicity;}

    virtual std::string to_string() const override final;
};


class TriggerGroup: public LogicalElement {
    protected:
    std::vector<std::unique_ptr<LogicalElement> > m_inputs{};

    public:
    // Can't construct abstract LogicalElement, and don't want to
    // override with all possible derived types
    // So take ownership of the provided unique_ptr
    void append(std::unique_ptr<LogicalElement> pel);

    std::set<std::pair<std::string,size_t> > getAllThresholdsMults() const;

    virtual bool isGroup() const override final  {return true;};
};

class TriggerGroupAND: public TriggerGroup {
    public:
    TriggerGroupAND() {};

    virtual bool evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug=false)const override final;

    virtual std::string to_string() const override final;

};

class TriggerGroupOR: public TriggerGroup {

    public:
    TriggerGroupOR() {};

    virtual bool evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug=false) const override final;

    virtual std::string to_string() const override final;
};

class L1Emu : public AthMessaging {
    private:
    std::string m_name;
    std::string m_def;
    std::unique_ptr<LogicalElement> m_topLogicalElement{nullptr};

    public:
    L1Emu(const std::string& name, const std::string& def, const MSG::Level lvl=MSG::INFO);

    std::string to_string() const {return m_topLogicalElement->to_string();};

    bool evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug=false) const;

    std::unique_ptr<TriggerElement> parseThreshold(const std::string& def);

    std::unique_ptr<LogicalElement> parseDefinition(const std::string& def);

    std::set<std::pair<std::string,size_t> > getAllThresholdsMults() const;
};

#endif
