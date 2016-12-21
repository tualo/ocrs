
DELIMITER //
DROP PROCEDURE `BBS_CHECK_OCR_ID` //
CREATE PROCEDURE `BBS_CHECK_OCR_ID`(
  IN in_machine varchar(3),
  IN in_state varchar(255),
  IN in_note varchar(255),
  IN in_id varchar(100)
)
    MODIFIES SQL DATA
BEGIN
  call BBS_CHECK_OCR(in_machine,in_state,in_note);
  UPDATE bbs_data SET ocr_state = in_state, ocr_time=now() WHERE id = in_id and ocr_state='';
END
//

DROP PROCEDURE `BBS_CHECK_OCR` //
CREATE PROCEDURE `BBS_CHECK_OCR`(
  IN in_machine varchar(3),
  IN in_state varchar(255),
  IN in_note varchar(255)
)
    MODIFIES SQL DATA
BEGIN

  DELETE FROM `bbs_check_data`
    WHERE
      `machine`=in_machine and
      `state`=in_state and
      `createtime`<= DATE_ADD(NOW(),INTERVAL -24 DAY_HOUR)
    ;

  INSERT INTO
  `bbs_check_data`
  (`machine`,`state`,`note`) VALUES
  (in_machine,in_state,in_note);

  INSERT INTO
  `bbs_status`
  (`machine`,`last_contact`,`last_scan`) VALUES
  (in_machine,now(),now())
  ON DUPLICATE KEY
  UPDATE
    `last_contact`=values(`last_contact`),
    `last_scan`=values(`last_scan`);

  CALL BBS_CHECK_OCR_UPDATE(in_machine);
END
//
DELIMITER ;
