
ALTER TABLE quicksv_table ADD match_street varchar(80);
DELIMITER //

DROP PROCEDURE `loop_clear_street` //
CREATE PROCEDURE `loop_clear_street`(
  IN in_zipcode varchar(10)
)
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
    quicksv_table.match_street is null
    and quicksv_table.ocrtext<>''
    and quicksv_table.zipcode = in_zipcode;

  DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;
  OPEN curs;

  read_loop: LOOP

    FETCH curs INTO useid;

    IF done THEN
      LEAVE read_loop;
    END IF;

    select
      streetname
    into
      suche
    from
      quicksv_table
    where
      code = useid;

    update quicksv_table set match_street=cleanstreetname(suche) where code=useid;


  END LOOP;
  CLOSE curs;

END //
DELIMITER ;
