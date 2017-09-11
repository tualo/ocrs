// ExtractAddress.h
#ifndef EXCTRACTADDRESS_H
#define EXCTRACTADDRESS_H

#include <iostream>
#include <stdio.h>
//#include <regex>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

class ExtractAddress{
  public:
    void setString(std::string text);
    void extract();
    bool foundAddress();
    std::string getString();
    std::string getHouseNumber();
    std::string getStreetName();
    std::string getZipCode();
    std::string getTown();
    std::string getSortRow();
    std::string getSortBox();
    void setZipcodeHash(std::string zipcode,std::string s_sortiergang,std::string s_sortierfach);
    
  private:
    bool hasAddress;
    std::string orignalString;
    std::string town;
    std::string zipCode;
    std::string streetName;
    std::string housenumber;
    std::string sortiergang;
    std::string sortierfach;
    std::map<std::string,std::string> zipCodeMap;


};
#endif
