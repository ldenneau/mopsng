drop table if exists export.email_alerts;
CREATE TABLE `export`.`email_alerts` (
  `user_id` bigint NOT NULL     comment 'User to send alert to',
  `rule` varchar(128) NOT NULL  comment 'Rule to send email alert for',
  CONSTRAINT `fk_AlertUsers` FOREIGN KEY (`user_id`) REFERENCES `alert_users` (`user_id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB COMMENT='Linking of rules with users that are interested in being alerted when the rule criteria is met';

CREATE INDEX RuleIndex
ON export.email_alerts (rule);