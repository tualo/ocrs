#include "ImageRecognizeEx.h"
#include "recognize/barcode.cpp"
#include "recognize/imageoperations.cpp"
#include "recognize/text.cpp"

void ImageRecognizeEx::showImage(cv::Mat& src){
  if (showDebugWindow){
    cv::Mat rotated=src.clone();

    int x=src.cols /5;
    int y=src.rows /5;
    if ((x>0) && (y>0)){
      cv::Mat res = cv::Mat(x, y, CV_8UC1);
      cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);
      cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
      cv::imshow("DEBUG", res );
      cv::waitKey(windowWait);
    }else{
      std::cerr << "ImageRecognizeEx::showImage can't show image" << std::endl;
    }
  }
}

void ImageRecognizeEx::showImage(cv::Mat& src,int ww){
  if (showDebugWindow){
    cv::Mat rotated=src.clone();
    int x=src.cols /5;
    int y=src.rows /5;
    cv::Mat res = cv::Mat(x, y, CV_8UC1);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);
    cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
    cv::imshow("DEBUG", res );
    cv::waitKey(ww);
  }
}

void ImageRecognizeEx::setMachine(std::string val){
  machine = val;
}

void ImageRecognizeEx::setDBConnection(MYSQL *connection){
  con = connection;
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


void ImageRecognizeEx::_debugTime(std::string str){
  if (showDebugTime){
    double time_since_last = ((double)cv::getTickCount() - debug_last_time)/cv::getTickFrequency();
    std::cout << "(ImageRecognizeEx::_debugTime)\t" << str << ": " << time_since_last << "s " << std::endl;
  }
  debug_last_time = (double)cv::getTickCount();
}


void ImageRecognizeEx::setSubtractMean(int value){
  subtractMean =value;
}


void ImageRecognizeEx::setBlockSize(int value){
  blockSize=value;
}

void ImageRecognizeEx::setCalcMean(bool value){
  calcMean=value;
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
  //cvtColor(in_mat,in_mat,CV_GRAY2BGR);
	return true;
}

void ImageRecognizeEx::setImage(cv::Mat mat){
  orignalImage = mat.clone();
  roiImage=orignalImage.clone();
  showImage(orignalImage);

}
void ImageRecognizeEx::open(const char* filename){
  fileName = filename;
  cv::setUseOptimized(true);
  cv::Mat mat = cv::imread( filename, cv::IMREAD_GRAYSCALE );
  setImage(mat);

  showImage(orignalImage);

}
void ImageRecognizeEx::setPixelPerCM(int _x_cm,int _y_cm){
  x_cm=_x_cm;
  y_cm=_y_cm;

  oneCM = x_cm;
}

void ImageRecognizeEx::rescale(){
  double rescale_cols=1;
  double rescale_rows=1;
  if (x_cm!=y_cm){
    // only rescale if it's nessesary
    rescale_rows=(double)x_cm/(double)y_cm;
    if (orignalImage.channels()>1){
      cv::Mat result = cv::Mat(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows, CV_32FC3);
      cv::resize(orignalImage, result, cv::Size(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows), 0, 0, 3);
      orignalImage=result;
    }else{
      cv::Mat result = cv::Mat(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows, CV_8UC1);
      cv::resize(orignalImage, result, cv::Size(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows), 0, 0, 3);
      orignalImage=result;
    }
    showImage(orignalImage);
  }
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

void ImageRecognizeEx::setMeanFactor(float value){
  meanfactor=value;
}

ImageRecognizeEx::ImageRecognizeEx() :
  showDebug(false),
  showDebugWindow(false),
  psmAuto(false),
  barcodeClahe(false),
  barcodeFP(true),
  windowWait(50),
  debug_last_time((double)cv::getTickCount()) {


  meanfactor=1;

  bSaveRescaledOriginal=false;
  code="";
  codes="";
  addressfield = "L";

  extractAddress = new ExtractAddress();


  tess=new tesseract::TessBaseAPI();
  if (psmAuto){
    //tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
    tesseract::PageSegMode pagesegmode = tesseract::PSM_SINGLE_BLOCK;
    tess->SetPageSegMode(pagesegmode);
  }
//  tess->Init(NULL, (char*)"deu", tesseract::OEM_DEFAULT);
tess->Init(NULL, (char*)"deu", tesseract::OEM_TESSERACT_ONLY);
//tess->Init(NULL, (char*)"deu", static_cast<tesseract::OcrEngineMode>(1));//tesseract::OEM_LSTM_ONLY);


//  tesseract::OcrEngineMode enginemode = tesseract::OEM_DEFAULT;
}

void ImageRecognizeEx::correctSize(){
  MYSQL_RES *result;
  MYSQL_ROW row;
  unsigned int num_fields;
//  unsigned int i;

  float result_cols = 0;
  float result_rows = 0;
  double rescale_cols = 1;
  double rescale_rows = 1;


  std::string sql = "select height/100,length/100,addressfield,job_id from bbs_data where id = '"+code+"'; ";
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }else{

    result = mysql_use_result(con);
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))){
       //unsigned long *lengths;
       result_cols = atof(row[0]);
       result_rows = atof(row[1]);
       addressfield = row[2];
       if (showDebug){
         std::cout << "result_cols "  << result_cols << std::endl;
         std::cout << "result_rows "  << result_rows << std::endl;
        }
       // length is messured exact by the fp machine
       // the height is fixed by the camera

       rescale_rows = result_rows*oneCM / ((double)orignalImage.rows)*1.0;
       if (showDebug){
         std::cout << "rescale_rows "  << rescale_rows << " --- " << orignalImage.rows*rescale_rows << std::endl;
       }

       cv::resize(orignalImage, orignalImage, cv::Size(orignalImage.cols*rescale_cols, orignalImage.rows*rescale_rows), 0, 0, 3);
       cv::resize(roiImage, roiImage, cv::Size(roiImage.cols*rescale_cols, roiImage.rows*rescale_rows), 0, 0, 3);
       showImage(roiImage);


    }
    mysql_free_result(result);
  }

  if (bSaveRescaledOriginal==true){
    cv::imwrite(sSaveRescaledOriginal.c_str(),orignalImage);
  }

}

void ImageRecognizeEx::saveRescaledOriginal(std::string filename){
  bSaveRescaledOriginal=true;
  sSaveRescaledOriginal=filename;
}

ImageRecognizeEx::~ImageRecognizeEx() {
  tess->End();
}




void ImageRecognizeEx::initZipCodeMap(){
  std::string sql = "select bereiche_plz.plz, sortiergaenge_zuordnung.sortiergang, sortiergaenge_zuordnung.sortierfach, sortiergaenge_zuordnung.regiogruppe from sortiergaenge_zuordnung join bereiche_plz on sortiergaenge_zuordnung.bereich = bereiche_plz.name and sortiergaenge_zuordnung.regiogruppe = bereiche_plz.regiogruppe having sortiergaenge_zuordnung.regiogruppe='Zustellung' ";
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }else{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int num_fields;



    result = mysql_use_result(con);
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))){
       //unsigned long *lengths;
       extractAddress->setZipcodeHash(std::string( row[0] ),std::string( row[1] ),std::string( row[2] ));
    }
    mysql_free_result(result);
  }
}
