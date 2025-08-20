/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_GLOBALLARCELL_H
#define GLOBALSIM_GLOBALLARCELL_H

#include <vector>
#include <string>
#include <boost/dynamic_bitset.hpp>

namespace GlobalSim{
  class GlobalLArCell
  {

  public:

    /** @brief default constructor */
    GlobalLArCell();

    /** @brief Main constructor */
    GlobalLArCell(uint32_t ID, 
                  std::string FEB2, 
                  int channel);

    /** @brief copy constructor (explicitly defaulted) */
    GlobalLArCell(const GlobalLArCell&) = default;

    /** @brief copy assignment operator (explicitly defaulted) */
    GlobalLArCell& operator=(const GlobalLArCell&) = default;

    /** @brief default destructor */
    ~GlobalLArCell() { };

    // ----- setter functions -----

    /** @brief set transverse energy in MeV */
    void setEnergy (float energy);
    /** @brief set transverse energy in MeV and write bitstring encoding it */
    void setEnergy (float energy, boost::dynamic_bitset<>&& energy_bitset);
    /** @brief set position of cell in eta-phi space */
    void setPosition (float eta, float phi);
    /** @brief set significancy of energy deposit */
    void setSigma (float sigma);
    /** @brief set name of associated MUX */
    void setMUX(std::string muxname);
    /** @brief set name of associated LASP */
    void setLASP(std::string laspname);
    /** @brief set properties of associated board connector */
    void setBoardConnector(std::string connector, std::string type, int number, int fiber);

    // ----- getter functions -----

    /** @brief get transverse energy in MeV */
    float getEnergy () const;
    /** @brief get the energy bitstring */
    const boost::dynamic_bitset<>& getEnergyBitstring() const;
    /** @brief get the short identifier of the cell */
    uint32_t getID () const;
    /** @brief get the channel of this cell on its associated FEB2 */
    int getChannel () const;
    /** @brief get the eta position of the cell */
    float eta () const;
    /** @brief get the phi position of the cell */
    float phi () const;
    /** @brief get the name of the FEB2 this cell is associated with */
    const std::string& getFEB2 () const;
    /** @brief get the name of the MUX this cell is associated with */
    const std::string& getMUX() const;
    /** @brief get the name of the LASP this cell is associated with */
    const std::string& getLASP() const;
    /** @brief get the connector of the FEB2 this cell is associated with */
    const std::string& getConnector() const;
    /** @brief get the type of connector of the FEB2 this cell is associated with */
    const std::string& getConnectorType() const;


  protected:

    /** @brief identifier of this cell */
    uint32_t m_id = -1;

    /** @brief eta position of this cell */
    float m_eta = -99.9;

    /** @brief phi position of this cell */
    float m_phi = -99.9;

    /** @brief name of the FEB2 through which this cell is read out */
    std::string m_feb2 = "";

    /** @brief channel number of this cell on its associated FEB2 */
    int m_channel = -1;

    /** @brief transverse energy (in MeV) */
    float m_energy = -1;

    /** @brief string of the bitstream encding the transverse energy (in MeV) */
    boost::dynamic_bitset<> m_energy_bitset;

    /** @brief significance of energy deposit (transverse energy divided by  total expected noise) */
    float m_sigma = -99.9;

    /** @brief name of associated MUX */
    std::string m_mux = "";

    /** @brief name of associated LASP */
    std::string m_lasp = "";

    /** @brief connector of associated FEB2 */
    std::string m_connector = "";

    /** @brief type of connector of associated FEB2 */
    std::string m_connectorType = "";

    /** @brief connector number of associated FEB2 */
    int m_connectorNumber = -1;

    /** @brief fiber number of associated FEB2 */
    int m_fiber = -1;

  };

  // inline functions

  inline GlobalLArCell::GlobalLArCell() :
    m_id(0),
    m_feb2("NONE"),
    m_channel(0),
    m_energy(0),
    m_sigma(0)
  {}

  inline GlobalLArCell::GlobalLArCell(uint32_t ID,
          std::string FEB2,
          int channel) :
    m_id(ID),
    m_feb2(FEB2),
    m_channel(channel),
    m_energy(0),
    m_sigma(0)
  {}

  // Setter functions
  inline void GlobalLArCell::setEnergy (float energy) { m_energy = energy; }
  inline void GlobalLArCell::setEnergy (float energy, boost::dynamic_bitset<>&& energy_bitset) { 
      m_energy = energy;;
      m_energy_bitset = std::move(energy_bitset);
  }
  inline void GlobalLArCell::setPosition (float eta, float phi) { 
      m_eta = eta;
      m_phi = phi;
  }
  inline void GlobalLArCell::setSigma (float sigma) { m_sigma = sigma; }
  inline void GlobalLArCell::setMUX (std::string muxname) { m_mux = std::move(muxname); };
  inline void GlobalLArCell::setLASP (std::string laspname) { m_lasp = std::move(laspname); };
  inline void GlobalLArCell::setBoardConnector (std::string connector, std::string type, int number, int fiber) { 
      m_connector = std::move(connector);
      m_connectorType = std::move(type);
      m_connectorNumber = number;
      m_fiber = fiber;
  };

  // Getter functions
  inline float GlobalLArCell::getEnergy () const { return m_energy; }
  inline const boost::dynamic_bitset<>& GlobalLArCell::getEnergyBitstring() const { return m_energy_bitset; }
  inline uint32_t GlobalLArCell::getID () const { return m_id; }
  inline int GlobalLArCell::getChannel () const { return m_channel; }
  inline float GlobalLArCell::eta () const { return m_eta; }
  inline float GlobalLArCell::phi () const { return m_phi; }
  inline const std::string& GlobalLArCell::getFEB2 () const { return m_feb2; }
  inline const std::string& GlobalLArCell::getMUX() const { return m_mux; }
  inline const std::string& GlobalLArCell::getLASP() const { return m_lasp; }
  inline const std::string& GlobalLArCell::getConnector() const { return m_connector; }
  inline const std::string& GlobalLArCell::getConnectorType() const { return m_connectorType; }

}

#endif //GLOBALSIM_GLOBALLARCELL_H
