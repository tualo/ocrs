
int ImageRecognizeEx::calcmeanValue(cv::Mat m){
  int int_mean = 128;
  cv::Mat mean;
  cv::Mat stddev;
  cv::meanStdDev(m,mean,stddev);
  int idx =0;
  int minStd = 255;
  int maxStd = 0;
  int minMean = 255;
  int maxMean = 0;
  for(idx=0;idx<sizeof(stddev.data);idx++){
    if (minStd>(int)stddev.data[idx]){
      minStd=(int)stddev.data[idx];
    }
    if (maxStd<(int)stddev.data[idx]){
      maxStd=(int)stddev.data[idx];
    }
  }

  for(idx=0;idx<sizeof(mean.data);idx++){
    if (minMean>(int)mean.data[idx]){
      minMean=(int)mean.data[idx];
    }
    if (maxMean<(int)mean.data[idx]){
      maxMean=(int)mean.data[idx];
    }
  }

  std::cout << "minStd " << minStd << " maxStd " << maxStd << std::endl;
  std::cout << "minMean " << minMean << " maxMean " << maxMean << std::endl;
  int_mean = (-1*( (int)mean.data[6] - 128 ));
  return minMean;//255-maxMean;
}

void ImageRecognizeEx::recalcSubstractMean(cv::Mat m){
  if(calcMean){
    _debugTime("start recalcSubstractMean");
    cv::Mat mean;
    cv::Mat stddev;
    cv::meanStdDev(m,mean,stddev);
    int idx =0;
    int minStd = 255;
    int maxStd = 0;
    for(idx=0;idx<sizeof(stddev.data);idx++){
      if (minStd>(int)stddev.data[idx]){
        minStd=(int)stddev.data[idx];
      }
      if (maxStd<(int)stddev.data[idx]){
        maxStd=(int)stddev.data[idx];
      }
    }
    subtractMean = (-1*( (int)mean.data[6] - 128 ))*meanfactor ; //light up
    if (showDebug){
      std::cout << "use calculated substract mean " << subtractMean<< std::endl;
    }
    _debugTime("stop recalcSubstractMean");
  }
}



int ImageRecognizeEx::linearize(cv::Mat& src){
  _debugTime("start linearize");
  cv::Mat thr(src.rows,src.cols,CV_8UC1);
  /*
  cvtColor(src,thr,CV_BGR2GRAY); //Convert to gray
  */
  if (src.channels()>1){
    throw std::runtime_error("Error: ImageRecognizeEx::linearize not a gray image");
  }

  //std::cout << "calcmeanValue(src) " << calcmeanValue(src) << std::endl;

  cv::adaptiveThreshold(
      src,
      src,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,//blockSize,calcmeanValue(src));/*,
      blockSize,
      subtractMean
  );

  _debugTime("stop linearize");
  showImage(src);
  showImage(src);
  return 0;
}

void ImageRecognizeEx::rotate(cv::Mat& src, int direction){
  if (direction>0){
    while(direction>0){
      transpose(src, src);
      flip(src, src, direction);
      direction--;
    }
  }else{
    while(direction<0){
      transpose(src, src);
      flip(src, src, direction);
      direction++;
    }
  }
}



void ImageRecognizeEx::largestContour(bool useSlow){
  orignalImage = largestSimpleContour(orignalImage);
  //orignalImage = largestComplexContour(orignalImage);
  //todo resize roi image!!!

}

void ImageRecognizeEx::checkPixels(){
  _debugTime("start checkPixels");

  std::string sql = "select length,height from bbs_data where id = '"+code+"'  ";
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }else{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int num_fields;
    double length;
    double height;
    bool wasfound=false;
    result = mysql_use_result(con);
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))){
       length = atof(row[0]);
       height = atof(row[1]);

       wasfound=true;
    }
    for(; mysql_next_result(con) == 0;) /* do nothing */;
    mysql_free_result(result);


    updateStatistics("letter_height",height);
    updateStatistics("letter_length",length);
    updateStatistics("image_rows",orignalImage.rows);
    updateStatistics("image_cols",orignalImage.cols);

    updateStatistics("CMY", (orignalImage.rows/rescale_rows/(length/100)) );
    updateStatistics("CMX", (orignalImage.cols/rescale_cols/(height/100)) );

    updateStatistics("CALC_CMY", (orignalImage.rows/(length/100)) );
    updateStatistics("CALC_CMX", (orignalImage.cols/(height/100)) );

    if (wasfound==true){
      setPixelPerCM((orignalImage.cols/(height/100)),(orignalImage.rows/(length/100)));
      rescale();
    }

    if (showDebug){
     std::cout << "################################################" << std::endl;
     if (wasfound==true){
       std::cout << "FOUND SIZE FROM MACHINE " << std::endl;
     }
     std::cout << "LETTERSIZE height " << height << ", length " << length << std::endl;
     std::cout << "IMAGE cols " << orignalImage.cols << ", rows " << orignalImage.rows << std::endl;
     std::cout << "CMY SHOULD BE " << (orignalImage.rows/(length/100)) << std::endl;
     std::cout << "CMY " << (orignalImage.rows/rescale_rows/(length/100)) << std::endl;
     std::cout << "CMX SHOULD BE" << (orignalImage.cols/(height/100)) << " " << std::endl;
     std::cout << "CMX " << (orignalImage.cols/rescale_cols/(height/100)) << " (only valid on DIN LANG)" << std::endl;
     std::cout << "################################################" << std::endl;
     if (wasfound==false){
       std::cout << "LETTERSIZE calculated " << orignalImage.rows/y_cm << "cm x " <<  orignalImage.cols/x_cm  << "cm "<<std::endl;

     }
   }

  }
  _debugTime("stop checkPixels");
}

cv::Mat ImageRecognizeEx::largestSimpleContour(cv::Mat& src){
  _debugTime("start largestSimpleContour");

  cv::Mat thr;
  cv::threshold(src, thr,subtractMean, 255,cv::THRESH_BINARY);


  int blength = thr.cols;
  int i=blength;
  int h=0;
  int height = thr.rows;


  double lastAVG = 255;
  int currentAVG = 255;
  int divisor=0;
  int avglength = 20;
  double cAVG=0;
  char avgbuffer[avglength];
  bzero(avgbuffer,avglength);
  char cavgbuffer[avglength];
  bzero(cavgbuffer,avglength);
  for(;i>199;i--){

    divisor=0;
    currentAVG=0;
    for(h=0;h<height;h+=5){
      currentAVG += thr.at<uchar>(h,i);
      divisor++;
    }
    currentAVG /=divisor;
    /*
    if (currentAVG>lastAVG*2){
      break;
    }
    */
    //std::cout << "currentAVG at i " << currentAVG << " - " << lastAVG << std::endl;
    lastAVG=0;
    cAVG=0;
    for(h=avglength-1;h>0;h--){
      avgbuffer[h]=avgbuffer[h-1];
      lastAVG+=avgbuffer[h];
    }
    for(h=avglength-1;h>0;h--){
      cavgbuffer[h]=cavgbuffer[h-1];
      cAVG+=cavgbuffer[h];
    }
    lastAVG=lastAVG/(avglength-1);
    cAVG=(cAVG+currentAVG)/(avglength);

    //std::cout << "currentAVG at i "<< i << " current " << cAVG << " - last " << lastAVG << std::endl;
    if ((i<blength-avglength*2) && (cAVG>lastAVG)) {
      //std::cout << "rising edge " << cAVG << " vs. " << lastAVG << std::endl;
      break;
    }
    avgbuffer[0]=currentAVG;
    cavgbuffer[0]=currentAVG;
  }
  //std::cout << "largestSimpleContour at i " << i << std::endl;
  if (i<200){
    std::cerr << "this should not happen the contour is to small " << i << " use the hole image "<< std::endl;
    i=blength;
  }
  cv::Rect myROI(0, 0, i, thr.rows);
  cv::Mat result = orignalImage(myROI);
  roiImage = roiImage(myROI);

  _debugTime("stop largestSimpleContour");
  return result;
}

cv::Mat ImageRecognizeEx::largestComplexContour(cv::Mat& src){
  _debugTime("start largestComplexContour");
  int largest_area=0;
  int largest_contour_index=0;
  int type = cv::NORM_MINMAX;
  int dtype = -1;
  int min=0;
  int max=255;
  cv::Mat mask;
  cv::Point point;
  cv::Size ksize(15,15);

  int x = src.cols/2;
  int y = src.rows/2;
  cv::Mat minmat = cv::Mat(x, y, CV_8UC1);
  cv::resize(src, minmat, cv::Size(x, y), 0, 0, 3);

  cv::Rect bounding_rect;
  cv::Mat thr(minmat.rows,minmat.cols,CV_8UC1);
  cv::Mat dst(minmat.rows,minmat.cols,CV_8UC1,cv::Scalar::all(0));
  //cvtColor(minmat,thr,CV_BGR2GRAY); //Convert to gray
  cv::GaussianBlur(minmat, thr, ksize, 0);
  cv::threshold(thr, thr,15, 25,cv::THRESH_BINARY); //Threshold the gray
  cv::normalize(thr, thr, min, max, type, dtype, mask);
  showImage(thr);

  cv::Scalar color( 255,255,255);
  std::vector< std::vector<cv::Point> > contours; // Vector for storing contour
  std::vector<cv::Vec4i> hierarchy;
  double angle;
  cv::findContours( thr, contours, hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image
  for( int i = 0; i< contours.size(); i++ ) // iterate through each contour.
  {
    double a=cv::contourArea( contours[i],false);  //  Find the area of contour
    if(a>largest_area){
      largest_area=a;
      largest_contour_index=i;                //Store the index of largest contour
      bounding_rect=cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
    }

  }
  angle = getOrientation(contours[largest_contour_index], minmat);
  cv::drawContours( dst, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.


  std::vector<cv::Point> points;
  cv::Mat_<uchar>::iterator it = dst.begin<uchar>();
  cv::Mat_<uchar>::iterator end = dst.end<uchar>();
  for (; it != end; ++it)
    if (*it)
      points.push_back(it.pos());

  cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));

  cv::Size box_size = box.size;
  if (box_size.width<box_size.height){
    box.angle += 90.;
  }


  cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(box.center.x*2,box.center.y*2), box.angle, 1);
  cv::Mat rotated;
  rotated = src.clone();
  cv::Mat cropped;
  cv::getRectSubPix(rotated, box_size, box.center, cropped);
  largest_area=0;
  largest_contour_index=0;
  //cvtColor(rotated,thr,CV_BGR2GRAY); //Convert to gray
  cv::GaussianBlur(rotated, thr, ksize, 0);
  cv::threshold(thr, thr,35, 255,cv::THRESH_BINARY); //Threshold the gray
  cv::normalize(thr, thr, min, max, type, dtype, mask);
  cv::findContours( thr, contours, hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image
  for( int i = 0; i< contours.size(); i++ ) // iterate through each contour.
  {
    double a=cv::contourArea( contours[i],false);  //  Find the area of contour
    if(a>largest_area){
      largest_area=a;
      largest_contour_index=i;                //Store the index of largest contour
      bounding_rect=cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
    }

  }

  cv::Rect myROI(bounding_rect.x, bounding_rect.y, bounding_rect.width, bounding_rect.height);
  cv::Mat result = rotated(myROI);
  _debugTime("stop largestComplexContour");
  return result;
}



double ImageRecognizeEx::getOrientation(std::vector<cv::Point> &pts, cv::Mat &img){
  _debugTime("start getOrientation");

  //Construct a buffer used by the pca analysis
  cv::Mat data_pts = cv::Mat(pts.size(), 2, CV_64FC1);
  for (int i = 0; i < data_pts.rows; ++i)
  {
      data_pts.at<double>(i, 0) = pts[i].x;
      data_pts.at<double>(i, 1) = pts[i].y;
  }

  //Perform PCA analysis
  cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);

  //Store the position of the object
  //cv::Point pos = cv::Point(pca_analysis.mean.at<double>(0, 0), pca_analysis.mean.at<double>(0, 1));

  //Store the eigenvalues and eigenvectors
  std::vector<cv::Point2d> eigen_vecs(2);
  std::vector<double> eigen_val(2);
  for (int i = 0; i < 2; ++i)
  {
      eigen_vecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                              pca_analysis.eigenvectors.at<double>(i, 1));

      eigen_val[i] = pca_analysis.eigenvalues.at<double>(0, i);
  }

  // Draw the principal components
  //circle(img, pos, 3, CV_RGB(255, 0, 255), 2);
  //line(img, pos, pos + 0.02 * Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]) , CV_RGB(255, 255, 0));
  //line(img, pos, pos + 0.02 * Point(eigen_vecs[1].x * eigen_val[1], eigen_vecs[1].y * eigen_val[1]) , CV_RGB(0, 255, 255));

  _debugTime("stop getOrientation");
  return atan2(eigen_vecs[0].y, eigen_vecs[0].x);
}




cv::Mat ImageRecognizeEx::getResultImage(){
  cv::Mat resultMat;
  cv::adaptiveThreshold(orignalImage,resultMat,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,blockSize,subtractMean);
  int x=resultMat.cols /2;
  int y=resultMat.rows /2;
  cv::Mat res = cv::Mat(x, y, CV_8UC1);
  cv::resize(resultMat, res, cv::Size(x, y), 0, 0, 3);
  resultMat = res;
  transpose(resultMat, resultMat);
  flip(resultMat, resultMat,1);
  return resultMat;

}

cv::Mat ImageRecognizeEx::getOriginalImage(){
  return oImage;
}
