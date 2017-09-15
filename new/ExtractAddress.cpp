#include "ExtractAddress.h"


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string>  &elems){
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss,item,delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> split(const std::string &s,char delim){
  std::vector<std::string> elems;
  split(s,delim,elems);
  return elems;
}

void ExtractAddress::setString(std::string text){
  orignalString = text;
}

std::string ExtractAddress::getString(){
  return orignalString;
}

std::string ExtractAddress::getTown(){
  boost::trim_right(town);
  return town;
}

std::string ExtractAddress::getZipCode(){
  boost::trim_right(zipCode);
  return zipCode;
}

std::string ExtractAddress::getHouseNumber(){
  boost::trim_right(housenumber);
  return housenumber;
}

std::string ExtractAddress::getStreetName(){
  boost::trim_right(streetName);
  return streetName;
}


std::string ExtractAddress::getSortBox(){
  return sortierfach;
}

std::string ExtractAddress::getSortRow(){
  return sortiergang;
}

void ExtractAddress::setZipcodeHash(std::string zipcode,std::string s_sortiergang,std::string s_sortierfach){
  zipCodeMap[zipcode]= ""+s_sortiergang+"|"+s_sortierfach+"";
}

bool ExtractAddress::foundAddress(){
  return hasAddress;
}

void ExtractAddress::extract(){
  hasAddress=false;

  sortierfach = "NT";
  sortiergang = "NT";
  // postleitzahl muss am anfang sein
  const boost::regex plz_regex("\\d{5}\\s");

  // hausnummer muss am ende sein
  const boost::regex hn_regex("\\d+\\s{0,1}[a-zA-Z]{0,1}(\\s+[-/]\\d+){0,1}");
  boost::cmatch char_matches;
  int mode = 0;

  // modes:
  // * 0 nothing
  // * 1 zip code found
  // * 3 street found
  // * 4 hn found

  std::vector<std::string> strs;
  std::vector<std::string> elems = split(orignalString,(char)10);
  std::vector<std::string> v(elems.rbegin(),elems.rend());
  v.swap(elems);

  town = "";
  zipCode = "";
  streetName = "";

  for(int i=0; i< elems.size();i++){
    std::string line = elems.at(i);

    if (mode==0){
      boost::match_flag_type flags = boost::match_default;
      if (boost::regex_search(elems.at(i).c_str() , char_matches , plz_regex, flags)==true){
        // todo extract zip code
        const boost::sregex_iterator i(line.begin(),line.end(),plz_regex);
        const std::string t = i->str();

        zipCode = t;
        town = line.replace(line.find(zipCode),zipCode.length(),"");;
        mode++;
      }
    }else if (mode==1){
      streetName = line;
      if (streetName.length()>3){
        mode++;
        hasAddress=true;
      }
      if (boost::regex_search(streetName , hn_regex)==true){
        const boost::sregex_iterator i(line.begin(),line.end(),hn_regex);
        const std::string t = i->str();
        housenumber = t;
        streetName = line.replace(line.find(housenumber),housenumber.length(),"");;
        mode++;
      }
    }
    if (mode>=2){
      break;
    }
  }
  boost::split(strs,zipCodeMap[getZipCode()],boost::is_any_of("|"));
  if (strs.size()==2){
    sortiergang=strs[0];
    sortierfach=strs[1];
  }
}