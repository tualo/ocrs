DELIMITER //
DROP  FUNCTION `STRIP_NON_DIGIT` //
CREATE  FUNCTION `STRIP_NON_DIGIT`(input VARCHAR(255)) RETURNS VARCHAR(255)
READS SQL DATA
BEGIN
   DECLARE output    VARCHAR(255) DEFAULT '';
   DECLARE iterator  INT          DEFAULT 1;
   DECLARE lastDigit INT          DEFAULT 1;
   DECLARE len       INT;

   SET len = LENGTH(input) + 1;
   WHILE iterator < len DO
      -- skip past all digits
      SET lastDigit = iterator;
      WHILE ORD(SUBSTRING(input, iterator, 1)) BETWEEN 48 AND 57 AND iterator < len DO
         SET iterator = iterator + 1;
      END WHILE;

      IF iterator != lastDigit THEN
         SET output = CONCAT(output, SUBSTRING(input, lastDigit, iterator - lastDigit));
      END IF;

      WHILE ORD(SUBSTRING(input, iterator, 1)) NOT BETWEEN 48 AND 57 AND iterator < len DO
         SET iterator = iterator + 1;
      END WHILE;
   END WHILE;

   RETURN output;
END //

DROP  FUNCTION `STRIP_NON_ALPHANUMERIC` //
CREATE  FUNCTION `STRIP_NON_ALPHANUMERIC`(input VARCHAR(255)) RETURNS VARCHAR(255)
READS SQL DATA
BEGIN
   DECLARE output    VARCHAR(255) DEFAULT '';
   DECLARE iterator  INT          DEFAULT 1;
   DECLARE lastDigit INT          DEFAULT 1;
   DECLARE len       INT;

   SET len = LENGTH(input) + 1;
   WHILE iterator < len DO
      -- skip past all digits
      SET lastDigit = iterator;
      IF
        (
          ORD(LOWER(SUBSTRING(input, iterator, 1))) BETWEEN 48 AND 57 or
          ORD(LOWER(SUBSTRING(input, iterator, 1))) BETWEEN 97 AND 122
        )
      THEN
        SET output = CONCAT(output, SUBSTRING(input, iterator,1));
      ELSE
        SET output = CONCAT(output, '*');
      END IF;
      SET iterator = iterator + 1;
   END WHILE;

   RETURN output;
END //

select STRIP_NON_ALPHANUMERIC('test dfhgh -f') //
DELIMITER ;
