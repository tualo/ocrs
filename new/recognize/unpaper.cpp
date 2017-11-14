
void ImageRecognizeEx::unpaper(cv::Mat& src){


  cv::Scalar cm = cv::mean(src);

  std::cout << "mean "<< cm[0] << std::endl;
  //x_cm
  int bs = x_cm/10 * 2; // 3mm
  if (bs%2==0){
    bs+=1;
  }
  if (bs<3){
    bs=3;
  }
  subtractMean = (128-cm[0])/2;
  if (subtractMean<0){
    subtractMean*=-1;
  }
  cv::Mat test;
  cv::Mat test_inv;
  cv::adaptiveThreshold(
      src,
      test,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,//blockSize,calcmeanValue(src));/*,
      bs,
      subtractMean
  );
  cv::GaussianBlur(test,test,cv::Size(13,13),2,2);
  //cv::imshow("1. \"blur\"", test);

//  cv::threshold(test, test_inv, 150, 255, cv::THRESH_BINARY_INV);
//  cv::imshow("2. \"blur thres\"", test_inv);

  cv::threshold(test, test, 150, 255, cv::THRESH_BINARY);
//  cv::imshow("2. \"blur thres norm\"", test);

  cv::adaptiveThreshold(
      src,
      src,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,//blockSize,calcmeanValue(src));/*,
      bs,
      subtractMean
  );

/*
  cv::bitwise_or(src,test,src);


  cv::Mat x;
  cv::bitwise_xor(src,test_inv,x);
  cv::imshow("xor", x);

  cv::bitwise_not(src,test_inv,x);
  cv::imshow("not", x);


  cv::bitwise_and(src,test_inv,x);
  cv::imshow("and", x);

  cv::bitwise_or(src,test_inv,x);
  cv::imshow("or", x);


  cv::bitwise_xor(src,test,x);
  cv::imshow("xor n", x);

  cv::bitwise_not(src,test,x);
  cv::imshow("not n", x);


  cv::bitwise_and(src,test,x);
  cv::imshow("and n ", x);

  cv::bitwise_or(src,test,x);
  cv::imshow("or n", x);

  cv::waitKey(0);
*/
  cv::bitwise_or(src,test,src);
}


cv::Mat ImageRecognizeEx::sobel(cv::Mat gray){
	cv::Mat edges;

	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	cv::Mat edges_x, edges_y;
	cv::Mat abs_edges_x, abs_edges_y;
	cv::Sobel(gray, edges_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);
	cv::convertScaleAbs( edges_x, abs_edges_x );
	cv::Sobel(gray, edges_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
	cv::convertScaleAbs(edges_y, abs_edges_y);
	cv::addWeighted(abs_edges_x, 0.5, abs_edges_y, 0.5, 0, edges);

	return edges;
}

cv::Mat ImageRecognizeEx::canny(cv::Mat src)
{
	cv::Mat detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 250;
	int highThreshold = 750;
	int kernel_size = 5;
	cv::Canny(src, detected_edges, lowThreshold, highThreshold, kernel_size);

	return detected_edges;
 }
