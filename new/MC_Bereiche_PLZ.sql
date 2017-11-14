
select fast_access_tour.plz,count(*) from fast_access_tour
join
 (
select
bereiche_plz.plz,
bereiche_plz.regiogruppe
 from
bereiche join
bereiche_plz on
bereiche.regiogruppe = bereiche_plz.regiogruppe
and bereiche.name = bereiche_plz.name
 where alleplz=0

and bereiche_plz.regiogruppe='Zustellung'
) b
on b.plz=fast_access_tour.plz
and b.regiogruppe=fast_access_tour.regiogruppe

group by plz


DELIMITER $$
DROP  PROCEDURE `quicksv`;
CREATE  PROCEDURE `quicksv`(
  IN in_code varchar(50),
  IN in_zipcode varchar(10),
  IN in_town varchar(255),
  IN in_streetname varchar(255),
  IN in_housenumber varchar(255),
  IN in_sortrow varchar(50),
  IN in_sortbox varchar(50),
  IN in_string varchar(4000),
  IN in_customerno varchar(20),
  IN in_costcenter varchar(20)
)
    MODIFIES SQL DATA
BEGIN
  insert into quicksv_table(
    code,
    zipcode,
    town,
    streetname,
    housenumber,
    sortrow,
    sortbox,
    ocrtext,
    customerno,
    costcenter,
    createtime
  ) values(
    in_code,
    in_zipcode,
    in_town,
    in_streetname,
    in_housenumber,
    in_sortrow,
    in_sortbox,
    in_string,
    in_customerno,
    in_costcenter,
    now()
  )
  on duplicate key update
  zipcode=values(zipcode),
  town=values(town),
  streetname=values(streetname),
  housenumber=values(housenumber),
  sortrow=values(sortrow),
  sortbox=values(sortbox),
  ocrtext=values(ocrtext),
  customerno=values(customerno),
  costcenter=values(costcenter);
  call SET_QUICK_TABLE_SV(in_code);
END $$
DELIMITER ;
