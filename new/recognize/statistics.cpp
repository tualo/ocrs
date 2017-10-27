/*
create table ocrs_statistics
(
 code varchar(25) primary key,
 cmx fixed(15,6),
 cmy fixed(15,6),
 letter_height fixed(15,6),
 letter_length fixed(15,6),
 image_cols fixed(15,6),
 image_rows fixed(15,6),
 roi varchar(50),
 state varchar(20),
 orignalfilename varchar(100),
 resultfilename varchar(100)
);
alter table ocrs_statistics add createtime datetime default current_timestamp;
alter table ocrs_statistics add running fixed(15,6) default 0;

*/
void ImageRecognizeEx::initStatistics(){
  std::string sql = "insert into ocrs_statistics (code,orignalfilename,resultfilename,cmx,cmy,letter_height,letter_length,image_cols,image_rows,roi) values ('"+code+"','"+fileName+"','',0,0,0,0,0,0,'') on duplicate key update code=values(code)";
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}


void ImageRecognizeEx::updateStatistics(std::string field,double val){
  std::string sql = boost::str(updateStatisticsDouble % field % val % code );
  std::cout << "updateStatistics (double) " << sql << std::endl;
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}

void ImageRecognizeEx::updateStatistics(std::string field,int val){
  std::string sql = boost::str(updateStatisticsString % field % val % code );
  std::cout << "updateStatistics (int) " << sql << std::endl;
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}

void ImageRecognizeEx::updateStatistics(std::string field,std::string val){
  std::string sql = boost::str(updateStatisticsString % field % val % code );
  std::cout << "updateStatistics (string) " << sql << std::endl;
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}
