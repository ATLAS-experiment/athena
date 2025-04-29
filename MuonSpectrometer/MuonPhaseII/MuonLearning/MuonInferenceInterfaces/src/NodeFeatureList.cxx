/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonInferenceInterfaces/NodeFeatureList.h"
#include "MuonInferenceInterfaces/NodeFeatureFactory.h"
#include "MuonInferenceInterfaces/GraphData.h"


#include "AthenaBaseComps/AthMessaging.h"
#include "Acts/Utilities/Enumerate.hpp"
#include "MuonPatternHelpers/MatrixUtils.h"

using namespace MuonR4;
namespace MuonML {
   bool NodeFeatureList::isValid() const {
      return m_connector && numFeatures();
   }
   bool NodeFeatureList::setConnector(const std::string& conName, MsgStream& msg) {
      m_connector = Factory::makeConnector(conName , msg);
      return m_connector != nullptr;
   }

   void NodeFeatureList::setConnector(const std::string& conName, NodeConnector::Evaluator_t evalFunc) {
      m_connector = std::make_unique<NodeConnector>(conName, evalFunc);
   }
   bool NodeFeatureList::operator==(const NodeFeatureList& other) const {
      if (numFeatures() != other.numFeatures()) {
         return false;
      }
      if (!m_connector || !other.m_connector || m_connector->name() != other.m_connector->name()) {
         return false;
      }
      for (size_t f =0 ; f < numFeatures(); ++f) {
         if (m_features[f]->name() != other.m_features[f]->name()) {
            return false;
         }
      }
      return true;
   }
   size_t NodeFeatureList::numFeatures() const {
      return m_features.size();
   }
   
   /** @brief Returns the name of the features in the list */
   std::vector<std::string> NodeFeatureList::featureNames() const {
      std::vector<std::string> names{};
      std::ranges::transform(m_features,std::back_inserter(names),
                              [](const Feature_t& ft){ return ft->name();});
      return names;
   }

   bool NodeFeatureList::addFeature(const std::string& featName, MsgStream& msg) {
      return addFeature(Factory::makeFeature(featName, msg), msg);
   }

   bool NodeFeatureList::addFeature(const Feature_t& newFeat,  MsgStream& msg) {
      if (!newFeat) {
         msg<<MSG::ERROR<<"No feature has been parsed. "<<endmsg;
         return false;
      }
      if (std::ranges::find_if(m_features, [&newFeat](const Feature_t& known){
         return known == newFeat || known->name() == newFeat->name();
      }) != m_features.end()) {
         msg<<MSG::ERROR<<" The feature "<<newFeat->name()<<" has already been added & "
                        <<" cannot be added again. "<<endmsg;
         return false;
      }
      if (msg.level() <= MSG::DEBUG) {
         msg<<MSG::DEBUG<<__FILE__<<":"<<__LINE__<<" - Add new feature "<< newFeat->name()<<endmsg;
      }
      m_features.push_back(newFeat);
      return true;
   }
   
   void NodeFeatureList::fillInData(const Bucket_t& bucket, 
                                    GraphRawData& prepGraph) const {
      for (size_t sp = 0 ; sp < bucket.size(); ++sp) {
         /** @brief Fill the graph features  */
         for(const Feature_t& feat : m_features) {
            assert(prepGraph.currLeave != prepGraph.featureLeaves.end());
            (*prepGraph.currLeave++) = feat->eval(bucket, sp);
         }
         for (size_t ot = 0; ot < sp; ++ot) {
            size_t from = prepGraph.nodeIndex +sp;
            size_t to = prepGraph.nodeIndex + ot;
            if (m_connector->connect(bucket, sp, ot)) {
               /// Connection i->j
               prepGraph.srcEdges.emplace_back(from);
               prepGraph.desEdges.emplace_back(to);
               ///Connection j-> i
               prepGraph.srcEdges.emplace_back(to);
               prepGraph.desEdges.emplace_back(from);
            }
         }
      }
      prepGraph.nodeIndex+=bucket.size();
   }

}