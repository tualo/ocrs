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
        join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung';

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
