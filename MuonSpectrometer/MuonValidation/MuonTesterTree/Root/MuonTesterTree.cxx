/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/EventHashBranch.h>
#include <AthenaBaseComps/AthMsgStreamMacros.h>
#include <GaudiKernel/ConcurrencyFlags.h>

namespace {
    // Erase all objects from the branch vector
    void Remove(std::vector<MuonVal::IMuonTesterBranch*>& vec, std::function<bool(const MuonVal::IMuonTesterBranch*)> remove_func) {
        // Could use std::erase_if with C++20...
        std::vector<MuonVal::IMuonTesterBranch*>::iterator itr = std::remove_if(vec.begin(), vec.end(), remove_func);
        vec.erase (itr, vec.end());
    }

}  // namespace
namespace MuonVal {

std::string MuonTesterTree::name() const { return m_tree->GetName(); }

TTree* MuonTesterTree::tree() { return m_tree.get(); }
TTree* MuonTesterTree::operator->() { return m_tree.get(); }
const TTree* MuonTesterTree::tree() const { return m_tree.get(); }
const TTree* MuonTesterTree::operator->() const { return m_tree.get(); }


bool MuonTesterTree::registerBranch(std::shared_ptr<IMuonTesterBranch> branch) {    
    if (!branch) {
        ATH_MSG_ERROR("Nullptr given");
        return false;
    }
    /// Check whether the branch belongs to the same TTree
    if (branch->tree() != tree()) {
        for (const FriendTreePtr& friend_tree : getFriends()){
            if (friend_tree->registerBranch(branch)) {
                m_branches.push_back(branch);
                return true;
            }
        }
        return false;
    }
    std::vector<std::shared_ptr<IMuonTesterBranch>>::const_iterator itr = std::find_if(
        m_branches.begin(), m_branches.end(),
        [&branch](const std::shared_ptr<IMuonTesterBranch>& known) { 
            return known == branch || known->name() == branch->name(); 
        });
    if (itr != m_branches.end()) {
        if (typeid((*itr).get()) != typeid(branch.get())) {
            ATH_MSG_FATAL("Different branches have been added here under " << branch->name());
            return false;
        }
        return true;
    } else if (m_filled) {
        ATH_MSG_FATAL("Tree structure is already finalized");
        return false;
    }
    m_branches.push_back(branch);
    return true;
}
bool MuonTesterTree::addBranch(std::shared_ptr<IMuonTesterBranch> branch) { return registerBranch(branch) && addBranch(branch.get()); }
bool MuonTesterTree::addBranch(IMuonTesterBranch& branch) { return addBranch(&branch); }
bool MuonTesterTree::addBranch(IMuonTesterBranch* branch) {
    if (isActive(branch))
        return true;
    else if (!branch) {
        ATH_MSG_ERROR("Nullptr given");
        return false;
    } else if (m_filled) {
        ATH_MSG_ERROR("Tree structure is already finalized. Cannot add " << branch->name());
        return false;
    } 
    if (branch->tree() != tree()) {
        for (const FriendTreePtr& friend_tree : getFriends()) {
            if (friend_tree->addBranch(branch)) return true;
        }
        return false;
    }
    m_branches_to_init.push_back(branch);
    return true;
}

void MuonTesterTree::removeBranch(IMuonTesterBranch* branch) {
    Remove(m_branches_to_init, [branch](const IMuonTesterBranch* br) { return br == branch; });
}
void MuonTesterTree::removeBranch(IMuonTesterBranch& branch) { removeBranch(&branch); }

bool MuonTesterTree::initialized() const { return m_init; }
bool MuonTesterTree::fill(const EventContext& ctx) {
    if (!initialized()) {
        ATH_MSG_ERROR("The TTree has not been initialized yet");
        return false;
    }
    /// Remove the information that is no longer of value
    if (!m_filled) {
        m_excludedBranches.clear();
        m_initialized_br.clear();
        m_dependencies.clear();
    }
    /// Common trees arecan only be filled once in an event
    if (isCommonTree() && m_hash_br->is_dumped(ctx)) {
        return true;
    }
    /// Call the fill method on all friends
    for (const FriendTreePtr& friend_tree : getFriends()){
        if (!friend_tree->fill(ctx)) return false;
    }
    /// These branches are actually initialized
    for (auto& branch : m_branches_to_init) {
        ATH_MSG_VERBOSE("Try to fill "<<branch->name());
        if (!branch->fill(ctx)) {
            ATH_MSG_ERROR("fill()  --- Failed to fill branch " << branch->name() << " in tree " << name() );
            return false;
        }
    }
    m_tree->Fill();
    m_filled = true;
    return true;
}
StatusCode MuonTesterTree::init(ServiceHandle<ITHistSvc> hist_svc) {
    if (fileStream().empty()) {
        ATH_MSG_ERROR("The file stream of " << name() << " has not been set yet" );
        return StatusCode::FAILURE;
    }
    if (!initialized()) {
        if (!m_tree) {
            ATH_MSG_FATAL("No TTree object");
            return StatusCode::FAILURE;
        }
        /// push back the ds id index
        std::stringstream full_path{};
        full_path<<"/"<<fileStream()<<"/"<<path()<<(path().empty() ? "" : "/")<<name();
        if (!hist_svc->regTree(full_path.str(), tree()).isSuccess()) { return StatusCode::FAILURE; }
        m_directory = m_tree->GetDirectory();
        if (!m_directory) {
            ATH_MSG_ERROR("Where is my directory to write later?");
            return StatusCode::FAILURE;
        }
        m_hist_svc = hist_svc;
        m_init = true;
    }
    for (const FriendTreePtr& friend_tree : getFriends()){
        if (!friend_tree->init(hist_svc).isSuccess()) return StatusCode::FAILURE;
    }

    Remove(m_branches_to_init, [this](const IMuonTesterBranch* br) {
        return !br || m_excludedBranches.count(br->name());
    });
    /// Sort by alphabet
    std::sort(m_branches_to_init.begin(), m_branches_to_init.end(),
              [](IMuonTesterBranch* a, IMuonTesterBranch* b) { return a->name() < b->name(); });
    for (IMuonTesterBranch* branch : m_branches_to_init) {
        if (m_initialized_br.count(branch)) {
            ATH_MSG_VERBOSE("Do not count the initialize function on "<<branch->name()<<" twice.");
            continue;
        }
        if (!branch->init()) {
            ATH_MSG_ERROR("Failed to initialize branch " << branch->name());
            return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG(" Branch " << branch->name() << " has been initialized");
        std::vector<DataDependency> data_dep = branch->data_dependencies();
        for (const DataDependency& data : data_dep) {
            m_dependencies.emplace_back(data);
        }
        m_initialized_br.insert(branch);
    }
    /// Kick everything that is not owned by the class itself
    Remove(m_branches_to_init, [this](const IMuonTesterBranch* br) {
        return std::find_if(m_branches.begin(), m_branches.end(),
                            [br](const std::shared_ptr<IMuonTesterBranch>& owned) { 
                                return owned.get() == br; }) == m_branches.end();
    });
    ATH_MSG_INFO("Initialization of "<<name()<<" successfully completed");
    return StatusCode::SUCCESS;
}

StatusCode MuonTesterTree::write() {
    if (!initialized() || !m_tree || m_written) { return StatusCode::SUCCESS; }
    if (!m_hist_svc->deReg(tree()).isSuccess()) {
        ATH_MSG_ERROR(__func__<<"() --- Failed to put the tree "<<name()<<" out of the HistService");
        return StatusCode::FAILURE;
    }
    for (const FriendTreePtr& friend_tree : getFriends()) {
        if (!friend_tree->write().isSuccess()) return StatusCode::FAILURE;
        if (!tree()->AddFriend(friend_tree->tree())) {
            ATH_MSG_ERROR("Failed to establish the Friendship between "<<name()<<" & "<<friend_tree->tree());
            return StatusCode::FAILURE;
        }
    }
    m_directory->WriteObject(tree(), tree()->GetName(), "overwrite");
    if (!isCommonTree()) m_tree.reset();
    m_branches_to_init.clear();
    m_branches.clear();
    m_friendLinks.clear();
    m_written = true;
    return StatusCode::SUCCESS;
}
void MuonTesterTree::disableBranch(const std::string& b_name) {
    if (!m_filled) m_excludedBranches.insert(b_name);
}
void MuonTesterTree::disableBranch(const std::vector<std::string>& br_names) {
    if (!m_filled) m_excludedBranches.insert(br_names.begin(), br_names.end());
}
std::string MuonTesterTree::fileStream() const { return m_stream; }
bool MuonTesterTree::isActive(const IMuonTesterBranch* branch) const {
    if (!branch) {        
        ATH_MSG_ERROR("Nullptr was given");
        return false;
    }
    if (branch->tree() != tree()) {
        for (const FriendTreePtr& friend_tree : getFriends()){
            if (friend_tree->isActive(branch)) return true;
        }
        return false;
    }
    return std::find_if(m_branches_to_init.begin(), m_branches_to_init.end(), 
        [&branch](const IMuonTesterBranch* known) {
            return known == branch || known->name() == branch->name();
        }) != m_branches_to_init.end();
}
void MuonTesterTree::setPath(const std::string& new_path) { m_path = new_path; }
std::string MuonTesterTree::path() const { return m_path; }

bool MuonTesterTree::addClient(MuonTesterTree* client) {
    if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
        ATH_MSG_WARNING("Common TTree mechanism only supported in single thread environment");
        return false;
    }
    if (!m_hash_br) {
        m_hash_br = std::make_shared<EventHashBranch>(tree());
        if (!addBranch(m_hash_br)) return false;
    }
    m_commonClients.insert(client);
    return true;
}
bool MuonTesterTree::addCommonTree(FriendTreePtr common_tree) {
    if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
        ATH_MSG_WARNING("Common TTree mechanism only supported in single thread environment");
        return false;
    }
    if (!common_tree) {
        ATH_MSG_ERROR("Nullptr given");
        return false;
    }
    if (common_tree->fileStream() != fileStream()) {
        ATH_MSG_ERROR("The common tree "<<common_tree->name()<<" and "<<name()<<" have to be written into the same file");
        return false;
    }
    if (m_commonClients.count(common_tree.get())) {
        ATH_MSG_ERROR(name()<<" is already a friend of "<<common_tree->name());
        return false;
    }
    /// Ensure that the event entries are synchronized
    if (!m_hash_br) {
        m_hash_br = std::make_shared<EventHashBranch>(tree());
        if (!addBranch(m_hash_br)) return false;
    }
    m_friendLinks.push_back(common_tree);
    return common_tree->addClient(this);
}
const std::vector<MuonTesterTree::FriendTreePtr>& MuonTesterTree::getFriends() const {
    return m_friendLinks;
}
bool MuonTesterTree::isCommonTree() const {return !m_commonClients.empty(); }

MuonTesterTree::MuonTesterTree(const std::string& tree_name, const std::string& stream) :
    AthMessaging{"MuonTesterTree"},
    m_tree{std::make_unique<TTree>(tree_name.c_str(), "MuonTesterTree")}, m_stream(stream) {}

MuonTesterTree::~MuonTesterTree(){
    m_branches_to_init.clear();
}
}
