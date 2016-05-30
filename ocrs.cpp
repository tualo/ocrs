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
int main( int argc, char** argv )
{
  const char* env_pza = std::getenv("USEPZA");
  std::cout << "env_pza" << env_pza << std::endl;



  std::string imagepath = "/imagedata/";

  if(const char* env_path = std::getenv("IMAGEPATH")){
    imagepath = std::string(env_path);
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
  if(const char* env_password = std::getenv("DB_PASWORD")){
    db_password = env_password;
  }
  const char* db_name = "sorter";
  if(const char* env_dbname = std::getenv("DB_NAME")){
    db_name = env_dbname;
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
    ir->cmWidth = 28;
    ir->scale = 1.38;
    ir->open(argv[1]);
  }

  std::string fullname(argv[1]);
  boost::filesystem::path bpath(argv[1]);
  std::string fname = bpath.filename().c_str();
  std::string kundenid = "";

  std::vector<int> params;
  params.push_back(CV_IMWRITE_JPEG_QUALITY);
  params.push_back(10);

  //params.push_back(CV_IMWRITE_JPEG_OPTIMIZE);
  //params.push_back(1);

  std::vector<std::string> strs;
  boost::split(strs,fname,boost::is_any_of("N"));
  if (strs.size()==2){
    kundenid=strs[0];
    fname=strs[1];
  }

  if (ir->code.length()>4){

    std::string sg = "";
    std::string sf = "NA";
    std::string mandant = "0000";
    std::string modell = "OCR Erfassung";
    std::string login = "sorter";

    std::string strasse = "";
    std::string hausnummer = "";
    std::string plz = "";
    std::string ort = "";


    if (ir->addresstext.length()>0){
      ExtractAddress* ea = new ExtractAddress();
      ea->setString(ir->addresstext);
      ea->extract();

      sql = "insert into ocrs (code,ocrtext,street,housenumber,zipcode,town) values ('"+ir->code+"','"+ir->addresstext+"','"+ea->getStreetName()+"','"+ea->getHouseNumber()+"','"+ea->getZipCode()+"','"+ea->getTown()+"') on duplicate key update ocrtext=values(ocrtext),town=values(town),street=values(street),housenumber=values(housenumber),zipcode=values(zipcode), processed=0 ";
      if (mysql_query(con, sql.c_str())){
          //fprintf(stderr, "%s\n", mysql_error(con));
          //mysql_close(con);
          //exit(1);
      }

      std::string fuzzysql = "select GET_SORTBOX('"+ea->getStreetName()+" "+ea->getZipCode()+" "+ea->getTown()+"','"+ea->getZipCode()+"','"+ea->getHouseNumber()+"','"+kundenid+"') res";
      std::cout << "fuzzysql " << fuzzysql << std::endl;
      if (mysql_query(con, fuzzysql.c_str())){
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
      }


      sf = "NT";
      mandant = "0000";
      modell = "OCR Erfassung";
      login = "sorter";

      strasse = ea->getStreetName();
      hausnummer = ea->getHouseNumber();
      plz = ea->getZipCode();
      ort = ea->getTown();

      MYSQL_RES *result;
      MYSQL_ROW row;
      unsigned int num_fields;
      unsigned int i;
      result = mysql_use_result(con);
      num_fields = mysql_num_fields(result);
      while ((row = mysql_fetch_row(result))){
         unsigned long *lengths;
         lengths = mysql_fetch_lengths(result);
         for(i = 0; i < num_fields; i++)
         {
           if (i==0){
             std::string box = row[i];
             std::vector<std::string> elems = osplit(box,'|');
             if (elems.size()==2){
               sg = elems.at(0);
               sf = elems.at(1);
             }else{
               sf = box;
             }
           }
         }
      }
      std::cout << "sf " << sf << " sg " << sg << std::endl;
      std::string newfile = imagepath+"result/good."+ir->code+".jpg";
      cv::imwrite(newfile.c_str(),ir->resultMat,params);

    }else{
      // there is no adresstext
      std::string newfile = imagepath+"result/noaddress."+ir->code+".jpg";
      cv::imwrite(newfile.c_str(),ir->resultMat,params);
    }


    sql = "insert into sv_daten (mandant,modell,id,datum,zeit,login,sortiergang,sortierfach,strasse,hausnummer,plz,ort,width,height) ";
    sql = sql + " values ('"+mandant+"','"+modell+"','"+ir->code+"',now(),now(),'"+login+"','"+sg+"','"+sf+"','"+strasse+"','"+hausnummer+"','"+plz+"','"+ort+"','"+boost::lexical_cast<std::string>(ir->width)+"','"+boost::lexical_cast<std::string>(ir->height)+"') ";
    sql = sql + " on duplicate key update id=values(id),mandant=values(mandant),modell=values(modell),datum=values(datum),zeit=values(zeit),login=values(login),sortiergang=values(sortiergang),sortierfach=values(sortierfach),strasse=values(strasse),hausnummer=values(hausnummer),plz=values(plz),ort=values(ort),width=values(width),height=values(height)";
    if (mysql_query(con, sql.c_str())){
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }

  }else{
    // ok there is no barcode
    // move that file
    std::string newfile = imagepath+"result/nocode."+fname+".jpg";
    cv::imwrite(newfile.c_str(),ir->orignalImage,params);

  }



  mysql_close(con);
  exit(0);

  return -1;
}
