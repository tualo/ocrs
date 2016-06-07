#ocrs

This program reads scanned letter images. It regonizes the barcode, the address
text and if possible it find the sort box. It support customer depending sort
box exceptions. So it is possible to block some zip codes for special customers. The customers will be defined by the file name. If the filename
contains the letter `N` the textpart before will be used as customer
identification.

This program needs a database for optaining the sort informations. It's
recommend to use a local database for that. The local database may use
connected tables (federated or connection engine).

##Environment variables

This program can be controlled by some environment variables.

* USEPZA if set to 1 the PZA algorithm will be used
* IMAGEPATH the path where to put the result files
* STORE_ORIGINAL if set to an path the original image will be stores there as an jpeg file
* DB_HOST the db host name defaults to localhost
* DB_USER the db user name defaults to sorter
* DB_PASSWORD the users password defaults to sorter
* DB_NAME the database name defaults to sorter
* MODELL the sort model name to be stored defaults to OCR Erfassung
* MANDANT the sorting client defaults to 0000

Depending on your database configuration some values may lead to errors if there
are not set right.

##The Sorting Database


This script `create_db.sh` will support by configuring the database.

This program need the folowing tables.

This table contains all ZIP Codes that are not splitted by there streets. The box name will be used to retreive the sortrow and sortbox directly.

```
CREATE TABLE `short_boxes` (
  `zipcode` varchar(6) NOT NULL,
  `boxname` varchar(20) NOT NULL,
  PRIMARY KEY (`zipcode`)
)
```

If you have customers that don't want to be sorted by some special zip codes - list them here.
```
CREATE TABLE `short_boxes_locked` (
  `zipcode` varchar(6) NOT NULL,
  `kundenid` varchar(20) NOT NULL,
  PRIMARY KEY (`zipcode`,`kundenid`)
)
```

All reconized results will be stored there in plain text. *But keep in mind to clean that table by a cronjob*
```
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
)
```

Reconized sort informations will be stored
here.
```
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
)
```

If you need to sort the data be streetnames, housenumber and so on - put that informations in there.

```
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
)
```


So the tricky part happens here. The column `adr` need to be filled with all your street, zip code and town combinations you are interested in. The program will query that column by an full text search to optain the best match.
```
create table ocrhash_complex (
    id varchar(32) primary key,
    date_added timestamp ,
    adr text,
    strasse varchar(255),plz varchar(255),
    ort varchar(255)
) engine myisam;
```


The function that do all that for you is below. You have to change that function to your needs.

```
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
    IF EXISTS(select * from `short_boxes` where zipcode = in_zipcode)
    THEN
      return (select `boxname` from `short_boxes` where zipcode = in_zipcode);
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
            and regiogruppe = 'Zustellung PZA EE'
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
            and regiogruppe = 'Zustellung PZA EE'
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
```
