DELIMITER //
DROP PROCEDURE SET_SORTBOX_EXT //
CREATE PROCEDURE SET_SORTBOX_EXT
  (
    IN in_short_address varchar(4000),
    IN in_zipcode varchar(255),
    IN in_town varchar(255),
    IN in_street varchar(255),
    IN in_housenumber varchar(255),
    IN in_kundenid varchar(255),
    IN in_product varchar(255),
    IN in_code varchar(20),
    IN in_codes varchar(255),


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

  IF (@debug=1) THEN
   select concat(in_zipcode,' ',in_town);
  END IF;

  IF (@regiogruppe is null) THEN
    SET @regiogruppe='Zustellung';
  END IF;

  IF (@use_fast_access_tour is null) THEN
    SET @use_fast_access_tour=0;
  END IF;

  set @zipcode = STRIP_NON_ALPHANUMERIC(in_zipcode );
  set @town = STRIP_NON_ALPHANUMERIC(in_town );

  set @street = PERPARE_STREETNAME(in_street);
  select in_street,@street txxxxxxxt;

  set @hn_number = STRIP_NON_DIGIT(in_housenumber );


  set @format_hn_number = @hn_number;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;


  SELECT
    plz,
    ort,
    LEVENSHTEIN_RATIO(concat(@zipcode,' ',@town),adr) lvrval,
    LEVENSHTEIN(concat(@zipcode,' ',@town),adr) lv
  INTO out_plz,out_ort,@lvr,@lvx

  FROM (

    SELECT
      ocrtownhash_complex.id,
      ocrtownhash_complex.plz,
      ocrtownhash_complex.ort,
      ocrtownhash_complex.adr,
      match(adr) against(concat(@zipcode,' ',@town) IN BOOLEAN MODE) as rel

    FROM ocrtownhash_complex
    WHERE plz = in_zipcode
    having rel>0
    limit 100
  ) b
  order by lv desc,lvrval desc limit 1;

  IF (out_plz is null)
  THEN
    if (@debug=1)
    THEN
      select 'try search all' msg;
      SELECT
        plz,
        ort,
        LEVENSHTEIN_RATIO(concat(in_zipcode,' ',in_town),adr) lvrval,
        LEVENSHTEIN_RATIO(in_zipcode,plz) lvrval_in_zipcode
      FROM (

        SELECT
          ocrtownhash_complex.id,
          ocrtownhash_complex.plz,
          ocrtownhash_complex.ort,
          ocrtownhash_complex.adr,
          match(adr) against(concat(@zipcode,' ',@town) in BOOLEAN MODE) as rel

        FROM ocrtownhash_complex
        having rel>0
        limit 100
      ) b
      having lvrval_in_zipcode<>0
      order by lvrval_in_zipcode desc,lvrval desc limit 10;
    END IF;



    SELECT
      plz,
      ort,
      LEVENSHTEIN_RATIO(concat(in_zipcode,' ',in_town),adr) lvrval,
      LEVENSHTEIN_RATIO(in_zipcode,plz) lvrval_in_zipcode
    INTO out_plz,out_ort,@lvr,@lvrzc

    FROM (

      SELECT
        ocrtownhash_complex.id,
        ocrtownhash_complex.plz,
        ocrtownhash_complex.ort,
        ocrtownhash_complex.adr,
        match(adr) against(concat(@zipcode,' ',@town) IN BOOLEAN MODE) as rel

      FROM ocrtownhash_complex
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
        plz,
        ort,
        LEVENSHTEIN_RATIO(concat(in_zipcode,' ',in_town),adr) lvrval,
        LEVENSHTEIN(concat(in_zipcode,' ',in_town),adr) lv
      FROM (
        SELECT
          ocrtownhash_complex.id,
          ocrtownhash_complex.plz,
          ocrtownhash_complex.ort,
          ocrtownhash_complex.adr,
          match(adr) against(concat(@zipcode,' ',@town) IN BOOLEAN MODE) as rel

        FROM ocrtownhash_complex
        WHERE plz = in_zipcode
        having rel>0
        limit 100
      ) b
      order by lv desc,lvrval desc limit 100;
    END IF;
  END IF;


  SELECT
    strasse,
    LEVENSHTEIN_RATIO(@street,adr) lvrval,
    LEVENSHTEIN(@street,adr) lv
    INTO
     out_strasse,
     @lvr,
     @lv
  FROM (
    SELECT
      ocrstreethash_complex.id,
      ocrstreethash_complex.strasse,
      ocrstreethash_complex.adr,
      match(adr) against(@street IN BOOLEAN MODE) as rel

    FROM ocrstreethash_complex
    having rel>0
    limit 100
  ) b
  order by lv desc, lvrval desc limit 1;
  IF (@debug=1) THEN
    select '@street >> ' info, @street,out_strasse;
  END IF;

  set @calc_address = concat(if(out_strasse is null,replace(in_street,' ',' +'), concat(' +',replace(out_strasse,' ',' +')) ),' ',out_plz,' +',out_ort);


  IF (@debug=1) THEN

    SELECT
      strasse,
      plz,
      ort,
      LEVENSHTEIN_RATIO(@calc_address,adr) lvrval,
      LEVENSHTEIN(@calc_address,adr) lv,
      'against calc_address' info
    FROM (
      SELECT
        ocradrhash_complex.id,
        ocradrhash_complex.strasse,
        ocradrhash_complex.plz,
        ocradrhash_complex.ort,
        ocradrhash_complex.adr,
        match(adr) against(@calc_address IN BOOLEAN MODE) as rel
      FROM ocradrhash_complex
      WHERE plz = out_plz
      having rel>0
      limit 100
    ) b
    order by lv desc,lvrval desc limit 10;

  END IF;

  SELECT
    strasse,
    plz,
    ort,
    LEVENSHTEIN_RATIO(@calc_address,adr) lvrval,
    LEVENSHTEIN(@calc_address,adr) lv
  INTO
    out_strasse,
    out_plz,
    out_ort,
    @lvr,
    @lv
  FROM (
    SELECT
      ocradrhash_complex.id,
      ocradrhash_complex.strasse,
      ocradrhash_complex.plz,
      ocradrhash_complex.ort,
      ocradrhash_complex.adr,
      match(adr) against(@calc_address IN BOOLEAN MODE) as rel

    FROM ocradrhash_complex
    WHERE plz = out_plz
    having rel>0
    limit 100
  ) b
  order by lv desc, lvrval desc limit 1;


  if (@debug=1)
  THEN
    select
      in_zipcode 'in_zipcode',
      in_town 'in_town',
      in_street 'in_street',
      out_plz 'out_plz',
      out_ort 'out_ort',
      out_strasse 'out_strasse',
      @calc_address 'calc_address'
    ;
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
      ifnull(out_strasse,in_street),
      in_housenumber,
      ifnull(out_plz,in_zipcode),
      ifnull(out_ort,in_town),
      in_short_address,
      in_kundenid,
      in_product
    );
    update sv_daten set alt_codes=in_codes where id = in_code;

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
        ifnull(out_strasse,in_street),
        in_housenumber,
        ifnull(out_plz,in_zipcode),
        ifnull(out_ort,in_town),
        in_short_address,
        in_kundenid,
        in_product
      );
      update sv_daten set alt_codes=in_codes where id = in_code;

    ELSE


    IF EXISTS(select * from bereiche_plz join bereiche on bereiche.alleplz=1 and  bereiche_plz.plz= ifnull(out_plz,in_zipcode) and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = @regiogruppe)
    THEN


      select
        ifnull(sortiergaenge_zuordnung.sortiergang,'NT') sortiergang,
        ifnull(sortiergaenge_zuordnung.sortierfach,'NT') sortierfach
        into out_sortiergang,out_sortierfach
      from
        sortiergaenge_zuordnung
        join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=ifnull(out_plz,in_zipcode)
        join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = @regiogruppe
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
          ifnull(out_strasse,in_street),
          in_housenumber,
          ifnull(out_plz,in_zipcode),
          ifnull(out_ort,in_town),
          in_short_address,
          in_kundenid,
          in_product
        );

        update sv_daten set alt_codes=in_codes where id = in_code;

        if (@debug=1)
        THEN
          select out_sortiergang msgsg,out_sortierfach,ifnull(out_plz,in_zipcode);
        END IF;

    ELSE

    IF (@debug=1) THEN
      select 'alleplz ggf. nicht gesetzt!!!' msg;

      select * from
        fast_access_tour
      where
        plz=ifnull(out_plz,in_zipcode)
        and strasse = ifnull(out_strasse,in_street)
        and regiogruppe=@regiogruppe
        and hnvon<=@format_hn_number
        and hnbis>=@format_hn_number
        and ort=out_ort
      limit 10;

    END IF;




      SET out_sortiergang = 'NT';
      SET out_sortierfach = 'NT';




      IF @use_fast_access_tour=1 THEN
        SELECT
          sortiergang,
          sortierfach
        INTO
          out_sortiergang,
          out_sortierfach
        FROM
          fast_access_tour
        WHERE
          plz=ifnull(out_plz,in_zipcode)
          and strasse = ifnull(out_strasse,in_street)
          and regiogruppe=@regiogruppe
          and hnvon<=@format_hn_number
          and hnbis>=@format_hn_number
          and ort=out_ort
        ;
      ELSE
        SELECT
          sortiergaenge_zuordnung.sortiergang,
          sortiergaenge_zuordnung.sortierfach
        INTO
          out_sortiergang,
          out_sortierfach
        FROM
          bereiche_strassen
          join
          strassenverzeichnis
          on bereiche_strassen.id = strassenverzeichnis.id
          join sortiergaenge_zuordnung
          on sortiergaenge_zuordnung.bereich = bereiche_strassen.name
            and sortiergaenge_zuordnung.regiogruppe = bereiche_strassen.regiogruppe
            and sortiergaenge_zuordnung.mandant = bereiche_strassen.mandant
        where
          strassenverzeichnis.strasse=ifnull(out_strasse,in_street)
          and strassenverzeichnis.plz = ifnull(out_plz,in_zipcode)
          and strassenverzeichnis.ort = ifnull(out_ort,in_town)
          and bereiche_strassen.verwenden = 1
          and bereiche_strassen.HNVON<=@hn_number
          and bereiche_strassen.HNBIS>=@hn_number;



      END IF;

      CALL SET_SV
      (
        in_code,
        out_sortiergang,
        out_sortierfach,
        ifnull(out_strasse,in_street),
        in_housenumber,
        ifnull(out_plz,in_zipcode),
        ifnull(out_ort,in_town),
        in_short_address,
        in_kundenid,
        in_product
      );



    END IF;


    END IF;
  END IF;

  update sv_daten set alt_codes=in_codes where id = in_code and length(alt_codes)<length(in_codes);

END;
//


DROP PROCEDURE SET_SV_STATI //
CREATE PROCEDURE SET_SV_STATI
  (
    IN in_id varchar(255),
    IN in_date date,
    IN in_time time,
    IN in_state integer
  )
  MODIFIES SQL DATA
  BEGIN

  set @id = in_id;
  set @lid = concat('%|',in_id,'|%');

  set @login = ifnull(@sessionuser,'variable not set');
  set @datum = in_date;
  set @zeit = in_time;
  set @status = in_state;

  IF EXISTS(select * from sv_daten where alt_codes like @lid) THEN
    select id into @id from sv_daten where alt_codes like @lid;
  END IF;
  IF EXISTS(select * from archiv_sv_daten where alt_codes like @lid) THEN
    select id into @id from archiv_sv_daten where alt_codes like @lid;
  END IF;

  IF EXISTS(select * from sv_daten where id=@id) THEN
    insert into sv_stati
      (id,login,datum,zeit,status)
    values (@id,@login,@datum,@zeit,@status)
    on duplicate key update id=values(id);
  END IF;

  IF EXISTS(select * from archiv_sv_daten where id=@id) THEN
    insert into archiv_sv_stati
      (id,login,datum,zeit,status)
    values (@id,@login,@datum,@zeit,@status)
    on duplicate key update id=values(id);
  END IF;

END;
//
DELIMITER ;
