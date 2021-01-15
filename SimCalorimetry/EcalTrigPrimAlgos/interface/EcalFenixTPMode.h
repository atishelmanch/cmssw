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
    if (TPmode_ & 0x1) enable_EE_odd_filter =  true;
    if ( (TPmode_ >> 1) & 0x1) enable_EE_odd_peak_finder =  true;
    if ( (TPmode_ >> 2) & 0x1) disable_EE_even_peak_finder =  true;
    EE_strip_formatter_output =  (TPmode_ >> 3) & 0x3;
    if ( (TPmode_ >> 5) & 0x1) flag_EE_odd_even_strip =  true;

    if ( (TPmode_ >> 6) & 0x1) enable_EB_odd_filter =  true;
    if ( (TPmode_ >> 7) & 0x1) enable_EB_odd_peak_finder =  true;
    if ( (TPmode_ >> 8) & 0x1) disable_EB_even_peak_finder =  true;
    EB_strip_formatter_output =  (TPmode_ >> 9) & 0x3;
    if ( (TPmode_ >> 11) & 0x1) flag_EB_odd_even_strip =  true;

  };

private:
  
public:
  
  uint TPmode_;
  bool enable_EE_odd_filter = false;
  bool enable_EB_odd_filter = false;
  bool enable_EE_odd_peak_finder = false;
  bool enable_EB_odd_peak_finder = false;
  bool disable_EE_even_peak_finder = false;
  bool disable_EB_even_peak_finder = false;
  int EE_strip_formatter_output = 0;  // 0 = even filter,  1 = odd filter, 2 = larger of the two, 3 = odd + even
  int EB_strip_formatter_output = 0;  // 0 = even filter,  1 = odd filter, 2 = larger of the two, 3 = odd + even
  bool flag_EE_odd_even_strip = false;  // flag that odd amplitude > even amplitude 
  bool flag_EB_odd_even_strip = false;

  void print(){
    std::cout << ">>> Trigger primitive mode:  0b"<< std::bitset<15>(TPmode_).to_string() << std::endl;
    std::cout << "    enable EE odd filter  "<< enable_EE_odd_peak_finder << std::endl;
    std::cout << "    enable EB odd filter  "<< enable_EB_odd_peak_finder << std::endl;
    std::cout << "    enable EE odd peak finder  "<< enable_EE_odd_peak_finder << std::endl;
    std::cout << "    enable EB odd peak finder  "<< enable_EB_odd_peak_finder << std::endl;
    std::cout << "    disable EE even peak finder  "<< disable_EE_even_peak_finder << std::endl;
    std::cout << "    disable EB even peak finder  "<< disable_EB_even_peak_finder << std::endl;
    if (EE_strip_formatter_output == 0) std::cout << "    EE strip formatter output: even filter " << std::endl;
    if (EE_strip_formatter_output == 1) std::cout << "    EE strip formatter output: odd filter " << std::endl;
    if (EE_strip_formatter_output == 2) std::cout << "    EE strip formatter output: larger of odd and even " << std::endl;
    if (EE_strip_formatter_output == 3) std::cout << "    EE strip formatter output: odd + even " << std::endl;
    if (EB_strip_formatter_output == 0) std::cout << "    EB strip formatter output: even filter " << std::endl;
    if (EB_strip_formatter_output == 1) std::cout << "    EB strip formatter output: odd filter " << std::endl;
    if (EB_strip_formatter_output == 2) std::cout << "    EB strip formatter output: larger of odd and even " << std::endl;
    if (EB_strip_formatter_output == 3) std::cout << "    EB strip formatter output: odd + even " << std::endl;
    std::cout << "    Flag EE odd>even strip  "<< flag_EE_odd_even_strip << std::endl;
    std::cout << "    Flag EB odd>even strip  "<< flag_EB_odd_even_strip << std::endl;
  };

  
};
#endif
