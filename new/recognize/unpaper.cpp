
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
  cv::adaptiveThreshold(
      src,
      src,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,//blockSize,calcmeanValue(src));/*,
      bs,
      (128-cm[0])/2
  );
  blockSize = bs;
  subtractMean = (128-cm[0])/2;
  //cv::imshow("3. \"adaptiveThreshold\"", c3);


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
