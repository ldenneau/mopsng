drop table if exists export.alert_users;
CREATE TABLE `export`.`alert_users` (
  `user_id` bigint NOT NULL auto_increment      comment 'Auto-assigned internal MOPS alert user ID',
  `first_name` varchar(128)                     comment 'First name of alert recipient',
  `last_name` varchar(128)                      comment 'Last name of alert recipient',
  `email_addr` varchar(128) NOT NULL            comment 'Email address of alert recipient',
  `status` varchar(1) NOT NULL DEFAULT 'A'      comment 'Status can be A (active), or I (inactive)',
  PRIMARY KEY  (`user_id`),
  UNIQUE (`email_addr`),
  CONSTRAINT chk_status CHECK (status = 'A' OR status = 'I')
) ENGINE=InnoDB COMMENT='Repository of all MOPS email alert recipients.';