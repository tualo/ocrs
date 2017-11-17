
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/chrono.hpp>
#include <sys/time.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>


#include "ExtractAddress.h"

int main( int argc, char** argv ){
 //[[:digit:]]

 //std::string text1 = "DE-O7613 Königshofen :";
 //std::string text2 = "DE-07613 Königshofen :";
 std::string text1 = "Knießcnsgasse 16 c";
 std::string text2 = "Knießcnsgasse 16 c";
 std::cout << "start " << std::endl;

 //std::string regextext = "(O|7|I|i|Q|\\d){5}\\s";
 std::string regextext = "\\d+([a-zA-Z]|\\s|-|/|d+)*$";

 const boost::regex plz_regex(regextext);

 std::cout << "text 1: try regex *" << regextext <<"*";
 if(boost::regex_search(text1 , plz_regex )==true){
   std::cout << "found plz text 1 " << std::endl;
 }
 std::cout << std::endl;

 std::cout << "text 2: try regex *" << regextext <<"*";
 if(boost::regex_search(text2 , plz_regex )==true){
   std::cout << "found plz text 2 " << std::endl;
 }
 std::cout << std::endl;


std::string text_adr = "\n MINI Phnrmn GmbH   Kurz  Stlaßo l   30938 Burgwedel Fuhrbefg\n Frau DL med   \n Ulla Berger\n Fachärztin für Allgemeinmedizin\n Knießensgasse 16 e ß     \n 993 10 Bornheim";
ExtractAddress *ea = new ExtractAddress();
ea->setZipcodeRegexText("(D|O|7|I|i|Q|\\d){5}");
ea->setString(text_adr);
ea->extract();

std::cout << "zipcode: " << ea->getZipCode() << std::endl;
std::cout << "town: " << ea->getTown() << std::endl;
std::cout << "street: " << ea->getStreetName() << std::endl;
std::cout << "housenumber: " << ea->getHouseNumber() << std::endl;


 exit(-1);
}
