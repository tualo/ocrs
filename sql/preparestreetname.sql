DELIMITER //

DROP  FUNCTION `preparestreetname` //
CREATE  FUNCTION `preparestreetname`(
  street VARCHAR(255)
) RETURNS VARCHAR(255)
READS SQL DATA
BEGIN

  DECLARE output varchar(255);
  DECLARE searchname varchar(255);
  SET searchname = TRIM(street);
  SET searchname = REPLACE(searchname,'strasse','');
  SET searchname = REPLACE(searchname,'straße','');
  SET searchname = REPLACE(searchname,'str.','');
  SET searchname = REPLACE(searchname,'str','');
  RETURN searchname;
END //

DELIMITER ;
