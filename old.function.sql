
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
    IF EXISTS(select * from bereiche_plz join bereiche on bereiche.alleplz=1 and  bereiche_plz.plz=in_zipcode and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung')
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
