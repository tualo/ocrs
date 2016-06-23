
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

DROP FUNCTION `LEVENSHTEIN_RATIO`;;;
CREATE FUNCTION `LEVENSHTEIN_RATIO`(s1 VARCHAR(255), s2 VARCHAR(255)) RETURNS int(11) DETERMINISTIC
BEGIN
    DECLARE s1_len, s2_len, max_len INT;
    SET s1_len = LENGTH(s1), s2_len = LENGTH(s2);
    IF s1_len > s2_len THEN SET max_len = s1_len; ELSE SET max_len = s2_len; END IF;
    RETURN ROUND((1 - LEVENSHTEIN(s1, s2) / max_len) * 100);
END;;;

DELIMITER ;



DROP PROCEDURE IF EXISTS SET_SORTBOX;
DROP PROCEDURE IF EXISTS SET_SV;

DELIMITER //

CREATE PROCEDURE SET_SV
(
  IN in_code varchar(20),
  IN in_sortiergang varchar(255),
  IN in_sortierfach varchar(255),
  IN in_strasse varchar(255),
  IN in_hausnummer varchar(255),
  IN in_plz varchar(20),
  IN in_ort varchar(255),
  IN in_ocrtxt varchar(20),
  IN in_kunde varchar(20),
  IN in_product varchar(20)
)
MODIFIES SQL DATA
BEGIN
  IF EXISTS(SELECT * FROM SV_DATEN WHERE ID=in_code)
  THEN
    UPDATE SV_DATEN SET
      DATUM=CURRENT_DATE(),
      ZEIT=CURRENT_TIME(),
      MANDANT='0000',
      MODELL='Clearing',

      SORTIERGANG=in_sortiergang,
      SORTIERFACH=in_sortierfach,
      STRASSE=in_strasse,
      HAUSNUMMER=in_hausnummer,
      PLZ=in_plz,
      ORT=in_ort,
      OCRTXT=in_ocrtxt,

      KUNDE=in_kunde,
      PRODUKT=in_product

    WHERE ID = in_code;
  ELSE
    INSERT INTO SV_DATEN
      (
        ID,
        DATUM,
        ZEIT,
        MANDANT,
        MODELL,

        SORTIERGANG,
        SORTIERFACH,
        STRASSE,
        HAUSNUMMER,
        PLZ,
        ORT,
        OCRTXT,

        KUNDE,
        PRODUKT

      ) VALUES (

        in_code,

        CURRENT_DATE(),
        CURRENT_TIME(),
        '0000',
        'Clearing',

        in_sortiergang,
        in_sortierfach,
        in_strasse,
        in_hausnummer,
        in_plz,
        in_ort,
        in_ocrtxt,

        in_kunde,
        in_product

      );
  END IF;
END;
//








CREATE PROCEDURE SET_SORTBOX
  (
    IN in_short_address varchar(4000),
    IN in_zipcode varchar(255),
    IN in_housenumber varchar(255),
    IN in_kundenid varchar(255),
    IN in_product varchar(255),
    IN in_code varchar(20),


    OUT out_stortiergang varchar(10),
    OUT out_stortierfach varchar(10),
    OUT out_strasse varchar(255),
    OUT out_plz varchar(255),
    OUT out_ort varchar(255)
  )
MODIFIES SQL DATA
BEGIN
  DECLARE lvr INT;
  SELECT
    strasse,
    plz,
    ort,
    LEVENSHTEIN_RATIO(in_short_address,adr) lvr
  INTO out_strasse,out_plz,out_ort,@lvr

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
  order by lvr desc limit 1;


  IF EXISTS(select * from `short_boxes_locked` where zipcode = out_plz and kundenid=in_kundenid)
  THEN

    SET out_stortiergang = 'DPAG';
    SET out_stortierfach = 'DPAG';
    CALL SET_SV
    (
      in_code,
      out_stortiergang,
      out_stortierfach,
      '',
      '',
      out_plz,
      '',
      in_short_address,
      in_kundenid,
      in_product
    );
  ELSE
    IF EXISTS(select * from `short_boxes_locked_by_product` where zipcode = out_plz and product=in_product)
    THEN
      SET out_stortiergang = 'DPAG';
      SET out_stortierfach = 'DPAG';
      CALL SET_SV
      (
        in_code,
        out_stortiergang,
        out_stortierfach,
        '',
        '',
        out_plz,
        '',
        in_short_address,
        in_kundenid,
        in_product
      );
    ELSE


    IF EXISTS(select * from bereiche_plz join bereiche on bereiche.alleplz=1 and  bereiche_plz.plz=out_plz and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung')
    THEN
      select
        ifnull(sortiergaenge_zuordnung.sortiergang,'NT') sortiergang,
        ifnull(sortiergaenge_zuordnung.sortierfach,'NT') sortierfach
        into out_stortiergang,out_stortierfach
      from
        sortiergaenge_zuordnung
        join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=out_plz
        join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
      limit 1;

        CALL SET_SV
        (
          in_code,
          out_stortiergang,
          out_stortierfach,
          '',
          '',
          out_plz,
          '',
          in_short_address,
          in_kundenid,
          in_product
        );
    ELSE

      SET out_stortiergang = 'NT';
      SET out_stortierfach = 'NT';

      CALL SET_SV
      (
        in_code,
        out_stortiergang,
        out_stortierfach,
        '',
        '',
        out_plz,
        '',
        in_short_address,
        in_kundenid,
        in_product
      );

    END IF;


    END IF;
  END IF;


END;
//
DELIMITER ;

CALL SET_SORTBOX('Behnerstieg 3 37308 Geisleden','37308','13','101400','Standard','789',@sg,@sf,@str,@plz,@ort);
SELECT @sg,@sf,@str,@plz,@ort;

CALL SET_SORTBOX('Behnerstieg 3 72379 Hechingen','72379','13','101400','Standard','456',@sg,@sf,@str,@plz,@ort);
SELECT @sg,@sf,@str,@plz,@ort;

CALL SET_SORTBOX('Behnerstieg 3 50823 Köln','50823','13','101400','Standard','123',@sg,@sf,@str,@plz,@ort);
SELECT @sg,@sf,@str,@plz,@ort;