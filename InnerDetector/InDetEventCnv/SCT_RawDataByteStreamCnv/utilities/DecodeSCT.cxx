/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <sys/mman.h> //PROT_READ, MAP_PRIVATE
#include <fcntl.h> //O_RDONLY
#include <unistd.h> //close

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <cstdint>
#include <filesystem>


namespace fs = std::filesystem;
namespace {
//check the filename exists and return it as a string, or return empty string.
fs::path
getFilename(const char * filenameArg);

void 
decodeEventWord(uint16_t currWord, int errorType, bool &linkCondensed, 
                std::string &type, std::vector<std::pair<std::string, int> > &arguments, 
                std::vector<std::string> &errors);

std::vector<std::string> 
decodeEventFlags(uint32_t flags);

bool 
findNextEvent(uint32_t* &start, uint32_t* &finish, uint32_t *end, uint32_t startWord);

void 
decodeEvent(uint32_t *buffer, int length);

void 
decodeSubFragment(uint32_t *buffer, int length);
}

inline std::string 
str(const char * word){
  return word ? word : "";
} 

void
helpMessage(){
  std::cout<< "'DecodeSCT' takes exactly one argument, the bytestream filename." <<std::endl;
}


int main(int argc, char **argv) {
  if (argc < 2){
    helpMessage();
    return 1;
  } 
  fs::path input_file = getFilename(argv[1]);
  if (input_file.empty()) {
    std::cerr << argv[1] <<" does not exist."<<std::endl;
    return 2;
  }
  auto length = fs::file_size(input_file);
  printf("Length: %lu\n", length);
  auto fd = open(input_file.c_str(), O_RDONLY);
  if(fd<0) {
    perror("File open failed");
    return 3;
  }
  uint32_t *buffer = (uint32_t *) mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);
  if(buffer == MAP_FAILED) {
    close(fd);
    perror("mmap failed");
    return 4;
  }
  uint32_t *endBuffer = &buffer[length/4];
  uint32_t *begin = buffer;
  uint32_t *end{};
  while(findNextEvent(begin, end, endBuffer, 0xee1234ee)) {
    //cppcheck-suppress invalidPrintfArgType_sint
    printf("Event from %li to %li\n", begin-buffer, end-buffer);
    int length = static_cast<int>(end - 1l - begin);
    decodeEvent(begin, length);
  }
  munmap(buffer, length);
  close(fd);
}

namespace {

  bool findNextEvent(uint32_t* &start, uint32_t* &finish, uint32_t *end, uint32_t startWord) {
    start ++;
    if(start >= end) return false;
    while(*start != startWord) {
      start++;
      if(start >= end) return false;
    }
    finish = start+1;
    // ignore sub-ROD fragments
    while((!( ((*finish & 0x00ffff00) == 0x00123400) || ((*finish & 0xffff0000) == 0x12340000))) ||  ((*finish) == 0xff1234ff)) {
      // Last one, return length
      if(finish >= end) break;
      finish++;
    }
    return true;
  }

  bool findNextSubFragment(uint32_t* &start, uint32_t* &finish, uint32_t *end) {
    start ++;
    if(start >= end) return false;
    while(*start != 0xff1234ff) {
      start++;
      if(start >= end) return false;
    }
    finish = start+1;
    while((!( ((*finish & 0x00ffff00) == 0x00123400) || ((*finish & 0xffff0000) == 0x12340000))) ) {
      // Last one, return length
      if(finish >= end) break;
      finish++;
    }
    return true;
  }

  inline uint32_t 
  eventWord(const uint32_t * const buffer, const int index, const int length) {
    if (index > length) {
      std::cout << "Attempt to read from the end of the buffer " <<  index << " " << length << std::endl;
      return 0;
    } else {
      return buffer[index-1];
    }
  }

  void decodeEvent(uint32_t *frameBuffer, int length) {
    if(frameBuffer[0] == 0xee1234ee) {
      std::cout << "Valid header\n";
    } else {
      std::cout << "Bad check 0x" << std::hex << frameBuffer[0] << std::dec << "\n";
      std::cout << "Skipping decode as SCT ROD fragment\n";
      return;
    }
    uint32_t headerLength = 9;     // Including bof
    uint32_t trailerLength = 5;    // Excluding eof
    // The below assumes the old definition of headerLength (ie including 0xbof)
    headerLength=10;
    // 2 is to correct for the bof and eof that aren't in the s-link format
    int eventLength = length - headerLength - trailerLength + 2;
    uint32_t rawOffset = headerLength;
    if(frameBuffer[1] != headerLength) {
      std::cout << "Unknown header length (" << frameBuffer[1] << ")\n";
    } 
    std::cout << "New formatter version:\n";
    std::cout << "Version: 0x" << std::hex << eventWord(frameBuffer, 3, length) << std::dec
      << " ID: 0x" <<std::hex  << eventWord(frameBuffer, 4, length) <<std::dec
      << " Run number = " << eventWord(frameBuffer, 5, length) << "\n";
      std::cout << "L1ID = 0x" << std::hex << eventWord(frameBuffer, 6, length) << std::dec 
      << " BCID = 0x" << std::hex << eventWord(frameBuffer, 7, length) << std::dec 
      << " TType = " << eventWord(frameBuffer, 8, length) 
      << " det type = " << eventWord(frameBuffer, 9, length) << "\n";
    uint32_t SubDetID = (eventWord(frameBuffer, 4, length) & 0xff0000) >> 16 ;
    std::cout << "SubDetID = " << std::hex << SubDetID <<std::dec;
    switch (SubDetID){
      case 0x20:
      case 0x21:
      case 0x22:
      case 0x23:
      case 0x24:
      {
        std::cout << " Found SCT fragment [" << SubDetID << "]. Decoding...\n";
        
        // Something different to the first word
        uint16_t lastWord = 1 + ((eventWord(frameBuffer, rawOffset, length) & 0xffff0000) >> 16);
        uint32_t repeats = 0;
        
        bool linkCondensed = true;
        
        for(int i=0; i<eventLength * 2; i++) {
          uint16_t currWord;
          {
            uint32_t rawWord = eventWord(frameBuffer, rawOffset + i/2, length);
            if(i&1) {
              currWord = rawWord & 0x00ffff;
            } else {
              currWord = (rawWord & 0xffff0000) >> 16;
            }
          }
          
          if(currWord == lastWord) {
            repeats ++;
            continue;
          } else if(repeats) {
            std::cout << "        Repeated " << repeats << " times\n";
            repeats = 0;
          }
          lastWord = currWord;
          std::cout.width(4);
          std::cout.fill('0');
          std::cout << std::hex << currWord << std::dec;
          std::cout.fill(' ');
          std::string type;
          std::vector<std::pair<std::string, int> > arguments;
          std::vector<std::string> errors;
          decodeEventWord(currWord, 0, linkCondensed,type, arguments, errors);
          std::cout << " " << type << " ";
          for(auto iter = arguments.begin();iter != arguments.end();++iter ) {
            std::cout << "  " << iter->first << ": " << iter->second;
          }
          if(errors.size() > 0) {
            std::cout << "  ERRORS: ";
          }
          for(auto iter = errors.begin();iter != errors.end();++iter) {
            std::cout << " " << *iter;
          }
          std::cout << std::endl;
        }
        if(repeats) {
          std::cout << "        Repeated " << repeats << " times\n";
          repeats = 0;
        }
        std::cout << "Error count = 0x" << std::hex << eventWord(frameBuffer, length-trailerLength + 2, length) << std::dec << "\n";
        std::cout << "Error flags = 0x" << std::hex << eventWord(frameBuffer, length-trailerLength + 3, length) << std::dec << "\n";
        int flags = eventWord(frameBuffer, length-trailerLength + 3, length);
        std::vector<std::string> flagStrings = decodeEventFlags(flags);
        for(auto iter = flagStrings.begin();iter != flagStrings.end();++iter) {
          std::cout << *iter << " ";
        }
        if(flagStrings.size() > 0) 
          std::cout << std::endl;
        std::cout << "nData = " << eventWord(frameBuffer, length-trailerLength + 5, length) << " words found = " << (length - (headerLength + trailerLength) + 2) << std::endl;
        return;
      }
      case 0x70: {
        std::cout << " TDAQ [" << SubDetID << "] beam crate fragment: " << std::endl;
        uint32_t *endBuffer = &frameBuffer[length];
        uint32_t *begin = frameBuffer;
        uint32_t *end = frameBuffer;
        bool found = findNextSubFragment(begin, end, endBuffer);
        //cppcheck-suppress invalidPrintfArgType_sint   
        printf("SubFragment from %ld to %ld\n", begin-frameBuffer, end-frameBuffer);
        while (found) {
          const int extent = static_cast<int>(end-begin);
          decodeSubFragment(begin, extent);
          found = findNextSubFragment(begin, end, endBuffer);
          //cppcheck-suppress invalidPrintfArgType_sint    
          if (found) printf("SubFragment from %ld to %ld\n", begin-frameBuffer, end-frameBuffer);
        }
        return;
      }
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
      case 0x34: {
        std::cout << " TRT fragment [" << SubDetID << "] ... skipping" << std::endl;
        return;
      }
      default: {
        std::cout << " Not an known fragment [" << SubDetID << "] ... skipping" << std::endl;
        return;
      }
    }
  }

  void 
  decodeSubFragment(uint32_t *frameBuffer, int length) {
    uint32_t SubFragID = eventWord(frameBuffer, 3, length) ;
    std::cout << "SubFragmentID = "  << SubFragID ;
    std::cout <<  " Length =  " << length; 
    std::string Type[8]={"data","res ","head","res ","eob ","res ","inv ","res "};
    switch(SubFragID) {
      case 0x1: {
        std::cout << " Found TDC fragment. Decoding..."<<  std::endl;
        for (int i =0; i<length-3;i++) {
          uint32_t data = eventWord(frameBuffer, i+4 ,length);
          std::cout << "data[" << i << "]=0x" << std::hex <<(data&0xFFF) << std::dec << " ";
          std::cout << "type=" << Type[(data>>24)&0x07] << " ";
          std::cout << "chan=" << ((data>>16)&0x001f) << " ";
          std::cout << "un=" << ((data>>13)&1) << " ";
          std::cout << "ov=" << ((data>>12)&1) << " ";
          std::cout << "value=" << ((data&0x0fff)) << " ";
          std::cout << std::endl;
        }
        return;
      }
      case 0x2: {
        std::cout << " Found ADC fragment. Decoding..." << std::endl;
        for (int i =0; i<length-5;i++) {
          uint32_t data = eventWord(frameBuffer, i+4 ,length);
          std::cout << "data[" << i << "]=0x" << std::hex <<data << std::dec << " ";
          std::cout << "type=" << Type[(data>>24)&0x07] << " ";
          std::cout << "chan=" << ((data>>16)&0x001f) << " ";
          std::cout << "un=" << ((data>>13)&1) << " ";
          std::cout << "ov=" << ((data>>12)&1) << " ";
          std::cout << "value=" << ((data&0x0fff)) << " ";
          std::cout << std::endl;  
        }
        return;
      }
      default: {
        std::cout << " Not an known SubFragment [" << SubFragID << "] ... skipping" << std::endl;
        return;
      }
    }
  }

  void 
  decodeEventWord(uint16_t currWord, int errorType, bool &linkCondensed, 
                  std::string &type, std::vector<std::pair<std::string, int> > &arguments, 
                  std::vector<std::string> &errors) {
  type = "UNKNOWN";
  arguments.clear();
  errors.clear();
  
  switch((currWord & 0xe000) >> 13) {
    case 0:
      {
        if(currWord & 0x1f80) 
          type = "INVALID";
        else {
          // Flagged error
          type = "Flagged error";
          errors.push_back("FLAGGED");
          arguments.push_back(std::make_pair("chip", (currWord & 0x78) >> 3));
          arguments.push_back(std::make_pair("value", currWord & 0x7));
        }
      }
      break;
    case 1:
      {
        // Header
        type = "Header";
        if(errorType) {
          // No indication of whether its condensed...??
          arguments.push_back(std::make_pair("L1", ((currWord & 0x0f00) >> 8)));
          arguments.push_back(std::make_pair("BCID", ((currWord & 0xff))));
          if(currWord & 0x1000) {
            errors.push_back("Preamble err");
          }
        } else {
          int link = (currWord & 0x7f);
          arguments.push_back(std::make_pair("Link", link));
          if(currWord & 0x100) {
            arguments.push_back(std::make_pair("Condensed mode", 1));
            linkCondensed = true;
          } else {
            linkCondensed = false;
          }
          if(currWord & 0x200) 
            errors.push_back("BC err");
          if(currWord & 0x400)
            errors.push_back("L1 err");
          if(currWord & 0x800)
            errors.push_back("Time out err");
          if(currWord & 0x1000)
            errors.push_back("Preamble err");
        }
      }
    break;
    case 2:
      {
        if(currWord & 0x3ff) 
          type = "INVALID";
        else {
          // Trailer
          type = "Trailer";
          if(currWord & 0x400)
            errors.push_back("Data overflow err");
          if(currWord & 0x800)
            errors.push_back("H/T limit err");
          if(currWord & 0x1000)
            errors.push_back("Trailer bit err");
        }
      }
      break;
    case 3:
      {
        if(currWord & 0x300) 
          type = "INVALID";
        else {
          // Raw data
          type = "Raw";
          int bits = ((currWord & 0x1c00) >> 10) + 1;
          arguments.push_back(std::make_pair("bits", bits));
          int value = ((currWord<<(8-bits)) & 0xff) >> (8-bits);
          arguments.push_back(std::make_pair("value", value));
          // Construct decimal number that looks like binary...
          int binValue = 0;
          for(int b=8-bits; b<8; b++) {
            binValue = binValue * 10 + ((value>>(7-b)) & 1);
          }
          // Unfortunately leading zeros are supressed when it's printed as an int...
          arguments.push_back(std::make_pair("binValue", binValue));
        }
      }
      break;
    default:
      // Everything else (hits)
      {
        if((currWord & 0x2) == 0) {
          // Check if it should be condensed
          if(linkCondensed || errorType) {
            arguments.push_back(std::make_pair("Chip", ((currWord & 0x7800) >> 11)));
            arguments.push_back(std::make_pair("Channel", ((currWord & 0x7f0) >> 4)));
            if(currWord & 1) {
              type = "Condensed double hit";
              if(currWord & 0x4) errors.push_back("Error in hit1");
              if(currWord & 0x8) errors.push_back("Error in hit2");
            } else {
              type = "Condensed hit";
              if(currWord & 0x4) errors.push_back("Error in hit");
            }
          }
        }
      
        // Only check if expanded is a posibility
        if(!linkCondensed || errorType) {
          if((currWord & 0x8) == 0) {
            type = "1st hit clust exp";
            arguments.push_back(std::make_pair("Chip", ((currWord & 0x7800) >> 11)));
            arguments.push_back(std::make_pair("Channel", ((currWord & 0x7f0) >> 4)));
            arguments.push_back(std::make_pair("hits", currWord & 0x7));
          } else {
            if((currWord & 0x7f00) == 0) {
              type = "Clust exp";
              arguments.push_back(std::make_pair("hits", currWord & 0x7));
              if(currWord & 0x80) {
                arguments.push_back(std::make_pair("hits2", (currWord & 0x70) >> 4));
              }
            }
          }
        }
      }
    }
  }

  std::vector<std::string> 
  decodeEventFlags(uint32_t flags) {
    std::vector<std::string> result;
    if(flags & 0x1) result.push_back("HEADER");
    if(flags & 0x2) result.push_back("TRAILER");
    if(flags & 0x4) result.push_back("FLAGGED");
    if(flags & 0x8) result.push_back("\"HIT PATTERN\"");
    if(flags & 0x10) result.push_back("SYNC");
    if(flags & 0x20) result.push_back("L1ID");
    if(flags & 0x40) result.push_back("BCID");
    if(flags & 0x80) result.push_back("TIMEOUT");
    if(flags & 0x100) result.push_back("\"ALMOST FULL\"");
    if(flags & 0x200) result.push_back("OVERFLOW");
    if(flags & 0x400) result.push_back("\"CHIP SEQ\"");
    if(flags & 0x800) result.push_back("\"BAD CHIP\"");
    return result;
  }

  fs::path
  getFilename(const char * filenameArg){
    std::string result;
    const fs::path file(filenameArg);
    if (fs::exists(file)) result = filenameArg;
    return fs::path(result);
  }
} // Close null namespace

