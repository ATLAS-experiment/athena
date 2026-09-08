/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_MUONSetBranch_H
#define MUONTESTER_MUONSetBranch_H

#include <MuonTesterTree/MuonTesterBranch.h>

// Implementation to store vector like variables
namespace MuonVal {
class MuonTesterTree;
template <class T> class SetBranch : public MuonTesterBranch, virtual public IMuonTesterBranch {
public:
    /// Standard constructor
    SetBranch(TTree* tree, const std::string& name);
    SetBranch(MuonTesterTree& tree, const std::string& name);

    virtual ~SetBranch() = default;
    /// Clears vector in cases that it has not been updated in this event
    /// Retursn falls if the vector has not been initialized yet
    bool fill(const EventContext& ctx) override;
    /// Initialized the Branch
    bool init() override;

    /// Returns the number of actual saved elements
    inline size_t size() const;

    /// Adds a new element at the end of the vector
    inline void insert(const T& value);
    inline void operator+=(const T& value);

    /// Getter methods
    std::set<T>& operator->();
    std::set<T>& get();    
    const std::set<T>& operator->() const;
    const std::set<T>& get() const;
    /// Assignment operator
    void operator=(const std::set<T>& other);

    inline friend std::ostream& operator<<(std::ostream& ostr, const SetBranch& br) {
        ostr<<br.name()<<": ";
        if (br.isUpdated()){
            ostr<<br.m_variable;
        } else ostr<<"{}";
        return ostr;
    };

    inline bool isUpdated() const;

private:
    std::set<T> m_variable{};   
    bool m_updated{false};
};
}
#include <MuonTesterTree/SetBranch.icc>
#endif
