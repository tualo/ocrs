create table if not exists cleanstreetname_mapping_table (
  street varchar(80),
  street_written varchar(80),
  primary key(street_written)
);

DELIMITER //

DROP  FUNCTION `cleanstreetname_lv_slowest` //
CREATE  FUNCTION `cleanstreetname_lv_slowest`(
  in_street VARCHAR(255)
) RETURNS VARCHAR(255)
READS SQL DATA
BEGIN
  DECLARE output varchar(255);
  DECLARE lval INTEGER;
  SELECT
    strasse,
    LEVENSHTEIN_RATIO(in_street,strasse) lvrval
  INTO
    output,
    lval
  FROM (
    SELECT
      strasse,
      match(adr) against(in_street) as rel
    FROM faststreethash_complex
    order by rel desc
  ) b
  order by lvrval desc limit 1;

  RETURN output;
END //

DROP  FUNCTION `cleanstreetname` //
CREATE  FUNCTION `cleanstreetname`(
  in_street VARCHAR(255)
) RETURNS VARCHAR(255)
READS SQL DATA
BEGIN
  DECLARE output varchar(255);
  DECLARE preparedsearchname varchar(255);

  DECLARE preparedsearchname_like varchar(255);
  DECLARE lval INTEGER;

  SET preparedsearchname = trim(replace_all(in_street,'0123456789',''));
  SET preparedsearchname_like = concat('%',concat(preparedsearchname,'%'));

  SELECT
    street
  INTO
    output
  FROM
    cleanstreetname_mapping_table
  WHERE
    street_written=trim(in_street);


  IF output IS NULL THEN
    SELECT
      strasse,
      LEVENSHTEIN_RATIO(in_street,strasse) lvrval
    INTO
      output,
      lval
    FROM (
      SELECT
       strasse
      FROM
       faststreethash_complex
      WHERE strasse like preparedsearchname_like
    ) b
    ORDER BY lvrval DESC LIMIT 1;
  END IF;


  IF output is null THEN
    SET output = cleanstreetname_lv_slowest(preparedsearchname);

    INSERT INTO cleanstreetname_mapping_table(street,street_written) values
    (output,trim(in_street))
    on duplicate key update street=values(street);

  END IF;

  RETURN output;
END //
DELIMITER ;

select cleanstreetname('  Fuschersand') t;
