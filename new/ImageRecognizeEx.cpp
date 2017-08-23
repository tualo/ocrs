#include "ImageRecognizeEx.h"
#include "recognize/barcode.cpp"
#include "recognize/imageoperations.cpp"
#include "recognize/text.cpp"

void ImageRecognizeEx::showImage(cv::Mat& src){

  if (showDebugWindow){
    cv::Mat rotated=src.clone();
    int x=src.cols /5;
    int y=src.rows /5;
    cv::Mat res = cv::Mat(x, y, CV_32FC3);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);
    cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
    cv::imshow("DEBUG", res );
    cv::waitKey(windowWait);
  }
}

void ImageRecognizeEx::setMachine(std::string val){
  machine = val;
}

void ImageRecognizeEx::setDBConnection(MYSQL *connection){
  con = connection;
  initRegions();
}

void ImageRecognizeEx::setWait(int val){
  windowWait=val;
}

void ImageRecognizeEx::setDebug(bool val){
  showDebug=val;
}

void ImageRecognizeEx::setDebugWindow(bool val){
  showDebugWindow=val;
}

void ImageRecognizeEx::setDebugTime(bool val){
  showDebugTime=val;
}

void ImageRecognizeEx::setSubtractMean(int value){
  subtractMean =value;
}


void ImageRecognizeEx::setBlockSize(int value){
  blockSize=value;
}

bool ImageRecognizeEx::readMatBinary(std::ifstream& ifs, cv::Mat& in_mat)
{
	if(!ifs.is_open()){
		return false;
	}

	int rows, cols, type;
	ifs.read((char*)(&rows), sizeof(int));
	if(rows==0){
		return true;
	}
	ifs.read((char*)(&cols), sizeof(int));
	ifs.read((char*)(&type), sizeof(int));

	in_mat.release();
	in_mat.create(rows, cols, type);
	ifs.read((char*)(in_mat.data), in_mat.elemSize() * in_mat.total());
  cvtColor(in_mat,in_mat,CV_GRAY2BGR);
	return true;
}

void ImageRecognizeEx::open(const char* filename){
  //double t = (double)cv::getTickCount();
  //double te;
  fileName = filename;

  cv::setUseOptimized(true);
  orignalImage = cv::imread( filename, cv::IMREAD_COLOR );

/*
  std::ifstream is;
  is.open (filename, std::ios::binary );
  readMatBinary(is,orignalImage);
  is.close();
*/
  showImage(orignalImage);

}

void ImageRecognizeEx::rescale(int x_cm,int y_cm){
  double rescale_cols=1;
  double rescale_rows=1;
  if (x_cm!=y_cm){
    // only rescale if it's nessesary
    rescale_rows=(double)x_cm/(double)y_cm;
    if (showDebug){
      std::cout << "rescale_rows: " << rescale_rows << " (" << x_cm << "x"<< y_cm << ")" << '\n';
    }
    cv::Mat result = cv::Mat(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows, CV_32FC3);
    cv::resize(orignalImage, result, cv::Size(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows), 0, 0, 3);
    orignalImage=result;


    showImage(orignalImage);
  }
  oneCM = x_cm;
  roiImage=orignalImage.clone();
}


bool ImageRecognizeEx::is_digits(const std::string &str){
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}


void ImageRecognizeEx::rotateX(cv::Mat& src,float angle,cv::Point center){
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

ImageRecognizeEx::ImageRecognizeEx() :
  showDebug(false),
  showDebugWindow(false),
  psmAuto(false),
  barcodeClahe(true),
  barcodeFP(true),
  windowWait(50) {

  code="";
  codes="";
  extractAddress = new ExtractAddress();
  tess=new tesseract::TessBaseAPI();
  if (psmAuto){
    tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
    tess->SetPageSegMode(pagesegmode);
  }
  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);
}

ImageRecognizeEx::~ImageRecognizeEx() {
  tess->End();
}
