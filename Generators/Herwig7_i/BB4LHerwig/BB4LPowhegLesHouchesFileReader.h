// -*- C++ -*-
//
// myLesHouchesReader.h is - (c) Silvia Ferrario Ravasio and Tomas Jezo
// inspired by LesHouchesFileReader.h which is a part of ThePEG
//
#ifndef THEPEG_BB4LPowhegLesHouchesFileReader_H
#define THEPEG_BB4LPowhegLesHouchesFileReader_H
// This is the declaration of the BB4LPowhegLesHouchesFileReader class.

#include "ThePEG/LesHouches/LesHouchesReader.h"
#include "ThePEG/PDT/Decayer.h"
#include "ThePEG/Utilities/CFileLineReader.h"

#include "herwig7_interface.h"
#include "CxxUtils/checker_macros.h"
#include <string>
#include <map>
#include <vector>

double powheg_weight;

namespace ThePEG {


/**
 * BB4LPowhegLesHouchesFileReader is an base class to be used for objects which
 * reads event files from matrix element generators. It inherits from
 * LesHouchesReader and extends it by defining a file handle to be
 * read from, which is opened and closed by the open() and close()
 * functions. Note that the file handle is a standard C filehandle and
 * not a C++ stream. This is because there is no standard way in C++
 * to connect a pipe to a stream for reading eg. gzipped files. This
 * class is able to read plain event files conforming to the Les
 * Houches Event File accord.
 *
 * @see \ref BB4LPowhegLesHouchesFileReaderInterfaces "Th1e interfaces"
 * defined for BB4LPowhegLesHouchesFileReader.
 * @see Event
 * @see LesHouchesReader
 */
class ATLAS_NOT_THREAD_SAFE BB4LPowhegLesHouchesFileReader: public LesHouchesReader {

public:

  /** @name Standard constructors and destructors. */
  //@{
  /**
   * Default constructor.
   */
  BB4LPowhegLesHouchesFileReader() : m_neve(0), m_ieve(0), m_theQNumbers(false),
			   m_theIncludeFxFxTags(false),
			   m_theIncludeCentral(true) {}

  /**
   * Copy-constructor. Note that a file which is opened in the object
   * copied from will have to be reopened in this.
   */
  BB4LPowhegLesHouchesFileReader(const BB4LPowhegLesHouchesFileReader &);

  /**
   * Destructor.
   */
  virtual ~BB4LPowhegLesHouchesFileReader();
  //@}

public:

  /** @name Virtual functions specified by the LesHouchesReader base class. */
  //@{
  /**
   * Initialize. This function is called by the LesHouchesEventHandler
   * to which this object is assigned.
   */
  virtual void initialize(LesHouchesEventHandler & eh);

  /**
   * Open a file with events. Derived classes should overwrite it and
   * first calling it before reading in the run information into the
   * corresponding protected variables.
   */
  virtual void open  ();

  /**
   * Close the file from which events have been read.
   */
  virtual void close();
 

  /**
   * Read the next event from the file or stream into the
   * corresponding protected variables. Return false if there is no
   * more events or if this was not a LHF event file.
   */
  virtual bool doReadEvent  ();
  //@}

  /**
   * Return the name of the file from where to read events.
   */
  const std::string & filename() const { return m_theFileName; }

  /** 
   * Return the optional weights information string ("Names")
   */

  virtual std::vector<std::string> optWeightsNamesFunc();
 
  double eventWeight() {return powheg_weight;}

public:

  /** @name Functions used by the persistent I/O system. */
  //@{
  /**
   * Function used to write out object persistently.
   * @param os the persistent output stream written to.
   */
  void persistentOutput(PersistentOStream & os) const;

  /**
   * Function used to read in object persistently.
   * @param is the persistent input stream read from.
   * @param version the version number of the object when written.
   */
  void persistentInput(PersistentIStream & is, int version);
  //@}

  /**
   * Standard Init function used to initialize the interfaces.
   */
  static void Init  ();


  /** 
   * Erases all occurences of a substring from a string 
   */
  
  void erase_substr(std::string& subject, const std::string& search);


protected:

  /** @name Clone Methods. */
  //@{
  /**
   * Make a simple clone of this object.
   * @return a pointer to the new object.
   */
  virtual IBPtr clone() const;

  /** Make a clone of this object, possibly modifying the cloned object
   * to make it sane.
   * @return a pointer to the new object.
   */
  virtual IBPtr fullclone() const;
  //@}

  /** @name Standard (and non-standard) Interfaced functions. */
  //@{
  /**
   * Initialize this object after the setup phase before saving an
   * EventGenerator to disk.
   * @throws InitException if object could not be initialized properly.
   */
  virtual void doinit  ();

  /**
   * Return true if this object needs to be initialized before all
   * other objects because it needs to extract PDFs from the event file.
   */
  virtual bool preInitialize() const;
  //@

protected:

  /**
   * The wrapper around the C FILE stream from which to read
   */
  CFileLineReader m_cfile;

protected:

  /**
   * The number of events in this file.
   */
  long m_neve;

  /**
   * The current event number.
   */
  long m_ieve;

  /**
   * If the file is a standard Les Houches formatted file (LHF) this
   * is its version number. If empty, this is not a Les Houches
   * formatted file
   */
  std::string m_LHFVersion;

  /**
   * If LHF. All lines (since the last open() or readEvent()) outside
   * the header, init and event tags.
   */
  std::string m_outsideBlock;

  /**
   * If LHF. All lines from the header block.
   */
  std::string m_headerBlock;

  /**
   * If LHF. Additional comments found in the init block.
   */
  std::string m_initComments;

  /**
   * If LHF. Map of attributes (name-value pairs) found in the init
   * tag.
   */
  std::map<std::string,std::string> m_initAttributes;

  /**
   * If LHF. Additional comments found with the last read event.
   */
  std::string m_eventComments;

  /**
   * If LHF. Map of attributes (name-value pairs) found in the last
   * event tag.
   */
  std::map<std::string,std::string> m_eventAttributes;

private:

  /**
   * The name of the file from where to read events.
   */
  std::string m_theFileName;

  /**
   *  Whether or not to search for QNUMBERS stuff
   */
  bool m_theQNumbers;

  /**
   * Include/Read FxFx tags
   */
  bool m_theIncludeFxFxTags;

  /**
   * Include central weight (for backup use)
   */
  bool m_theIncludeCentral;

  /**
   *  Decayer for any decay modes read from the file
   */
  DecayerPtr m_theDecayer;
  
  /**
   * Further information on the weights
   */
  std::map<std::string,std::string> m_scalemap;

  /**
   * Temporary holder for optional weights
   */
  
  std::map<std::string,double> m_optionalWeightsTemp;
  std::map<std::string,std::string> m_optionalWeightsLabel;


private:

  /**
   * Describe an abstract base class with persistent data.
   */
  static ClassDescription<BB4LPowhegLesHouchesFileReader> m_initBB4LPowhegLesHouchesFileReader;

  /**
   * Private and non-existent assignment operator.
   */
  BB4LPowhegLesHouchesFileReader & operator=(const BB4LPowhegLesHouchesFileReader &) = delete;

public:

  /** @cond EXCEPTIONCLASSES */
  /** Exception class used by BB4LPowhegLesHouchesFileReader if reading the file
   *  fails. */
  class powhegLesHouchesFileError: public Exception {};
  /** @endcond */

};

}


#include "ThePEG/Utilities/ClassTraits.h"

namespace ThePEG {

/** @cond TRAITSPECIALIZATIONS */

/**
 * This template specialization informs ThePEG about the
 * base class of BB4LPowhegLesHouchesFileReader.
 */
template <>
struct BaseClassTrait<BB4LPowhegLesHouchesFileReader,1>: public ClassTraitsType {
  /** Typedef of the base class of BB4LPowhegLesHouchesFileReader. */
  typedef LesHouchesReader NthBase;
};

/**
 * This template specialization informs ThePEG about the name of the
 * BB4LPowhegLesHouchesFileReader class and the shared object where it is
 * defined.
 */
template <>
struct ClassTraits<BB4LPowhegLesHouchesFileReader>
  : public ClassTraitsBase<BB4LPowhegLesHouchesFileReader> {
  /**
   * Return the class name.
   */
  static std::string className() { return "ThePEG::BB4LPowhegLesHouchesFileReader"; }
  /**
   * Return the name of the shared library to be loaded to get access
   * to the BB4LPowhegLesHouchesFileReader class and every other class it uses
   * (except the base class).
   */
  static std::string library() { return "libpowhegHerwigBB4L.so"; }

};

/** @endcond */

}

#endif /* THEPEG_BB4LPowhegLesHouchesFileReader_H */
