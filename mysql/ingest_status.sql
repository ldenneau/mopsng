drop table if exists `ingest_status`;
create table `ingest_status`(
    file_id varchar(100) not null           comment 'IPP datastore file ID',
    file_size int not null                  comment 'Size of file in bytes',
    file_type char(12)                      comment 'Datastore file type (usually ipp-mops)',

    ingest_date timestamp not null          comment 'Date of ingest',
    status char(1) default 'N'              comment 'processing status of field N=new I=ingested',

    PRIMARY KEY (file_id)
) engine=InnoDB;
