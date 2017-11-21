#include "ImageRecognizeEx.h"
#include "recognize/barcode.cpp"
#include "recognize/imageoperations.cpp"
#include "recognize/text.cpp"
#include "recognize/statistics.cpp"
#include "recognize/unpaper.cpp"

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
void ImageRecognizeEx::setZipcodeRegexText(std::string str){
  zipcodeRegexText = str;
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
  oImage = mat.clone();
  roiImage=orignalImage.clone();

  showImage(orignalImage);

}
void ImageRecognizeEx::open(const char* filename){
  fileName = filename;
  std::cout << "opening file \"" << filename << "\"." << std::endl;
  try{
    cv::setUseOptimized(true);
    cv::Mat mat = cv::imread( filename, cv::IMREAD_GRAYSCALE );
    setImage(mat);
    showImage(orignalImage);
  } catch (cv::Exception& e) {
      std::cerr << "Error opening file \"" << filename << "\". Reason: " << e.msg << std::endl;
      exit(1);
  }
}
void ImageRecognizeEx::setPixelPerCM(int _x_cm,int _y_cm){
  x_cm=_x_cm;
  y_cm=_y_cm;
  std::cout << "set x_cm \"" << x_cm << "\"." << std::endl;
  std::cout << "set y_cm \"" << y_cm << "\"." << std::endl;

  oneCM = x_cm;
}

void ImageRecognizeEx::rescale(){
  _debugTime("start rescale");
  rescale_cols=1;
  rescale_rows=1;
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
  _debugTime("stop rescale");
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
void ImageRecognizeEx::setKundennummer(std::string value){
  kundennummer=value;
}
void ImageRecognizeEx::setKostenstelle(std::string value){
  kostenstelle=value;
}

ImageRecognizeEx::ImageRecognizeEx() :
  showDebug(false),
  showDebugWindow(false),
  psmAuto(false),
  barcodeClahe(false),
  barcodeFP(false),
  windowWait(50),
  debug_last_time((double)cv::getTickCount()) {

  kundennummer="0";
  kostenstelle="0";
  i_bc_thres_start=15;
  i_bc_thres_stop=180;
  i_bc_thres_step=5;

  zipcodeRegexText = "(O|7|I|i|Q|\\d){5}\\s";

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
         std::cout << "oneCM "  << oneCM << std::endl;
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
    for(; mysql_next_result(con) == 0;) /* do nothing */;
    mysql_free_result(result);
  }

  if (bSaveRescaledOriginal==true){
    cv::imwrite(sSaveRescaledOriginal.c_str(),orignalImage);
  }

}

void ImageRecognizeEx::setBarcodeStepSize(int value){ i_bc_thres_step = value; }
void ImageRecognizeEx::setBarcodeStartThreshold(int value){ i_bc_thres_start = value; }
void ImageRecognizeEx::setBarcodeStopThreshold(int value){ i_bc_thres_stop = value; }
void ImageRecognizeEx::setBarcodeClahe(bool value){ barcodeClahe = value; }
void ImageRecognizeEx::setBarcodeFP(bool value){ barcodeFP = value; }


void ImageRecognizeEx::saveRescaledOriginal(std::string filename){
  bSaveRescaledOriginal=true;
  sSaveRescaledOriginal=filename;
}

ImageRecognizeEx::~ImageRecognizeEx() {
  tess->End();
}




void ImageRecognizeEx::initZipCodeMap(){
  if (showDebug){
    std::cout << "initZipCodeMap ToDo: check customer for locked zipcodes" << std::endl;
  }
    _debugTime("start initZipCodeMap");
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
    for(; mysql_next_result(con) == 0;) /* do nothing */;
    mysql_free_result(result);
  }
  _debugTime("stop initZipCodeMap");
}



bool ImageRecognizeEx::forceFPNumber(){
  bool res = false;

  std::string fdate = "date_add(now(), interval -2 minute)";
  std::string fdateend = "date_add(now(), interval +2 minute)";

  if(const char* env_fdate = std::getenv("FORCEFPDATE")){
    std::cout << "forceFPNumber FORCEFPDATE" << std::getenv("FORCEFPDATE") <<  std::endl;
    fdate = "'"+std::string(env_fdate)+ " 00:00:01'";
    fdateend = "'"+std::string(env_fdate)+ " 23:59:59'";
  }


  if (kundennummer!=""){
    std::string sql = "select id from  bbs_data where kundennummer = '"+kundennummer+"' and kostenstelle = "+kostenstelle+" and machine_no='"+machine+"0' and createtime>="+fdate+" and createtime<="+fdateend+" order by id desc limit 1";
    if (mysql_query(con, sql.c_str())){

    }else{
     MYSQL_RES *result;
     MYSQL_ROW row;
     unsigned int num_fields;
  //   unsigned int i;
     result = mysql_use_result(con);
     num_fields = mysql_num_fields(result);
     while ((row = mysql_fetch_row(result))){
      std::cout << "forceFPNumber true" << std::endl;
      res=true;
     }
     for(; mysql_next_result(con) == 0;) /* do nothing */;
     mysql_free_result(result);
    }

  }
  return res;

}

void ImageRecognizeEx::updateTimeStatistic(double tm){
  updateStatistics("running",tm);
}
void ImageRecognizeEx::updateResultfilename(std::string res){
  updateStatistics("resultfilename",res);
}

std::string ImageRecognizeEx::getKundennummer(){ return kundennummer; }
std::string ImageRecognizeEx::getKostenstelle(){ return kostenstelle; }
