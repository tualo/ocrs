#include "ImageRecognize.h"


void ImageRecognize::showImage(cv::Mat& src){
  if (showWindow){
    cv::Mat rotated=src.clone();

    int x=src.cols /3;
    int y=src.rows /3;

    if ( src.cols < src.rows ){
      rotate(src, -90, rotated);
    }

    cv::Mat res = cv::Mat(x, y, CV_32FC3);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);


    cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
    cv::imshow("DEBUG", res );
    cv::waitKey(5000);
  }
}

void ImageRecognize::showImage(cv::Mat& src,char* title){
  if (showWindow){
    cv::Mat rotated=src.clone();

    int x=src.cols /3;
    int y=src.rows /3;

    if ( src.cols < src.rows ){
      rotate(src, -90, rotated);
    }

    cv::Mat res = cv::Mat(x, y, CV_32FC3);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);


    cv::namedWindow(title, CV_WINDOW_AUTOSIZE );
    cv::imshow(title, res );
    cv::waitKey(5000);
  }
}

void ImageRecognize::open(const char* filename){
  showWindow=false;
  fileName = filename;
  orignalImage = cv::imread( filename, 1 );
  tess=new tesseract::TessBaseAPI();
  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);

  //std::cout << "######################" << std::endl;
  //std::cout << filename << std::endl;
  //std::cout << "image readed " << (orignalImage.rows/2) << "* " << orignalImage.cols << 'x' << orignalImage.rows << std::endl;
  oneCM = orignalImage.cols/28;
  cv::Mat mat(  orignalImage.rows*2,orignalImage.cols*2, orignalImage.type(), cv::Scalar(0));
  cv::Rect roi( cv::Point( orignalImage.cols/2, (orignalImage.rows/2) ), orignalImage.size() );

  orignalImage.copyTo( mat( roi ) );
  //std::cout << "image relocated" << std::endl;

  cv::Mat largest = largestContour(mat);
  //std::cout << "image largestContour" << std::endl;

  bcResult bcRes = barcode(largest);
  //cv::Point bc_point = barcode(largest);
  cv::Point bc_point=bcRes.point;

  if (bc_point.y>(largest.rows/2)){
    rotateX(largest,180,cv::Point(largest.cols/2,largest.rows/2));
  }

  const char* out = text(largest);

  std::string s (out);
  std::smatch m;
  std::regex e ("[0-9][0-9][0-9][0-9][0-9]");   // matches words beginning by "sub"
  /*
  if (std::regex_search (s,m,e)){
    std::cout << out << std::endl;
  }else{
    std::cout << "don't found PLZ" << std::endl;
  }
  */
  std::string output = "";
  std::replace( s.begin(), s.end(), '"', ' ');
  output = "{ \"code\": \""+bcRes.code+"\", \"text\": \""+s+"\" }";
  std::cout << output << std::endl;
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
  cv::Scalar color( 255,255,255);

  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour passed in seconds: " << te << std::endl;

  std::vector< std::vector<cv::Point> > contours; // Vector for storing contour
  std::vector<cv::Vec4i> hierarchy;
  double angle;
  cv::findContours( thr, contours, hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour passed in seconds: " << te << std::endl;

  for( int i = 0; i< contours.size(); i++ ) // iterate through each contour.
  {
    double a=cv::contourArea( contours[i],false);  //  Find the area of contour
    if(a>largest_area){
      largest_area=a;
      largest_contour_index=i;                //Store the index of largest contour
      bounding_rect=cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
    }

  }
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour passed in seconds: " << te << std::endl;

  angle = getOrientation(contours[largest_contour_index], minmat);
  //std::cout << "largest angle" << angle << std::endl;
  cv::drawContours( dst, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.
  //rectangle(src, bounding_rect,  Scalar(0,255,0),1, 8,0);
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "largestContour passed in seconds: " << te << std::endl;

  //cv::bitwise_not(img, img);


  std::vector<cv::Point> points;
  cv::Mat_<uchar>::iterator it = dst.begin<uchar>();
  cv::Mat_<uchar>::iterator end = dst.end<uchar>();
  for (; it != end; ++it)
    if (*it)
      points.push_back(it.pos());

  cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
  //std::cout << "box angle" << box.angle << std::endl;
  cv::Size box_size = box.size;
  if (box_size.width<box_size.height){
    box.angle += 90.;
  }

  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
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
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
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
  //std::cout << "largestContour passed in seconds: " << te << std::endl;
  //showImage(result, "LC result");
  return result;
}


double ImageRecognize::getOrientation(std::vector<cv::Point> &pts, cv::Mat &img)
{
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


bool ImageRecognize::is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}

bcResult ImageRecognize::barcode(cv::Mat& im){
  bcResult res = {cv::Point(0,0),std::string(""),std::string("")};

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

  for (int thres=45;((thres<120)&&(found==false));thres+=10){
    cv::cvtColor(im, gray, CV_BGR2GRAY);
    cv::threshold(gray,gray,thres,150, CV_THRESH_BINARY);

    cv::normalize(gray, norm, min, max, type, dtype, mask);
    cv::GaussianBlur(norm, norm, ksize, 0);


    zbar::ImageScanner scanner;
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
    scanner.set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ADD_CHECK, 1);
    zbar::Image image(norm.cols, norm.rows, "Y800", (uchar *)norm.data, norm.cols * norm.rows);

    int n = scanner.scan(image);
    for(zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
      //std::cout << "thres " << thres << " Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
      found = true;

      std::string code = std::string(symbol->get_data().c_str());

      if ((code.length() > res.code.length())&&(is_digits(code))){
        res.code = std::string(symbol->get_data().c_str());
        res.type = std::string(symbol->get_type_name().c_str());
        int loc_size = symbol->get_location_size();
        for(int i=0;i<loc_size;i++){
          tmp = (symbol->get_location_y(i)*100/gray.rows);
          if (rel<tmp){
            rel=tmp;
            //point=cv::Point(symbol->get_location_x(i),symbol->get_location_y(i));
            res.point = cv::Point(symbol->get_location_x(i),symbol->get_location_y(i));
          }
        }
      }
    }
    image.set_data(NULL, 0);
    scanner.recycle_image(image);
  }
  if (res.type=="I2/5"){
    //std::cout << res.code << " to ";
    res.code = res.code.substr(0,res.code.length()-1);
    //std::cout << res.code << std::endl;
  }
  return res;
}

cv::Rect ImageRecognize::fittingROI(int x,int y,int w,int h, cv::Mat& m1){
  int rX=x*oneCM;
  int rY=y*oneCM;
  int rW=w*oneCM;
  int rH=h*oneCM;
  if (rX+rW > m1.cols){
    rX -= (rX+rW)-m1.rows;
    if (rX<0){
      rX=0;
      rW -= (rX+rW)-m1.rows;
    }
  }
  if (rY+rH > m1.rows){
    rY -= (rY+rH)-m1.rows;
    if (rY<0){
      rY=0;
      rH -= (rY+rH)-m1.rows;
    }
  }
  return cv::Rect(rX,rY,rW,rH);
}

const char* ImageRecognize::text(cv::Mat& im){
  //cv::Rect roi2( (int)3*oneCM, (int)5*oneCM, (int)12*oneCM, (int)8.5*oneCM);
  cv::Rect roi2 = fittingROI(2,5,12,9,im);
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
  --------------------------------------
  */
  int breite = im.cols/oneCM;
  int hoehe = im.rows/oneCM;
  //std::cout << "width: " << im.cols / oneCM << std::endl;
  //std::cout << "height: " << im.rows / oneCM << std::endl;
  float ratio = ( ((im.rows *1.0) / (im.cols *1.0 )) );
  //std::cout << "ratio: " << ratio  << std::endl;

  int letterType = 0;
  if (hoehe < 15){
    if (breite > 20){
      letterType=1;
    }
  }

  if (hoehe > 15){
    if (breite > 20){
      letterType=2;
    }
  }

  if (letterType==1){
    cv::Mat c2 = im(roi2);
    linearize(c2);
    showImage(c2);

    tess->SetImage((uchar*)c2.data, c2.size().width, c2.size().height, c2.channels(), c2.step1());

    tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
    tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");

    tess->Recognize(0);
    out = tess->GetUTF8Text();

    std::string s (out);
    std::smatch m;
    std::regex e ("[0-9][0-9][0-9][0-9][0-9]");   // matches words beginning by "sub"
    if (std::regex_search (s,m,e)){
      //std::cout << "found PLZ" << std::endl;
    }else{
      cv::Rect roi3 = fittingROI(11,5,12,9,im);
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
      cv::Mat c3 = im(roi3);
      linearize(c3);
      //this->showWindow =true;
      showImage(c3);
      tess->SetImage((uchar*)c3.data, c3.size().width, c3.size().height, c3.channels(), c3.step1());
      tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
      tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
      tess->Recognize(0);
      out = tess->GetUTF8Text();
    }
  }else if (letterType==2){
    rotateX(im,90,cv::Point(im.cols/2,im.rows/2));
    linearize(im);
    tess->SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), im.step1());
    tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
    tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
    tess->Recognize(0);
    out = tess->GetUTF8Text();
  }else{
    linearize(im);
    tess->SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), im.step1());
    tess->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
    tess->SetVariable("tessedit_reject_bad_qual_wds","TRUE");
    tess->Recognize(0);
    out = tess->GetUTF8Text();
  }
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

    for(int i=40;i<histSize;i++){
      if (b_hist.at<float>(i)>avg*0.5){
        if (min==0){
          min=i;
          break;
        }
      }
    }
    for(int i=255;i>41;i--){
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
    cv::threshold(src,src,min-10,max+10, CV_THRESH_BINARY);
}
