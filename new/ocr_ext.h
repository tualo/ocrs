
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
  std::string saveRescaledAs,
  float fmeanfactor
);

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
  std::string saveRescaledAs,
  float fmeanfactor,
  bool clahe,
  int bc_thres_start,
  int bc_thres_stop,
  int bc_thres_step,
  bool forceFPCode
);
