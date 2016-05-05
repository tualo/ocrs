#include "ImageRecognize.h"
#include "ExtractAddress.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>

#include "glob.h"

#define NUM_THREADS 100

#include <my_global.h>
#include <mysql.h>


/* MySQL Connector/C++ specific headers */
/*



CREATE TABLE `strassenverzeichnis` (
  `id` varchar(32) NOT NULL,
  `strasse` varchar(255) NOT NULL,
  `plz` varchar(5) DEFAULT NULL,
  `ort` varchar(255) DEFAULT NULL,
  `amtlicher_gemeindeschluessel` varchar(10) DEFAULT NULL,
  `regionalschluessel` varchar(10) DEFAULT NULL,
  `landkreis` varchar(100) DEFAULT NULL,
  `kreis_regionalschluessel` varchar(10) DEFAULT NULL,
  `create_date` date,
  PRIMARY KEY (`id`),
    KEY `idx_strassenverzeichnis_str` (`strasse`),
    KEY `idx_strassenverzeichnis_plz` (`plz`),
    KEY `idx_strassenverzeichnis_ort` (`ort`),
    KEY `idx_strassenverzeichnis_ag` (`amtlicher_gemeindeschluessel`)
);


CREATE TABLE `short_boxes` (
  `zipcode` varchar(6) NOT NULL,
  `boxname` varchar(20) NOT NULL,
  PRIMARY KEY (`zipcode`)
);


CREATE TABLE `ocrs` (
  `code` varchar(20) NOT NULL,
  `street` varchar(255) not null,
  `housenumber` varchar(255) not null,
  `town` varchar(255) not null,
  `zipcode` varchar(255) not null,
  `ocrtext` varchar(4000) not null,
  `processed` tinyint(4) DEFAULT '0',
  PRIMARY KEY (`code`)
);


CREATE TABLE `sv_daten` (
`MANDANT` varchar(4)  NOT NULL DEFAULT '0000',
`MODELL` varchar(50) NOT NULL,
`ID` varchar(50) NOT NULL,
`DATUM` date NOT NULL,  `ZEIT` time NOT NULL,
`LOGIN` varchar(50) DEFAULT NULL,
`flat` tinyint(4) DEFAULT '0',
`sortiergang` varchar(20) DEFAULT NULL,
`sortierfach` varchar(20) DEFAULT NULL,
`strasse` varchar(100) DEFAULT NULL,
`hausnummer` varchar(10) DEFAULT NULL,
`begleitschein` int(11) DEFAULT NULL,  `plz` varchar(10) DEFAULT NULL,
`ort` varchar(50) DEFAULT NULL,
`strid` int(11) DEFAULT NULL,  `alt_codes` varchar(255) DEFAULT NULL,
`width` int(11) DEFAULT '0',  `height` int(11) DEFAULT '0',
`weight` int(11) DEFAULT '0',  PRIMARY KEY (`ID`),
KEY `SVMF_SVD` (`MANDANT`,`MODELL`),
KEY `IDX_SVD_US` (`LOGIN`),
KEY `SVD_DT` (`DATUM`),
KEY `idx_svd_mod_dt` (`MODELL`,`DATUM`),
KEY `idx_sv_d_blgs_sg_sf` (`begleitschein`,`sortiergang`,`sortierfach`),
KEY `idx_sv_d_blgs` (`begleitschein`),
KEY `idx_sv_daten_dt_fl` (`DATUM`,`flat`),
CONSTRAINT `sv_daten_modelle` FOREIGN KEY (`MANDANT`, `MODELL`)
REFERENCES `sv_modelle` (`MANDANT`, `MODELL`) ON UPDATE CASCADE
);

CREATE TABLE `fast_access_tour` (
  `strid` varchar(32) DEFAULT NULL,
  `mandant` varchar(10) DEFAULT NULL,
  `regiogruppe` varchar(100) DEFAULT NULL,
  `bereich` varchar(100) DEFAULT NULL,
  `sortiergang` varchar(100) DEFAULT NULL,
  `sortierfach` varchar(100) DEFAULT NULL,
  `plz` varchar(100) DEFAULT NULL,
  `ort` varchar(100) DEFAULT NULL,
  `ortsteil` varchar(100) DEFAULT NULL,
  `hnvon` varchar(4) DEFAULT NULL,
  `hnbis` varchar(4) DEFAULT NULL,
  `zuvon` varchar(10) DEFAULT NULL,
  `zubis` varchar(10) DEFAULT NULL,
  `gerade` varchar(3) DEFAULT NULL,
  `ungerade` varchar(3) DEFAULT NULL,
  `strasse` varchar(255) DEFAULT NULL,
  KEY `idx_fat_id` (`strid`),  KEY `idx_fat_mnd` (`mandant`),
  KEY `idx_fat_rg` (`regiogruppe`),
  KEY `idx_fat_be` (`bereich`),
  KEY `idx_fat_sf` (`sortierfach`),
  KEY `idx_fat_sg` (`sortiergang`),
  KEY `idx_fat_plz` (`plz`),
  KEY `idx_fat_str` (`strasse`)
);

drop table ocrhash_complex;
create table ocrhash_complex (id varchar(32) primary key, date_added timestamp ,adr text ) engine myisam;
insert into ocrhash_complex (id,date_added,adr)
  select id, UNIX_TIMESTAMP(now()), concat(strasse,' ',plz,' ',ort) adr
  from strassenverzeichnis
  group by ort,plz,strasse;
create fulltext index id_ft_hash_complex on ocrhash_complex(adr);

insert into short_boxes (zipcode,boxname) values ('99610','THB'), ('99084','MC');

DROP FUNCTION GET_SORTBOX;
DELIMITER $$
CREATE FUNCTION GET_SORTBOX(in_short_address varchar(255),in_zipcode varchar(255),in_housenumber varchar(255))
RETURNS varchar(255)
READS SQL DATA
DETERMINISTIC
BEGIN
  IF EXISTS(select * from `short_boxes` where zipcode = in_zipcode)
  THEN
    RETURN (select `boxname` from `short_boxes` where zipcode = in_zipcode);
  ELSE
   IF EXISTS(SELECT ocrhash_complex.id, ocrhash_complex.adr, match(adr) against(in_short_address) as rel FROM ocrhash_complex HAVING rel > 0 ORDER BY rel DESC LIMIT 10)
   THEN

    IF (select cast(in_housenumber as UNSIGNED) %2)=1
    THEN
      return (
        SELECT concat('',sortiergang,'|',sortierfach)
        FROM fast_access_tour
        WHERE
          hnvon<=lpad(in_housenumber,4,'0')
          and hnbis>=lpad(in_housenumber,4,'0')
          and strid in ( select id from ( SELECT ocrhash_complex.id, ocrhash_complex.adr, match(adr) against(in_short_address) as rel FROM ocrhash_complex HAVING rel > 0 ORDER BY rel DESC) a )
          and ungerade = 1
          limit 1
      );
    ELSE
      return (
        SELECT concat('',sortiergang,'|',sortierfach)
        FROM fast_access_tour
        WHERE
          hnvon<=lpad(in_housenumber,4,'0')
          and hnbis>=lpad(in_housenumber,4,'0')
          and strid in ( select id from ( SELECT ocrhash_complex.id, ocrhash_complex.adr, match(adr) against(in_short_address) as rel FROM ocrhash_complex HAVING rel > 0 ORDER BY rel DESC) a )
          and gerade = 1
          limit 1
      );
    END IF;

   ELSE
    return "NT";
   END IF;
  END IF;
END $$

DELIMITER ;
select GET_SORTBOX('','99084','');
select GET_SORTBOX('','99610','');
select GET_SORTBOX('EL ßn-Richter-Str   99085  Erfurt','99084','14');
select GET_SORTBOX('EL ßn-Richter-Str   99085  Erfurt','99084','15');




#include <driver.h>
#include <connection.h>
#include <statement.h>
#include <prepared_statement.h>
#include <resultset.h>
#include <metadata.h>
#include <resultset_metadata.h>
#include <exception.h>
#include <warning.h>
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
  double t = (double)cv::getTickCount();
  int cthread = 0;

  printf("MySQL client version: %s\n", mysql_get_client_info());

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  if (mysql_real_connect(con, "localhost", "sorter", "sorter", "sorter", 0, NULL, 0) == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
  }


  gl=new glob::Glob(argv[1]);
  if (!gl->hasNext()){
    ImageRecognize* ir = new ImageRecognize();
    ir->open(argv[1]);

    if (ir->addresstext.length()>0){
      ExtractAddress* ea = new ExtractAddress();
      ea->setString(ir->addresstext);
      ea->extract();

      std::cout << "barcode " << ir->code << std::endl;
      std::cout << "zip code " << ea->getZipCode() << std::endl;
      std::cout << "town " << ea->getTown() << std::endl;
      std::cout << "streetName " << ea->getStreetName() << std::endl;
      std::cout << "housenumber " << ea->getHouseNumber() << std::endl;
      // create table ocrs (code varchar(20) primary key, json longtext, processed tinyint default 0);
      std::string sql = "insert into ocrs (code,ocrtext,street,housenumber,zipcode,town) values ('"+ir->code+"','"+ir->addresstext+"','"+ea->getStreetName()+"','"+ea->getHouseNumber()+"','"+ea->getZipCode()+"','"+ea->getTown()+"') on duplicate key update ocrtext=values(ocrtext),town=values(town),street=values(street),housenumber=values(housenumber),zipcode=values(zipcode), processed=0";


      if (mysql_query(con, sql.c_str())){
          fprintf(stderr, "%s\n", mysql_error(con));
          mysql_close(con);
          exit(1);
      }

      std::string fuzzysql = "select GET_SORTBOX('"+ea->getStreetName()+" "+ea->getZipCode()+" "+ea->getTown()+"','"+ea->getZipCode()+"','"+ea->getHouseNumber()+"') res";
      std::cout << "fuzzysql " << fuzzysql << std::endl;
      if (mysql_query(con, fuzzysql.c_str())){
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
      }


      std::string sg = "";
      std::string sf = "";
      std::string mandant = "0000";
      std::string modell = "OCR Erfassung";
      std::string login = "sorter";

      std::string strasse = ea->getStreetName();
      std::string hausnummer = ea->getHouseNumber();
      std::string plz = ea->getZipCode();
      std::string ort = ea->getTown();

      MYSQL_RES *result;
      MYSQL_ROW row;
      unsigned int num_fields;
      unsigned int i;
      result = mysql_use_result(con);
      num_fields = mysql_num_fields(result);
      while ((row = mysql_fetch_row(result)))
      {
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
             std::cout << "box " << row[i] << "" << std::endl;
           }
         }
      }


      sql = "insert into sv_daten (mandant,modell,id,datum,zeit,login,sortiergang,sortierfach,strasse,hausnummer,plz,ort,width,height) ";
      sql = sql + " values ('"+mandant+"','"+modell+"','"+ir->code+"',now(),now(),'"+login+"','"+sg+"','"+sf+"','"+strasse+"','"+hausnummer+"','"+plz+"','"+ort+"','"+boost::lexical_cast<std::string>(ir->width)+"','"+boost::lexical_cast<std::string>(ir->height)+"') on duplicate key update mandant=values(mandant),modell=values(modell),datum=values(datum),zeit=values(zeit),login=values(login),sortiergang=values(sortiergang),sortierfach=values(sortierfach),strasse=values(strasse),hausnummer=values(hausnummer),plz=values(plz),ort=values(ort),width=values(width),height=values(height)";
      if (mysql_query(con, sql.c_str())){
          fprintf(stderr, "%s\n", mysql_error(con));
          mysql_close(con);
          exit(1);
      }

    }
  }else{

    while (gl->hasNext()) {
      //std::cout << gl->GetFileName() << " #"<< cthread << std::endl;
      if (cthread>=NUM_THREADS){
        cthread=0;
      }
      if (cthread<NUM_THREADS){


        std::stringstream str;
        str << argv[1] << "/" << gl->GetFileName();
        std::string filename = str.str();
        char *fn = new char[filename.length() + 1];
        strcpy(fn, filename.c_str());

        thread_info* t_info = &threadInfo[cthread];
        t_info->filename = fn;
        pthread_t thread;
        int ct = pthread_create( &thread, NULL, processImage, (void*) t_info);
        if( ct  != 0) {
         printf("something went wrong while threading %i\n",ct);
        }else{

        }
        std::cout << gl->GetFileName() << " started" << std::endl;
        cthread++;
        if (cthread==1){
          pthread_join(thread,NULL);
          cthread=0;

          //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
          //std::cout << "Times passed in seconds: " << t << std::endl;
          //t = (double)cv::getTickCount();

        }else{
          pthread_detach(thread);
        }
      }
      gl->Next();
      /*
      ImageRecognize* ir = new ImageRecognize();
      std::stringstream str;
      str << argv[1] << "/" << gl->GetFileName();
      std::string filename = str.str();
      char *fn = new char[filename.length() + 1];
      strcpy(fn, filename.c_str());
      ir->open(fn);
      delete[] fn;
      t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
      std::cout << "Times passed in seconds: " << t << std::endl;
      t = (double)cv::getTickCount();
      */
    }
  }
  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cout << "Times passed in seconds: " << t << std::endl;

  //ImageRecognize* ir = new ImageRecognize();
  //ir->open(argv[1]);

  //filename
  //const unsigned char * constStr =
  /*
  thread_info packet_info = {0,reinterpret_cast<const unsigned char *> (strVar.c_str())};
  pthread_t thread;
  int ct = pthread_create( &thread, NULL, processImage, (void*)packet_info);
  if( ct  != 0) {
   printf("something went wrong while threading %i\n",ct);
   return 0;
  }
  pthread_join(thread,NULL);
  */



  mysql_close(con);
  exit(0);

  return -1;
}
