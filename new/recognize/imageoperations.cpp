

void ImageRecognizeEx::recalcSubstractMean(cv::Mat m){
  if(showDebug){
    std::cout << "TODO: recalcSubstractMean args" << std::endl;
  }
  if(const char* env_cl = std::getenv("CALC_MEAN")){
    if(atoi(env_cl)==1){

      cv::Mat mean;
      cv::Mat stddev;
      cv::meanStdDev(m,mean,stddev);
      int idx =0;
      int minStd = 255;
      int maxStd = 0;

      if (showDebug){
        for(idx=0;idx<sizeof(mean.data);idx++){
          std::cout << "mean " << (int)mean.data[idx]-128 << std::endl;
        }
      }
      for(idx=0;idx<sizeof(stddev.data);idx++){
        if (showDebug){
          std::cout << "stddev " << (int)stddev.data[idx] << std::endl;
        }
        if (minStd>(int)stddev.data[idx]){
          minStd=(int)stddev.data[idx];
        }
        if (maxStd<(int)stddev.data[idx]){
          maxStd=(int)stddev.data[idx];
        }
      }

      subtractMean = -1*( (int)mean.data[6] - 128 )/2;
      if (showDebug){
        std::cout << "use substract mean " << subtractMean<< std::endl;
      }
    }
  }
}



int ImageRecognizeEx::linearize(cv::Mat& src,float multiply){
    cv::Mat thr(src.rows,src.cols,CV_8UC1);
    cvtColor(src,thr,CV_BGR2GRAY); //Convert to gray
    cv::adaptiveThreshold(
        thr,src,255,
        CV_ADAPTIVE_THRESH_GAUSSIAN_C,
        CV_THRESH_BINARY, blockSize,
        subtractMean);
    showImage(src);
    return 0;
}

void ImageRecognizeEx::rotate(cv::Mat& src, int direction){
  transpose(src, src);
  flip(src, src, direction);
}
