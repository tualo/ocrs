

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
    subtractMean = (-1*( (int)mean.data[6] - 128 )/2)*1.8; //light up
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
