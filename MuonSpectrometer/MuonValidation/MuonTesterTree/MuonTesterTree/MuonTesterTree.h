/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_MUONTESTERTREE_H
#define MUONTESTERTREE_MUONTESTERTREE_H

#include <GaudiKernel/ITHistSvc.h> //service handle template argument
#include <GaudiKernel/ServiceHandle.h>
#include <MuonTesterTree/IMuonTesterBranch.h> //for IMuonTesterBranch::DataDependency
#include <AthenaBaseComps/AthMessaging.h> //parent class
//
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <type_traits> //for std::enable_if
//
//need the full class declarations, function templates cannot return incomplete types
#include <MuonTesterTree/MatrixBranch.h>
#include <MuonTesterTree/ScalarBranch.h>
#include <MuonTesterTree/VectorBranch.h>
#include <MuonTesterTree/SetBranch.h>
//
class TTree;

namespace MuonVal {

class EventHashBranch;

class MuonTesterTree: public AthMessaging {
public:
    MuonTesterTree(const std::string& tree_name, const std::string& stream);
    ~MuonTesterTree();
    
    /// TTree object
    TTree* tree();
    const TTree* tree() const;
    
    /// Operator to the TTree object
    TTree* operator->();
    const TTree* operator->() const;

    /// Has the init method been called and the tree is connected with the output file
    bool initialized() const;

    /// Fills the tree per call
    bool fill(const EventContext& ctx);

    /// Name of the tree
    std::string name() const;
    /// file_stream of the analysis to which the tree belongs
    const std::string& fileStream() const;
    /// sub directory in the TFile
    const std::string& path() const;
  
    /// Save the TTree in a subfolder of the TFile
    void setPath(const std::string& new_path);
    
    /// Initialize method
   template <class OWNER,
            typename = typename std::enable_if<std::is_base_of<IProperty, OWNER>::value>::type>
   StatusCode init(OWNER* instance);

    /// Finally write the TTree objects
    StatusCode write();
    /// This method adds the branch to the tree and hands over the ownership
    /// to the MuonAnalysisTree instance
    /// IF the second argument is set to false
    /// the branch is not added to the active list of branches i.e.
    /// no fill and initialize function is called on it
    bool registerBranch(std::shared_ptr<IMuonTesterBranch> branch);

    /// Branch is added to the tree without transferring the ownership
    bool addBranch(std::shared_ptr<IMuonTesterBranch> branch);
    bool addBranch(IMuonTesterBranch& branch);
    bool addBranch(IMuonTesterBranch* branch);
    /// In case instances of a certain branch type are destroyed before hand
    void removeBranch(IMuonTesterBranch* branch);
    void removeBranch(IMuonTesterBranch& branch);

    /// Skips the branch from being added to the tree
    void disableBranch(const std::string& br_name);
    void disableBranch(const std::vector<std::string>& br_names);

    /// Retrieves the branches owned by the muon_tree
    template <class T> std::shared_ptr<T> getBranch(const std::string& str) const;

    /// Creates new branches and returns their reference
    template <typename T> VectorBranch<T>& newVector(const std::string& name);
    template <typename T> ScalarBranch<T>& newScalar(const std::string& name);
    template <typename T> MatrixBranch<T>& newMatrix(const std::string& name);
    template <typename T> SetBranch<T>& newSet(const std::string& name);
    
    /// Creates and returns branches with a default value
    template <typename T> VectorBranch<T>& newVector(const std::string& name, const T def_val);
    template <typename T> ScalarBranch<T>& newScalar(const std::string& name, const T def_val);
    template <typename T> MatrixBranch<T>& newMatrix(const std::string& name, const T def_val);
    

    /// returns the reference of the branch
    template <typename T> T& newBranch(std::shared_ptr<T> br);

    /// Returns a boolean whether the branch is already part of the tree or one of the deligated friends
    bool isActive(const IMuonTesterBranch* branch) const;

    /// Returns Whether the Tree is a common tree or not
    bool isCommonTree() const;    
    /// Appends the other tester Tree as friend to this instance
    using FriendTreePtr = std::shared_ptr<MuonTesterTree>;
    
    bool addCommonTree(FriendTreePtr common_tree);
    ///
    const std::vector<FriendTreePtr>& getFriends() const;
private:
    /// Adds the other TTree as a Client and declares this instance
    /// as a Common Tree
    bool addClient(MuonTesterTree* client);
    
     /// Initialze the tree with the output file. The stream corresponds to the stream
    /// of the file e.g MDTTester HighEtaTester
    StatusCode init(const ServiceHandle<ITHistSvc> & hist_svc);
    
    using DataDependency = IMuonTesterBranch::DataDependency;
    std::vector<DataDependency> m_dependencies{};
    unsigned int m_depCounter{0};

    std::unique_ptr<TTree> m_tree{nullptr};
    std::string m_stream{};
    std::string m_path{};
    /// Flag to avoid double initialization with the StoreGate
    bool m_init{false};
    /// Flag to indicate whether the TTree is written to the file or not
    bool m_written{false};

    // Careful: m_branches_to_init must come before m_branches.
    // When m_branches is destroyed, the branches being deleted will
    // try to remove themselves from m_branches_to_init.  If these are
    // in the opposite order, then m_branches_to_init will already
    // have been destroyed, and we'll be accessing freed memory.

    // List of branches which shall be initialized. The list will be sorted before initialization and then cleared
    // None of the branches is explicitly owned by the object
    std::vector<IMuonTesterBranch*> m_branches_to_init{};
    // List of branches which are owned by the tree. They will be deleted if the object is destroyed
    std::vector<std::shared_ptr<IMuonTesterBranch>> m_branches{};
    /// Set of branches that were already initialized
    std::set<IMuonTesterBranch*> m_initialized_br{};

    std::set<std::string> m_excludedBranches{};

    TDirectory* m_directory{nullptr};
    bool m_filled{false};
    ServiceHandle<ITHistSvc> m_hist_svc{"", ""};
  
    /// List of all other common instances that are acting as friends.
    std::vector<FriendTreePtr> m_friendLinks{};
    /// List of all other MuonTesterTree instances using this instance as a common Tree
    /// If the Tree has one client it is declared as a common Tree. It can then only be
    /// filled once in an event
    std::set<MuonTesterTree*> m_commonClients{};

    std::shared_ptr<EventHashBranch> m_hash_br;

};
}
#include <MuonTesterTree/MuonTesterTree.icc>
#endif
