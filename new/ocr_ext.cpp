
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>


#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>

#include "ImageRecognizeEx.h"


ImageRecognizeEx* ocr_ext(
  MYSQL* con,
  std::string std_str_machine,
  const char* str_db_host,
  const char* str_db_user,
  const char* str_db_name,
  const char* str_db_password,
  const char* str_db_encoding,
  int blockSize,
  int subtractMean,
  bool debug,
  bool debugtime,
  bool debugwindow,
  bool calculateMean,
  std::string saveRescaledAs
){


    con = mysql_init(NULL);

    mysql_options(con, MYSQL_SET_CHARSET_NAME, str_db_encoding);
    mysql_options(con, MYSQL_INIT_COMMAND, "SET NAMES utf8");

    if (con == NULL){
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
    }


    if (mysql_real_connect(con, str_db_host, str_db_user, str_db_password, str_db_name, 0, NULL, 0) == NULL){
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }

    if (debug){
      std::cout << "DB-Connection: " << "" << std::endl;
      std::cout << "  " << "Host: " << str_db_host << std::endl;
      std::cout << "  " << "DB: " << str_db_name << std::endl;
      std::cout << "  " << "User: " << str_db_user << std::endl;
    }


    ImageRecognizeEx* ir = new ImageRecognizeEx();
    ir->setDebug(debug);
    ir->setDebugWindow(debugwindow);
    ir->setDebugTime(debugtime);
    ir->setWait(1500);
    ir->setBlockSize(blockSize);
    ir->setSubtractMean(subtractMean);
    ir->setMachine(std_str_machine);
    ir->setDBConnection(con);
    ir->setCalcMean(calculateMean);


    if (saveRescaledAs.length()>0){
      ir->saveRescaledOriginal(saveRescaledAs);
    }



    return ir;
}
