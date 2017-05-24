DELIMITER $$

CREATE FUNCTION strSplit(x VARCHAR(65000), delim VARCHAR(12), pos INTEGER)
RETURNS VARCHAR(65000)
BEGIN
  DECLARE output VARCHAR(65000);
  SET output = REPLACE(SUBSTRING(SUBSTRING_INDEX(x, delim, pos)
                 , LENGTH(SUBSTRING_INDEX(x, delim, pos - 1)) + 1)
                 , delim
                 , '');
  IF output = '' THEN SET output = null; END IF;
  RETURN output;
END $$
DELIMITER ;





DELIMITER $$
DROP PROCEDURE SPLIT_INTO_ROWS $$
CREATE PROCEDURE SPLIT_INTO_ROWS(
  IN in_string varchar(4000),
  OUT out_tablename varchar(32)
)
BEGIN
  DECLARE i INTEGER;


  SET @TABLE_NAME = concat('TEMP_SPLIT_',left(UUID(),8));

  SET @s = CONCAT('DROP TABLE IF EXISTS  ',@TABLE_NAME );
  PREPARE stmt FROM @s;
  EXECUTE stmt;
  DEALLOCATE PREPARE stmt;

  SET @s = CONCAT('CREATE TABLE  ',@TABLE_NAME, ' (word varchar(50)) engine = MEMORY' );
  PREPARE stmt FROM @s;
  EXECUTE stmt;
  DEALLOCATE PREPARE stmt;


  SET i = 1;
  REPEAT

    SET @s = CONCAT('INSERT INTO ',@TABLE_NAME, ' select str from ( select strSplit(\'',in_string,'\',\' \',',i,') str ) a where  str is not null ' );
    PREPARE stmt FROM @s;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    SET i = i + 1;
    UNTIL i >= 10
  END REPEAT;

  set out_tablename=@TABLE_NAME;
END $$

DELIMITER ;
call SPLIT_INTO_ROWS('sample test me now',@tablename);


DELIMITER $$
DROP PROCEDURE SPLIT_STREET_INTO_ROWS $$
CREATE PROCEDURE SPLIT_STREET_INTO_ROWS(
  IN input varchar(4000),
  OUT out_tablename varchar(32)
)
BEGIN
  set @street = trim(input);
  set @street =  LOWER(@street)   ;
  set @street =  REPLACE(@street, 'str.','str')   ;
  set @street =  REPLACE(@street, 'ü','u');
  set @street =  REPLACE(@street, 'ö','o');
  set @street =  REPLACE(@street, 'ä','a');
  set @street =  REPLACE(@street, 'ß','ss');

  set @street =  REPLACE(@street, 'è','e');
  set @street =  REPLACE(@street, 'é','e');

  set @street =  REPLACE(@street, 'á','a');
  set @street =  REPLACE(@street, 'à','a');

  set @street = STRIP_NON_ALPHANUMERIC( @street  ) ;
  set @street = REPLACE(@street, '**',' ');
  set @street = REPLACE(@street, '*',' ');

  CALL SPLIT_INTO_ROWS(@street,out_tablename);

END $$

DELIMITER ;
