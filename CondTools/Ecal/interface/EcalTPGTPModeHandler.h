#ifndef ECAL_TPG_WEIGHTGROUP_HANDLER_H
#define ECAL_TPG_WEIGHTGROUP_HANDLER_H

#include <vector>
#include <typeinfo>
#include <string>
#include <map>
#include <iostream>
#include <ctime>

#include "CondCore/PopCon/interface/PopConSourceHandler.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/EventSetupRecordKey.h"

#include "CondFormats/EcalObjects/interface/EcalTPGTPMode.h"
#include "CondFormats/DataRecord/interface/EcalTPGTPModeRcd.h"

#include "OnlineDB/EcalCondDB/interface/all_monitoring_types.h"
#include "OnlineDB/Oracle/interface/Oracle.h"
#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"

#include "DataFormats/Provenance/interface/Timestamp.h"

class EcalElectronicsMapping;

namespace edm {
  class ParameterSet;
  class Event;
  class EventSetup;
}  // namespace edm

namespace popcon {

  class EcalTPGTPModeHandler : public popcon::PopConSourceHandler<EcalTPGTPMode> {
  public:
    EcalTPGTPModeHandler(edm::ParameterSet const&);
    ~EcalTPGTPModeHandler() override;

    void getNewObjects() override;

    std::string id() const override { return m_name; }

    void readtxtFile();
    // void readxmlFile();

    // void readFromFile(const char* inputFile);
    // void writeFile(const char* inputFile);

    EcalCondDBInterface* econn;

  private:
    std::string to_string(char value[]) {
      std::ostringstream streamOut;
      streamOut << value;
      return streamOut.str();
    }

    unsigned int m_firstRun;
    unsigned int m_lastRun;

    std::string m_location;
    std::string m_gentag;
    std::string m_sid;
    std::string m_user;
    std::string m_pass;
    std::string m_locationsource;
    std::string m_name;
    unsigned int m_runnr;
    std::string m_runtype;
    std::string m_i_tag;
    std::string m_file_type;
    std::string m_file_name;
    int m_i_version;
    unsigned int m_i_run_number;
    
    
  };
}  // namespace popcon
#endif
