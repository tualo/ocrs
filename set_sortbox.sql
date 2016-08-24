
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

set @datum=CURRENT_DATE();
set @zeit=CURRENT_TIME();

IF EXISTS(SELECT * FROM BBS_DATA WHERE ID=in_code)
THEN
  SELECT substring(inserttime,1,10),substring(inserttime,11,19) into @datum,@zeit FROM BBS_DATA WHERE ID=in_code;
END IF;


  IF EXISTS(SELECT * FROM SV_DATEN WHERE ID=in_code)
  THEN
    SELECT

      STRASSE,
      PLZ,
      ORT,
      HAUSNUMMER,
      OCRTXT,
      KUNDE,
      PRODUKT

    INTO
      @TMP_STRASSE,
      @TMP_PLZ,
      @TMP_ORT,
      @TMP_HAUSNUMMER,
      @TMP_OCRTXT,
      @TMP_KUNDE,
      @TMP_PRODUKT
    FROM
      SV_DATEN
    WHERE ID=in_code;

    IF (in_strasse IS NULL or in_strasse='') THEN
      SET in_strasse = @TMP_STRASSE;
    END IF;

    IF (in_hausnummer IS NULL or in_hausnummer='') THEN
      SET in_hausnummer = @TMP_HAUSNUMMER;
    END IF;

    IF (in_plz IS NULL or in_plz='') THEN
      SET in_plz = @TMP_PLZ;
    END IF;

    IF (in_ort IS NULL or in_ort='') THEN
      SET in_ort = @TMP_ORT;
    END IF;

    IF (in_ocrtxt IS NULL or in_ocrtxt='') THEN
      SET in_ocrtxt = @TMP_OCRTXT;
    END IF;

    IF (in_kunde IS NULL or in_kunde='') THEN
      SET in_kunde = @TMP_KUNDE;
    END IF;

    IF (in_product IS NULL or in_product='') THEN
      SET in_product = @TMP_PRODUKT;
    END IF;

  END IF;


  IF (@debug=1)
  THEN
    select concat('modell',@svmodell) msg;
    select concat('in_sortiergang',in_sortiergang) msg;
    select concat('in_sortierfach',in_sortierfach) msg;
    select concat('in_plz',ifnull(in_plz,' NULL')) msg;
    select concat('in_kunde',ifnull(in_kunde,' NULL')) msg;
    select concat('in_product',ifnull(in_product,' NULL')) msg;
  END IF;

  IF EXISTS(SELECT * FROM SV_DATEN WHERE ID=in_code)
  THEN
    UPDATE SV_DATEN SET
      MANDANT='0000',
      MODELL=@svmodell,

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

        @datum,
        @zeit,
        '0000',
        @svmodell,

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
DELIMITER ;


DELIMITER //
CREATE PROCEDURE SET_SORTBOX
  (
    IN in_short_address varchar(4000),
    IN in_zipcode varchar(255),
    IN in_housenumber varchar(255),
    IN in_kundenid varchar(255),
    IN in_product varchar(255),
    IN in_code varchar(20),


    OUT out_sortiergang varchar(100),
    OUT out_sortierfach varchar(20),
    OUT out_strasse varchar(255),
    OUT out_plz varchar(255),
    OUT out_ort varchar(255)
  )
MODIFIES SQL DATA
BEGIN
  DECLARE lvr INT;

  SET @adr=in_short_address;

  SELECT
    strasse,
    plz,
    ort,
    LEVENSHTEIN_RATIO(in_short_address,adr) lvrval
  INTO out_strasse,out_plz,out_ort,@lvr

  FROM (

    SELECT
      ocrhash_complex.id,
      ocrhash_complex.strasse,
      ocrhash_complex.plz,
      ocrhash_complex.ort,
      ocrhash_complex.adr,
      match(adr) against(in_short_address) as rel

    FROM ocrhash_complex
    WHERE plz = in_zipcode
    having rel>0
    limit 100
  ) b
  order by lvrval desc limit 1;

  IF (out_plz is null)
  THEN
    if (@debug=1)
    THEN
      select 'try search all' msg;
      SELECT
        strasse,
        plz,
        ort,
        LEVENSHTEIN_RATIO(in_short_address,adr) lvrval
      FROM (

        SELECT
          ocrhash_complex.id,
          ocrhash_complex.strasse,
          ocrhash_complex.plz,
          ocrhash_complex.ort,
          ocrhash_complex.adr,
          match(adr) against(in_short_address) as rel

        FROM ocrhash_complex
        having rel>0
        limit 100
      ) b
      order by lvrval desc limit 10;
    END IF;

    SELECT
      strasse,
      plz,
      ort,
      LEVENSHTEIN_RATIO(in_short_address,adr) lvrval
    INTO out_strasse,out_plz,out_ort,@lvr

    FROM (

      SELECT
        ocrhash_complex.id,
        ocrhash_complex.strasse,
        ocrhash_complex.plz,
        ocrhash_complex.ort,
        ocrhash_complex.adr,
        match(adr) against(in_short_address) as rel

      FROM ocrhash_complex
      having rel>0
      limit 100
    ) b
    order by lvrval desc limit 1;
  ELSE
    if (@debug=1)
    THEN
      select 'found by zip code' msg;
      SELECT
        strasse,
        plz,
        ort,
        LEVENSHTEIN_RATIO(in_short_address,adr) lvrval
      FROM (
        SELECT
          ocrhash_complex.id,
          ocrhash_complex.strasse,
          ocrhash_complex.plz,
          ocrhash_complex.ort,
          ocrhash_complex.adr,
          match(adr) against(in_short_address) as rel

        FROM ocrhash_complex
        WHERE plz = in_zipcode
        having rel>0
        limit 100
      ) b
      order by lvrval desc limit 100;
    END IF;
  END IF;


  if (@debug=1)
  THEN
    select 'PLZ' plz,in_zipcode msg;
    select 'PLZ out' plz,out_plz msg;
  END IF;





  IF EXISTS(select * from `short_boxes_locked` where zipcode = ifnull(out_plz,in_zipcode) and kundenid=in_kundenid)
  THEN

    SET out_sortiergang = 'DPAG';
    SET out_sortierfach = 'DPAG';
    CALL SET_SV
    (
      in_code,
      out_sortiergang,
      out_sortierfach,
      '',
      '',
      ifnull(out_plz,in_zipcode),
      '',
      in_short_address,
      in_kundenid,
      in_product
    );
  ELSE

    IF EXISTS(select * from `short_boxes_locked_by_product` where zipcode = ifnull(out_plz,in_zipcode) and product=in_product)
    THEN


      IF (@debug=1) THEN
        select 'short_boxes_locked_by_product' msg;
      END IF;

      SET out_sortiergang = 'DPAG';
      SET out_sortierfach = 'DPAG';
      CALL SET_SV
      (
        in_code,
        out_sortiergang,
        out_sortierfach,
        '',
        '',
        ifnull(out_plz,in_zipcode),
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
        into out_sortiergang,out_sortierfach
      from
        sortiergaenge_zuordnung
        join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=ifnull(out_plz,in_zipcode)
        join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
      limit 1;


      if (@debug=1)
      THEN
        select out_sortiergang msgsg,out_sortierfach msg;
      END IF;

        CALL SET_SV
        (
          in_code,
          out_sortiergang,
          out_sortierfach,
          '',
          '',
          ifnull(out_plz,in_zipcode),
          '',
          in_short_address,
          in_kundenid,
          in_product
        );


        if (@debug=1)
        THEN
          select out_sortiergang msgsg,out_sortierfach,ifnull(out_plz,in_zipcode);
        END IF;

    ELSE

      SET out_sortiergang = 'NT';
      SET out_sortierfach = 'NT';

      CALL SET_SV
      (
        in_code,
        out_sortiergang,
        out_sortierfach,
        '',
        '',
        ifnull(out_plz,in_zipcode),
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


SET @svmodell='Clearing';
SET @debug=1;
CALL SET_SORTBOX('70191','70191','41','','','TEST', @stortiergang, @stortierfach, @strasse, @plz, @ort);
SELECT @stortiergang,@stortierfach,@str,@plz,@ort;
