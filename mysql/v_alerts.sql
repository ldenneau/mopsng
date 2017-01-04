CREATE VIEW `export`.`v_alerts` AS 
select `a`.`alert_id` AS `alert_id`,`a`.`rule` AS `rule`,
`a`.`channel` AS `channel`,`a`.`status` AS `status`,
`a`.`pub_timestamp` AS `pub_timestamp`, `a`.`message` AS `message`,
`a`.`subject` AS `subject`, `b`.`object_id` AS `object_id`,
`b`.`object_type` AS `object_type`,`b`.`db_name` AS `db_name`,
`b`.`classification` AS `classification` 
from (`export`.`alerts` `a` join `export`.`aqueue` `b` 
on((`a`.`alert_id` = `b`.`alert_id`)));