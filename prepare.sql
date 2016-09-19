
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
