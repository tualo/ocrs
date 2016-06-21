DELIMITER $$

DROP PROCEDURE IF EXISTS  `SET_SORTBOX` $$
CREATE PROCEDURE `SET_SORTBOX` (
  in_short_address varchar(4000),
  in_zipcode varchar(255),
  in_housenumber varchar(255),
  in_kundenid varchar(255),
  in_product varchar(255),
  in_code varchar(20)
)
BEGIN

IF EXISTS(select * from `short_boxes_locked` where zipcode = in_zipcode and kundenid=in_kundenid)
THEN
  select 'DPAG';
ELSE
  IF EXISTS(select * from `short_boxes_locked_by_product` where zipcode = in_zipcode and product=in_kundenid)
  THEN
    select 'DPAG';
  ELSE
    IF EXISTS(select * from bereiche_plz join bereiche on bereiche.alleplz=1 and  bereiche_plz.plz=in_zipcode and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung')
    THEN

      IF EXISTS(select * from sv_daten where id = in_code)
        UPDATE sv_daten SET sortiergang =  (select ifnull(sortiergaenge_zuordnung.sortiergang,'NT') from
            sortiergaenge_zuordnung
            join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=in_zipcode
            join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
          ),
          sortierfach = (select ifnull(sortiergaenge_zuordnung.sortierfach,'NT') from
            sortiergaenge_zuordnung
            join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=in_zipcode
            join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
          )
        WHERE id = in_code;

      ELSE

        INSERT INTO sv_daten (
          mandant,
          modell,
          id,
          datum,
          zeit,
          login,
          sortiergang,
          sortierfach,
          strasse,
          hausnummer,
          plz,
          ort,
          width,
          height
        ) values (
          '0000',
          'Zustellung',
          in_code,
          now(),
          now(),
          'sorter',
          (select ifnull(sortiergaenge_zuordnung.sortiergang,'NT') from
            sortiergaenge_zuordnung
            join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=in_zipcode
            join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
          ),
          (select ifnull(sortiergaenge_zuordnung.sortierfach,'NT') from
            sortiergaenge_zuordnung
            join bereiche_plz on (sortiergaenge_zuordnung.bereich,sortiergaenge_zuordnung.mandant,sortiergaenge_zuordnung.regiogruppe) = (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) and bereiche_plz.plz=in_zipcode
            join bereiche on bereiche.alleplz=1 and (bereiche_plz.name,bereiche_plz.mandant,bereiche_plz.regiogruppe) = (bereiche.name,bereiche.mandant,bereiche.regiogruppe) and bereiche.mandant='0000' and bereiche.regiogruppe = 'Zustellung'
          ),
          '',
          '',
          '',
          '',
          -1,
          -1
        );
      END IF;
    END IF;

END
$$

DELIMITER ;

CALL SET_SORTBOX('Johannestr. 156 99084 Erfurt','99084','156','100000|0','Standardsendungen','12345');
