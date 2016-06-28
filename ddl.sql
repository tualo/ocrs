/*
/etc/mysql/my.cnf
[mysqld]
bind-address                   = 0.0.0.0
lower_case_table_names=1

*/
DROP TABLE IF EXISTS `strassenverzeichnis`;
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
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/strassenverzeichnis';

DROP TABLE IF EXISTS `short_boxes`;
CREATE TABLE `short_boxes` (
  `zipcode` varchar(6) NOT NULL,
  `boxname` varchar(20) NOT NULL,
  PRIMARY KEY (`zipcode`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/short_boxes';

DROP TABLE IF EXISTS `short_boxes_locked`;
CREATE TABLE `short_boxes_locked` (
  `zipcode` varchar(6) NOT NULL,
  `kundenid` varchar(20) NOT NULL,
  PRIMARY KEY (`zipcode`,`kundenid`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/short_boxes_locked';

DROP TABLE IF EXISTS `short_boxes_locked_by_product`;
CREATE TABLE `short_boxes_locked_by_product` (
  `zipcode` varchar(6) NOT NULL,
  `product` varchar(20) NOT NULL,
  PRIMARY KEY (`zipcode`,`product`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/short_boxes_locked_by_product';



DROP TABLE  IF EXISTS `ocrs`;
CREATE TABLE `ocrs` (
  `code` varchar(20) NOT NULL,
  `street` varchar(255) not null,
  `housenumber` varchar(255) not null,
  `town` varchar(255) not null,
  `zipcode` varchar(255) not null,
  `ocrtext` varchar(4000) not null,
  `processed` tinyint(4) DEFAULT '0',
  `insert_time` timestamp DEFAULT NOW(),
  PRIMARY KEY (`code`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/ocrs';




DROP TABLE  IF EXISTS `plz_orte`;
CREATE TABLE `plz_orte` (
 plz char(5),
 ort varchar(50),
 primary key (plz,ort)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/plz_orte';


DROP TABLE  IF EXISTS `sortiergaenge_zuordnung`;
create table `sortiergaenge_zuordnung`(
  `MANDANT` varchar(4) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0000',
  `REGIOGRUPPE` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `BEREICH` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `SORTIERGANG` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `SORTIERFACH` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `DATUM` date NOT NULL,
  PRIMARY KEY (`MANDANT`,`REGIOGRUPPE`,`BEREICH`,`SORTIERGANG`,`SORTIERFACH`),
  KEY `IDX_SORT_BER` (`BEREICH`),
  KEY `IDX_SORT_MAN` (`MANDANT`),
  KEY `IDX_SORT_REG` (`REGIOGRUPPE`),
  KEY `fk_sortiergaenge_zuordnung_bereiche` (`MANDANT`,`BEREICH`,`REGIOGRUPPE`),
  KEY `fk_sortiergaenge_zuordnung_sortiergang` (`MANDANT`,`REGIOGRUPPE`,`SORTIERGANG`),
  KEY `fk_sortiergaenge_zuordnung_sortierfach` (`MANDANT`,`REGIOGRUPPE`,`SORTIERFACH`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/sortiergaenge_zuordnung';

DROP TABLE  IF EXISTS `bereiche_plz`;
create table `bereiche_plz`(
  `MANDANT` varchar(4) COLLATE utf8_unicode_ci NOT NULL,
  `REGIOGRUPPE` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `NAME` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `PLZ` varchar(5) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`MANDANT`,`REGIOGRUPPE`,`NAME`,`PLZ`),
  KEY `BNME_BER` (`MANDANT`,`NAME`,`REGIOGRUPPE`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/bereiche_plz';


DROP TABLE  IF EXISTS `bereiche`;
create table `bereiche`(
  `MANDANT` varchar(4) COLLATE utf8_unicode_ci NOT NULL,
  `REGIOGRUPPE` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `NAME` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `PLZ` varchar(5) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`MANDANT`,`REGIOGRUPPE`,`NAME`,`PLZ`),
  KEY `BNME_BER` (`MANDANT`,`NAME`,`REGIOGRUPPE`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/bereiche';


DROP TABLE `bereiche`;
CREATE TABLE `bereiche` (
  `MANDANT` varchar(4) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0000',
  `NAME` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `REGIOGRUPPE` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `KOMMENTAR` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `GUELTIG_AB` date DEFAULT '2001-01-01',
  `GUELTIG_BIS` date DEFAULT '2099-12-31',
  `alleplz` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`MANDANT`,`NAME`,`REGIOGRUPPE`),
  KEY `fk_bereiche_regiogruppen` (`MANDANT`,`REGIOGRUPPE`),
  CONSTRAINT `fk_bereiche_regiogruppen` FOREIGN KEY (`MANDANT`, `REGIOGRUPPE`) REFERENCES `regiogruppen` (`MANDANT`, `NAME`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/bereiche';



DROP TABLE  IF EXISTS `sv_daten`;
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
  `ocrtxt` varchar(255) DEFAULT NULL,
  `kunde` varchar(255) DEFAULT NULL,
  `produkt` varchar(255) DEFAULT NULL,
  `strid` int(11) DEFAULT NULL,  `alt_codes` varchar(255) DEFAULT NULL,
  `width` int(11) DEFAULT '0',  `height` int(11) DEFAULT '0',
  `weight` int(11) DEFAULT '0',  PRIMARY KEY (`ID`)
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/sv_daten';



DROP TABLE  IF EXISTS `fast_access_tour`;
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
) ENGINE=FEDERATED CONNECTION='mysql://USER:PASSWORD@HOST:PORT/DBNAME/fast_access_tour';


DROP TABLE  IF EXISTS ocrhash_complex;
create table ocrhash_complex (id varchar(32) primary key, date_added timestamp ,adr text,strasse varchar(255),plz varchar(255),ort varchar(255) ) engine myisam;
insert into ocrhash_complex (id,date_added,strasse,plz,ort,adr)
  select id, UNIX_TIMESTAMP(now()),strasse,plz,ort, concat(strasse,' ',plz,' ',ort) adr
  from strassenverzeichnis
  group by ort,plz,strasse;

insert into ocrhash_complex (id,date_added,strasse,plz,ort,adr)
  select concat('OWN',plz,ort) id, UNIX_TIMESTAMP(now()),'' strasse,plz,ort, concat(plz,' ',ort) adr
  from plz_orte
on duplicate key update ort=values(ort);

create fulltext index id_ft_hash_complex on ocrhash_complex(adr);
