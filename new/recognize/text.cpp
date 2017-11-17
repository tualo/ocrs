

ExtractAddress* ImageRecognizeEx::texts(){
  initAddressRegions();
  initZipCodeMap();

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};

  if (addressRegions.empty()){
    std::cout << "There are not Address Regions defined " << std::endl;
  }

  RegionOfInterest* roi;
  cv::Mat bc_roi;

  for(std::list<RegionOfInterest*>::iterator list_iter = addressRegions.begin();
    list_iter != addressRegions.end(); list_iter++){


      roi=*list_iter;

      if (showDebug){
        std::cout << "check region image: " << orignalImage.cols << " "  << orignalImage.rows << " oneCM " << oneCM << std::endl;
      }
      roi->setImage(orignalImage);
      if (showDebug){
        std::cout << "check region: " << roi->name()
          << " x: " << roi->rect().x << " y: "<< roi->rect().y
          << " w: " << roi->rect().width << " h: "<< roi->rect().height << std::endl;
      }

      bc_roi = orignalImage(roi->rect());
      //res = barcode_internal(bc_roi,barcodeFP);
      std::cout << "before usingRoi" << std::endl;

      cv::rectangle(
        roiImage,
        roi->rect(),
        cv::Scalar(1,1, 1),
        5
      );
      if (usingRoi(orignalImage,roi->rect(),roi->rotate(),roi->rotateSteps())){

        extractAddress->setZipcodeRegexText(zipcodeRegexText);
        extractAddress->setString(resultText);
        extractAddress->extract();


        if (showDebug){
          updateStatistics("roi",roi->name());
          std::cout << "used address roi: " << roi->name() << std::endl;
          std::cout << "zipcode: " << extractAddress->getZipCode() << std::endl;
          std::cout << "town: " << extractAddress->getTown() << std::endl;
          std::cout << "street: " << extractAddress->getStreetName() << std::endl;
          std::cout << "housenumber: " << extractAddress->getHouseNumber() << std::endl;

          std::cout << "sortiergang: " << extractAddress->getSortRow() << std::endl;
          std::cout << "sortierfach: " << extractAddress->getSortBox() << std::endl;
          updateStatistics("state","address");
        }

        // draw result roi oly if there is an address
        cv::rectangle(
          roiImage,
          roi->rect(),
          cv::Scalar(255,255, 255),
          5
        );
        break;
      }
      showImage(roiImage);
  }
  return extractAddress;
}

std::vector<std::string> &ImageRecognizeEx::isplit(const std::string &s, char delim, std::vector<std::string>  &elems){
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss,item,delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> ImageRecognizeEx::isplit(const std::string &s,char delim){
  std::vector<std::string> elems;
  isplit(s,delim,elems);
  return elems;
}



bool ImageRecognizeEx::usingRoi(cv::Mat& im,cv::Rect roi2, int irotate, int istep_rotate){
  _debugTime("start usingRoi");
  //const char* out;
  std::vector<std::string> lines;
  const boost::regex plz_regex(zipcodeRegexText);
  const boost::regex no_plz_regex("\\d{6}\\s");
  boost::cmatch char_matches;

  //int breite = im.cols/oneCM;
  //int hoehe = im.rows/oneCM;
  //float ratio = ( ((im.rows *1.0) / (im.cols *1.0 )) );
  cv::Mat c2 = (im.clone())(roi2);

  if (irotate!=0){
    if (showDebug){
      std::cout << "irotate: " << irotate << std::endl;
    }

    showImage(c2);
    rotate(c2,irotate);
    showImage(c2);
  }else{
    if (showDebug){
      std::cout << "do not rotate: " << irotate << std::endl;
    }
  }

  if (istep_rotate>0){
    while (istep_rotate>0){
      if (containsZipCode(c2,im)){
        _debugTime("stop usingRoi");
        return true;
      }
      rotate(c2,1);
      istep_rotate--;
    }
  }else{
    while (istep_rotate<0){
      if (containsZipCode(c2,im)){
        _debugTime("stop usingRoi");
        return true;
      }
      rotate(c2,1);
      istep_rotate++;
    }
  }

  if (containsZipCode(c2,im)){
    _debugTime("stop usingRoi");
    return true;
  }

  _debugTime("stop usingRoi");
  return false;
}


bool ImageRecognizeEx::containsZipCode(cv::Mat& im,cv::Mat& orig){
  _debugTime("before containsZipCode");
  int i=0;
  int j=0;
  int m=0;
  int lastThreshold=0;
  std::vector<std::string> lines;
  const boost::regex plz_regex(zipcodeRegexText);
  const boost::regex no_plz_regex("\\d{6}\\s");

  /*
  cv::Scalar cmean = cv::mean(im);
  if(showDebug){
    std::cout << "mean " << cmean[0] << " " << cmean[1] << " "  << cmean[2] << std::endl;
    //[0];
  }
  */
  //subtractMean = calcmeanValue(im);

  //recalcSubstractMean(im);
  unpaper(im);

  //lastThreshold = linearize(im);//,-0.30);


  showImage(im);
  std::string s1 = getText(im);

  boost::replace_all(s1,code,"-------------");
  boost::replace_all(s1,"\n\n","\n");

  lines = isplit(s1,'\n');
  if (lines.size()<3){
    _debugTime("stop containsZipCode");
    return false;
  }
  std::vector<std::string>::iterator it;
  for (it = lines.begin(); it != lines.end(); it++){
    if ((*it).length()<3){
      lines.erase(it,it);
    }
  }
  if (showDebug){
    std::cout << "erase lines " << lines.size() << std::endl;
  }


  if (showDebug){
    std::cout << "Lines found" << lines.size() << std::endl;
  }

  if ((lines.size()>1)&&(lines.size()<15)){

    m = lines.size()-1;
    for(i=m;i>0;i--){
      std::string ln=lines.at(i);
      boost::replace_all(ln," ","");
      if ((boost::regex_search(ln , plz_regex)==true)&&(boost::regex_search(ln , no_plz_regex)==false)){
        s1="";
        for(j=0;j<=i;j++){
          s1+=lines.at(j)+"\n";
        }
        resultText=s1;
        ocrText = s1.c_str();
        if (showDebug){
          std::cout << "ImageRecognize::contains ZipCode" << std::endl << resultText << std::endl << std::endl;
        }
        resultThres = lastThreshold;
        _debugTime("stop containsZipCode, found");

        return true;
      }
    }

  }else{
    if (showDebug){
      std::cout << "dont think it is an address (less than 2 or more than 15 lines)" << std::endl;
    }
  }

  _debugTime("stop containsZipCode");
  return false;
}



std::string ImageRecognizeEx::getText(cv::Mat& im){
  _debugTime("start getText");


/*  cv::Mat tim = im.clone();
  _debugTime("start getText, cloned");
  cv::cvtColor(tim, tim, CV_GRAY2BGR);
  _debugTime("start getText, converted");
*/
/*
  int x=tim.cols /2;
  int y=tim.rows /2;
  cv::Mat res = cv::Mat(x, y, CV_32FC3);
  cv::resize(tim, res, cv::Size(x, y), 0, 0, 3);
  tim = res.clone();
*/

  //showImage(tim,5000);
  tess->SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), im.step1());
  //tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
  //tess->SetVariable("tessedit_char_whitelist", "0123456789ABCEFGHJKLMNPSRTUVWXYZabcdefghjkmnopqrstuvwxyzäöüÄÖÜß/éè -");
  tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß/éè -");
  tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
  tess->SetVariable("textord_min_linesize","0.8");
  tess->Recognize(0);


  const char* out = tess->GetUTF8Text();

  _debugTime("start getText, found text");
  const boost::regex number_space("(?<=\\d)([^\\S\\r\\n])+(?=\\d)");
  std::string intermedia (out);


  const boost::regex plz_regex(zipcodeRegexText);
  const boost::regex no_plz_regex("\\d{6}\\s");


  if (
    (boost::regex_search(intermedia , plz_regex)==true)&&
    (boost::regex_search(intermedia , no_plz_regex)==false)
  ){
  //if ( (boost::regex_search(intermedia , plz_regex)==true)  ){
    if(showDebug){
      std::cout << "########INTERMEDIA##########" << std::endl;
      std::cout << intermedia << std::endl;
      std::cout << "########INTERMEDIA##########" << std::endl;
    }
    _debugTime("stop getText");
    return intermedia;
  }else{



    std::string newtext;
    newtext = boost::regex_replace(intermedia, number_space, "");

    if(showDebug){
      std::cout << "##################" << std::endl;
      std::cout << newtext << std::endl;
      std::cout << "##################" << std::endl;
    }

    _debugTime("stop getText");
    return newtext;
  }
}
