/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_MUONTESTERBRANCH_H
#define MUONTESTER_MUONTESTERBRANCH_H

#include <MuonTesterTree/IMuonTesterBranch.h>
#include <AthenaBaseComps/AthMessaging.h>
//
#include <string>
#include <vector>
//
class TTree;

/// First stage of the IMuonTesterBranch implementation
///  --> saves the name of the MuonTree as member and
///      provides a generic method to add the data variable
///      to the TTree in the initialization stage
namespace MuonVal {
class MuonTesterTree;
class MuonTesterBranch : public AthMessaging, virtual public IMuonTesterBranch {
public:
    MuonTesterBranch(MuonTesterTree& tree, const std::string& br_name);
    /// Standard constructor taking the TTree object and the name of the branch to be added to the TTree
    MuonTesterBranch(TTree* tree, const std::string& br_name);

    MuonTesterBranch(const MuonTesterBranch&) = delete;
    void operator=(const MuonTesterBranch&) = delete;
    virtual ~MuonTesterBranch();

    std::string name() const override final;
    /// Returns whether the object has been initialized or not
    /// The outcome of the variable is bound to a successful call
    /// of the addToTree method
    bool initialized() const;
    /// Returns the underlying TTree object
    TTree* tree() override final;
    const TTree* tree() const override final;
    /// Returns the data dependencies needed by the MuonTesterBranch
    std::vector<DataDependency> data_dependencies() override final;

    static std::string eraseWhiteSpaces(const std::string& In);

protected:
    /// Connects the Memory buffer with the TTree
    template <class T> bool addToTree(T& variable);
    /// Declares the ReadHandle/ ReadCondHandleKey as data dependency of the algorithm
    template<class Key> bool declare_dependency(Key& key);

    /// Returns the reference to the MuonTesterTree parent. 
    // Does not work if the TTree pointer is given during construction
    MuonTesterTree& parent();




private:
    TTree* m_tree{nullptr};
    MuonTesterTree* m_parent{nullptr};
    std::string m_name{};
    bool m_init{false};
    std::vector<DataDependency> m_dependencies{};
};
}
#include <MuonTesterTree/MuonTesterBranch.icc>
#endif
