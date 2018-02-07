
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

  DECLARE force_customer_by_id varchar(255);
  DECLARE force_product_by_id varchar(255);

  SET @adr=replace(in_short_address,'Postfach','');

  IF in_kundenid is null or in_kundenid='' THEN
    select concat(kundennummer,'|',kostenstelle) into in_kundenid from bbs_data where id = in_code;
  END IF;

  select name into force_customer_by_id from sv_registered_code_customer where id = in_code;
  select name into force_product_by_id from sv_registered_code_product where id = in_code;
  IF force_customer_by_id is not null THEN
    SET in_kundenid = force_customer_by_id;
  END IF;
  IF force_product_by_id is not null THEN
    SET in_product = force_product_by_id;
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
        LEVENSHTEIN_RATIO(in_short_address,adr) lvrval,
        LEVENSHTEIN_RATIO(in_zipcode,plz) lvrval_in_zipcode
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
      having lvrval_in_zipcode<>0
      order by lvrval_in_zipcode desc,lvrval desc limit 10;
    END IF;

    SELECT
      strasse,
      plz,
      ort,
      LEVENSHTEIN_RATIO(in_short_address,adr) lvrval,
      LEVENSHTEIN_RATIO(in_zipcode,plz) lvrval_in_zipcode
    INTO out_strasse,out_plz,out_ort,@lvr,@lvrzc

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
    having lvrval_in_zipcode<>0
    order by lvrval_in_zipcode desc,lvrval desc limit 1;

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


    IF EXISTS(select * from bereiche_plz join bereiche on bereiche.alleplz=1 and  bereiche_plz.plz= ifnull(out_plz,in_zipcode) and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung')
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

CALL SET_SORTBOX('Steueißéöollmächtigte g 92 07352 Bad Lobcnstein1','07352',92,'1234','test','2900000007',@sg,@sf,@str,@plz,@ort); select @sg,@sf,@str,@plz,@ort;
select * from sv_daten where id = '2900000007'\G
