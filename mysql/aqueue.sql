drop table if exists export.aqueue;
CREATE TABLE `export`.`aqueue` (
  `alert_id` bigint(20) NOT NULL auto_increment,
  `object_id` bigint(20) NOT NULL,
  `object_type` varchar(128) NOT NULL,
  `nn` int(11) NOT NULL,
  `db_name` varchar(255) NOT NULL,
  `status` char(1) NOT NULL,
  `classification` char(1) NOT NULL,
  `insert_timestamp` timestamp NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`alert_id`),
  UNIQUE KEY `Unique` (`object_id`,`object_type`,`db_name`),
  KEY `Status` (`status`),
  KEY `Object_Type` (`object_type`)
) ENGINE=InnoDB AUTO_INCREMENT=1 COMMENT='Repository of tracklets and derived objects.';
