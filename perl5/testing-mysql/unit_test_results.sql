CREATE TABLE `unit_test_results` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `test_run` bigint(20) unsigned NOT NULL default '0' COMMENT 'Matches id in table test_runs.',
  `number` int(10) unsigned NOT NULL default '0',
  `psmops_function` tinyint(4) NOT NULL default '0' COMMENT '1 if a testing a specific PSMOPS function, else 0.',
  `module` varchar(255) NOT NULL default '' COMMENT 'Fully-qualified module name.',
  `cvs_version` varchar(16) NOT NULL default '',
  `cvs_time` datetime default NULL,
  `test` varchar(128) NOT NULL default '',
  `modifier1` varchar(128) default NULL,
  `modifier2` varchar(128) default NULL,
  `result` char(1) default NULL COMMENT '''P'' for pass, ''F'' for fail, ''N'' for not implemented.',
  `wall` float NOT NULL default '0',
  `cpu` float NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `test_run` (`test_run`),
  CONSTRAINT `unit_test_results_ibfk_1` FOREIGN KEY (`test_run`) REFERENCES `test_runs` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
