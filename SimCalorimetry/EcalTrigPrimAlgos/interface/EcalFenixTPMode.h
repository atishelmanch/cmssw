#ifndef ECAL_FENIX_TPMODE_
#define ECAL_FENIX_TPMODE_

#include <string>
#include <bitset>
#include <iostream>

/**
    \class EcalFenixTPMode
    \brief class interpretaint the TP mode from python configuration or from condDB.
    Davide Valsecchi 15/01/2020
*/
class EcalFenixTPMode {
public:
  // constructor, destructor
  EcalFenixTPMode( ): TPmode_(0){};
  EcalFenixTPMode(uint TPmode): TPmode_(TPmode){
    // Readout TP mode options
    if (TPmode_ & 0x1) use_EE_odd_filter =  true;
    if ( (TPmode_ > 1) & 0x1) use_EE_odd_peak_finder =  true;
    if ( (TPmode_ > 2) & 0x1) disable_EE_even_peak_finder =  true;
    if ( (TPmode_ > 6) & 0x1) use_EB_odd_filter =  true;
    if ( (TPmode_ > 7) & 0x1) use_EB_odd_peak_finder =  true;
    if ( (TPmode_ > 8) & 0x1) disable_EB_even_peak_finder =  true;
  };

private:
  
public:
  
  uint TPmode_;
  bool use_EE_odd_filter = false;
  bool use_EB_odd_filter = false;
  bool use_EE_odd_peak_finder = false;
  bool use_EB_odd_peak_finder = false;
  bool disable_EE_even_peak_finder = false;
  bool disable_EB_even_peak_finder = false;

  void print(){
    std::cout << ">>> Trigger primitive mode:  "<< std::bitset<15>(TPmode_).to_string() << std::endl;
    std::cout << "    Use EE odd filter  "<< use_EE_odd_peak_finder << std::endl;
    std::cout << "    Use EB odd filter  "<< use_EB_odd_peak_finder << std::endl;
    std::cout << "    Use EE odd peak finder  "<< use_EE_odd_peak_finder << std::endl;
    std::cout << "    Use EB odd peak finder  "<< use_EB_odd_peak_finder << std::endl;
    std::cout << "    Disable EE even peak finder  "<< disable_EE_even_peak_finder << std::endl;
    std::cout << "    Disable EB even peak finder  "<< disable_EE_even_peak_finder << std::endl;
  };

  
};
#endif
