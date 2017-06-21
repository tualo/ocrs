#include "ImageRecognize.h"



std::vector<std::string> &isplit(const std::string &s, char delim, std::vector<std::string>  &elems){
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss,item,delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> isplit(const std::string &s,char delim){
  std::vector<std::string> elems;
  isplit(s,delim,elems);
  return elems;
}

void ImageRecognize::showImage(cv::Mat& src){

  if (showWindow){
    cv::Mat rotated=src.clone();

    int x=src.cols /5;
    int y=src.rows /5;

    /*
    if ( src.cols < src.rows ){
      rotate(src, -90, rotated);
    }
    */

    cv::Mat res = cv::Mat(x, y, CV_32FC3);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);


    cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
    cv::imshow("DEBUG", res );
    cv::waitKey(window_wait);
  }
}

void ImageRecognize::showDebugImage(){
  if (showWindow){
    cv::Mat rotated=debugImage.clone();

    int x=debugImage.cols /8;
    int y=debugImage.rows /8;

    /*
    if ( src.cols < src.rows ){
      rotate(src, -90, rotated);
    }
    */

    cv::Mat res = cv::Mat(x, y, CV_32FC3);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);


    cv::namedWindow("DEBUG Image", CV_WINDOW_AUTOSIZE );
    cv::imshow("DEBUG Image", res );
    //cv::waitKey(0);
  }
}

void ImageRecognize::showImage(cv::Mat& src,char* title){
  if (showWindow){
    cv::Mat rotated=src.clone();

    int x=src.cols /2;
    int y=src.rows /2;
    /*
    if ( src.cols < src.rows ){
      rotate(src, -90, rotated);
    }
    */
    cv::Mat res = cv::Mat(x, y, CV_32FC3);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);


    cv::namedWindow(title, CV_WINDOW_AUTOSIZE );
    cv::imshow(title, res );
    cv::waitKey(window_wait);
  }
}

void ImageRecognize::recalcSubstractMean(cv::Mat m){
  if(const char* env_cl = std::getenv("CALC_MEAN")){
    if(atoi(env_cl)==1){

      cv::Mat mean;
      cv::Mat stddev;
      cv::meanStdDev(m,mean,stddev);
      int idx =0;
      int minStd = 255;
      int maxStd = 0;

      for(idx=0;idx<sizeof(mean.data);idx++){
        std::cout << "mean " << (int)mean.data[idx]-128 << std::endl;
      }
      for(idx=0;idx<sizeof(stddev.data);idx++){
        std::cout << "stddev " << (int)stddev.data[idx] << std::endl;
        if (minStd>(int)stddev.data[idx]){
          minStd=(int)stddev.data[idx];
        }
        if (maxStd<(int)stddev.data[idx]){
          maxStd=(int)stddev.data[idx];
        }
      }

      subtractMean = -1*( (int)mean.data[6] - 128 )/2;
      std::cout << "use substract mean " << subtractMean<< std::endl;
    }
  }
}

void ImageRecognize::open(const char* filename){
  double t = (double)cv::getTickCount();
  double te;

  //showWindow=true;
  resultThres = 128;
  fileName = filename;
  orignalImage = cv::imread( filename, 1 );



  if (light_up_original==true){
    cv::Mat lab_image;
    cv::cvtColor(orignalImage, lab_image, CV_BGR2Lab);

    // Extract the L channel
    std::vector<cv::Mat> lab_planes(3);
    cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    cv::Mat dst;
    clahe->apply(lab_planes[0], dst);

    // Merge the the color planes back into an Lab image
    dst.copyTo(lab_planes[0]);
    cv::merge(lab_planes, lab_image);

   // convert back to RGB
   showImage(orignalImage);
   cv::cvtColor(lab_image, orignalImage, CV_Lab2BGR);
   showImage(orignalImage);
   std::cout << "Light Up" << std::endl;
  }

  tess=new tesseract::TessBaseAPI();
  //if(!tess->SetVariable("tessedit_enable_doc_dict", "0")){
  //}
  //if(!tess->SetVariable("textord_min_linesize", "2.5")){
  //}

  /*
  if(!tess->SetVariable("enable_new_segsearch", "1")){
  }
  if(!tess->SetVariable("use_new_state_cost", "1")){
  }
  */
  if (psmAuto){
    tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
    tess->SetPageSegMode(pagesegmode);
  }

  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);

  if (false){
    cv::Mat blured;
    int x=orignalImage.cols*0.8;
    int y=orignalImage.rows*0.8;

    cv::Mat resized = cv::Mat(x, y, CV_32FC3);
    cvtColor(orignalImage,blured,CV_BGR2GRAY);
    cv::Size ksize(5,5);
    cv::GaussianBlur(blured, blured, ksize, 0);
    cv::resize(blured, resized, cv::Size(x, y), 0, 0, 3);
    cvtColor(resized,orignalImage,CV_GRAY2BGR);
  }
  /*
  cv::Mat minmat = cv::Mat(orignalImage.cols*scale, orignalImage.rows, CV_32FC3);
  cv::resize(orignalImage, minmat, cv::Size(orignalImage.cols*scale, orignalImage.rows), 0, 0, 3);
  orignalImage = minmat;
  */

  oneCM = orignalImage.cols/cmWidth;

  cv::Mat mat(  orignalImage.rows*2,orignalImage.cols*2, orignalImage.type(), cv::Scalar(0));
  cv::Rect roi( cv::Point( orignalImage.cols/2, (orignalImage.rows/2) ), orignalImage.size() );

  orignalImage.copyTo( mat( roi ) );
  //makeResultImage(orignalImage,1);

  cv::Mat tmp = orignalImage.clone();
  int x=orignalImage.cols /2;
  int y=orignalImage.rows /2;
  cv::Mat res = cv::Mat(x, y, CV_8UC1);
  cv::resize(tmp, res, cv::Size(x, y), 0, 0, 3);
  resultMat = res;


  //cv::Mat mat(orignalImage);

  std::string output = "";
  bcResult bcRes;
  cv::Point bc_point;
  cv::Mat largest;
  const char* out;
  if (debug){
    std::cout << "analysisType " << analysisType << std::endl;
  }
  if (analysisType==0){
    showImage(mat);
    largest = largestContour(mat);
    showImage(largest);

    if (headOver){
      transpose(largest, largest);
      flip(largest, largest,1); //transpose+flip(1)=CW
      transpose(largest, largest);
      flip(largest, largest,1); //transpose+flip(1)=CW
    }

    if (barcode_algorthim==0){
      bcRes = barcode(largest);
    }else if (barcode_algorthim==1){
      bcRes = fast_barcode(largest);
    }else if (barcode_algorthim==2){
      bcRes = fast_barcode(largest);
      if (bcRes.found==false){
        bcRes = barcode(largest);
      }
    }

    code = bcRes.code;
    if ((forceaddress==0) && (code.length()<4) ){
      return;
    }
    bc_point=bcRes.point;


    double t1 = (double)cv::getTickCount();
    double te1;

    if (barcode_only==false){
      out = text(largest);
    }

    if (debug){
      te1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
      std::cout << "text passed in seconds: " << te1 << std::endl;
    }
    std::string s = resultText;
    std::replace( s.begin(), s.end(), '"', ' ');
    code = bcRes.code;
    addresstext = s;

  }else if (analysisType==1){

    largest = getRectangle(mat);
    if (debug){
      std::cout << "-------" << std::endl;
      showImage(largest);
    }

    if (headOver){
      if (debug){
        std::cout << "headOver *************" << std::endl;
      }
      transpose(largest, largest);
      flip(largest, largest,1); //transpose+flip(1)=CW
      transpose(largest, largest);
      flip(largest, largest,1); //transpose+flip(1)=CW
    }
    if (barcode_algorthim==0){
      bcRes = barcode(largest);
    }else if (barcode_algorthim==1){
      bcRes = fast_barcode(largest);
    }else if (barcode_algorthim==2){
      bcRes = fast_barcode(largest);
      if (bcRes.found==false){
        bcRes = barcode(largest);
      }
    }


    code = bcRes.code;
    if ((forceaddress==0) && (code.length()<4) ){
      return;
    }
    //cv::rectangle(largest,bcRes.rect,cv::Scalar(255,255,255),CV_FILLED);
    rotateX(largest,90,cv::Point(largest.cols/2,largest.rows/2));
    double t1 = (double)cv::getTickCount();
    double te1;
    out = text(largest);
    //std::cout << ">>>>>>" << resultText << std::endl;

    if (debug){
      te1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
      std::cout << "text passed in seconds: " << te1 << std::endl;
    }
    std::string s  = resultText;
    std::replace( s.begin(), s.end(), '"', ' ');
    addresstext = s;
    bc_point=bcRes.point;

  }


  if (debug){
    te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
    std::cout << "all passed in seconds: " << te << std::endl;
  }

  if (debug){
    std::cout << "try_reduced " << try_reduced << std::endl;
  }

  if (try_reduced==true){
    if (addresstext.length()==0){
      // give a try with smaller image
      if (debug){
        std::cout << "give a try with smaller image" << std::endl;
      }
      cv::Mat blured;
      int x=largest.cols*0.8;
      int y=largest.rows*0.8;

      cv::Mat resized = cv::Mat(x, y, CV_32FC3);
      cvtColor(largest,blured,CV_BGR2GRAY);
      cv::Size ksize(5,5);
      cv::GaussianBlur(blured, blured, ksize, 0);
      cv::resize(blured, resized, cv::Size(x, y), 0, 0, 3);
      cvtColor(resized,largest,CV_GRAY2BGR);

      out = text(largest);
      std::string s = resultText;
      std::replace( s.begin(), s.end(), '"', ' ');
      addresstext = s;
    }else{
      if (debug){
        std::cout << "addresstext.length() " << addresstext.length() << std::endl;
      }
    }
  }



  if (rotate_inline==false){

    cv::Mat rotated;
    transpose(largest, rotated);
    flip(rotated, rotated,1); //transpose+flip(1)=CW
    cv::Mat rotated2(rotated.cols,rotated.rows,rotated.type());
    transpose(rotated, rotated2);
    flip(rotated2, rotated2,1); //transpose+flip(1)=CW
    out = text(rotated2);
    std::string s = resultText;
    std::replace( s.begin(), s.end(), '"', ' ');
    addresstext = s;

  }

}


void ImageRecognize::openPZA(const char* filename){
  double t = (double)cv::getTickCount();
  double te;
  int analysisType = 1;

  if (!barcode_only){
    tess=new tesseract::TessBaseAPI();
    tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);

    if(!tess->SetVariable("tessedit_enable_doc_dict", "0")){
    }
  }
  showWindow=false;
  fileName = filename;
  orignalImage = cv::imread( filename, 1 );

  ocr_text="";
  oneCM = orignalImage.cols/21;
  //cv::Mat mat(  orignalImage.rows,orignalImage.cols, orignalImage.type(), cv::Scalar(0));
  cv::Rect roi( 0, 0, orignalImage.cols,orignalImage.rows/3);
  cv::Mat mat = orignalImage(roi);
  //orignalImage.copyTo( mat( roi ) );
  //showImage(mat, "X");

  bcResult bcRes;
  bcRes = barcode(mat);

  std::string str_barcode (bcRes.code);
  if (str_barcode.length()>4){
    if(debug){
      std::cout << "Code: " << bcRes.code << std::endl;
    }
    code = bcRes.code;
    if (!barcode_only){

    getPZAText(orignalImage);
  }
  }else{
    if(debug){
      std::cout << "Nocode: " << "try flipped" << std::endl;
    }
    transpose(orignalImage, orignalImage);
    flip(orignalImage, orignalImage,1); //transpose+flip(1)=CW
    transpose(orignalImage, orignalImage);
    flip(orignalImage, orignalImage,1); //transpose+flip(1)=CW

    cv::Rect roif( 0, 0, orignalImage.cols,orignalImage.rows/3);
    mat = orignalImage(roif);

    bcRes = barcode(mat);
    str_barcode = (bcRes.code);
    if (str_barcode.length()>4){
      code = bcRes.code;
      if(debug){
        std::cout << "Code: " << bcRes.code << std::endl;
      }
      if (!barcode_only){

        getPZAText(orignalImage);
      }
    }else{
    }
  }
  if(debug){
    te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
    std::cout << "all passed in seconds: " << te << std::endl;
  }
}

void ImageRecognize::getPZAText(cv::Mat& src){
  cv::Rect roi( 0, 0, src.cols,src.rows/3);
  cv::Mat m = src(roi);
  usingLetterType1_2(m);
  addresstext = ocr_text;
}

void ImageRecognize::rotateX(cv::Mat& src,float angle,cv::Point center){
  cv::Mat rotMatrix(2, 3, CV_32FC1);
  cv::Mat res;
  bool rightOrStraight = (ceil(angle) == angle) && (!((int)angle % 90)) ;
  if (rightOrStraight) {
    int angle2 = ((int)angle) % 360;
    if (!angle2) {return;}
    if (angle2 < 0) {angle2 += 360;}
    // See if we do right angle rotation, we transpose the matrix:
    if (angle2 % 180) {
      cv::transpose(src, src);
    }
    int mode = -1;// flip around both axes
    // If counterclockwise, flip around the x-axis
    if (angle2 == 90) {mode = 0;}
    // If clockwise, flip around the y-axis
    if (angle2 == 270) {mode = 1;}
    cv::flip(src, src, mode);
    return;
  }
  rotMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
  cv::warpAffine(src, src, rotMatrix, src.size());
  return;
}


cv::Mat ImageRecognize::getRectangle(cv::Mat& src){
  double t = (double)cv::getTickCount();
  double te;

  cv::Mat src_gray;
  cv::Mat dst;
  cv::Mat cdst;
  cv::Mat detected_edges;

  std::vector<cv::Vec4i> lines;
  int edgeThresh = 1;
  /*
  int const lowThreshold = 5;
  int const maxThreshold = 200;
*/
  int const lowThreshold = 15;
  int const maxThreshold = 50;

  int ratio = 3;
  int kernel_size = 3;

  cvtColor( src, src_gray, CV_BGR2GRAY );

  //showImage(src_gray,"src_gray");
  cv::blur( src_gray, detected_edges, cv::Size(3,3) );
  /// Canny detector
  cv::Canny( detected_edges, detected_edges, lowThreshold, maxThreshold, kernel_size );
  //showImage(detected_edges,"detected_edges");
  /// Using Canny's output as a mask, we display our result
  src.copyTo( dst, detected_edges);
  cvtColor( dst, cdst, CV_BGR2GRAY );
  cv::HoughLinesP( cdst, lines, 25, CV_PI/180, 1, 0, 0);

  cv::Point top_left = cv::Point(cdst.cols, cdst.rows);
  cv::Point top_right = cv::Point(0, cdst.rows);
  cv::Point bottom_left = cv::Point(cdst.cols, 0);
  cv::Point bottom_right = cv::Point(0, 0);

  for( size_t i = 0; i < lines.size(); i++ ){
    int x1 = lines[i][0];
    int x2 = lines[i][2];
    int y1 = lines[i][1];
    int y2 = lines[i][3];
    cv::line( cdst, cv::Point(x1, y1),
    cv::Point(x2, y2), cv::Scalar(255,255,255), 1, 8 );

    if (top_left.x > x1 ){ top_left.x = x1; }
    if (top_left.x > x2 ){ top_left.x = x2; }
    if (top_left.y > y1 ){ top_left.y = y1; }
    if (top_left.y > y2 ){ top_left.y = y2; }

    if (top_right.x < x1 ){ top_right.x = x1; }
    if (top_right.x < x2 ){ top_right.x = x2; }
    if (top_right.y > y1 ){ top_right.y = y1; }
    if (top_right.y > y2 ){ top_right.y = y2; }

    if (bottom_left.x > x1 ){ bottom_left.x = x1; }
    if (bottom_left.x > x2 ){ bottom_left.x = x2; }
    if (bottom_left.y < y1 ){ bottom_left.y = y1; }
    if (bottom_left.y < y2 ){ bottom_left.y = y2; }

    if (bottom_right.x < x1 ){ bottom_right.x = x1; }
    if (bottom_right.x < x2 ){ bottom_right.x = x2; }
    if (bottom_right.y < y1 ){ bottom_right.y = y1; }
    if (bottom_right.y < y2 ){ bottom_right.y = y2; }

  }
  if (top_left.x>bottom_right.x){
    int t = bottom_right.x;
    bottom_right.x=top_left.x;
    top_left.x=t;
  }
  if (top_left.y>bottom_right.y){
    int t = bottom_right.y;
    bottom_right.y=top_left.y;
    top_left.y=t;
  }
  if (debug){
    std::cout << "getRectangle " << top_left << bottom_right << std::endl;
  }

  cv::Rect roi(top_left.x,top_left.y,bottom_right.x-top_left.x,bottom_right.y-top_left.y);
  cv::Mat c_a = src(roi);
  if (debug){
    te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
    std::cout << "largestContour (rect) passed in seconds: " << te << std::endl;
  }
  return c_a;
}

cv::Mat ImageRecognize::largestContour(cv::Mat& src){

  double t = (double)cv::getTickCount();
  double te;

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
  cv::Mat minmat = cv::Mat(x, y, CV_32FC3);
  cv::resize(src, minmat, cv::Size(x, y), 0, 0, 3);

  cv::Rect bounding_rect;
  cv::Mat thr(minmat.rows,minmat.cols,CV_8UC1);
  cv::Mat dst(minmat.rows,minmat.cols,CV_8UC1,cv::Scalar::all(0));
  cvtColor(minmat,thr,CV_BGR2GRAY); //Convert to gray
  cv::GaussianBlur(thr, thr, ksize, 0);
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

  /*
  if(debug){
    if ( ( box.angle < 0.01 ) && ( box.angle > -0.01 ) ){
      std::cout << "angle " << box.angle << std::endl;
      if (box.angle>0){
        box.angle = 0.5;
      }else{
        box.angle = -0.5;
      }
    }
  }
  */

  if (box_size.width<box_size.height){
    box.angle += 90.;
  }


  if(debug){
    std::cout << "angle " << box.angle << " BWidth "  << box_size.width << " BHeight " << box_size.height << std::endl;
  }

  //te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour passed in seconds: " << te << std::endl;


  cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(box.center.x*2,box.center.y*2), box.angle, 1);

  cv::Mat rotated;
  //cv::Mat rotated(src.rows*1.5,src.cols*1.5,src.type());
  if (false){
    rotated = cv::Mat(src.rows,src.cols,src.type());
    cv::warpAffine(src, rotated, rot_mat, rotated.size(), cv::INTER_CUBIC);
  }else{
    rotated = src.clone();
  }
  /*
  if (box.angle < -45.){
    std::cout << "swap";
    std::swap(box_size.width, box_size.height);
  }
  std::cout << "bw bh " << box_size.width  << "-" << box_size.height  << std::endl;
  */
  //
  cv::Mat cropped;
  cv::getRectSubPix(rotated, box_size, box.center, cropped);
  //te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour* passed in seconds: " << te << std::endl;

  // ok it`s rotated
  largest_area=0;
  largest_contour_index=0;
  cvtColor(rotated,thr,CV_BGR2GRAY); //Convert to gray
  cv::GaussianBlur(thr, thr, ksize, 0);
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

  //showImage(rotated,"LC rotated");
  cv::Rect myROI(bounding_rect.x, bounding_rect.y, bounding_rect.width, bounding_rect.height);
  cv::Mat result = rotated(myROI);

  if(debug){
    std::cout << "myROI " << " BWidth "  << myROI.width << " BHeight " << myROI.height  << std::endl;
  }


  if (debug){
    te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
    std::cout << "largestContour passed in seconds: " << te << std::endl;
  }
  return result;
}


double ImageRecognize::getOrientation(std::vector<cv::Point> &pts, cv::Mat &img){
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
    cv::Point pos = cv::Point(pca_analysis.mean.at<double>(0, 0),
                      pca_analysis.mean.at<double>(0, 1));

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

    return atan2(eigen_vecs[0].y, eigen_vecs[0].x);
}


bool ImageRecognize::is_digits(const std::string &str){
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}

bcResult ImageRecognize::barcode(cv::Mat& im){
  double t = (double)cv::getTickCount();
  double te;

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};

  int rel=0;
  int tmp=0;

  cv::Mat gray;
  bool found=false;
  cv::Mat norm;
  cv::Mat mask;
  int type = cv::NORM_MINMAX;
  int dtype = -1;
  int min=0;
  int max=255;
  cv::Point point;
  cv::Size ksize(5,5);
  int i = 0;
  cv::Mat part = im.clone();
  res = barcode_internal(part);

  if (debug){
    te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
    std::cout << "barcode " << res.code << " passed in seconds: " << te << std::endl;
  }
  return res;
}


bcResult ImageRecognize::barcode_internal(cv::Mat &part) {

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};
  cv::Mat gray;
  cv::Mat norm;
  cv::Mat mask;
  int type = cv::NORM_MINMAX;
  int dtype = -1;
  int min=0;
  int max=255;
  cv::Point point;
  cv::Size ksize(5,5);
  int i = 0;
  int rel=0;
  int tmp=0;
  bool codeRetry=false;
  if (debug){
    std::cout << "barcode_internal " << std::endl;
  }

  cv::Mat image_clahe;
  if (barcode_light_correction==true){
    cv::Mat lab_image;
    cv::cvtColor(part, lab_image, CV_BGR2Lab);

    // Extract the L channel
    std::vector<cv::Mat> lab_planes(3);
    cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    cv::Mat dst;
    clahe->apply(lab_planes[0], dst);

    // Merge the the color planes back into an Lab image
    dst.copyTo(lab_planes[0]);
    cv::merge(lab_planes, lab_image);

   // convert back to RGB
   cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);
  }else{
    image_clahe=part.clone();
  }


  codes="";
  // counting here down
  for (int thres=15;((thres<220)&&(
    res.found==false && codeRetry==false
  ));thres+=5){

    cv::cvtColor(image_clahe, gray, CV_BGR2GRAY);

    cv::threshold(gray,gray,thres,255, CV_THRESH_BINARY );

    cv::normalize(gray, norm, min, max, type, dtype, mask);
    cv::GaussianBlur(norm, norm, ksize, 0);


//    cv::adaptiveThreshold(gray,norm,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,7,40);


    zbar::ImageScanner scanner;
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
    scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
    scanner.set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ADD_CHECK, 1);

    zbar::Image image(norm.cols, norm.rows, "Y800", (uchar *)norm.data, norm.cols * norm.rows);
    int tmp_window_wait=window_wait;
    window_wait=50;
    showImage(norm);
    window_wait=tmp_window_wait;
    int n = scanner.scan(image);
    for(zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
      if (debug){
        std::cout << "thres " << thres << " Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
      }
      std::string code = std::string(symbol->get_data().c_str());
      codes += code+" ";
      std::string type = std::string(symbol->get_type_name().c_str());
      if ((code.length() > res.code.length())){
        if (
          (
            ( (type=="I2/5") && (is_digits(code)) ) ||
            ( (type!="I2/5")  )
          ) && (
            code.substr(0,4) != "0000"
          )
        ){
          if (debug){
            std::cout << "Code Length: " << code.length()-1 << std::endl;
          }
          if (type=="I2/5"){
            res.code = code.substr(0,code.length()-1);
            if (code.length()-1<11){
            }else{
              res.found = true;
            }
          }else{
            res.code = code;//std::string(symbol->get_data().c_str());
            res.found = true;
          }
          resultThres = thres;
          res.type = std::string(symbol->get_type_name().c_str());
          int loc_size = symbol->get_location_size();

          int min_x=9999;
          int max_x=0;
          int min_y=9999;
          int max_y=0;

          for(int i=0;i<loc_size;i++){
            tmp = (symbol->get_location_y(i)*100/gray.rows);

            if (max_y<symbol->get_location_y(i)){
              max_y=symbol->get_location_y(i);
            }
            if (min_y>symbol->get_location_y(i)){
              min_y=symbol->get_location_y(i);
            }

            if (max_x<symbol->get_location_x(i)){
              max_x=symbol->get_location_x(i);
            }
            if (min_x>symbol->get_location_x(i)){
              min_x=symbol->get_location_x(i);
            }
            if (rel<tmp){
              rel=tmp;
              res.point = cv::Point(symbol->get_location_x(i),symbol->get_location_y(i));
            }
          }

          int margin = 0;//oneCM*1;

          if (min_x>margin){
            min_x-=margin;
          }else{
            min_x=0;
          }

          if (min_y>margin){
            min_y-=margin;
          }else{
            min_y=0;
          }

          if (max_x<gray.cols-margin){
            max_x+=margin;
          }else{
            max_x=gray.cols;
          }

          if (max_y<gray.rows-margin){
            max_y+=margin;
          }else{
            max_y=gray.rows;
          }
          // to do
          res.rect = cv::Rect(min_x,min_y,max_x-min_x,max_y-min_y);
        }
      }
    }
    image.set_data(NULL, 0);
    scanner.recycle_image(image);
  }
  return res;
}


bcResult ImageRecognize::fast_barcode(cv::Mat& im){
  double t = (double)cv::getTickCount();
  double te;
  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string("")};

  int rel=0;
  int tmp=0;




  cv::Mat gray;
  cv::Mat norm;
  cv::Mat mask;
  int type = cv::NORM_MINMAX;
  int dtype = -1;
  int min=0;
  int max=255;
  cv::Point point;
  cv::Size ksize(5,5);
  int i = 0;

  cv::Mat useIMG = (im.clone());
  if (useIMG.cols<useIMG.rows){
    rotateX(useIMG,90,cv::Point(useIMG.cols/2,useIMG.rows/2));
  }
  //cv::Rect roi = fittingROI((useIMG.cols/oneCM)-15 ,1.5,15,4,useIMG);
  cv::Rect roi(0,0,useIMG.cols,useIMG.rows/4);//fittingROI((useIMG.cols/oneCM)-15 ,2,15,4,useIMG);
  cv::Mat part = useIMG(roi);

  res = barcode_internal(part);
  if (res.found==false){


    cv::Mat rotated(useIMG.cols,useIMG.rows,useIMG.type());
    transpose(useIMG, rotated);
    flip(rotated, rotated,1); //transpose+flip(1)=CW
    cv::Mat rotated2(rotated.cols,rotated.rows,rotated.type());
    transpose(rotated, rotated2);
    flip(rotated2, useIMG,1); //transpose+flip(1)=CW
    part = useIMG(roi);

    res = barcode_internal(part);
    if (debug){
      std::cout << "fast barcode scanned flipped "<< std::endl;
    }
  }

  if (res.found==false){
    useIMG = (im.clone());
    res = barcode_internal(part);
    if (debug){
      std::cout << "fast barcode scanned all "<< std::endl;
    }
  }
  if (debug){
    te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
    std::cout << "fast barcode passed in seconds: " << te << std::endl;
  }
  return res;
}

cv::Rect ImageRecognize::fittingROI(double x,double y,double w,double h, cv::Mat& m1){
  int rX=x*oneCM;
  int rY=y*oneCM;
  int rW=w*oneCM;
  int rH=h*oneCM;

  if (rX+rW > m1.cols){
    rX -= (rX+rW)-m1.cols;
    if (rX<0){
      rX=0;
      rW -= (rX+rW)-m1.cols;
    }
  }

  if (rY+rH > m1.rows){
    rY -= (rY+rH)-m1.rows;
    if (rY<0){
      rY=0;
      rH -= (rY+rH)-m1.rows;
    }
  }

  if (rX<0){
    rX=0;
  }

  if (rY<0){
    rY=0;
  }

  if (rX+rW>m1.cols){
    rX = 0;
    rW = m1.cols;
  }

  if (rY+rH>m1.rows){
    rY = 0;
    rH = m1.rows;
  }
  if (rW<=0){
    rX = 0;
    rW = m1.cols;
  }
  if (rH<=0){
    rY = 0;
    rH = m1.rows;
  }

  return cv::Rect(rX,rY,rW,rH);
}

std::string ImageRecognize::getText(cv::Mat& im){

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
    if(debug){
      std::cout << "########INTERMEDIA##########" << std::endl;
      std::cout << intermedia << std::endl;
      std::cout << "########INTERMEDIA##########" << std::endl;
    }
    return intermedia;
  }else{



    std::string newtext;
    std::cout << out << std::endl;
    newtext = boost::regex_replace(intermedia, number_space, "");

    if(debug){
      std::cout << "##################" << std::endl;
      std::cout << newtext << std::endl;
      std::cout << "##################" << std::endl;
    }

    return newtext;
  }
}


bool ImageRecognize::containsZipCode(cv::Mat& im,cv::Mat& orig){
  int i=0;
  int j=0;
  int m=0;
  int lastThreshold=0;
  std::vector<std::string> lines;
  const boost::regex plz_regex("\\d{5}\\s");
  const boost::regex no_plz_regex("\\d{6}\\s");

  recalcSubstractMean(im);

  cv::Mat c2 = im.clone();

  if (debug){
    std::cout << "before linearize" << std::endl;
  }

  lastThreshold = linearize(im,-0.30);
  showImage(im);

  if (debug){
    std::cout << "after linearize" << std::endl;
  }

  std::string s1 = getText(im);//(out);
  if (debug){
    std::cout << "after getText" << std::endl;
  }
  boost::replace_all(s1,code,"-------------");
  boost::replace_all(s1,"\n\n","\n");
  if (debug){
    std::cout << "after replace_all" << std::endl;
  }
  lines = isplit(s1,'\n');
  if (debug){
    std::cout << "spliting lines"  << lines.size() << std::endl;
  }

  if (lines.size()<3){
    return false;
  }
  std::vector<std::string>::iterator it;
  for (it = lines.begin(); it != lines.end(); it++){
    if ((*it).length()<3){
      lines.erase(it,it);
    }
  }
  if (debug){
    std::cout << "erase lines " << lines.size() << std::endl;
  }


  if (debug){
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
        ocr_text = s1.c_str();
        if (debug){
          std::cout << "ImageRecognize::contains ZipCode" << std::endl << resultText << std::endl << std::endl;
        }
        resultThres = lastThreshold;
        makeResultImage(orig,0.85);
        return true;
      }
    }

  }else{
    if (debug){
      std::cout << "dont think it is an address (less than 2 or more than 15 lines)" << std::endl;
    }
  }

  return false;
}

bool ImageRecognize::usingLetterRoi(cv::Mat& im,cv::Rect roi2){
  const char* out;
  std::vector<std::string> lines;
  const boost::regex plz_regex("\\d{5}\\s");
  const boost::regex no_plz_regex("\\d{6}\\s");
  boost::cmatch char_matches;

  int breite = im.cols/oneCM;
  int hoehe = im.rows/oneCM;
  float ratio = ( ((im.rows *1.0) / (im.cols *1.0 )) );
  cv::Mat c2 = (im.clone())(roi2);
  if (containsZipCode(c2,im)){
    return true;
  }

  if (rotate_inline==true){
    if(debug){
      std::cout << "try rotated" << std::endl;
    }

    allTogether += "\n\n" + resultText;

    cv::Mat rotated(im.cols,im.rows,im.type());

    transpose(im, rotated);
    flip(rotated, rotated,1); //transpose+flip(1)=CW
    cv::Mat rotated2(rotated.cols,rotated.rows,rotated.type());
    transpose(rotated, rotated2);
    flip(rotated2, rotated2,1); //transpose+flip(1)=CW
    c2 = rotated2(roi2);
    if(debug){
      std::cout << "**try rotated" << std::endl;
    }

    if (containsZipCode(c2,rotated2)){
      if(debug){
        std::cout << "***try rotated" << std::endl;
      }
      return true;
    }

    allTogether += "\n\n" + std::string(resultText);
  }
  return false;
}

void ImageRecognize::makeResultImage(cv::Mat& src,float multiply){
  cv::Mat clone;


  cvtColor( src, clone, CV_BGR2GRAY );
  // 8812
  //cv::threshold(clone, resultMat, resultThres, 255, CV_THRESH_BINARY );//| CV_THRESH_OTSU);
//  cv::threshold(clone, resultMat, resultThres, 255, CV_THRESH_BINARY );//| CV_THRESH_OTSU);
//   cv::adaptiveThreshold(thr,src,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,55,subtractMean);


  cv::adaptiveThreshold(clone,resultMat,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,blockSize,subtractMean);
  //cv::adaptiveThreshold(clone,resultMat,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,159,20);
  //  cv::adaptiveThreshold(clone,resultMat,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY,11,10);
  int x=resultMat.cols /2;
  int y=resultMat.rows /2;
  cv::Mat res = cv::Mat(x, y, CV_8UC1);
  cv::resize(resultMat, res, cv::Size(x, y), 0, 0, 3);
  resultMat = res;

  if (headOver==false){
    transpose(resultMat, resultMat);
    flip(resultMat, resultMat,1);
    transpose(resultMat, resultMat);
    flip(resultMat, resultMat,1);
  }



}



bool ImageRecognize::usingLetterType1(cv::Mat& im){
  cv::Rect roi2 = fittingROI(2,6,10,6,im);
  /*
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |   -------                          |
  |   -------                          |
  |   -------                          |
  |                                    |
  --------------------------------------
  */
  return usingLetterRoi(im,roi2);
}



bool ImageRecognize::usingLetterType1_1(cv::Mat& im){
  cv::Rect roi2 = fittingROI(12,5,10,9,im);
  /*
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |                     -------        |
  |                     -------        |
  |                     -------        |
  |                                    |
  --------------------------------------
  */
  return usingLetterRoi(im,roi2);
}


bool ImageRecognize::usingLetterType1_2(cv::Mat& im){
  cv::Rect roi2 = fittingROI(2.2,2.5,9,4,im);
  /* PZA
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |   -------                          |
  |   -------                          |
  |   -------                          |
  |                                    |
  --------------------------------------
  */
  return usingLetterRoi(im,roi2);
}


bool ImageRecognize::usingLetterType3(cv::Mat& im){
  cv::Rect roi2 = fittingROI( 1 , (im.rows/oneCM)/2 , (im.cols/oneCM) - 2, 6,im);
  /*
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |  -------------------------------   |
  |  -------------------------------   |
  |  -------------------------------   |
  |                                    |
  --------------------------------------
  */
  return usingLetterRoi(im,roi2);
}


bool ImageRecognize::usingLetterType2(cv::Mat& im){
  //cv::Rect roi2 = fittingROI(12,5,12,14,im);
  //cv::Rect roi2 = fittingROI(2,5,11,7,im);
  cv::Rect roi2 = fittingROI(2,5,11,7,im);
  const char* out;
  /*
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |   -------                          |
  |   -------                          |
  |   -------                          |
  |                                    |
  |                                    |
  ~                                    ~
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  --------------------------------------
  */

  if (windowalltogether){
    cv::rectangle(debugImage,roi2,cv::Scalar(255,255,255));
    showDebugImage();
  }

  cv::Mat  ix = (im.clone())(roi2);

  return usingLetterRoi(im,roi2);
}


bool ImageRecognize::usingLetterType2_1(cv::Mat& im){
  cv::Rect roi2 = fittingROI((im.cols/oneCM)-15,(im.rows/oneCM)-10,11,7,im);
  const char* out;
  /*
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  ~                                    ~
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  |                      -------       |
  |                      -------       |
  |                      -------       |
  |                                    |
  --------------------------------------
  */
  cv::Mat  ix = (im.clone())(roi2);
  if (windowalltogether){
    cv::rectangle(debugImage,roi2,cv::Scalar(255,255,255));
    showDebugImage();
  }
  return usingLetterRoi(im,roi2);
}



bool ImageRecognize::usingLetterType2_2(cv::Mat& im){
  cv::Rect roi2 = fittingROI((im.cols/oneCM)-17,(im.rows/oneCM)-10,13,7,im);
  const char* out;
  /*
  --------------------------------------
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  ~                                    ~
  |                                    |
  |                                    |
  |                                    |
  |                                    |
  |                      -------       |
  |                      -------       |
  |                      -------       |
  |                                    |
  --------------------------------------
  */
  cv::Mat  ix = (im.clone())(roi2);
  if (windowalltogether){
    cv::rectangle(debugImage,roi2,cv::Scalar(255,255,255));
    showDebugImage();
  }
  return usingLetterRoi(im,roi2);
}

const char* ImageRecognize::text(cv::Mat& im){
  const char* out;
  int letterType = 0;




  if (windowalltogether){
    debugImage = im.clone();
    showDebugImage();
  }



  cv::Mat usemat = im.clone();
  std::string addressfield = "L";


  if (code=="123456789012"){
    /*
    std::cout << "TEST CARD FOUND!!!! " << std::endl;

    if (debug){
      std::cout << "H " << hoehe << " | B " << breite << std::endl;
    }

    int result_width = 12;
    int result_height = 10;

    double rescale_height = result_height*1.0/height*1.0;
    double rescale_width = (result_width*1.0/width*1.0)/rescale_height;
    std::cout << "INITIAL SCALE rescale_height " << std::endl;
    printf("%'.2f", rescale_height);
    std::cout << "INITIAL SCALE rescale_width " << std::endl;
    printf("%'.2f", rescale_width);

    usemat = cv::Mat(im.cols*rescale_width, im.rows*rescale_height, CV_32FC3);
    cv::resize(im, usemat, cv::Size(im.cols*rescale_width, im.rows*rescale_height), 0, 0, 3);
    int breite=result_width;
    int hoehe=result_height;

    std::cout << "INITIAL SCALE SHOULD BE " << std::endl;
    printf("%'.2f", scale/rescale_width);
    std::cout << "* " << std::endl;


    int lastweight = 0;
    int last_subtractMean = 0;
    for(subtractMean=0;subtractMean<40;subtractMean++){
      if (usingLetterType1(usemat)){
        int weight = 0;

        std::string r = std::string(ocr_text);
        std::size_t found = r.find("Max Mustermann");
        if (found!=std::string::npos){
          weight = 1;

          found = r.find("Musterweg 1234a");
          if (found!=std::string::npos){
            weight += 10;
            found = r.find("8765 Musterhausen");
            if (found!=std::string::npos){
              weight += 100;
            }
          }
        }
        if (lastweight<weight){
          lastweight=weight;
          last_subtractMean = subtractMean;
        }
      }
    }

    std::cout << std::endl;
    std::cout << "SUBSTRACT_MEAN should be " << last_subtractMean << std::endl;
    std::cout << std::endl;

    */
  }else{

    float result_cols = 0;
    float result_rows = 0;
    double rescale_cols = 1;// result_height*1.0/height*1.0;
    double rescale_rows = scale;


    int height = usemat.rows / oneCM;
    int width = usemat.cols / oneCM;

    std::cout << "width "  << width << " cm" << std::endl;
    std::cout << "height "  << height << " cm" << std::endl;
    // alter table bbs_data add addressfield varchar(2) default 'L';
    std::string sql = "select height/100,length/100,addressfield from bbs_data where id = '"+code+"'; ";
    std::cout << "SQL " << sql << std::endl;
    if (mysql_query(con, sql.c_str())){
      std::cout << "EE " << sql << std::endl;
      fprintf(stderr, "%s\n", mysql_error(con));
    }else{
      MYSQL_RES *result;
      MYSQL_ROW row;
      unsigned int num_fields;
      unsigned int i;

      result = mysql_use_result(con);
      num_fields = mysql_num_fields(result);
      while ((row = mysql_fetch_row(result))){
         unsigned long *lengths;
         result_cols = atof(row[0]);
         result_rows = atof(row[1]);

         addressfield = row[2];

         std::cout << "result_cols "  << result_cols << std::endl;
         std::cout << "result_rows "  << result_rows << std::endl;


         // length is messured exact by the fp machine
         // the height is fixed by the camera
         rescale_cols = 1;// result_height*1.0/height*1.0;
         rescale_rows = result_rows*oneCM / ((double)im.rows)*1.0;



      }


      std::cout << "rescale_cols "  << rescale_cols << std::endl;
      std::cout << "rescale_rows "  << rescale_rows << std::endl;
      showImage(im);
      usemat = cv::Mat(im.cols*rescale_cols, im.rows*rescale_rows, CV_32FC3);
      cv::resize(im, usemat, cv::Size(im.cols*rescale_cols, im.rows*rescale_rows), 0, 0, 3);
      std::cout << "INITIAL SCALE ROWS " << rescale_rows << std::endl;

      if ((result_cols>11.0) && (result_cols<12.6)){
        if ((result_rows>21.0) && (result_rows<22.6)){
          std::cout << "LETTER DIN Lang should store the SCALE value!" << std::endl;
          std::string usql = "update bbs_maschine set scale_rows= "+(std::to_string(rescale_rows))+" where prefix='"+machine_id+"'";
          std::cout << usql << std::endl;
          if (mysql_query(con, usql.c_str())){
            std::cout << "EE " << usql << std::endl;
            fprintf(stderr, "%s\n", mysql_error(con));
          }
        }
      }

    }



    if (usemat.rows>usemat.cols){
      std::cout << "ROTATE IMAGE" << std::endl;
      // lange seite -> drehen
      cv::Mat rotated;
      transpose(usemat, rotated);
      flip(rotated, rotated,1);
      usemat = rotated.clone();

      transpose(usemat, rotated);
      flip(rotated, rotated,1);
      usemat = rotated.clone();

      transpose(usemat, rotated);
      flip(rotated, rotated,1);
      usemat = rotated.clone();

    }


    int hoehe = usemat.rows / oneCM;
    int breite = usemat.cols / oneCM;

    std::cout << "hoehe " << hoehe << " cm" << std::endl;
    std::cout << "breite " << breite << " cm" << std::endl;


    // din lang
    // din a5
    if (hoehe <= 21){
      if (breite >= 20){
        letterType=1;
      }
    }

    // grossbrief
    if (hoehe > 21){
      if (breite > 20){
        letterType=2;
      }
    }

    // c5 c6
    if (hoehe <= 21){
      if (breite < 20){
        letterType=3;
      }
    }




    if (debug){
      std::cout << "H " << hoehe << " | B " << breite << std::endl;
    }

    allTogether = "";

    if(debug){
      std::cout << "type " << letterType  << std::endl;
    }

    std::list<std::string> algorithm_order;
    //letterType=1;
    std::cout << "letterType " << letterType << std::endl;

    if (letterType==1){
      if (addressfield=="L"){
        algorithm_order.push_back( std::string("usingLetterType1_0") );
        algorithm_order.push_back( std::string("usingLetterType1_1") );
        algorithm_order.push_back( std::string("usingLetterType3_0") );
      }else{
        algorithm_order.push_back( std::string("usingLetterType1_1") );
        algorithm_order.push_back( std::string("usingLetterType1_0") );
        algorithm_order.push_back( std::string("usingLetterType3_0") );
      }
    }else if (letterType==2){
      cv::Mat rotated;
      transpose(usemat, rotated);
      flip(rotated, rotated,1);
      usemat = rotated.clone();
      if (addressfield=="L"){
        algorithm_order.push_back( std::string("usingLetterType2_0") );
        algorithm_order.push_back( std::string("usingLetterType2_1") );
      }else{
        algorithm_order.push_back( std::string("usingLetterType2_1") );
        algorithm_order.push_back( std::string("usingLetterType2_0") );
      }
      algorithm_order.push_back( std::string("usingLetterType2_2") );
    }else if (letterType==3){
      algorithm_order.push_back( std::string("usingLetterType3_0") );
    }

    for(std::list<std::string>::const_iterator i = algorithm_order.begin(); i != algorithm_order.end(); ++i){
       std::cout << i->c_str() << std::endl;

       if (strcmp(i->c_str(),"usingLetterType1_0")==0){
         if (usingLetterType1(usemat)){
           return ocr_text;
         }
       }
       if (strcmp(i->c_str(),"usingLetterType1_1")==0){
         if (usingLetterType1_1(usemat)){
           return ocr_text;
         }
       }
       if (strcmp(i->c_str(),"usingLetterType2_0")==0){
         if (usingLetterType2(usemat)){
           return ocr_text;
         }
       }
       if (strcmp(i->c_str(),"usingLetterType2_1")==0){
         if (usingLetterType2_1(usemat)){
           return ocr_text;
         }
       }
       if (strcmp(i->c_str(),"usingLetterType2_2")==0){
         if (usingLetterType2_2(usemat)){
           return ocr_text;
         }
       }
       if (strcmp(i->c_str(),"usingLetterType3_0")==0){
         if (usingLetterType3(usemat)){
           return ocr_text;
         }
       }
    }

    /*
    if (letterType==1){


      if (debug){
        std::cout << "usingLetterType1" << std::endl;
      }
      if (usingLetterType1(usemat)){
        if (debug){
          std::cout << "return usingLetterType1" << std::endl;
        }
        return ocr_text;
      }else{
        if (debug){
          std::cout << "usingLetterType1_1" << std::endl;
        }
        if (usingLetterType1_1(usemat)){
          if (debug){
            std::cout << "return usingLetterType1_1" << std::endl;
          }
          return ocr_text;
        }else{


          if (debug){
            std::cout << "Lettertype 1, do something" << std::endl;
          }
        }
      }
    }else if (letterType==2){

      cv::Mat rotated;
      transpose(usemat, rotated);
      flip(rotated, rotated,1); //transpose+flip(1)=CW
      // sichtfenster voran

      if (windowalltogether){
        debugImage = rotated.clone();
        transpose(rotated, debugImage);
        flip(debugImage, debugImage,1); //transpose+flip(1)=CW
        showDebugImage();
      }


      if (usingLetterType2(rotated)){
        return ocr_text;
      }else{
        if (usingLetterType2_1(rotated)){
          return ocr_text;
        }else{
          if (usingLetterType2_2(rotated)){
            return ocr_text;
          }else{



            rotated = usemat.clone();

            if (windowalltogether){
              debugImage = usemat.clone();
              showDebugImage();
            }


            if (usingLetterType2(rotated)){
              return ocr_text;
            }else{
              if (usingLetterType2_1(rotated)){
                return ocr_text;
              }else{
                if (usingLetterType2_2(rotated)){
                  return ocr_text;
                }else{




                      transpose(rotated, rotated);
                      flip(rotated, rotated,1);
                      transpose(rotated, rotated);
                      flip(rotated, rotated,1);

                      if (windowalltogether){
                        debugImage = rotated.clone();
                        transpose(rotated, debugImage);
                        flip(debugImage, debugImage,1); //transpose+flip(1)=CW
                        showDebugImage();
                      }


                      if (usingLetterType2(rotated)){
                        return ocr_text;
                      }else{
                        if (usingLetterType2_1(rotated)){
                          return ocr_text;
                        }else{
                          if (usingLetterType2_2(rotated)){
                            return ocr_text;
                          }else{



                            transpose(rotated, rotated);
                            flip(rotated, rotated,1);

                            if (windowalltogether){
                              debugImage = usemat.clone();
                              showDebugImage();
                            }


                            if (usingLetterType2(rotated)){
                              return ocr_text;
                            }else{
                              if (usingLetterType2_1(rotated)){
                                return ocr_text;
                              }else{
                                if (usingLetterType2_2(rotated)){
                                  return ocr_text;
                                }else{

                                  if (debug){
                                    std::cout << "Lettertype 2, do something" << std::endl;
                                  }
                                }
                              }
                            }


                          }
                        }
                      }


                }
              }
            }


          }
        }
      }
    }else if (letterType==3){

      if (usingLetterType3(usemat)){
        return ocr_text;
      }else{


        if(debug){
          std::cout << "Lettertype 3, do something" << std::endl;
        }



      }
    }else{
      if (usingLetterType1(usemat)){
        return ocr_text;
      }else{

        if (usingLetterType1_1(usemat)){
          return ocr_text;
        }else{
        }
      }
    }
    */


    /*
    if(debug){
      std::cout << "------>>>>>"  << std::endl;
    }
    // thomas hoffmann 22.08.
    // Infosendungen mit fenster rechts unten
    cv::Rect roi2 = fittingROI((usemat.cols/oneCM)-12,(usemat.rows/oneCM)-17,13,7,usemat);
    if(debug){
      std::cout << "------>>>>>"  << std::endl;
    }
    cv::Mat imx = usemat(roi2);
    if(debug){
      std::cout << "------>>>>>"  << std::endl;
    }
    if (usingLetterRoi(usemat,roi2)){
      if(debug){
        std::cout << "------>>>>>"  << std::endl;
      }
        return ocr_text;
    }
    */
    if(debug){
      std::cout << "found nothing "  << std::endl;
    }

  } // no testcard!

  return out;
}


void ImageRecognize::rotate(cv::Mat& src, double angle, cv::Mat& dst)
{
    int len = std::max(src.cols, src.rows);
    cv::Point2f pt(len/2., len/2.);
    cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);

    cv::warpAffine(src, dst, r, cv::Size(len, len));
}


void ImageRecognize::linearize(cv::Mat& src){

    std::vector<cv::Mat> bgr_planes;
    cv::split( src, bgr_planes );
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat b_hist;
    cv::calcHist( &bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
    float min=0;
    float max=255;
    float sum=0;
    float avg=0;
    for(int i=0;i<histSize;i++){
      sum+=b_hist.at<float>(i);
    }
    avg = sum/histSize;

    for(int i=30;i<histSize;i++){
      if (b_hist.at<float>(i)>avg*0.5){
        if (min==0){
          min=i;
          break;
        }
      }
    }
    for(int i=255;i>31;i--){
      if (b_hist.at<float>(i)>avg){
        if (max==255){
          max=i;
          break;
        }
      }
    }
    if (max-min<40){
      max+=20;
      min-=20;
    }
    if (max>255){
      max=255;
    }
    if (min<0){
      min=0;
    }
    /*
    if(debug){
      std::cout << "min" << min << std::endl;
      std::cout << "max" << max << std::endl;
    }*/

    // todo play around with that value
    int minX = min*0.85;
    if (minX<10){
      minX=10;
    }
    /*
    if (debug){
      std::cout << "minX" << minX << std::endl;
    }*/
    cv::threshold(src,src,minX,255, CV_THRESH_BINARY);
    showImage(src);
}


int ImageRecognize::linearize(cv::Mat& src,float multiply){
  /*
    cv::Mat clone = src.clone();
    std::vector<cv::Mat> bgr_planes;
    cv::split( src, bgr_planes );
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat b_hist;
    cv::calcHist( &bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
    float min=0;
    float max=255;
    float sum=0;
    float avg=0;
    for(int i=0;i<histSize;i++){
      sum+=b_hist.at<float>(i);
    }
    avg = sum/histSize;

    for(int i=30;i<histSize;i++){
      if (b_hist.at<float>(i)>avg*0.5){
        if (min==0){
          min=i;
          break;
        }
      }
    }
    for(int i=255;i>31;i--){
      if (b_hist.at<float>(i)>avg){
        if (max==255){
          max=i;
          break;
        }
      }
    }
    if (max-min<40){
      max+=20;
      min-=20;
    }
    if (max>255){
      max=255;
    }
    if (min<0){
      min=0;
    }

    if(debug){
      std::cout << "min" << min << std::endl;
      std::cout << "max" << max << std::endl;
    }


    // todo play around with that value
    minX = (min +((max-min)/2)*multiply );
    if (minX<10){
      minX=10;
    }

    if (minX>245){
      minX=245;
    }
    if (debug){
      std::cout << "minX" << minX << " d"  << min +((max-min)/2) << std::endl;
    }

    //cv::blur( src, src, cv::Size(3,3) );
    cv::Mat bw;
    int xx =cv::threshold(thr,bw, 0 ,255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    minX = xx/2;
    if (debug){
      std::cout << "XX" << xx << std::endl;
    }
    */
//    cv::threshold(src,src, 0 ,xx-25, CV_THRESH_BINARY);

    cv::Mat thr(src.rows,src.cols,CV_8UC1);
    cvtColor(src,thr,CV_BGR2GRAY); //Convert to gray
//    cv::adaptiveThreshold(thr,src,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,55,20);
/*cv::adaptiveThreshold(
    thr,src,255,
    CV_ADAPTIVE_THRESH_GAUSSIAN_C,
    CV_THRESH_BINARY, 55,
    subtractMean);
    */
    cv::adaptiveThreshold(
        thr,src,255,
        CV_ADAPTIVE_THRESH_GAUSSIAN_C,
        CV_THRESH_BINARY, blockSize,
        subtractMean);
//ADAPTIVE_THRESH_GAUSSIAN_C

    //int x = cv::threshold(src,src, xx-5 ,255, CV_THRESH_BINARY);
    if (false){

//    cv::threshold(src,src, 100 ,255, CV_THRESH_BINARY);
    /*
    std::vector<cv::Mat> bgr_planes_sec;
    cv::split( src, bgr_planes_sec );
    int histSize_sec = 256;
    float range_sec[] = { 0, 256 } ;
    const float* histRange_sec = { range_sec };

    bool uniform_sec = true;
    bool accumulate_sec = false;

    cv::Mat b_hist_sec;
    cv::calcHist( &bgr_planes_sec[0], 1, 0, cv::Mat(), b_hist_sec, 1, &histSize_sec, &histRange_sec, uniform_sec, accumulate_sec );
    float min_sec=0;
    float max_sec=255;
    float sum_sec=0;
    float avg_sec=0;
    for(int i=0;i<histSize_sec;i++){
      sum_sec+=b_hist_sec.at<float>(i);
    }
    avg_sec = sum_sec/histSize_sec;

    for(int i=30;i<histSize_sec;i++){
      if (b_hist_sec.at<float>(i)>avg_sec*0.5){
        if (min_sec==0){
          min_sec=i;
          break;
        }
      }
    }
    for(int i=255;i>31;i--){
      if (b_hist_sec.at<float>(i)>avg_sec){
        if (max_sec==255){
          max_sec=i;
          break;
        }
      }
    }
    if (max_sec-min_sec<40){
      max_sec+=20;
      min_sec-=20;
    }
    if (max_sec>255){
      max_sec=255;
    }
    if (min_sec<0){
      min_sec=0;
    }
    if(debug){
      std::cout << "min" << min_sec << std::endl;
      std::cout << "max" << max_sec << std::endl;
    }
    if (min_sec>(min +((max-min)/2))){
      minX = (min +((max-min)/2) );
      if(debug){
        std::cout << "use alternative minX" << minX << std::endl;
      }
      cv::threshold(clone,src, minX*0.8 ,255, CV_THRESH_BINARY);

    }

    */
    }
    showImage(src);
    return minX;
}
