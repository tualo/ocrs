#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>


#include "ImageRecognize.h"
#include "ExtractAddress.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>

#include "glob.h"

#define NUM_THREADS 100

#define HAVE_UINT64 1
#define HAVE_INT64 1
#define NO_LONGLONG 1

#undef min
#undef max
#undef test

#define HAVE_BOOL 1

#include <mysql.h>
#include <my_global.h>



/*


*/

std::vector<std::string> &osplit(const std::string &s, char delim, std::vector<std::string>  &elems){
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss,item,delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> osplit(const std::string &s,char delim){
  std::vector<std::string> elems;
  osplit(s,delim,elems);
  return elems;
}

struct thread_info {
  int index;
  std::string filename;
};
//typedef struct thread_info thread_info;
struct thread_info threadInfo[NUM_THREADS];// = {0,0,0,0};// = malloc(sizeof(struct thread_block_info));

std::vector<std::string> files;
glob::Glob* gl;

void* processImage(void *data ){
  struct thread_info *tib;
  tib = (struct thread_info *)data;
  std::cout << " inside thread " << tib->filename << std::endl;
  ImageRecognize* ir = new ImageRecognize();
  const char* fn = tib->filename.c_str();
  ir->open( fn );
  pthread_exit((void*)42);
	return 0;
}



/**
 * @function main
 */
int main( int argc, char** argv ){
  /*
  std::string test = "  uressat   - \nHerrn  1\nRein-hard Scherf |\nZur Grünen Schildmühle 2 - \n99084 Erfurt";
  ExtractAddress* eat = new ExtractAddress();
  eat->setString(test);
  eat->extract();
  std::string testsql = "select GET_SORTBOX('"+eat->getStreetName()+" "+eat->getZipCode()+" "+eat->getTown()+"','"+eat->getZipCode()+"','"+eat->getHouseNumber()+"','123') res";
  std::cout << "testsql " << testsql << std::endl;
  exit(1);
  */


  bool usePZA = false;
  if(const char* env_pza = std::getenv("USEPZA")){
    usePZA = (atoi(env_pza)==1)?true:false;
  }

  std::string protokollsql = "";


  std::string imagepath = "/imagedata/";
  if(const char* env_path = std::getenv("IMAGEPATH")){
    imagepath = std::string(env_path);
  }


  std::string imagepath_result = "/imagedata/result/";
  if(const char* env_pathir = std::getenv("IMAGEPATH_RESULT")){
    imagepath_result = std::string(env_pathir);
  }


  std::string force_customer = "";
  if(const char* env_force_customer = std::getenv("FORCECUSTOMER")){
    force_customer = std::string(env_force_customer);
  }

  int keepfiles = 0;
  if(const char* env_keep = std::getenv("KEEPFILES")){
    keepfiles = atoi(env_keep);
  }


  int forceaddress = 0;
  if(const char* env_forceaddress = std::getenv("FORCEADDRESS")){
    forceaddress = atoi(env_forceaddress);
  }

  std::string store_original = "";
  if(const char* env_store_original = std::getenv("STORE_ORIGINAL")){
    store_original = std::string(env_store_original);
  }

  const char* db_host = "localhost";
  if(const char* env_host = std::getenv("DB_HOST")){
    db_host = env_host;
  }

  const char* db_user = "sorter";
  if(const char* env_user = std::getenv("DB_USER")){
    db_user = env_user;
  }
  const char* db_password = "sorter";
  if(const char* env_password = std::getenv("DB_PASSWORD")){
    db_password = env_password;
  }
  const char* db_name = "sorter";
  if(const char* env_dbname = std::getenv("DB_NAME")){
    db_name = env_dbname;
  }

  bool window = false;
  if(const char* env_window = std::getenv("DEBUGWINDOW")){
    window = (atoi(env_window)==1)?true:false;
  }

  bool windowalltogether = false;
  if(const char* env_windowa = std::getenv("DEBUGWINDOW_ALL")){
    windowalltogether = (atoi(env_windowa)==1)?true:false;
  }


  int window_wait = 500;
  if(const char* env_windowwait = std::getenv("DEBUGWINDOWWAIT")){
    window_wait = atoi(env_windowwait);
  }

  bool debug = false;
  if(const char* env_debug = std::getenv("DEBUG")){
    debug = (atoi(env_debug)==1)?true:false;
  }
  bool headOver = false;
  if(const char* env_hover = std::getenv("HEAD_OVER")){
    headOver = (atoi(env_hover)==1)?true:false;
  }
  bool psmAuto = false;
  if(const char* env_psmAuto = std::getenv("PSM_AUTO")){
    psmAuto = (atoi(env_psmAuto)==1)?true:false;
  }
  bool light_up_original = false;
  if(const char* env_light_up_original = std::getenv("LIGHT_UP_ORIGINAL")){
    light_up_original = (atoi(env_light_up_original)==1)?true:false;
  }
  bool try_reduced = false;
  if(const char* env_try_reduced = std::getenv("TRY_REDUCED")){
    try_reduced = (atoi(env_try_reduced)==1)?true:false;
  }

  bool barcode_only = false;
  if(const char* env_try_reduced = std::getenv("BARCODE_ONLY")){
    barcode_only = (atoi(env_try_reduced)==1)?true:false;
  }

  bool rotate_inline = true;
  if(const char* env_rotate_inline = std::getenv("ROTATE_INLINE")){
    rotate_inline = (atoi(env_rotate_inline)==1)?true:false;
  }


  bool barcode_light_correction = true;
  if(const char* env_barcode_light_correction = std::getenv("BC_LIGHT_CORRECTION")){
    barcode_light_correction = (atoi(env_barcode_light_correction)==1)?true:false;
  }

  bool use_ext = false;
  if(const char* env_use_ext = std::getenv("USE_SET_SORTBOX_EXT")){
    use_ext = (atoi(env_use_ext)==1)?true:false;
  }



  int analysisType=1;
  if(const char* env_atype = std::getenv("ANALYSETYPE")){
    analysisType = atoi(env_atype);
  }

  int subtractMean=20;
  if(const char* env_subtractMean = std::getenv("ADAPTIVE_THRESH_SUBTRACT_MEAN")){
    subtractMean = atoi(env_subtractMean);
  }

  int blockSize=55;
  if(const char* env_blockSize = std::getenv("ADAPTIVE_THRESH_BLOCK_SIZE")){
    blockSize = atoi(env_blockSize);
  }
  std::cout << "ADAPTIVE_THRESH_SUBTRACT_MEAN="<<subtractMean << std::endl;
  std::cout << "ADAPTIVE_THRESH_BLOCK_SIZE="<<blockSize << std::endl;
  std::cout << "PSM_AUTO="<<psmAuto << std::endl;

  int barcode_algorthim=0;
  if(const char* env_ba = std::getenv("BARCODE_ALGORTHIM")){
    barcode_algorthim = atoi(env_ba);
  }

  std::string machine_id = "00";
  if(const char* env_machine_id = std::getenv("MACHINEID")){
    machine_id = std::string(env_machine_id);
  }



  const char* scale = "1.38";
  if(const char* env_scale = std::getenv("SCALE")){
    scale = env_scale;
  }

  const char* width = "28";
  if(const char* env_width = std::getenv("WIDTH_CM")){
    width = env_width;
  }


  std::string mandant = "0000";
  std::string modell = "Clearing";
  if(const char* env_mandant = std::getenv("MANDANT")){
    mandant = env_mandant;
  }
  if(const char* env_modell = std::getenv("MODELL")){
    modell = env_modell;
  }

  double t = (double)cv::getTickCount();
  int cthread = 0;

  MYSQL *con = mysql_init(NULL);

  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");
  mysql_options(con, MYSQL_INIT_COMMAND, "SET NAMES utf8");

  if (con == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  if (mysql_real_connect(con, db_host, db_user, db_password, db_name, 0, NULL, 0) == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
  }

  std::string sql = "";

  ImageRecognize* ir = new ImageRecognize();
  ir->debug=debug;
  ir->con = con;
  ir->machine_id = machine_id;
  ir->try_reduced = try_reduced;
  ir->psmAuto = psmAuto;
  ir->light_up_original = light_up_original;
  ir->rotate_inline=rotate_inline;
  ir->showWindow=window;
  ir->forceaddress=forceaddress;
  ir->barcode_light_correction=barcode_light_correction;
  std::cout << "debug window" << window << std::endl;
  ir->analysisType=analysisType;
  ir->headOver = headOver;
  ir->barcode_algorthim = barcode_algorthim;
  ir->window_wait = window_wait;
  ir->windowalltogether = windowalltogether;
  ir->subtractMean=subtractMean;
  ir->blockSize = blockSize;
  ir->barcode_only=barcode_only;



  if (usePZA){
    ir->cmWidth = 21;
    ir->scale = 1;
    ir->openPZA(argv[1]);


  }else{
    ir->cmWidth =  std::atoi(width);
    if (debug){
      std::cout << "WIDTH_CM " << width << std::endl;
      std::cout << "SCALE " << scale << std::endl;
    }
    ir->scale = std::atof(scale);
    ir->open(argv[1]);
  }

  std::vector<int> params;
  params.push_back(CV_IMWRITE_JPEG_QUALITY);
  params.push_back(80);

  if (barcode_only==true){
    std::cout << "barcode " << ir->code << std::endl;
    mysql_close(con);

    if (ir->code.length()>4){
      std::string newfile_bc_only = "barcode."+ir->code+".jpg";
      cv::imwrite(newfile_bc_only.c_str(),ir->orignalImage,params);
    }

    exit(0);
    return 1;
  }

  std::string fullname(argv[1]);
  boost::filesystem::path bpath(argv[1]);
  std::string fname = bpath.filename().c_str();
  std::string kundenid = "";
  std::string product = "";
  std::string bbs_check_sql = "";




  std::vector<std::string> strs;
  boost::split(strs,fname,boost::is_any_of("N"));

  if (strs.size()==2){
    kundenid=strs[0];
    fname=strs[1];
  }

  if (strs.size()==3){
    kundenid=strs[0];
    product=strs[1];
    fname=strs[2];
  }

  if (force_customer.length()>0){
    kundenid = force_customer;
  }




  std::string sql_modell = "set @svmodell='"+modell+"'";
  if (mysql_query(con, sql_modell.c_str())){
  }

  if(const char* env_sessionuser = std::getenv("SESSIONUSER")){
    std::string sessionuser = env_sessionuser;
    std::string sql_sessionuser = "set @sessionuser='"+sessionuser+"'";
    if (mysql_query(con, sql_sessionuser.c_str())){
    }
  }

  if(const char* env_regiogruppe = std::getenv("REGIOGRUPPE")){
    std::string regiogruppe = env_regiogruppe;
    std::string sql_regiogruppe = "set @regiogruppe='"+regiogruppe+"'";
    if (mysql_query(con, sql_regiogruppe.c_str())){
    }
  }

  if(const char* env_use_fast_access_tour = std::getenv("FAST_ACCESS_TOUR")){
    std::string use_fast_access_tour = env_use_fast_access_tour;
    std::string sql_use_fast_access_tour = "set @use_fast_access_tour="+use_fast_access_tour+"";
    if (mysql_query(con, sql_use_fast_access_tour.c_str())){
    }
  }

  if ( (forceaddress==1)||(ir->code.length()>4)){

    if ((forceaddress==1)&&(ir->code.length()<=4)){
      std::string fcode = fname;
      boost::replace_all(fcode, "nocode.", "");
      boost::replace_all(fcode, "noaddress.", "");
      boost::replace_all(fcode, ".tiff", "");
      boost::replace_all(fcode, ".jpg", "");
      ir->code=fcode;
    }

    std::string sg = "";
    std::string sf = "NA";
    std::string login = "sorter";

    std::string strasse = "";
    std::string hausnummer = "";
    std::string plz = "";
    std::string ort = "";



    if (ir->addresstext.length()>0){
      ExtractAddress* ea = new ExtractAddress();
      std::string sql_addresstext = boost::replace_all_copy(ir->addresstext, "'", " ");


      ea->setString(sql_addresstext);
      ea->extract();



      sql = "delete from ocrs where code = '"+ir->code+"'; ";
      if (mysql_query(con, sql.c_str())){

      }


      sql = "insert into ocrs (code,ocrtext,street,housenumber,zipcode,town) values ('"+ir->code+"','"+sql_addresstext+"','"+ea->getStreetName()+"','"+ea->getHouseNumber()+"','"+ea->getZipCode()+"','"+ea->getTown()+"') ";
      if (mysql_query(con, sql.c_str())){

      }

      if (ea->getStreetName().length()>1){


        std::string fuzzysql = "CALL SET_SORTBOX('"
          +sql_addresstext
          +"','"+ea->getZipCode()
          +"','"+ea->getHouseNumber()
          +"','"+kundenid
          +"','"+product
          +"','"+ir->code
          +"', @stortiergang"
          +", @stortierfach"
          +", @strasse"
          +", @plz"
          +", @ort"
          +")";
        if(use_ext==true){
          fuzzysql = "CALL SET_SORTBOX_EXT('"
            +sql_addresstext
            +"','"+ea->getZipCode()
            +"','"+ea->getTown()
            +"','"+ea->getStreetName()
            +"','"+ea->getHouseNumber()
            +"','"+kundenid
            +"','"+product
            +"','"+ir->code
            +"','"+ir->codes
            +"', @stortiergang"
            +", @stortierfach"
            +", @strasse"
            +", @plz"
            +", @ort"
            +")";
        }

        if (debug){
          std::cout << "fuzzysql " << fuzzysql << std::endl;
        }
        if (mysql_query(con, fuzzysql.c_str())){

          fprintf(stderr, "%s\n", mysql_error(con));
          mysql_close(con);
          exit(1);

        }


        std::string resultsql = "SELECT @stortiergang sg,@stortierfach sf,@strasse str,@plz plz,@ort ort";
        if (mysql_query(con, resultsql.c_str())){
          fprintf(stderr, "%s\n", mysql_error(con));
          mysql_close(con);
          exit(1);
        }


        sf = "NT";
        sg = "NT";
        login = "sorter";

        MYSQL_RES *result;
        MYSQL_ROW row;
        unsigned int num_fields;
        unsigned int i;
        result = mysql_use_result(con);
        num_fields = mysql_num_fields(result);
        while ((row = mysql_fetch_row(result))){
           unsigned long *lengths;
           lengths = mysql_fetch_lengths(result);
           //sg = row[0];
           //sf = row[1];
           //std::cout << "sg " << row[0] << ", sf " << row[1] << std::endl;
        }

        if (debug){
          std::cout << "good" << std::endl;
        }
        std::string newfile = imagepath_result+"good."+ir->code+".jpg";
        cv::imwrite(newfile.c_str(),ir->resultMat,params);

        if (store_original!=""){
          cv::imwrite( ( store_original+"good."+ir->code+".jpg" ).c_str(),ir->orignalImage);
        }

        bbs_check_sql = "call BBS_CHECK_OCR_ID('"+machine_id+"','good','','"+ir->code+"')";
        if (mysql_query(con, bbs_check_sql.c_str())){
          fprintf(stderr, "%s\n", mysql_error(con));
        }


        if (keepfiles==0){
          if ( remove( fullname.c_str() ) != 0 ) {
            perror( "Error deleting file" );
            exit(1);
          }
        }

      }else{

        // there is no streetname
        if (debug){
          std::cout << "no address: there is no streetname " << ea->getStreetName() << ": " << ea->getZipCode() << std::endl;
        }
        std::string newfile = imagepath_result+"noaddress."+ir->code+".jpg";
        cv::imwrite(newfile.c_str(),ir->resultMat,params);
        /*
        protokollsql = "update protokoll set state='noaddress' where code = '"+ir->code+"'; ";
        if (mysql_query(con, protokollsql.c_str())){
        }
        */
        bbs_check_sql = "call BBS_CHECK_OCR_ID('"+machine_id+"','noaddress','nostreet','"+ir->code+"')";
        if (mysql_query(con, bbs_check_sql.c_str())){
          fprintf(stderr, "%s\n", mysql_error(con));
        }


        if (store_original!=""){
          cv::imwrite( ( store_original+"noaddress."+ir->code+".jpg" ).c_str(),ir->orignalImage);
        }


        if (keepfiles==0){
          if( remove( fullname.c_str() ) != 0 ){
            perror( "Error deleting file" );
            exit(1);
          }
        }

      }

    }else{
      // there is no adresstext
      if (debug){
        std::cout << "no address *" << std::endl;
      }
      std::string newfile = imagepath_result+"noaddress."+ir->code+".jpg";
      if (ir->resultMat.cols>100){
        cv::imwrite(newfile.c_str(),ir->resultMat,params);
      }else{
        cv::imwrite( newfile.c_str(),ir->orignalImage,params);
      }

      bbs_check_sql = "call BBS_CHECK_OCR_ID('"+machine_id+"','noaddress','nozipcode','"+ir->code+"')";
      if (mysql_query(con, bbs_check_sql.c_str())){
        fprintf(stderr, "%s\n", mysql_error(con));
      }


      if (store_original!=""){
        cv::imwrite( ( store_original+"noaddress."+ir->code+".jpg" ).c_str(),ir->orignalImage);
      }

      if (keepfiles==0){
        if( remove( fullname.c_str() ) != 0 ){
          perror( "Error deleting file" );
          exit(1);
        }
      }

    }

  }else{
    // ok there is no barcode
    // move that file
    if (debug){
      std::cout << "no code" << std::endl;
    }
    std::string newfile = imagepath_result+"nocode."+fname+".jpg";
    cv::imwrite(newfile.c_str(),ir->orignalImage,params);
    if (store_original!=""){
      cv::imwrite( ( store_original+"nocode."+fname+".jpg" ).c_str(),ir->orignalImage);
    }
    /*
    protokollsql = "update protokoll set state='nocode' where code = '"+ir->code+"'; ";
    if (mysql_query(con, protokollsql.c_str())){

    }
    */
    bbs_check_sql = "call BBS_CHECK_OCR('"+machine_id+"','nocode','')";
    if (mysql_query(con, bbs_check_sql.c_str())){
      fprintf(stderr, "%s\n", mysql_error(con));
    }

    if (keepfiles==0){
      if( remove( fullname.c_str() ) != 0 ) {
        perror( "Error deleting file" );
        exit(1);
      }
    }
  }


  mysql_close(con);
  exit(0);

  return -1;
}
