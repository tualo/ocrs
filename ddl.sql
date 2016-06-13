
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
);

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
create fulltext index id_ft_hash_complex on ocrhash_complex(adr);



DELIMITER ;;;
DROP FUNCTION `LEVENSHTEIN`;;;
CREATE FUNCTION `LEVENSHTEIN`(s1 VARCHAR(255), s2 VARCHAR(255)) RETURNS int(11) DETERMINISTIC
BEGIN
    DECLARE s1_len, s2_len, i, j, c, c_temp, cost INT;
    DECLARE s1_char CHAR;
    DECLARE cv0, cv1 VARBINARY(256);
    SET s1_len = CHAR_LENGTH(s1), s2_len = CHAR_LENGTH(s2), cv1 = 0x00, j = 1, i = 1, c = 0;
    IF s1 = s2 THEN
        RETURN 0;
    ELSEIF s1_len = 0 THEN
        RETURN s2_len;
    ELSEIF s2_len = 0 THEN
        RETURN s1_len;
    ELSE
        WHILE j <= s2_len DO
            SET cv1 = CONCAT(cv1, UNHEX(HEX(j))), j = j + 1;
        END WHILE;
        WHILE i <= s1_len DO
            SET s1_char = SUBSTRING(s1, i, 1), c = i, cv0 = UNHEX(HEX(i)), j = 1;
            WHILE j <= s2_len DO
                SET c = c + 1;
                IF s1_char = SUBSTRING(s2, j, 1) THEN SET cost = 0; ELSE SET cost = 1; END IF;
                SET c_temp = CONV(HEX(SUBSTRING(cv1, j, 1)), 16, 10) + cost;
                IF c > c_temp THEN SET c = c_temp; END IF;
                SET c_temp = CONV(HEX(SUBSTRING(cv1, j+1, 1)), 16, 10) + 1;
                IF c > c_temp THEN SET c = c_temp; END IF;
                SET cv0 = CONCAT(cv0, UNHEX(HEX(c))), j = j + 1;
            END WHILE;
            SET cv1 = cv0, i = i + 1;
        END WHILE;
    END IF;
    RETURN c;
END;;;

DELIMITER ;;;
DROP FUNCTION `LEVENSHTEIN_RATIO`;;;
CREATE FUNCTION `LEVENSHTEIN_RATIO`(s1 VARCHAR(255), s2 VARCHAR(255)) RETURNS int(11) DETERMINISTIC
BEGIN
    DECLARE s1_len, s2_len, max_len INT;
    SET s1_len = LENGTH(s1), s2_len = LENGTH(s2);
    IF s1_len > s2_len THEN SET max_len = s1_len; ELSE SET max_len = s2_len; END IF;
    RETURN ROUND((1 - LEVENSHTEIN(s1, s2) / max_len) * 100);
END;;;

DELIMITER ;


DROP FUNCTION IF EXISTS  GET_SORTBOX;
DELIMITER $$
CREATE FUNCTION GET_SORTBOX(in_short_address varchar(255),in_zipcode varchar(255),in_housenumber varchar(255),in_kundenid varchar(255))
RETURNS varchar(255)
READS SQL DATA
DETERMINISTIC
BEGIN
  IF EXISTS(select * from `short_boxes_locked` where zipcode = in_zipcode and kundenid=in_kundenid)
  THEN
    return 'DPAG';
  ELSE
    IF EXISTS(
        select * from bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=in_zipcode
        join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung' where bereiche_plz.plz=in_zipcode)
    THEN
      return (select if(sortiergaenge_zuordnung.sortiergang is null,'NT', concat('',sortiergaenge_zuordnung.sortiergang,'|',sortiergaenge_zuordnung.sortierfach) ) from
              sortiergaenge_zuordnung
              join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=in_zipcode
              join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
            );
    ELSE
     IF EXISTS(SELECT ocrhash_complex.id, ocrhash_complex.adr, match(adr) against(in_short_address) as rel FROM ocrhash_complex HAVING rel > 0 ORDER BY rel DESC LIMIT 10)
     THEN

      IF (select cast(in_housenumber as UNSIGNED) %2)=1
      THEN
        return (
          SELECT if(sortiergang is null,'NT', concat('',sortiergang,'|',sortierfach) )
          FROM fast_access_tour
          WHERE
            hnvon<=lpad(in_housenumber,4,'0')
            and hnbis>=lpad(in_housenumber,4,'0')
            and strid in ( select id from (
              SELECT
                id,
                adr,
                strasse,
                plz,
                ort,
                rel,
                LEVENSHTEIN_RATIO(in_short_address,adr) lvr
              FROM (

              select id,adr,strasse,
              plz,
              ort,rel from
                (
                  SELECT
                    ocrhash_complex.id,
                    ocrhash_complex.strasse,
                    ocrhash_complex.plz,
                    ocrhash_complex.ort,
                    ocrhash_complex.adr,
                    match(adr) against(in_short_address) as rel

                  FROM ocrhash_complex
                  having rel>0
                ) o
                join
                (
                  SELECT       match(adr) against(in_short_address) as mrel
                  FROM ocrhash_complex having mrel > 0
                  order by mrel desc limit 1
                ) mx
                on o.rel = mx.mrel
              ) b
              order by lvr desc limit 1
            ) a )
            and ungerade = 1
            and regiogruppe = 'Zustellung'
            limit 1
        );
      ELSE
        return ifnull( (
          SELECT if(sortiergang is null,'NT', concat('',sortiergang,'|',sortierfach) )
          FROM fast_access_tour
          WHERE
            hnvon<=lpad(in_housenumber,4,'0')
            and hnbis>=lpad(in_housenumber,4,'0')
            and strid in ( select id from (


               SELECT
                 id,
                 adr,
                 strasse,
                 plz,
                 ort,
                 rel,
                 LEVENSHTEIN_RATIO(in_short_address,adr) lvr
               FROM (

               select id,adr,strasse,
               plz,
               ort,rel from
                 (
                   SELECT
                     ocrhash_complex.id,
                     ocrhash_complex.strasse,
                     ocrhash_complex.plz,
                     ocrhash_complex.ort,
                     ocrhash_complex.adr,
                     match(adr) against(in_short_address) as rel

                   FROM ocrhash_complex
                   having rel>0
                 ) o
                 join
                 (
                   SELECT       match(adr) against(in_short_address) as mrel
                   FROM ocrhash_complex having mrel > 0
                   order by mrel desc limit 1
                 ) mx
                 on o.rel = mx.mrel
               ) b
               order by lvr desc limit 1

            ) a )
            and gerade = 1
            and regiogruppe = 'Zustellung'
            limit 1
        ),'NT');
      END IF;

     ELSE
      return "NT";
     END IF;
    END IF;
  END IF;
  return 'NT';
END $$

DELIMITER ;
