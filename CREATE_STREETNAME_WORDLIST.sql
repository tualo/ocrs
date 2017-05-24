
DELIMITER $$
DROP PROCEDURE CREATE_STREETNAME_WORDLIST $$
CREATE PROCEDURE CREATE_STREETNAME_WORDLIST ( )
BEGIN

  DECLARE done INT DEFAULT FALSE;
  DECLARE useid VARCHAR(100);

  DECLARE curs CURSOR FOR
   select id from strassenverzeichnis limit 100000;

  DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;


  DROP TABLE IF EXISTS `streetnames_wordlist`;
  CREATE TABLE `streetnames_wordlist` (
    word varchar(50) not null primary key
  );

  DROP TABLE IF EXISTS `streetnames_wordlist_street`;
  CREATE TABLE `streetnames_wordlist_street` (
    word varchar(50) not null,
    streetid varchar(50) not null,
    primary key (word,streetid)
  );



  OPEN curs;

  read_loop: LOOP

    FETCH curs INTO useid;

    IF done THEN
      LEAVE read_loop;
    END IF;

    select strasse into @strasse from strassenverzeichnis where id = useid;
    CALL SPLIT_STREET_INTO_ROWS(@strasse,@tablename);


    SET @s = CONCAT('insert into streetnames_wordlist (word) select word from ',@tablename, ' on duplicate key update word=values(word) ' );
    PREPARE stmt FROM @s;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    SET @s = CONCAT('insert into streetnames_wordlist_street (word,streetid) select word,\'',useid,'\' streetid from ',@tablename, ' on duplicate key update word=values(word) ' );
    PREPARE stmt FROM @s;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    SET @s = CONCAT('DROP TABLE ',@tablename, ' ' );
    PREPARE stmt FROM @s;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

  END LOOP;
  CLOSE curs;


END $$

DELIMITER ;

CALL CREATE_STREETNAME_WORDLIST ( );

select word,count(*) c from streetnames_wordlist_street group by word order by c



DROP TABLE  IF EXISTS ocrword_hash;
create table ocrword_hash ( word varchar(100) ) engine myisam;
insert into ocrword_hash (word)
  select word
  from streetnames_wordlist;
create fulltext index id_ft_ocrword_hash on ocrword_hash(word);


select
word,
match(word) against('Ammenalweg' IN BOOLEAN MODE) as rel
from ocrword_hash order by rel limit 10;



select
  word,
  count(   ) 
from streetnames_wordlist where word like '%a%' or word like '%m%' or word like '%e%' or word like '%l%' or word like '%w%' or word like '%g%' or word like '%n%'  limit 10;
