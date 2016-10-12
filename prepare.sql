
DROP TABLE  IF EXISTS ocrhash_complex;
create table ocrhash_complex (id varchar(32) primary key, date_added timestamp ,adr text,strasse varchar(255),plz varchar(255),ort varchar(255) ) engine myisam;
insert into ocrhash_complex (id,date_added,strasse,plz,ort,adr)
  select id, UNIX_TIMESTAMP(now()),strasse,plz,ort, concat(strasse,' ',plz,' ',ort) adr
  from strassenverzeichnis
  where false
  group by ort,plz,strasse;


DROP TABLE  IF EXISTS ocrhash_complex;
create table ocrhash_complex (id varchar(32) primary key, date_added timestamp ,adr text,strasse varchar(255),plz varchar(255),ort varchar(255) ) engine myisam;
insert into ocrhash_complex (id,date_added,strasse,plz,ort,adr)
  select concat('OWN',plz,ort) id, UNIX_TIMESTAMP(now()),'' strasse,plz,ort, concat(plz,' ',ort) adr
  from plz_orte where gesondert=0
on duplicate key update ort=values(ort);

create fulltext index id_ft_hash_complex on ocrhash_complex(adr);



DROP TABLE  IF EXISTS ocrtownhash_complex;
create table ocrtownhash_complex (id varchar(50) primary key, date_added timestamp ,adr text,plz varchar(255),ort varchar(255) ) engine myisam;
insert into ocrtownhash_complex (id,date_added,plz,ort,adr)
  select concat(plz,ort) id, UNIX_TIMESTAMP(now()),plz,ort, concat(plz,' ',ort) adr
  from strassenverzeichnis
  group by ort,plz;
create fulltext index id_ft_ocrtownhash_complex on ocrtownhash_complex(adr);



DROP TABLE  IF EXISTS ocrstreethash_complex;
create table ocrstreethash_complex (id varchar(50) primary key, date_added timestamp ,adr text,strasse varchar(255) ) engine myisam;
insert into ocrstreethash_complex (id,date_added,strasse,adr)
  select md5(strasse) id, UNIX_TIMESTAMP(now()),strasse, strasse adr
  from strassenverzeichnis
  group by strasse;
create fulltext index id_ft_ocrstreethash_complex on ocrstreethash_complex(adr);



DROP TABLE  IF EXISTS ocradrhash_complex;
create table ocradrhash_complex (id varchar(50) primary key,
  date_added timestamp ,adr text,
  strasse varchar(255),plz varchar(10),ort varchar(255)
 ) engine myisam;
insert into ocradrhash_complex (id,date_added,strasse,plz,ort,adr)
  select id, UNIX_TIMESTAMP(now()),strasse,plz,ort,
  concat(strasse,' ',plz,' ',ort) adr
  from strassenverzeichnis;
create fulltext index id_ft_ocradrhash_complex on ocradrhash_complex(adr);
