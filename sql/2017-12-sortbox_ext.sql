


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

  SET @adr=replace(in_short_address,'Postfach','');

  set @hn_number = replace_all(in_housenumber,' abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ','');
  set @zu_hn_number= replace_all(in_housenumber,' 0123456789','');

  set @format_hn_number = @hn_number;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;
  IF LENGTH(@format_hn_number)<4 THEN set @format_hn_number = concat('0',@format_hn_number); END IF;


  -- ist die plz gesperrt für diesen kunde
  IF EXISTS(select * from `short_boxes_locked` where zipcode = in_zipcode and kundenid=in_kundenid)
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
      in_zipcode,
      '',
      in_short_address,
      in_kundenid,
      in_product
    );
  ELSE

  -- ist die plz gesperrt für dieses produkt
    IF EXISTS(select * from `short_boxes_locked_by_product` where zipcode = in_zipcode and product=in_product)
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


      IF EXISTS(select * from bereiche_plz join bereiche on bereiche.alleplz=1 and  bereiche_plz.plz= in_zipcode and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.regiogruppe = 'Zustellung')
      THEN


      IF (@debug=1) THEN
        select 'short_boxes_locked_by_product' msg;
      END IF;

        select
          ifnull(sortiergaenge_zuordnung.sortiergang,'NT') sortiergang,
          ifnull(sortiergaenge_zuordnung.sortierfach,'NT') sortierfach
          into out_sortiergang,out_sortierfach
        from
          sortiergaenge_zuordnung
          join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=ifnull(out_plz,in_zipcode)
          join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe)
           and bereiche.regiogruppe = 'Zustellung'
        limit 1;

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



        select
          plz,
          ort,
          strasse,
          sortiergang,
          sortierfach
        INTO
          out_plz,
          out_ort,
          out_strasse,
          out_sortiergang,
          out_sortierfach

        from
          fast_access_tour
        where
          regiogruppe = 'Zustellung'
          and
          strasse = in_street
          and
          plz = in_zipcode
          and
          hnvon<=@format_hn_number
          and
          hnbis>=@format_hn_number
          and
          zuvon<=@zu_hn_number
          and
          zubis>=@zu_hn_number
          and
          (
            (gerade=1 and @format_hn_number%2=0) or
            (ungerade=1 and @format_hn_number%2=1)
          )

          ;

          if out_sortiergang is null then
            SET out_sortiergang = 'NT';
          end if;
          if out_sortierfach is null then
            SET out_sortierfach = 'NT';
          end if;
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


END;
//




DELIMITER ;


SET @svmodell='Clearing';
SET @debug=1;
CALL SET_SORTBOX_EXT('Anger 1 99084 Erfnrt','99084','Erfnrt','Anger','1','100|0','Standardbrief','TEST','', @stortiergang, @stortierfach, @strasse, @plz, @ort);
SELECT @stortiergang,@stortierfach,@strasse,@plz,@ort;
