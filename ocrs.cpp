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
#include <my_global.h>
#include <mysql.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>

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

  const char* env_pza = std::getenv("USEPZA");
  std::cout << "env_pza" << env_pza << std::endl;

  std::string imagepath = "/imagedata/";
  if(const char* env_path = std::getenv("IMAGEPATH")){
    imagepath = std::string(env_path);
  }

  int keepfiles = 0;
  if(const char* env_keep = std::getenv("KEEPFILES")){
    keepfiles = atoi(env_keep);
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



  const char* scale = "1.38";
  if(const char* env_scale = std::getenv("SCALE")){
    scale = env_scale;
  }
  const char* width = "28";
  if(const char* env_width = std::getenv("WIDTH_CM")){
    scale = env_width;
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
  if (std::string(env_pza) == "1"){
    ir->cmWidth = 21;
    ir->scale = 1;
    ir->openPZA(argv[1]);
  }else{
    ir->cmWidth =  std::atoi(width);
    ir->scale = std::atof(scale);
    ir->open(argv[1]);
  }

  std::string fullname(argv[1]);
  boost::filesystem::path bpath(argv[1]);
  std::string fname = bpath.filename().c_str();
  std::string kundenid = "";
  std::string product = "";

  std::vector<int> params;
  params.push_back(CV_IMWRITE_JPEG_QUALITY);
  params.push_back(80);

  //params.push_back(CV_IMWRITE_JPEG_OPTIMIZE);
  //params.push_back(1);

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

  if (ir->code.length()>4){

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

      if (ea->getStreetName().length()>3){



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
        std::cout << "fuzzysql " << fuzzysql << std::endl;
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
           sg = row[0];
           sf = row[1];
           std::cout << "sg " << row[0] << ", sf " << row[1] << std::endl;
        }

        std::cout << "good" << std::endl;
        std::string newfile = imagepath+"result/good."+ir->code+".jpg";
        cv::imwrite(newfile.c_str(),ir->resultMat,params);

        if (store_original!=""){
          cv::imwrite( ( store_original+"good."+ir->code+".jpg" ).c_str(),ir->orignalImage);
        }

        if (keepfiles==0){
          if ( remove( fullname.c_str() ) != 0 ) {
            perror( "Error deleting file" );
            exit(1);
          }
        }

      }else{

        // there is no streetname
        std::cout << "no address: " << ea->getStreetName() << ": " << ea->getZipCode() << std::endl;
        std::string newfile = imagepath+"result/noaddress."+ir->code+".jpg";
        cv::imwrite(newfile.c_str(),ir->resultMat,params);
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
      std::cout << "no address *" << std::endl;
      std::string newfile = imagepath+"result/noaddress."+ir->code+".jpg";
      cv::imwrite(newfile.c_str(),ir->resultMat,params);
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
    std::cout << "no code" << std::endl;
    std::string newfile = imagepath+"result/nocode."+fname+".jpg";
    cv::imwrite(newfile.c_str(),ir->orignalImage,params);
    if (store_original!=""){
      cv::imwrite( ( store_original+"nocode."+fname+".jpg" ).c_str(),ir->orignalImage);
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
