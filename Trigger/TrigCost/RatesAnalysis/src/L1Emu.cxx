/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Class representing emulated L1 logic combining preexisting items in the menu

#include <stdexcept>
#include "RatesAnalysis/L1Emu.h"
#include "AthenaBaseComps/AthMessaging.h"


std::ostream& operator << ( std::ostream& outs, const LogicalElement& l) {
    return outs << l.to_string();
}

bool TriggerElement::evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug) const {
    if(debug) {std::cout << m_threshold << " has multiplicity " << resultMap.at(m_threshold) << ", require " << m_multiplicity << std::endl;}
    return resultMap.at(m_threshold)>=m_multiplicity;
}


std::string TriggerElement::to_string() const {
    std::stringstream ss;
    ss << m_threshold << "[x" << m_multiplicity << "]";
    return ss.str();
}



void TriggerGroup::append(std::unique_ptr<LogicalElement> pel) {
    m_inputs.emplace_back(std::move(pel));
}


std::set<std::pair<std::string,size_t> > TriggerGroup::getAllThresholdsMults() const {
    std::set<std::pair<std::string,size_t> > out{};
    for(const auto& i : m_inputs) {
        if(i->isGroup()) {
            const TriggerGroup* tg = static_cast<const TriggerGroup*>(i.get());
            auto itm = tg->getAllThresholdsMults();
            out.insert(itm.begin(),itm.end());
        } else {
            const TriggerElement* te = static_cast<const TriggerElement*>(i.get());
            out.insert(std::make_pair(te->threshold(), te->multiplicity()));
        }
    }
    return out;
}



bool TriggerGroupAND::evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug) const {
    bool result = std::all_of(m_inputs.cbegin(),m_inputs.cend(),[resultMap](const std::unique_ptr<LogicalElement>& l) {return l->evaluate(resultMap);});
    if(debug) {
        std::cout << "AND of" << std::endl;
        for(const auto& i : m_inputs) {
            std::cout << "  " << *i << " --> " << i->evaluate(resultMap, debug) << std::endl;
        }
        std::cout << "  --> " << result << std::endl;
    }
    return result;
}


std::string TriggerGroupAND::to_string() const {
    std::stringstream ss;
    ss << "(";
    for (size_t ii{1}; const auto& i : m_inputs) {
        ss << *i;
        if(ii<m_inputs.size()) {
            ss << " & ";
        }
        ++ii;
    }
    ss << ")";
    return ss.str();
};



bool TriggerGroupOR::evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug) const {
    bool result = std::any_of(m_inputs.cbegin(),m_inputs.cend(),[resultMap](const std::unique_ptr<LogicalElement>& l) {return l->evaluate(resultMap);});
    if(debug) {
        std::cout << "OR of" << std::endl;
        for(const auto& i : m_inputs) {
            std::cout << "  " << *i << " --> " << i->evaluate(resultMap, debug) << std::endl;
        }
        std::cout << "  --> " << result << std::endl;
    }
    return result;
}

std::string TriggerGroupOR::to_string() const {
    std::stringstream ss;
    ss << "(";
    for (size_t ii{1}; const auto& i : m_inputs) {
        ss << *i;
        if(ii<m_inputs.size()) {
            ss << " | ";
        }
        ++ii;
    }
    ss << ")";
    return ss.str();
}



L1Emu::L1Emu(const std::string& name, const std::string& def, const MSG::Level lvl) :
    AthMessaging(name),
    m_name(name),
    m_def(def)
{
    this->setLevel(lvl);
    ATH_MSG_DEBUG("Initialising L1 emulation of " << name << " with definition:");
    ATH_MSG_DEBUG("  " << def);
    m_topLogicalElement = parseDefinition(def);
}


bool L1Emu::evaluate(const std::unordered_map<std::string, size_t>& resultMap, bool debug) const {
    if(debug) {
        std::cout << "Evaluate " << m_name << " = " << m_def << std::endl;
    }
    return m_topLogicalElement->evaluate(resultMap, debug);
}


std::unique_ptr<TriggerElement> L1Emu::parseThreshold(const std::string& def) {
    ATH_MSG_DEBUG( "Parsing threshold " << def );
    // Regular expression to capture the name of the trigger and the multiplicity in square brackets
    // e.g. jJ160[x1]
    std::regex re(R"(([\w\d_-]+)\[x(\d+)\](.*))");
    std::sregex_iterator iter(def.begin(), def.end(), re);
    std::sregex_iterator end;
    if(iter==end) {
        throw std::runtime_error( "ERROR: Malformed threshold " + def );
    }
    std::smatch match = *iter;
    if(!match.str(3).empty()) {
        throw std::runtime_error( "ERROR: Matched chars beyond valid threshold '" + def + "'" );
    }
    std::string triggerName = match.str(1);
    size_t triggerCount = std::stoi(match.str(2));
    return std::make_unique<TriggerElement>(triggerName,triggerCount);
}

std::unique_ptr<LogicalElement> L1Emu::parseDefinition(const std::string& def) {
    if(def.empty()) {
        throw std::runtime_error( "Received empty string" );;
    }
    // First search for logical expressions
    // - For simplicity we only handle either all '&' or all '|' per level in the hierarchy
    bool doAnd = false;
    bool doOr = false;
    size_t pos = 0;
    std::unique_ptr<LogicalElement> out{nullptr};
    size_t it = 0;
    size_t maxit{100};
    std::string tmp_def(def);
    std::erase(tmp_def,' ');
    while(pos < tmp_def.size() && it<maxit) {
        ATH_MSG_DEBUG( "(" << pos << "/" << tmp_def.size() << "): " << tmp_def.substr(pos,tmp_def.size()) );
        if(doAnd && doOr) {
            throw std::runtime_error("Mixture of & and |");
        }
        size_t next_sep = tmp_def.find_first_of("&|(",pos);
        // No operations or groups, this is a single threshold
        if(next_sep==tmp_def.npos) {
            // If already constructed, then this must be a group
            if(out) {
                if (out->isGroup()) {
                    static_cast<TriggerGroup*>(out.get())->append(parseThreshold(tmp_def.substr(pos)));
                } else {
                    throw std::runtime_error("Defined output is not group");
                }
            } else { // If empty, then this is a simple threshold that we can return
                out = parseThreshold(tmp_def.substr(pos));
            }
            pos = tmp_def.size();
        } else {
            ATH_MSG_DEBUG( "Next sep = " << tmp_def[next_sep] << " @ " << next_sep );
            // Split the string into segments we can process
            // 
            if(tmp_def[next_sep] == '&') {
                if(!doAnd) {
                    doAnd = true;
                    out = std::make_unique<TriggerGroupAND>();
                }
                if(next_sep-pos>0) { // Ignore &(
                    static_cast<TriggerGroup*>(out.get())->append(parseDefinition(tmp_def.substr(pos,next_sep-pos)));
                }
                pos = next_sep+1;
            } else if (tmp_def[next_sep] == '|') {
                if(!doOr) {
                    doOr = true;
                    out = std::make_unique<TriggerGroupOR>();
                }
                if(next_sep-pos>0) { // Ignore |(
                    static_cast<TriggerGroup*>(out.get())->append(parseDefinition(tmp_def.substr(pos,next_sep-pos)));
                }
                pos = next_sep+1;
            } else if (tmp_def[next_sep] == '(') {
                size_t depth{1};
                size_t gstart{next_sep+1};
                size_t gend{next_sep+1};
                size_t itt=0;
                size_t maxitt{100};
                while((depth>0) && (gend<tmp_def.size()) && (itt<maxit)) {
                    ATH_MSG_VERBOSE("  In group("<<depth<<")["<<gstart<<":"<<gend<<"] " << tmp_def.substr(gstart,gend-gstart) );
                    ATH_MSG_VERBOSE("    Remaining to parse: " << tmp_def.substr(gend+1) );
                    size_t next_paren = tmp_def.find_first_of("()",gend+1);
                    if(next_paren==tmp_def.npos) {
                        throw std::runtime_error( "Closing parentheses not found" );
                    } // Bad string
                    gend = next_paren;
                    ATH_MSG_VERBOSE( "    Next parenthesis " << tmp_def[next_paren] << " @ " << next_paren );
                    if(tmp_def[next_paren]=='(') {
                        depth++;
                        ATH_MSG_VERBOSE( "    Increase depth to " << depth << " (" << next_paren << "): " << tmp_def.substr(gstart,gend-gstart) );
                    } // Enter the next level of nesting
                    else if(tmp_def[next_paren]==')') {
                        depth--;
                        ATH_MSG_VERBOSE( "    Reduce depth to " << depth << " (" << next_paren << "): " << tmp_def.substr(gstart,gend-gstart) );
                    }  // Exit nested group
                    else {
                        throw std::runtime_error( "ERROR: Not a parenthesis" );
                    }
                    ++itt;
                }
                if(itt==maxitt) {
                    throw std::runtime_error( "Reached iteration cap, breaking" );
                }
                if((depth==0) && (gend<tmp_def.size())) {
                    std::string group{tmp_def.substr(gstart,gend-gstart)};
                    ATH_MSG_DEBUG( "  Finished parsing group("<<depth<<")["<<gstart<<":"<<gend<<"] " << group );
                    // If already constructed, then this must be a group
                    if(out) {
                        if (out->isGroup()) {
                            static_cast<TriggerGroup*>(out.get())->append(parseDefinition(group));
                        } else {
                            throw std::runtime_error( "Defined output is not group" ); // Parsing must have gone wrong
                        }
                    } else { // If empty, then fill with the parsed result
                        out = parseDefinition(group);
                    }
                } else {throw std::runtime_error( "Unterminated parentheses" );} // Unterminated group
                pos = gend+1;
            } else {
                throw std::runtime_error( "Unrecognised char" );
            } // Match has gone wrong somehow, should be unreachable
            if(out) {
                ATH_MSG_DEBUG( "So far constructed " << *out );
            }
        }
        ATH_MSG_DEBUG( "It[{}]: {}", it, out ? out->to_string() : "(null)" );
        ++it;
    }
    if(it==maxit) {
        throw std::runtime_error( "Reached iteration cap, breaking" );
    }
    ATH_MSG_DEBUG( "Parsed definition into: " << *out );
    return out;
}

std::set<std::pair<std::string,size_t> > L1Emu::getAllThresholdsMults() const {
    std::set<std::pair<std::string,size_t> > out{};
    if(m_topLogicalElement->isGroup()) {
        const TriggerGroup* tg = static_cast<const TriggerGroup*>(m_topLogicalElement.get());
        out = tg->getAllThresholdsMults();
    } else {
        const TriggerElement* te = static_cast<const TriggerElement*>(m_topLogicalElement.get());
        out.insert(std::make_pair(te->threshold(), te->multiplicity()));
    }
    return out;
}
