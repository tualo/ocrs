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

void ImageRecognize::open(char* filename){
  showWindow=false;
  fileName = filename;
  orignalImage = cv::imread( filename, 1 );


  /*
  // reduce time approx 20%
  int x = orignalImage.cols/2;
  int y = orignalImage.rows/2;
  cv::Mat minmat = cv::Mat(x, y, CV_32FC3);
  cv::resize(orignalImage, minmat, cv::Size(x, y), 0, 0, 3);
  orignalImage=minmat.clone();
  */

  oneCM = orignalImage.cols/28;
  cv::Mat largest = largestContour(orignalImage);
  cv::Point bc_point = barcode(largest);
  if (bc_point.y>(largest.rows/2)){
    rotateX(largest,180,cv::Point(largest.cols/2,largest.rows/2));
  }

  const char* out = text(largest);
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
  std::cout << "largestContour passed in seconds: " << te << std::endl;

  std::vector< std::vector<cv::Point> > contours; // Vector for storing contour
  std::vector<cv::Vec4i> hierarchy;
  double angle;
  cv::findContours( thr, contours, hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "largestContour passed in seconds: " << te << std::endl;

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
  std::cout << "largestContour passed in seconds: " << te << std::endl;

  angle = getOrientation(contours[largest_contour_index], minmat);
  std::cout << "largest angle" << angle << std::endl;
  cv::drawContours( dst, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.
  //rectangle(src, bounding_rect,  Scalar(0,255,0),1, 8,0);
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "largestContour passed in seconds: " << te << std::endl;

  //cv::bitwise_not(img, img);


  std::vector<cv::Point> points;
  cv::Mat_<uchar>::iterator it = dst.begin<uchar>();
  cv::Mat_<uchar>::iterator end = dst.end<uchar>();
  for (; it != end; ++it)
    if (*it)
      points.push_back(it.pos());

  cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
  std::cout << "box angle" << box.angle << std::endl;
  cv::Size box_size = box.size;
  if (box_size.width<box_size.height){
    box.angle += 90.;
  }

  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "largestContour passed in seconds: " << te << std::endl;


  cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(box.center.x*2,box.center.y*2), box.angle, 1);

  //cv::Mat rotated;
  cv::Mat rotated(src.rows*1.5,src.cols*1.5,src.type());
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
  std::cout << "largestContour* passed in seconds: " << te << std::endl;

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

  showImage(rotated);
  cv::Rect myROI(bounding_rect.x, bounding_rect.y, bounding_rect.width, bounding_rect.height);
  cv::Mat result = rotated(myROI);
  te = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "largestContour passed in seconds: " << te << std::endl;
  showImage(result);
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




cv::Point ImageRecognize::barcode(cv::Mat& im){
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
      std::cout << "thres" << thres << "Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
      found = true;
      int loc_size = symbol->get_location_size();
      for(int i=0;i<loc_size;i++){
        tmp = (symbol->get_location_y(i)*100/gray.rows);
        if (rel<tmp){
          rel=tmp;
          point=cv::Point(symbol->get_location_x(i),symbol->get_location_y(i));
        }
      }
    }
    image.set_data(NULL, 0);
    scanner.recycle_image(image);
  }
  return point;
}


const char* ImageRecognize::text(cv::Mat& im){
  cv::Rect roi2( (int)3*oneCM, (int)5*oneCM, (int)12*oneCM, (int)8.5*oneCM);
  cv::Mat c2 = im(roi2);
  showImage(c2);

  linearize(c2);

  tesseract::TessBaseAPI tess;
  tess.Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);
  tess.SetImage((uchar*)c2.data, c2.size().width, c2.size().height, c2.channels(), c2.step1());

  tess.SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
  tess.SetVariable("tessedit_reject_bad_qual_wds","TRUE");

  tess.Recognize(0);
  const char* out = tess.GetUTF8Text();

  std::string s (out);
  std::smatch m;
  std::regex e ("[0-9][0-9][0-9][0-9][0-9]");   // matches words beginning by "sub"
  if (std::regex_search (s,m,e)){
    std::cout << "found PLZ";
  }else{
    /*
    cv::Mat large;
    cv::cvtColor(src, large, CV_BGR2GRAY);

    rotate(large,angle_on_max,large);
    rotate(large,angle_on_max,large);
    showImage(large);

    tesseract::TessBaseAPI tess;
    tess.Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);
    tess.SetImage((uchar*)large.data, large.size().width, large.size().height, large.channels(), large.step1());

    tess.SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQSRTUVWXYZabcdefghijklmnopqrstuvwxyzäöüÄÖÜß|/éè -");
    tess.SetVariable("tessedit_reject_bad_qual_wds","TRUE");

    tess.Recognize(0);
    */
  }

  std::cout << "****" << std::endl  << out << "****" << std::endl <<  std::endl;
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
