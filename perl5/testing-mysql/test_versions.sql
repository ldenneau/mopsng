CREATE TABLE `test_versions` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `test_run_id` bigint(20) unsigned NOT NULL default '0',
  `module` varchar(255) NOT NULL default '',
  `module_md5` varchar(128) default NULL,
  `cvs_version` varchar(16) NOT NULL default '',
  `cvs_time` datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`id`),
  KEY `test_run_id` (`test_run_id`),
  CONSTRAINT `test_versions_ibfk_1` FOREIGN KEY (`test_run_id`) REFERENCES `test_runs` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
