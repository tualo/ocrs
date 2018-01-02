
DELIMITER //

DROP  FUNCTION `getUpdateSVState` //
CREATE  FUNCTION `getUpdateSVState`()
RETURNS VARCHAR(5)
READS SQL DATA
BEGIN
  RETURN '99';
END //

DROP  FUNCTION `getInitialSVState` //
CREATE  FUNCTION `getInitialSVState`()
RETURNS VARCHAR(5)
READS SQL DATA
BEGIN
  RETURN '99';
END //


DROP PROCEDURE `SET_SV` //
CREATE PROCEDURE `SET_SV`(
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
  set @use_sessionuser=@sessionuser;

  IF EXISTS(SELECT * FROM BBS_DATA WHERE ID=in_code)
  THEN
    SELECT substring(inserttime,1,10),substring(inserttime,11,19) into @datum,@zeit FROM BBS_DATA WHERE ID=in_code;
  END IF;

  IF @use_sessionuser IS NULL THEN
    set @use_sessionuser='NOT SET';
  END IF;

  IF @svmodell IS NULL THEN
    set @svmodell='NOT SET';
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


  IF (@debug=1) THEN
    select concat('modell',@svmodell) msg;
    select concat('in_sortiergang',in_sortiergang) msg;
    select concat('in_sortierfach',in_sortierfach) msg;
    select concat('in_plz',ifnull(in_plz,' NULL')) msg;
    select concat('in_kunde',ifnull(in_kunde,' NULL')) msg;
    select concat('in_product',ifnull(in_product,' NULL')) msg;
  END IF;

  IF @debug=1 THEN

    select concat('nichts wird gespeichert! DEBUG MODE') msg;

  ELSE

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
      PRODUKT=in_product,

      LOGIN =  @use_sessionuser

    WHERE ID = in_code;


    INSERT INTO SV_STATI (status,id,datum,zeit,login)
    VALUES (
      getUpdateSVState(),
      in_code,
      CURRENT_DATE,
      CURRENT_TIME,
      @sessionuser
    );

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
        PRODUKT,

        LOGIN

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
        in_product,

        @use_sessionuser

      );



      INSERT INTO SV_STATI (status,id,datum,zeit,login)
      VALUES (
        getInitialSVState(),
        in_code,
        CURRENT_DATE,
        CURRENT_TIME,
        @sessionuser
      );

      END IF;


    INSERT INTO SV_DATEN_FELDER (
      MANDANT,
      MODELL,
      NAME,
      ID,
      WERT,
      DATUM,
      ZEIT,
      LOGIN
    ) values (
      '0000',
      @svmodell,
      'Barcode',
      in_code,
      in_code,
      @datum,
      @zeit,
      @sessionuser
    ),
    (
      '0000',
      @svmodell,
      'Ort',
      in_code,
      in_ort,
      @datum,
      @zeit,
      @sessionuser
    ),
    (
      '0000',
      @svmodell,
      'PLZ',
      in_code,
      in_plz,
      @datum,
      @zeit,
      @sessionuser
    ),
    (
      '0000',
      @svmodell,
      'Strasse',
      in_code,
      in_strasse,
      @datum,
      @zeit,
      @sessionuser
    ),
    (
      '0000',
      @svmodell,
      'Hausnummer',
      in_code,
      in_hausnummer,
      @datum,
      @zeit,
      @sessionuser
    ),
    (
      '0000',
      @svmodell,
      'Sortierfach',
      in_code,
      in_sortierfach,
      @datum,
      @zeit,
      @sessionuser
    ),
    (
      '0000',
      @svmodell,
      'Sortiergang',
      in_code,
      in_sortiergang,
      @datum,
      @zeit,
      @sessionuser
    )
    on duplicate key update wert=values(wert)
    ;
    END IF;

END //

DELIMITER ;
