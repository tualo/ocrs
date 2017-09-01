

ExtractAddress* ImageRecognizeEx::texts(){
  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};

  if (addressRegions.empty()){
    std::cout << "There are not Address Regions defined " << std::endl;
    exit(-1);
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
      if (usingRoi(orignalImage,roi->rect(),roi->rotate(),roi->rotateSteps())){

        extractAddress->setString(resultText);
        extractAddress->extract();
        if (showDebug){
          std::cout << "found ;) " << std::endl;
          std::cout << "zipcode: " << extractAddress->getZipCode() << std::endl;
          std::cout << "town: " << extractAddress->getTown() << std::endl;
          std::cout << "street: " << extractAddress->getStreetName() << std::endl;
          std::cout << "housenumber: " << extractAddress->getHouseNumber() << std::endl;
        }
      }

      cv::rectangle(
        roiImage,
        roi->rect(),
        cv::Scalar(0,205, 0),
        5
      );
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
  //const char* out;
  std::vector<std::string> lines;
  const boost::regex plz_regex("\\d{5}\\s");
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
    rotate(c2,irotate);
  }else{
    if (showDebug){
      std::cout << "do not rotate: " << irotate << std::endl;
    }
  }

  if (istep_rotate>0){
    while (istep_rotate>0){
      if (containsZipCode(c2,im)){
        return true;
      }
      rotate(c2,1);
      istep_rotate--;
    }
  }else{
    while (istep_rotate<0){
      if (containsZipCode(c2,im)){
        return true;
      }
      rotate(c2,1);
      istep_rotate++;
    }
  }

  if (containsZipCode(c2,im)){
    return true;
  }

  return false;
}


bool ImageRecognizeEx::containsZipCode(cv::Mat& im,cv::Mat& orig){
  int i=0;
  int j=0;
  int m=0;
  int lastThreshold=0;
  std::vector<std::string> lines;
  const boost::regex plz_regex("\\d{5}\\s");
  const boost::regex no_plz_regex("\\d{6}\\s");

  recalcSubstractMean(im);

  cv::Mat c2 = im.clone();
  lastThreshold = linearize(im,-0.30);
  std::string s1 = getText(im);

  boost::replace_all(s1,code,"-------------");
  boost::replace_all(s1,"\n\n","\n");
  lines = isplit(s1,'\n');
  if (lines.size()<3){
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
      if ((boost::regex_search(lines.at(i) , plz_regex)==true)&&(boost::regex_search(lines.at(i) , no_plz_regex)==false)){
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
        //makeResultImage(orig,0.85);
        return true;
      }
    }

  }else{
    if (showDebug){
      std::cout << "dont think it is an address (less than 2 or more than 15 lines)" << std::endl;
    }
  }

  return false;
}



std::string ImageRecognizeEx::getText(cv::Mat& im){

  tess->SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), im.step1());
  tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
  tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
  tess->SetVariable("textord_min_linesize","1.0");
  tess->Recognize(0);


  const char* out = tess->GetUTF8Text();

  const boost::regex number_space("(?<=\\d)([^\\S\\r\\n])+(?=\\d)");
  std::string intermedia (out);


  const boost::regex plz_regex("\\d{5}\\s");
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
    return intermedia;
  }else{



    std::string newtext;
    newtext = boost::regex_replace(intermedia, number_space, "");

    if(showDebug){
      std::cout << "##################" << std::endl;
      std::cout << newtext << std::endl;
      std::cout << "##################" << std::endl;
    }

    return newtext;
  }
}
