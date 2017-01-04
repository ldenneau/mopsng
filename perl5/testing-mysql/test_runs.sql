CREATE TABLE `test_runs` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00' COMMENT 'Time that test results were reported.',
  `user` varchar(32) default NULL COMMENT 'Username running the tests.',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
