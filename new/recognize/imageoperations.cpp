

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
  std::cout << "blockSize linearize" << blockSize << std::endl;

  cv::adaptiveThreshold(
      src,
      src,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,
      blockSize,
      subtractMean
  );

  _debugTime("stop linearize");
  showImage(src);
  return 0;
}

void ImageRecognizeEx::rotate(cv::Mat& src, int direction){
  transpose(src, src);
  flip(src, src, direction);
}



void ImageRecognizeEx::largestContour(bool useSlow){
  orignalImage = largestSimpleContour(orignalImage);
  //largestContour(orignalImage);
  //todo resize roi image!!!

}

cv::Mat ImageRecognizeEx::largestSimpleContour(cv::Mat& src){
  _debugTime("start largestSimpleContour");
  int blength = src.cols;
  int i=blength;
  int firstRow = (int)src.rows*0.1;
  int midRow = (int)src.rows*0.5;
  int lastRow = (int)src.rows*0.9;
  double lastAVG = 255;
  double currentAVG = 255;

  for(;i>0;i--){

     currentAVG = (double)src.at<uchar>(firstRow,i)/3 + (double)src.at<uchar>(midRow,i)/3 + (double)src.at<uchar>(lastRow,i)/3;
     if (currentAVG>lastAVG*2){
       break;
     }
     lastAVG=currentAVG;
  }
  std::cout << "largestSimpleContour at i " << i << std::endl;
  cv::Rect myROI(0, 0, i, src.rows);
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