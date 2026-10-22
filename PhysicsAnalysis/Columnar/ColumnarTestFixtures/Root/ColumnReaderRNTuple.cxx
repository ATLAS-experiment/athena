/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarTestFixtures/ColumnReaderBranch.h>

#include <xAODJet/JetContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>

#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleInspector.hxx>

//
// method implementations
//

namespace columnar 
{
  namespace TestUtils 
  {
    template <typename T>
    class RNTFieldReader final 
    {
      std::string m_FieldName;
      std::unique_ptr<ROOT::RNTupleView<T>> m_view;
      ROOT::Experimental::RNTupleInspector* m_inspector = nullptr;
      ROOT::RNTupleReader* m_reader = nullptr;
      const T* m_data = nullptr;

    public:
      RNTFieldReader(const std::string& val_fieldName)
      : m_FieldName(val_fieldName) 
      {}

      ~RNTFieldReader() noexcept {}
      RNTFieldReader(const RNTFieldReader&) = delete;
      RNTFieldReader& operator=(const RNTFieldReader&) = delete;

      [[nodiscard]] const std::string& fieldName() const 
      { 
        return m_FieldName; 
      }

      [[nodiscard]] std::string columnName() const 
      {
        std::string columnName = m_FieldName;
        if (auto index = columnName.find("AuxDyn:"); index != std::string::npos)
          columnName.replace(index, 6, "");
        else if (auto index = columnName.find("Aux:."); index != std::string::npos)
          columnName.replace(index, 4, "");
        else if (auto index = columnName.find("Aux:"); index != std::string::npos)
          columnName.replace(index, 3, "");
        else if (columnName.find(":") != std::string::npos)
          throw std::runtime_error("field name does not contain AuxDyn or Aux: " + m_FieldName);
        std::replace(columnName.begin(), columnName.end(), ':', '.');

        return columnName;
      }

      [[nodiscard]] std::string containerName() const 
      {
        if (auto index = m_FieldName.find("AuxDyn:"); index != std::string::npos)
          return m_FieldName.substr(0, index);
        else if (auto index = m_FieldName.find("Aux:"); index != std::string::npos)
          return m_FieldName.substr(0, index);
        else if (m_FieldName.find(":") == std::string::npos)
          return m_FieldName;
        else
          throw std::runtime_error("field name does not contain AuxDyn or Aux: " + m_FieldName);
      }

      void connectRNTuple(ROOT::RNTupleReader* reader,
                      ROOT::Experimental::RNTupleInspector* inspector) 
      {
        m_inspector = inspector;
        m_reader = reader;
        m_view = std::make_unique<ROOT::RNTupleView<T>>(reader->GetView<T>(m_FieldName));

        if (!m_view)
          throw std::runtime_error("failed to get field: " + m_FieldName);
      }

      void connectTree(const Backend& b) 
      {
        auto* rntbackend = std::get<RNTupleBackend*>(b);  // throws if wrong backend

        if (!rntbackend->reader || !rntbackend->inspector)
          throw std::runtime_error("RNTuple backend not properly initialized");
        connectRNTuple(rntbackend->reader, rntbackend->inspector);
      }
  
  
      const T& getEntry(Long64_t entry) 
      {
        if (!m_view)
          throw std::runtime_error("field not connected: " + m_FieldName);

        m_data = &((*m_view)(static_cast<ROOT::NTupleSize_t>(entry)));

        if (m_data == nullptr)
          throw std::runtime_error("got nullptr reading data for field: " + m_FieldName);
        return *m_data;
      }

      const T& getCachedEntry() const 
      {  
        return *m_data; 
      }

      std::optional<float> entrySize() const 
      {

        const ROOT::Experimental::RNTupleInspector::RFieldTreeInspector& fieldTreeInspector = m_inspector->GetFieldTreeInspector(m_FieldName);
        return static_cast<float>(fieldTreeInspector.GetCompressedSize()) /
          m_inspector->GetDescriptor().GetNEntries();
      }

      std::optional<float> uncompressedSize() const 
      {

        const ROOT::Experimental::RNTupleInspector::RFieldTreeInspector& fieldTreeInspector = m_inspector->GetFieldTreeInspector(m_FieldName);

        return static_cast<float>(fieldTreeInspector.GetUncompressedSize()) /
          m_inspector->GetDescriptor().GetNEntries();
      }

      std::optional<unsigned> numBaskets() 
      {
       // placeholder
       return std::nullopt;
      }
    };

    void makeRNTupleColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, RNTupleBackend* rntbackend)
    {
      std::unordered_map<std::string, ROOT::DescriptorId_t> fields;
      {
       const auto& desc = rntbackend->reader->GetDescriptor();

       for (const auto& field : desc.GetTopLevelFields()) 
       {
          fields.emplace(desc.GetQualifiedFieldName(field.GetId()), field.GetId());

         std::vector<ROOT::DescriptorId_t> subFieldIds{field.GetId()};
         while (!subFieldIds.empty()) 
         {
           const auto parentId = subFieldIds.back();
           auto parentname=desc.GetQualifiedFieldName(parentId);
           subFieldIds.pop_back();

           for (const auto& subField : desc.GetFieldIterable(parentId)) 
           {
             auto subFieldName = desc.GetQualifiedFieldName(subField.GetId());

             fields.emplace(desc.GetQualifiedFieldName(subField.GetId()), subField.GetId());

             subFieldIds.push_back(subField.GetId());
           }
          }
        }
      }

      const auto& desc = rntbackend->reader->GetDescriptor();
      for (const auto& [name, fieldId] : fields) 
      {
        auto fieldName = desc.GetQualifiedFieldName(fieldId);

        if (name.find("AuxDyn:") != std::string::npos ||
          name.find("Aux:") != std::string::npos) 
        {

          const auto& fieldDesc = desc.GetFieldDescriptor(fieldId);
          const std::string typeName = desc.GetTypeNameForComparison(fieldDesc);
          if (typeName == "std::int32_t" || typeName == "int") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::int32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::uint32_t" || typeName == "unsigned int") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::uint32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::uint64_t" || typeName == "unsigned long" || typeName == "unsigned long long") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::uint64_t, RNTFieldReader>>(name));
          } else if (typeName == "float") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderScalar<float, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<float>") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderVector<float, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<char>") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderVector<char, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int8_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderVector<std::int8_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint8_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderVector<std::uint8_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int16_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderVector<std::int16_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint16_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnReaderVector<std::uint16_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int32_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVector<std::int32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint32_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVector<std::uint32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int64_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVector<std::int64_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint64_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVector<std::uint64_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::vector<float>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVector<float, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::vector<std::int32_t>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVector<std::int32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::vector<std::uint64_t>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVector<std::uint64_t, RNTFieldReader>>(name));
          } else if (typeName =="std::vector<std::vector<std::vector<std::size_t>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVector<std::size_t, RNTFieldReader>>(name));
          }else if (typeName =="std::vector<std::vector<std::vector<std::uint64_t>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVector<std::uint64_t, RNTFieldReader>>(name));
          }else if (typeName =="std::vector<std::vector<std::vector<std::uint8_t>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVector<std::uint8_t, RNTFieldReader>>(name));
          } else if (typeName =="std::vector<std::vector<std::vector<unsigned char>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVector<unsigned char, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::string>") 
          {
             knownColumns.push_back(std::make_shared<ColumnReaderMetNames<RNTFieldReader>>(name));
          }
        }
      }
      knownColumns.push_back(std::make_shared<ColumnReaderSamplingPattern<RNTFieldReader>>("egammaClusters"));

      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::CaloClusterContainer,RNTFieldReader>>("AnalysisElectronsAuxDyn:caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::TrackParticleContainer, RNTFieldReader>>("AnalysisElectronsAuxDyn:trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::CaloClusterContainer,RNTFieldReader>>("AnalysisPhotonsAuxDyn:caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::VertexContainer, RNTFieldReader>>("AnalysisPhotonsAuxDyn:vertexLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorRLink<xAOD::TrackParticleContainer,RNTFieldReader>>("AnalysisMuonsAuxDyn:inDetTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorRLink<xAOD::TrackParticleContainer,RNTFieldReader>>("AnalysisMuonsAuxDyn:combinedTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorRLink< xAOD::TrackParticleContainer, RNTFieldReader>>("AnalysisMuonsAuxDyn:extrapolatedMuonSpectrometerTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::TrackParticleContainer, RNTFieldReader>>("GSFConversionVerticesAuxDyn:trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorRLink<xAOD::TrackParticleContainer,RNTFieldReader>>("GSFTrackParticlesAuxDyn:originalTrackParticle"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVariantLink<xAOD::IParticleContainer, RNTFieldReader>>("AnalysisJetsAuxDyn:GhostTrack"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorLink<xAOD::JetContainer, RNTFieldReader>>("METAssoc_AnalysisMETAux:.jetLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVariantLink<xAOD::IParticleContainer, RNTFieldReader>>("METAssoc_AnalysisMETAux:.objectLinks"));
    }
  }
}