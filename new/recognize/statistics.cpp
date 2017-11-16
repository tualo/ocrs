/*



CREATE TABLE `ocrs_statistics` (
  `code` varchar(25) NOT NULL,
  `cmx` decimal(15,6) DEFAULT NULL,
  `cmy` decimal(15,6) DEFAULT NULL,
  `letter_height` decimal(15,6) DEFAULT NULL,
  `letter_length` decimal(15,6) DEFAULT NULL,
  `image_cols` decimal(15,6) DEFAULT NULL,
  `image_rows` decimal(15,6) DEFAULT NULL,
  `roi` varchar(50) DEFAULT NULL,
  `state` varchar(20) DEFAULT NULL,
  `orignalfilename` varchar(100) DEFAULT NULL,
  `resultfilename` varchar(100) DEFAULT NULL,
  `createtime` datetime DEFAULT CURRENT_TIMESTAMP,
  `running` decimal(15,6) DEFAULT '0.000000',
  `calc_cmx` decimal(15,6) DEFAULT '0.000000',
  `calc_cmy` decimal(15,6) DEFAULT '0.000000',
  `new_ok` tinyint(4) DEFAULT '0',
  PRIMARY KEY (`code`)
);

CREATE TABLE `quicksv_table`  (
`code` varchar(50) NOT NULL,
`zipcode` varchar(10) DEFAULT NULL,
`town` varchar(255) DEFAULT NULL,
`streetname` varchar(255) DEFAULT NULL,
`housenumber` varchar(255) DEFAULT NULL,
`sortrow` varchar(50) DEFAULT NULL,
`sortbox` varchar(50) DEFAULT NULL,
`ocrtext` varchar(4000) DEFAULT NULL,
`customerno` varchar(20) DEFAULT NULL,
`costcenter` varchar(20) DEFAULT NULL,
`createtime` datetime DEFAULT NULL,
`match_town` varchar(100) DEFAULT NULL,
`match_zipcode` varchar(10) DEFAULT NULL,
`old_ocr_state` varchar(100) DEFAULT NULL,
`old_sv_model` varchar(100) DEFAULT NULL,
`old_sv_plz` varchar(10) DEFAULT NULL,
`old_sv_produkt` varchar(10) DEFAULT NULL,  PRIMARY KEY (`code`)
)
;




create or replace view view_ocrs_statistics as
select
  ocrs_statistics.code,
  ocrs_statistics.cmx,
  ocrs_statistics.cmy,
  ocrs_statistics.letter_height,
  ocrs_statistics.letter_length,
  ocrs_statistics.image_cols,
  ocrs_statistics.image_rows,
  ocrs_statistics.roi,
  ocrs_statistics.state,
  ocrs_statistics.orignalfilename,
  ocrs_statistics.resultfilename,
  ocrs_statistics.new_ok,
  concat('/probe/',SUBSTRING_INDEX(ocrs_statistics.resultfilename, '/', -1)) url,
  quicksv_table.zipcode,
  quicksv_table.customerno,
  quicksv_table.costcenter,
  quicksv_table.town,
  quicksv_table.sortrow,
  quicksv_table.sortbox,
  quicksv_table.match_zipcode,
  quicksv_table.match_town,
  quicksv_table.old_ocr_state,
  quicksv_table.old_sv_model,
  quicksv_table.old_sv_plz,
  if(
    (quicksv_table.old_sv_plz<>quicksv_table.zipcode) or
    (quicksv_table.old_sv_model<>'Maschine') or
    (quicksv_table.old_ocr_state<>'good') or
    (quicksv_table.match_zipcode='-----'),
    'obacht',
    ''
  ) info,
  quicksv_table.old_sv_produkt,
  if( (quicksv_table.old_sv_model='Maschine' and quicksv_table.match_zipcode<>'-----' and quicksv_table.old_sv_plz<>quicksv_table.match_zipcode),'sichten','') hinweis
from
  ocrs_statistics
  left join
  quicksv_table
  on ocrs_statistics.code=quicksv_table.code
;




72.741900		/probe/noaddress.22002666644.jpg		barcodecode	/largedisk/image/14107|1N001510309388.656114.tiff	/largedisk/result/noaddress.22002666644.jpg	71.959800	2286.000000	1240.000000	1645.000000	902.000000	22002666644


/largedisk/ocrs/new/ocrsex -dt -h 192.168.1.96 -n deg -u sorter -x sorter --cmx 72  --cmy 72  -f "/largedisk/image/14107|1N001510309462.545156.tiff" --substractmean 18 --savedb --result /largedisk/result/ --machine 22

  --rescaledfilename

72.419400	* DL und C5  - L	/probe/good.22002666728.jpg	Cospedn	address	/largedisk/image/14107|1N001510309462.545156.tiff	/largedisk/result/good.22002666728.jpg	71.980900	2302.000000	1240.000000	1657.000000	898.000000	22002666728	07751
72.661300		/probe/noaddress.22002666729.jpg		barcodecode	/largedisk/image/14107|1N001510309463.220315.tiff	/largedisk/result/noaddress.22002666729.jpg	71.959800	2286.000000	1240.000000	1645.000000	901.000000	22002666729

SUBSTRING_INDEX(Path, '/', -1) AS
delete from quicksv_table;
delete from ocrs_statistics;

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
  std::cout << "updateStatistics (double) " << field << " " << sql << std::endl;
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}

void ImageRecognizeEx::updateStatistics(std::string field,int val){
  std::string sql = boost::str(updateStatisticsString % field % val % code );
  std::cout << "updateStatistics (int) "<< field << " "  << sql << std::endl;
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}

void ImageRecognizeEx::updateStatistics(std::string field,std::string val){
  std::string sql = boost::str(updateStatisticsString % field % val % code );
  std::cout << "updateStatistics (string) "<< field << " "  << sql << std::endl;
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }
}
