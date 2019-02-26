DELIMITER $$
DROP FUNCTION numbertoletter$$
CREATE FUNCTION numbertoletter(in_val varchar(255))
  RETURNS TEXT
  LANGUAGE SQL
  DETERMINISTIC
BEGIN
SET in_val = replace(in_val,'1','i');
SET in_val = replace(in_val,'7','i');
SET in_val = replace(in_val,'0','o');

SET in_val = replace(in_val,'9',' ');
SET in_val = replace(in_val,'2',' ');
SET in_val = replace(in_val,'3',' ');
SET in_val = replace(in_val,'4',' ');
SET in_val = replace(in_val,'5',' ');
SET in_val = replace(in_val,'6',' ');
SET in_val = replace(in_val,'8',' ');

  RETURN in_val;
END;
$$

DROP FUNCTION lettertonumber$$
CREATE FUNCTION lettertonumber(in_val varchar(255))
  RETURNS TEXT
  LANGUAGE SQL
  DETERMINISTIC
BEGIN

SET in_val = replace(in_val,'i','1');
SET in_val = replace(in_val,'I','1');
SET in_val = replace(in_val,'L','1');
SET in_val = replace(in_val,'|','1');
SET in_val = replace(in_val,'J','1');
SET in_val = replace(in_val,'A','4');

  RETURN in_val;
END;
$$

alter table quicksv_table add match_town varchar(100) $$
alter table quicksv_table add match_zipcode varchar(10) $$
alter table ocrs_statistics add machine varchar(10) default ''$$


DROP TABLE  IF EXISTS ocrhash_complex_town $$
create table ocrhash_complex_town (id varchar(32) primary key, date_added timestamp ,adr text, plz varchar(255),ort varchar(255) ) engine myisam $$
insert into ocrhash_complex_town (id,date_added,plz,ort,adr)
  select concat('OWN',plz,ort) id, UNIX_TIMESTAMP(now()),plz,ort, concat(plz,' ',ort) adr
  from plz_orte where gesondert=0
on duplicate key update ort=values(ort) $$
create fulltext index id_ft_hash_complex_town on ocrhash_complex_town(adr) $$


DROP PROCEDURE `SET_QUICK_TABLE_SV` $$
DROP PROCEDURE `SET_QUICK_TABLE_SV_LOOP` $$

CREATE PROCEDURE `SET_QUICK_TABLE_SV_LOOP`()
    MODIFIES SQL DATA
BEGIN
  DECLARE lvr INT;

 DECLARE done INT DEFAULT FALSE;
 DECLARE useid VARCHAR(100);
 DECLARE suche VARCHAR(255);

 DECLARE curs CURSOR FOR
  select
    quicksv_table.code
  from
    quicksv_table
  where
    quicksv_table.match_town is null
    and quicksv_table.ocrtext<>''
    and createtime>=date_add(now(),interval -1 day);

  DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;
  OPEN curs;

  read_loop: LOOP

    FETCH curs INTO useid;

    IF done THEN
      LEAVE read_loop;
    END IF;

    CALL SET_QUICK_TABLE_SV(useid);

  END LOOP;
  CLOSE curs;

END $$



CREATE PROCEDURE `SET_QUICK_TABLE_SV`(in_code varchar(20))
    MODIFIES SQL DATA
BEGIN
  DECLARE qplz varchar(10);
  DECLARE qort varchar(200);
  DECLARE found_plz varchar(10);
  DECLARE found_ort varchar(200);
  DECLARE found_adr varchar(200);
  DECLARE q varchar(255);
  DECLARE found_rel fixed(15,6);

  SELECT zipcode,town INTO qplz,qort FROM quicksv_table where code = in_code;

  SET q =concat(lettertonumber(qplz),concat(' ',numbertoletter(qort) ));
  SELECT
      b.plz,
      b.ort,
      LEVENSHTEIN_RATIO(q,adr) lvrval
  INTO
    found_plz,
    found_ort,
    found_rel
  FROM
    (
    SELECT
      ocrhash_complex_town.plz,
      ocrhash_complex_town.ort,
      ocrhash_complex_town.adr,
      match(adr) against(q) as rel
    FROM ocrhash_complex_town
    HAVING rel>0
    ORDER BY rel desc
    LIMIT 20

    ) b
  order by lvrval desc limit 1;

  update quicksv_table
    set
    match_town=ifnull(found_ort,'-----'),
    match_zipcode=ifnull(found_plz,'-----')
  where code = in_code;

  if found_plz is not null THEN
    select
        concat(match_zipcode,' ',match_town),
        match_zipcode,
        housenumber,
        concat( customerno,'|',costcenter ),
        '',
        code
    into     
        @in_short_address,
        @in_zipcode,
        @in_housenumber,
        @in_kundenid,
        @in_product,
        @in_code
    from quicksv_table where code = in_code;
    select 
      @in_short_address,
      @in_zipcode,
      @in_housenumber,
      @in_kundenid,
      @in_product,
      @in_code;

    call SET_SORTBOX(
      @in_short_address,
      @in_zipcode,
      @in_housenumber,
      @in_kundenid,
      @in_product,
      @in_code,


      @out_sortiergang,
      @out_sortierfach,
      @out_strasse,
      @out_plz,
      @out_ort
    );
  END IF;

END $$
DELIMITER ;

call SET_QUICK_TABLE_SV_LOOP();
