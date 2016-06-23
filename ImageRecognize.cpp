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
    cv::waitKey(500);
  }
}

void ImageRecognize::showImage(cv::Mat& src,char* title){
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


    cv::namedWindow(title, CV_WINDOW_AUTOSIZE );
    cv::imshow(title, res );
    cv::waitKey(5000);
  }
}

void ImageRecognize::open(const char* filename){
  double t = (double)cv::getTickCount();
  double te;
  int analysisType = 1;
  showWindow=true;
  resultThres = 128;
  fileName = filename;
  orignalImage = cv::imread( filename, 1 );


  tess=new tesseract::TessBaseAPI();
  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);


  cv::Mat minmat = cv::Mat(orignalImage.cols*scale, orignalImage.rows, CV_32FC3);
  cv::resize(orignalImage, minmat, cv::Size(orignalImage.cols*scale, orignalImage.rows), 0, 0, 3);
  orignalImage = minmat;

  //std::cout << filename << std::endl;
  //std::cout << "image readed " << (orignalImage.rows/2) << "* " << orignalImage.cols << 'x' << orignalImage.rows << std::endl;
  oneCM = orignalImage.cols/cmWidth;

  cv::Mat mat(  orignalImage.rows*2,orignalImage.cols*2, orignalImage.type(), cv::Scalar(0));
  cv::Rect roi( cv::Point( orignalImage.cols/2, (orignalImage.rows/2) ), orignalImage.size() );

  orignalImage.copyTo( mat( roi ) );
  //std::cout << "image relocated" << std::endl;

  std::string output = "";
  bcResult bcRes;
  cv::Point bc_point;
  cv::Mat largest;
  const char* out;
  if (analysisType==0){

    largest = largestContour(mat);
    bcRes = barcode(largest);
    bc_point=bcRes.point;
    if (bc_point.y>(largest.rows/2)){
      rotateX(largest,180,cv::Point(largest.cols/2,largest.rows/2));
    }
    double t1 = (double)cv::getTickCount();
    double te1;

    out = text(largest);
    te1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
    std::cout << "text passed in seconds: " << te1 << std::endl;
    std::string s (out);
    std::replace( s.begin(), s.end(), '"', ' ');
    //output = "{ \"code\": \""+bcRes.code+"\", \"text\": \""+s+"\" }";
    code = bcRes.code;
    addresstext = s;
    //jsontext = output;

  }else if (analysisType==1){

    //std::cout << "L1" << std::endl;
    largest = getRectangle(mat);
    //std::cout << "L2" << std::endl;
    //bcRes = fast_barcode(largest);
    bcRes = barcode(largest);
    //std::cout << "L3" << std::endl;
    code = bcRes.code;

    cv::rectangle(largest,bcRes.rect,cv::Scalar(255,255,255),CV_FILLED);
    //std::cout << "x*y" << largest.cols/oneCM << "*" << largest.rows/oneCM << std::endl;

    rotateX(largest,90,cv::Point(largest.cols/2,largest.rows/2));

    double t1 = (double)cv::getTickCount();
    double te1;
    //showImage(largest,"T1");

    out = text(largest);
    te1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
    std::cout << "text passed in seconds: " << te1 << std::endl;
    std::string s (out);
    std::replace( s.begin(), s.end(), '"', ' ');
    addresstext = s;
    //output = "{ \"code\": \""+bcRes.code+"\", \"text\": \""+s+"\" }";
    //jsontext = output;
    bc_point=bcRes.point;

  }

  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "all passed in seconds: " << te << std::endl;
  //std::cout << output << std::endl;

}


void ImageRecognize::openPZA(const char* filename){
  double t = (double)cv::getTickCount();
  double te;
  int analysisType = 1;


  tess=new tesseract::TessBaseAPI();
  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);


  showWindow=true;
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
    std::cout << "Code: " << bcRes.code << std::endl;
    code = bcRes.code;
    getPZAText(orignalImage);
  }else{
    std::cout << "Nocode: " << "try flipped" << std::endl;


    transpose(orignalImage, orignalImage);
    flip(orignalImage, orignalImage,1); //transpose+flip(1)=CW
    transpose(orignalImage, orignalImage);
    flip(orignalImage, orignalImage,1); //transpose+flip(1)=CW

    cv::Rect roif( 0, 0, orignalImage.cols,orignalImage.rows/3);
    mat = orignalImage(roif);

//    cv::transpose(mat, mat);
//    cv::transpose(orignalImage, orignalImage);
    bcRes = barcode(mat);
    str_barcode = (bcRes.code);
    if (str_barcode.length()>4){
      code = bcRes.code;
      std::cout << "Code: " << bcRes.code << std::endl;
      getPZAText(orignalImage);
    }else{
    }
  }
/*
  showImage(mat,"test");

  tess=new tesseract::TessBaseAPI();
  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);

*/
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "all passed in seconds: " << te << std::endl;
}

void ImageRecognize::getPZAText(cv::Mat& src){
  cv::Rect roi( 0, 0, src.cols,src.rows/3);
  cv::Mat m = src(roi);
  usingLetterType1_2(m);
  addresstext = ocr_text;
  //std::cout << "TXT" << ocr_text << std::endl;
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
  int const lowThreshold = 5;
  int const maxThreshold = 200;
  int ratio = 3;
  int kernel_size = 3;

  cvtColor( src, src_gray, CV_BGR2GRAY );
  cv::blur( src_gray, detected_edges, cv::Size(3,3) );
  /// Canny detector
  cv::Canny( detected_edges, detected_edges, lowThreshold, maxThreshold, kernel_size );
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
  std::cout << "1*" << top_left << bottom_right << std::endl;
  cv::Rect roi(top_left.x,top_left.y,bottom_right.x-top_left.x,bottom_right.y-top_left.y);
  std::cout << "2*" << std::endl;
  cv::Mat c_a = src(roi);
  std::cout << "3*" << std::endl;
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "largestContour (rect) passed in seconds: " << te << std::endl;
  return c_a;
}

cv::Mat ImageRecognize::largestContour(cv::Mat& src){

  double t = (double)cv::getTickCount();
  double te;

  /*
  cv::Rect roia(0,0,src.cols/2,src.rows);
  cv::Rect roib(src.cols/2,0,src.cols/2,src.rows);
  cv::Mat c_a = src(roia);
  cv::Mat c_b = src(roib);

  cv::Mat mean;
  cv::Mat stddev;

  cv::meanStdDev(c_a,mean,stddev);
  std::cout << "mean " << (int)mean.data[0] << std::endl;
  std::cout << "stddev " << (int)stddev.data[0] << std::endl;

  cv::meanStdDev(c_b,mean,stddev);
  std::cout << "mean " << (int)mean.data[0] << std::endl;
  std::cout << "stddev " << (int)stddev.data[0] << std::endl;

  std::cout << "cols im " << src.cols << std::endl;
  std::cout << "cols c_a " << c_a.cols << std::endl;
  std::cout << "cols c_b " << c_b.cols << std::endl;

  */
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

  //te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour passed in seconds: " << te << std::endl;


  cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(box.center.x*2,box.center.y*2), box.angle, 1);

  //cv::Mat rotated;
  //cv::Mat rotated(src.rows*1.5,src.cols*1.5,src.type());
  cv::Mat rotated(src.rows,src.cols,src.type());
  cv::warpAffine(src, rotated, rot_mat, rotated.size(), cv::INTER_CUBIC);
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
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "largestContour passed in seconds: " << te << std::endl;
  //showImage(result, "LC result");
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

  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "barcode passed in seconds: " << te << std::endl;
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
  cv::Size ksize(7,7);
  int i = 0;
  int rel=0;
  int tmp=0;
  std::cout << "barcode_internal " << std::endl;

  for (int thres=15;((thres<220)&&(res.found==false));thres+=5){

    cv::cvtColor(part, gray, CV_BGR2GRAY);
    cv::threshold(gray,gray,thres,150, CV_THRESH_BINARY);

    cv::normalize(gray, norm, min, max, type, dtype, mask);
    cv::GaussianBlur(norm, norm, ksize, 0);


    zbar::ImageScanner scanner;
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
    scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
    scanner.set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ADD_CHECK, 1);

    zbar::Image image(norm.cols, norm.rows, "Y800", (uchar *)norm.data, norm.cols * norm.rows);
    //showImage(norm);
    int n = scanner.scan(image);
    for(zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
      std::cout << "thres " << thres << " Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
      //found = true;
      std::string code = std::string(symbol->get_data().c_str());
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
          //std::cout << "#" << code.substr(0,3) << std::endl;
          if (type=="I2/5"){
            res.code = code.substr(0,code.length()-1);
          }else{
            res.code = code;//std::string(symbol->get_data().c_str());
          }
          res.found = true;
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
  cv::Rect roi = fittingROI((useIMG.cols/oneCM)-15 ,2,15,4,useIMG);
  cv::Mat part = useIMG(roi);

  res = barcode_internal(useIMG);
  if (res.found==false){


    cv::Mat rotated(useIMG.cols,useIMG.rows,useIMG.type());
    transpose(useIMG, rotated);
    flip(rotated, rotated,1); //transpose+flip(1)=CW
    cv::Mat rotated2(rotated.cols,rotated.rows,rotated.type());
    transpose(rotated, rotated2);
    flip(rotated2, useIMG,1); //transpose+flip(1)=CW
    part = useIMG(roi);

    res = barcode_internal(part);
    std::cout << "fast barcode scanned flipped "<< std::endl;

  }

  if (res.found==false){
    useIMG = (im.clone());
    res = barcode_internal(part);
    std::cout << "fast barcode scanned all "<< std::endl;
  }

  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "fast barcode passed in seconds: " << te << std::endl;
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

  std::cout << "fittingROI rX " << rX << std::endl;
  std::cout << "fittingROI rY " << rY << std::endl;
  std::cout << "fittingROI rW " << rW << std::endl;
  std::cout << "fittingROI rH " << rH << std::endl;
  std::cout << "fittingROI m1.rows" << m1.rows << std::endl;
  std::cout << "fittingROI m1.cols" << m1.cols << std::endl;
  return cv::Rect(rX,rY,rW,rH);
}

const char* ImageRecognize::getText(cv::Mat& im){
  tess->SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), im.step1());
  tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
  tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
  tess->Recognize(0);
  return tess->GetUTF8Text();
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

  //showImage(c2,"T2-111");
  //showImage(c2,"TEST 2_1");
  linearize(c2);


  //cv::Mat showIM = im.clone();
  //cv::rectangle(showIM,roi2,cv::Scalar(100,100,100),10);
  //showImage(showIM,"1");
  out = getText(c2);
  std::cout << out << std::endl;
  std::string s1 (out);
  lines = isplit(s1,'\n');

  if ((lines.size()<20)&&(boost::regex_search(s1 , plz_regex)==true)&&(boost::regex_search(s1 , no_plz_regex)==false)){
      ocr_text = out;
      makeResultImage(im);
      return true;
  }

  cv::Mat rotated(im.cols,im.rows,im.type());
  transpose(im, rotated);
  flip(rotated, rotated,1); //transpose+flip(1)=CW
  cv::Mat rotated2(rotated.cols,rotated.rows,rotated.type());
  transpose(rotated, rotated2);
  flip(rotated2, rotated2,1); //transpose+flip(1)=CW
  c2 = rotated2(roi2);
  linearize(c2);

  //showIM = rotated2.clone();
  //showImage(showIM,"1");
  out = getText(c2);
  std::cout << out << std::endl;
  std::string s2 (out);
  lines = isplit(s2,'\n');
  if ((lines.size()<20)&&(boost::regex_search(s2 , plz_regex)==true)&&(boost::regex_search(s1 , no_plz_regex)==false)){
      ocr_text = out;
      makeResultImage(im);
      return true;
  }
  makeResultImage(im);

  return false;
}

void ImageRecognize::makeResultImage(cv::Mat& src){
  cv::Mat clone;


  cvtColor( src, clone, CV_BGR2GRAY );
  cv::threshold(clone, resultMat, resultThres, 255, CV_THRESH_BINARY );//| CV_THRESH_OTSU);

  int x=resultMat.cols /2;
  int y=resultMat.rows /2;
  cv::Mat res = cv::Mat(x, y, CV_8UC1);
  cv::resize(resultMat, res, cv::Size(x, y), 0, 0, 3);
  resultMat = res;

  transpose(resultMat, resultMat);
  flip(resultMat, resultMat,1);
  transpose(resultMat, resultMat);
  flip(resultMat, resultMat,1);




}

bool ImageRecognize::usingLetterType1(cv::Mat& im){
  cv::Rect roi2 = fittingROI(2,5,11,7,im);
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
  cv::Rect roi2 = fittingROI(11,5,12,9,im);
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

  return usingLetterRoi(im,roi2);
}

const char* ImageRecognize::text(cv::Mat& im){
  const char* out;
  int breite = im.cols/oneCM;
  int hoehe = im.rows/oneCM;
  int letterType = 0;

  width = breite;
  height = hoehe;

  // din lang
  if (hoehe < 15){
    if (breite >= 20){
      letterType=1;
    }
  }

  // grossbrief
  if (hoehe > 15){
    if (breite > 20){
      letterType=2;
    }
  }

  // c5 c6
  if (hoehe < 15){
    if (breite < 20){
      letterType=3;
    }
  }



  std::cout << "type " << letterType  << std::endl;
  if (letterType==0){
    std::cout << "width " << width  << std::endl;
    std::cout << "height " << height  << std::endl;
  }
  if (letterType==1){

    if (usingLetterType1(im)){
      return ocr_text;
    }else{

      if (usingLetterType1_1(im)){
        return ocr_text;
      }else{
        std::cout << "Lettertype 1, do something" << std::endl;
      }
    }
  }else if (letterType==2){

    if (usingLetterType2(im)){
      return ocr_text;
    }else{
      if (usingLetterType2_1(im)){
        return ocr_text;
      }else{
        if (usingLetterType2_2(im)){
          return ocr_text;
        }else{

          std::string concat = "";

          if (usingLetterType1(im)){
            concat += "\n\n" + std::string(ocr_text);
            //return ocr_text;
          }else if (usingLetterType1_1(im)){
            concat += "\n\n" + std::string(ocr_text);
            //return ocr_text;
          }else if (usingLetterType2(im)){
            concat += "\n\n" + std::string(ocr_text);
            //return ocr_text;
          }else if (usingLetterType2_1(im)){
            concat += "\n\n" + std::string(ocr_text);
            //return ocr_text;
          }else if (usingLetterType2_2(im)){
            concat += "\n\n" + std::string(ocr_text);
            //return ocr_text;
          }else if (usingLetterType3(im)){
            concat += "\n\n" + std::string(ocr_text);
            //return ocr_text;
          }

          std::cout << "::" << concat << std::endl;
          std::cout << "Lettertype 2, do something" << std::endl;
        }
      }
    }
  }else if (letterType==3){

    if (usingLetterType3(im)){
      return ocr_text;
    }else{

      std::string concat = "";

      if (usingLetterType1(im)){
        concat += "\n\n" + std::string(ocr_text);
        //return ocr_text;
      }else if (usingLetterType1_1(im)){
        concat += "\n\n" + std::string(ocr_text);
        //return ocr_text;
      }else if (usingLetterType2(im)){
        concat += "\n\n" + std::string(ocr_text);
        //return ocr_text;
      }else if (usingLetterType2_1(im)){
        concat += "\n\n" + std::string(ocr_text);
        //return ocr_text;
      }else if (usingLetterType2_2(im)){
        concat += "\n\n" + std::string(ocr_text);
        //return ocr_text;
      }else if (usingLetterType3(im)){
        concat += "\n\n" + std::string(ocr_text);
        //return ocr_text;
      }

      std::cout << ":3:" << concat << std::endl;

      std::cout << "Lettertype 3, do something" << std::endl;
    }
  }



  linearize(im);
  tess->SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), im.step1());
  tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
  tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
  tess->Recognize(0);
  out = tess->GetUTF8Text();
  ocr_text = out;

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
    cv::threshold(src,src,min-1,max+1, CV_THRESH_BINARY);
}
