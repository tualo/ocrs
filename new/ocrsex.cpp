

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>


#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>

#include "args.hxx"

#include "ImageRecognizeEx.h"

double debug_start_time = (double)cv::getTickCount();
double debug_last_time = (double)cv::getTickCount();
double debug_window_offset = 0;

bool bDebugTime=false;
void debugTime(std::string str){
  if (bDebugTime){
    double time_since_start = ((double)cv::getTickCount() - debug_start_time)/cv::getTickFrequency();
    double time_since_last = ((double)cv::getTickCount() - debug_last_time)/cv::getTickFrequency();
    std::cout << str << ": " << time_since_last << "s " << "(total: " << time_since_start  << "s)" << std::endl;
  }
  debug_last_time = (double)cv::getTickCount();
}
#include "ocr_ext.cpp"




int main( int argc, char** argv ){
  args::ArgumentParser parser("Ocrs reconize barcodes and text.", "Take care depending on speed Pixel per CM Y can vary");
  args::HelpFlag help(parser, "help", "Display this help menu", { "help"});
  args::Flag debug(parser, "debug", "Show debug messages", {'d', "debug"});
  args::Flag debugwindow(parser, "debugwindow", "Show debug window", {'w', "debugwindow"});
  args::Flag debugtime(parser, "debugtime", "Show times", {'t', "debugtime"});

  args::Flag calculateMean(parser, "calculatemean", "calculatemean for adaptiveThreshold", {"calculatemean"});

  args::ValueFlag<std::string> filename(parser, "filename", "The filename", {'f',"file"});

  args::ValueFlag<std::string> db_host(parser, "host", "The database server host", {'h',"host"});
  args::ValueFlag<std::string> db_name(parser, "name", "The database name", {'n',"name"});
  /*
  currently not supported
  args::ValueFlag<int> db_port(parser, "port", "The database server port", {'p',"port"});
  */
  args::ValueFlag<std::string> db_user(parser, "user", "The database server username", {'u',"user"});
  args::ValueFlag<std::string> db_pass(parser, "password", "The database server password", {'x',"password"});
  args::ValueFlag<std::string> db_encoding(parser, "encoding", "The database server encoding", {'e',"encoding"});


  args::ValueFlag<int> pixel_cm_x(parser, "cm_x", "Pixel per CM X (default 73)", {"cmx"});
  args::ValueFlag<int> pixel_cm_y(parser, "cm_y", "Pixel per CM Y (default 73)", {"cmy"});
  args::ValueFlag<int> blocksize(parser, "blocksize", "adaptiveThreshold Blocksize (default 55)", {"blocksize"});
  args::ValueFlag<int> substractmean(parser, "substractmean", "adaptiveThreshold subtractMean (default 20)", {"substractmean"});
  args::ValueFlag<float> meanfactor(parser, "meanfactor", "multibly calculated mean with (default 1.0)", {"meanfactor"});



  args::ValueFlag<std::string> machine(parser, "machine", "The machine ID (default 00)", {"machine"});
  args::ValueFlag<std::string> rescaledfilename(parser, "rescaledfilename", "Save the rescaled original image as", {"rescaledfilename"});



  try
  {
      parser.ParseCLI(argc, argv);
      if (db_host==0){
        std::cout << parser;
        return 0;
      }
      if (db_name==0){
        std::cout << parser;
        return 0;
      }
      if (db_user==0){
        std::cout << parser;
        return 0;
      }
      if (filename==0){
        std::cout << parser;
        return 0;
      }
  }
  catch (args::Help)
  {

      std::cout << parser;
      return 0;
  }
  catch (args::ParseError e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  if (debug){
    std::cout << "processing image: " << args::get(filename) << std::endl;
  }


  bDebugTime = (debugtime==1);
  debugTime("Start");


  std::string std_str_machine="00";
  std::string std_str_rescaledfilename="";

  const char* str_db_host = "localhost";
  const char* str_db_user = "root";
  const char* str_db_name = "sorter";
  const char* str_db_password = "";
  const char* str_db_encoding = "utf8";

  int int_pixel_cm_x = 73;
  int int_pixel_cm_y = 73;
  int blockSize=55;
  int subtractMean=20;
  float fmeanfactor=1;


  if (blocksize) { blockSize = args::get(blocksize); }
  if (substractmean) { subtractMean = args::get(substractmean); }
  if (pixel_cm_x) { int_pixel_cm_x = args::get(pixel_cm_x); }
  if (pixel_cm_y) { int_pixel_cm_y = args::get(pixel_cm_y); }

  if (db_encoding){ str_db_encoding= (args::get(db_encoding)).c_str(); }
  if (db_host){ str_db_host= (args::get(db_host)).c_str(); }
  if (db_user){ str_db_user= (args::get(db_user)).c_str(); }
  if (db_name){ str_db_name= (args::get(db_name)).c_str(); }
  if (db_pass){ str_db_password= (args::get(db_pass)).c_str(); }
  if (machine){ std_str_machine=args::get(machine); }
  if (rescaledfilename){ std_str_rescaledfilename=args::get(rescaledfilename); }

  if (meanfactor){ fmeanfactor=args::get(meanfactor); }


  MYSQL *con = mysql_init(NULL);
//str_db_encoding.c_str()
  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");
  mysql_options(con, MYSQL_INIT_COMMAND, "SET NAMES utf8");

  if (con == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  if (mysql_real_connect(con, str_db_host,str_db_user, str_db_password,   str_db_name, 0, NULL, 0) == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
  }

  ImageRecognizeEx* ir=ocr_ext(
    con,
    std_str_machine,
    blockSize,
    subtractMean,
    debug==1,
    debugtime==1,
    debugwindow==1,
    calculateMean==1,
    std_str_rescaledfilename,
    fmeanfactor
  );
  try{


    // kundennummer, kostenstelle aus dem dateinamen ermitteln
    std::string kundenid = "";
    std::vector<std::string> strs;
    boost::split(strs,args::get(filename),boost::is_any_of("N"));
    if (strs.size()==2){
      kundenid=strs[0];
    }
    std::vector<std::string> kstrs;
    boost::split(kstrs,kundenid,boost::is_any_of("|"));
    if (kstrs.size()==2){
      ir->setKundennummer(kstrs[0]);
      ir->setKostenstelle(kstrs[1]);
    }
    // -- kundennummer, kostenstelle aus dem dateinamen ermitteln

    ir->setPixelPerCM(int_pixel_cm_x,int_pixel_cm_y);
    ir->open((args::get(filename)).c_str());
    debugTime("after open");
    //only rescale height if param is set up
    if (pixel_cm_y){
      ir->rescale();
      debugTime("after rescale");
    }
    ir->barcode();
    debugTime("after barcode");

    ir->correctSize();
    debugTime("after correctSize");

    ir->largestContour(false);
    debugTime("after largestContour");

    ir->texts();
    debugTime("after text");
  }catch(cv::Exception cv_error){
    std::cerr << "***" << cv_error.msg << std::endl;
  }
  mysql_close(con);
  delete ir;

  debugTime("Stop");

  return -1;
}
