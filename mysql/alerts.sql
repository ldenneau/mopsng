drop table if exists alerts;
CREATE TABLE `export`.`alerts` (
  `alert_id` bigint(20) NOT NULL,
  `rule` varchar(128) NOT NULL,
  `channel` varchar(128) NOT NULL,
  `status` varchar(1) NOT NULL,
  `pub_timestamp` datetime default NULL,
  `subject` varchar(140) default NULL,
  `message` mediumtext default NULL,
  `vo_event` mediumtext default NULL,
  PRIMARY KEY  (`alert_id`,`rule`,`channel`),
  KEY `Alert Id` (`alert_id`),
  CONSTRAINT `Alert Id` FOREIGN KEY (`alert_id`) REFERENCES `aqueue` (`alert_id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB COMMENT='Repository of all alerts that have matched a rule.';