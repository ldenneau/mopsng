package PS::MOPS::DC::Efficiency;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modce_retrieve
    modce_retrieveProvisionalAttributions
    modce_retrieveIdentifiableDerivedObjects
    modce_retrieveAvailableLinkages
    modce_retrieveAvailableLink2ages

    $EFF_DETECTIONS
    $EFF_DETECTIONS_MULTIPLE
    $EFF_TRACKLETS

    $EFF_DERIVEDOBJECTS
    $EFF_DERIVEDOBJECTS_FAILIOD
    $EFF_DERIVEDOBJECTS_FAILDIFFCOR

    $EFF_ATTRIBUTIONS
    $EFF_PRECOVERIES
    $EFF_IDENTIFICATIONS
    $EFF_REMOVALS

    $EFF_LINK2
);
our $VERSION = '0.01';

# Our stuff here.
__PACKAGE__->mk_accessors(qw(
    category
    fieldId
    epoch_mjd
    avail
    found
    unfound
    unlinked
    failod
    failiod
    synthetic
    nonsynthetic
    mixed
    clean
    bad
));


use Params::Validate;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;
use PS::MOPS::DC::History;
use PS::MOPS::DC::History::Attribution;
use PS::MOPS::DC::Tracklet;
use PS::MOPS::DC::DerivedObject;
use PS::MOPS::DC::Orbit;
#use PS::MOPS::JPLEPH;               # used to calculate positions for provisional attr/precovs
use Astro::SLA;                     # for slaDsep()

our $by_value_validate_args = {
    category => 1,
    fieldId => 0,
    nn => 0,
    objectType => 0,
    avail => 0,
    found => 0,
    unfound => 0,
    unlinked => 0,
    failod => 0,
    failiod => 0,
    synthetic => 0,
    nonsynthetic => 0,
    mixed => 0,
    clean => 0,
    bad => 0,
    duplicate => 0,
};


our $EFF_DETECTIONS = 1;
our $EFF_DETECTIONS_MULTIPLE = 3;
our $EFF_TRACKLETS = 4;

our $EFF_DERIVEDOBJECTS = 6;
our $EFF_DERIVEDOBJECTS_FAILIOD = 8;
our $EFF_DERIVEDOBJECTS_FAILDIFFCOR = 9;

our $EFF_ATTRIBUTIONS = 10;
our $EFF_PRECOVERIES = 12;
our $EFF_IDENTIFICATIONS = 13;
our $EFF_REMOVALS = 14;

our $EFF_LINK2 = 15;

# Map std categories to event types.
our %cat2eventType = (
    $EFF_DERIVEDOBJECTS => $EVENT_TYPE_DERIVATION,
    $EFF_ATTRIBUTIONS => $EVENT_TYPE_ATTRIBUTION,
    $EFF_PRECOVERIES => $EVENT_TYPE_PRECOVERY,
    $EFF_IDENTIFICATIONS => $EVENT_TYPE_IDENTIFICATION,
    $EFF_REMOVALS => $EVENT_TYPE_REMOVAL,
    $EFF_LINK2 => $EVENT_TYPE_LINK2,
);


sub _remove_duplicates {
    # Perl idiom to remove duplicates from a list.
    my %temp;
    return grep ++$temp{$_} < 2, @_;            # beautiful, unreadable
}


sub new {
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    return bless \%self;
}


sub _new_from_row {
    # Create new generic efficiency object.
    my ($inst, %row) = @_;
    return undef if !%row;  # sanity check

    return PS::MOPS::DC::Efficiency->new($inst,
        category => $row{category},
        fieldId => $row{fieldId},
        nn => $row{nn},
        avail => $row{avail},
        found => $row{found},
        unfound => $row{unfound},
        unlinked => $row{unlinked},
        failod => $row{failod},
        failiod => $row{failiod},
        clean => $row{clean},
        mixed => $row{mixed},
        bad => $row{bad},
        nonsynthetic => $row{nonsynthetic},
        duplicate => $row{duplicate} || 0,
    );
}


sub _new2_from_row {
    # Create new generic efficiency object.
    my ($inst, %row) = @_;
    return undef if !%row;  # sanity check

    return PS::MOPS::DC::Efficiency->new($inst,
        category => $row{category},
        objectType => $row{object_type},
        avail => $row{avail},
        found => $row{found},
        unfound => $row{unfound},
        unlinked => $row{unlinked},
        failod => $row{failod},
        failiod => $row{failiod},
        clean => $row{clean},
        mixed => $row{mixed},
        bad => $row{bad},
        nonsynthetic => $row{nonsynthetic},
        duplicate => $row{duplicate},
    );
}


sub _do_detections {
    my ($inst, $epoch_mjd) = @_;
    my $dbh = $inst->dbh;

    my $sql = <<"SQL";
select 
  field_id fieldId,
  sum(1) avail,
  sum(if(d.status='$DETECTION_STATUS_FOUND',1,0)) found,
  sum(if(d.status='$DETECTION_STATUS_UNFOUND',1,0)) unfound,
  sum(if(d.object_name is not null,1,0)) synthetic,
  sum(if(d.object_name is null,1,0)) nonsynthetic
from detections d
where d.epoch_mjd >= ? and d.epoch_mjd < ?
group by field_id
SQL
    my $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute($epoch_mjd, $epoch_mjd + 1) or croak $dbh->errstr;
    my @res;
    my $href;
    while ($href = $sth->fetchrow_hashref) {
        push @res, _new_from_row(
            $inst,
            category => $EFF_DETECTIONS,
            epoch_mjd => $epoch_mjd,
            %{$href},
        );
    }

    return \@res;
}


sub _do_tracklets {
    my ($inst, $epoch_mjd, $ocnum) = @_;
    my $dbh = $inst->dbh;
    my $sth;
    my $gmt_offset_hours = $inst->getConfig()->{site}->{gmt_offset_hours};
    die "can't get site->gmt_offset_hours" unless defined($gmt_offset_hours);

    # Count the number and classification of tracklets associated with each field.
    my $sql;

    if (defined($epoch_mjd)) {
        $sql = <<"SQL";
select 
  d.field_id fieldId,
  sum(if(t.classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(t.classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(t.classification in ('$MOPS_EFF_CLEAN', '$MOPS_EFF_UNFOUND'),1,0)) avail,
  sum(if(t.classification='$MOPS_EFF_CLEAN',1,0)) clean,
  sum(if(t.classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(t.classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(if(t.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) nonsynthetic
from tracklets t 
left join tracklet_attrib ta on t.tracklet_id=ta.tracklet_id
left join detections d on ta.det_id=d.det_id
where t.ext_epoch >= ? and t.ext_epoch < ?
group by d.field_id
SQL
    
        $sth = $dbh->prepare($sql) or croak $dbh->errstr;
        $sth->execute($epoch_mjd, $epoch_mjd + 1) or croak $dbh->errstr;
    }
    else {
        # OC num
        $sql = <<"SQL";
select 
  floor(f.epoch_mjd - 0.5 + $gmt_offset_hours / 24.) nn,
  sum(if(t.classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(t.classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(t.classification in ('$MOPS_EFF_CLEAN', '$MOPS_EFF_UNFOUND'),1,0)) avail,
  sum(if(t.classification='$MOPS_EFF_CLEAN',1,0)) clean,
  sum(if(t.classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(t.classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(if(t.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) nonsynthetic
from tracklets t 
left join `fields` f on t.field_id=f.field_id
where f.ocnum=?
group by /*floor(f.epoch_mjd - 0.5 + $gmt_offset_hours / 24.)*/ nn
SQL
    
        $sth = $dbh->prepare($sql) or croak $dbh->errstr;
        $sth->execute($ocnum) or croak $dbh->errstr;
    }

    my @res;
    my $href;
    while ($href = $sth->fetchrow_hashref) {
        push @res, _new_from_row(
            $inst,
            category => $EFF_TRACKLETS,
            epoch_mjd => $epoch_mjd,
            %{$href},
        );
    }

    return \@res;
}


sub _do_history {
    my ($inst, $category, $nn, $ocnum) = @_;
    my $dbh = $inst->dbh;
    my $field_str;
    my $group_str;
    my @sql_args;
    my $key_str;
    my $mc =  $inst->getConfig();
    my $gmt_offset_hours = defined($mc->{site}->{gmt_offset_hours}) ? $mc->{site}->{gmt_offset_hours} : die "can't get gmt_offset_hours";

    if ($nn) {
        $key_str = 'f.field_id fieldId, f.epoch_mjd nn';
        $field_str = 'f.epoch_mjd > ? and f.epoch_mjd < ?';     # select fields in range
        $group_str = 'f.field_id';
        @sql_args = ($category, $nn, $nn + 1);
    }
    else {
        $key_str = "floor(f.epoch_mjd - 0.5 + $gmt_offset_hours / 24.) nn";
        $field_str = 'f.ocnum=?';
        $group_str = 'nn';
        @sql_args = ($category, $ocnum);
    }

    # Count the number and classification of derived objects associated with each field.
    my $sql = <<"SQL";
select
  $key_str,
  sum(if(h.classification in ('$MOPS_EFF_UNFOUND', '$MOPS_EFF_CLEAN'),1,0)) avail,
  sum(if(h.classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(h.classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_FAIL',1,0)) unlinked,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code<>'$MOPS_EFF_ORBIT_FAIL',1,0)) failod,
  0 failiod,
  sum(if(h.classification='$MOPS_EFF_CLEAN',1,0)) clean,
  sum(if(h.classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(h.classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(if(h.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) nonsynthetic
from history h
left join `fields` f on h.field_id=f.field_id
where h.event_type=?
and $field_str
group by $group_str
SQL
    
    my $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute(@sql_args) or croak $dbh->errstr;
    my @res;
    my $href;
    while ($href = $sth->fetchrow_hashref) {
        push @res, _new_from_row(
            $inst,
            category => $category,
            %{$href},
        );
    }

    return \@res;
}


sub _do_derived_by_night {
    # Tabulate stuff by entire OC number, grouped by night.
    my ($inst, $category, $ocnum) = @_;
    my $dbh = $inst->dbh;
    my $sth;
    my $href;
    my $mc =  $inst->getConfig();
    my $gmt_offset_hours = defined($mc->{site}->{gmt_offset_hours}) ? $mc->{site}->{gmt_offset_hours} : die "can't get gmt_offset_hours";

    my %clean_table;                #  clean (C) results
    my %nonclean_table;             #  nonclean (M/B/N) results

    # Perform two queries: one for clean objects only, so that we can count
    # duplicates, and another to count mixed, bad and nonsynthetics.
    my $sql = <<"SQL";
select
  sub_epoch_mjd nn,
  sum(sub_unique) clean,
  sum(sub_clean) - sum(sub_unique) duplicate
from (
  select
    floor(f.epoch_mjd - 0.5 + $gmt_offset_hours / 24.) sub_epoch_mjd,
    1 sub_unique,
    count(ssm_id) sub_clean
  from
    history h
      left join `fields` f on h.field_id=f.field_id
  where h.classification='$MOPS_EFF_CLEAN' and h.event_type=? and f.ocnum=?
  group by sub_epoch_mjd, ssm_id
) as sub group by sub_epoch_mjd
SQL

    $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute($category, $ocnum) or croak $dbh->errstr;
    while ($href = $sth->fetchrow_hashref) {
        $clean_table{$href->{nn}} = $href;
    }

    
    # Query #2.  Count of mixed, bad and nonsynthetic objects.
    $sql = <<"SQL";
select 
  floor(f.epoch_mjd - 0.5 + $gmt_offset_hours / 24.) nn,
  sum(if(h.classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(h.classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_FAIL',1,0)) unlinked,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code<>'$MOPS_EFF_ORBIT_FAIL',1,0)) failod,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_IODFAIL',1,0)) failiod,
  sum(if(h.classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(h.classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(if(h.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) nonsynthetic
from history h 
    left join `fields` f on h.field_id=f.field_id
where h.event_type=? and f.ocnum=?
group by floor(f.epoch_mjd - 0.5 + $gmt_offset_hours / 24.)
SQL

    $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute($category, $ocnum) or croak $dbh->errstr;
    while ($href = $sth->fetchrow_hashref) {
        $nonclean_table{$href->{nn}} = $href;
    }


    # Now combine results.  Since we're not totally guaranteed to have the same
    # night numbers in the different results sets, create a list of all night numbers.
    my @res;
    my %clean_args;
    my %nonclean_args;
    my %clean_dummy = (
        clean => 0, duplicate => 0,
    );
    my %nonclean_dummy = (
        found => 0, unfound => 0, unlinked => 0, failod => 0, failiod => 0, mixed => 0, bad => 0, nonsynthetic => 0,
    );
    foreach my $nn (sort(_remove_duplicates(keys %clean_table, keys %nonclean_table))) {

        if (exists($clean_table{$nn})) {
            %clean_args = %{$clean_table{$nn}};
        }
        else {
            %clean_args = (epoch_mjd => $nn, %clean_dummy);     # no result for this night; use dummy
        }

        if (exists($nonclean_table{$nn})) {
            %nonclean_args = %{$nonclean_table{$nn}};
        }
        else {
            %nonclean_args = (epoch_mjd => $nn, %nonclean_dummy);       # no result for this night; use dummy
        }

        # Assemble the 'avail' number: clean + unfound
        $clean_args{avail} = $clean_args{clean} + $nonclean_args{unfound};

        push @res, _new_from_row($inst, category => $category, %clean_args, %nonclean_args);
    }

    return \@res;
}


sub _do_derived_by_field {
    # Tabulate stuff by entire night, grouped by field_id.
    my ($inst, $category, $nn) = @_;
    my $dbh = $inst->dbh;
    my $sth;
    my $href;
    my $nn_str = 'f.epoch_mjd > ? and f.epoch_mjd < ?';     # select fields in range
    my $field_id;

    my %clean_table;                #  clean (C) results
    my %nonclean_table;             #  nonclean (M/B/N) results

    # Perform two queries: one for clean objects only, so that we can count
    # duplicates, and another to count mixed, bad and nonsynthetics.
    my $sql = <<"SQL";
select
  sub_field_id fieldId,
  sub_epoch_mjd nn,
  sum(sub_unique) clean,
  sum(sub_clean) - sum(sub_unique) duplicate
from (
  select
    f.field_id sub_field_id,
    f.epoch_mjd sub_epoch_mjd,
    1 sub_unique,
    count(ssm_id) sub_clean
  from
    history h
      left join `fields` f on h.field_id=f.field_id
  where h.classification='$MOPS_EFF_CLEAN' and h.event_type=? and $nn_str
  group by sub_field_id, ssm_id
) as sub group by sub_field_id
SQL

    $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute($category, $nn, $nn + 1) or croak $dbh->errstr;
    while ($href = $sth->fetchrow_hashref) {
        $clean_table{$href->{fieldId}} = $href;
    }

    
    # Query #2.  Count of mixed, bad and nonsynthetic objects.
    $sql = <<"SQL";
select 
  f.field_id fieldId,
  f.epoch_mjd nn,
  sum(if(h.classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(h.classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_FAIL',1,0)) unlinked,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code<>'$MOPS_EFF_ORBIT_FAIL',1,0)) failod,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_IODFAIL',1,0)) failiod,
  sum(if(h.classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(h.classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(if(h.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) nonsynthetic
from history h 
    left join `fields` f on h.field_id=f.field_id
where h.event_type=? and $nn_str
group by f.field_id
SQL

    $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute($category, $nn, $nn + 1) or croak $dbh->errstr;
    while ($href = $sth->fetchrow_hashref) {
        $nonclean_table{$href->{fieldId}} = $href;
    }


    # Now combine results.  Since we're not totally guaranteed to have the same
    # night numbers in the different results sets, create a list of all night numbers.
    my @res;
    my %clean_args;
    my %nonclean_args;
    my %clean_dummy = (
        clean => 0, duplicate => 0,
    );
    my %nonclean_dummy = (
        found => 0, unfound => 0, unlinked => 0, failod => 0, failiod => 0, mixed => 0, bad => 0, nonsynthetic => 0,
    );
    foreach my $field_id (sort(_remove_duplicates(keys %clean_table, keys %nonclean_table))) {

        if (exists($clean_table{$field_id})) {
            %clean_args = %{$clean_table{$field_id}};
        }
        else {
            %clean_args = (fieldId => $field_id, %clean_dummy);     # no result for this night; use dummy
        }

        if (exists($nonclean_table{$field_id})) {
            %nonclean_args = %{$nonclean_table{$field_id}};
        }
        else {
            %nonclean_args = (fieldId => $field_id, %nonclean_dummy);       # no result for this night; use dummy
        }

        # Assemble the 'avail' number: clean + unfound
        $clean_args{avail} = $clean_args{clean} + $nonclean_args{unfound};

        push @res, _new_from_row($inst, category => $category, %clean_args, %nonclean_args);
    }

    return \@res;
}


sub _do_derived_by_object_type {
    # Tabulate stuff by entire night, grouped by field_id.
    my ($inst, $category, $nn, $ocnum) = @_;
    my $dbh = $inst->dbh;
    my $sth;
    my $href;
    my $field_str;
    my $field_id;
    my @sql_args;

    my %clean_table;                #  clean (C) results
    my %nonclean_table;             #  nonclean (M/B/N) results

    if ($nn) {
        $field_str = 'f.epoch_mjd > ? and f.epoch_mjd < ?';     # select fields in range
        @sql_args = ($category, $nn, $nn + 1);
    }
    else {
        $field_str = 'f.ocnum=?';     # select fields in ocnum
        @sql_args = ($category, $ocnum);
    }

    # Perform two queries: one for clean objects only, so that we can count
    # duplicates, and another to count mixed, bad and nonsynthetics.
    my $sql = <<"SQL";
select
  substr(sub_object_name, 1, 2) object_type,
  sum(sub_unique) clean,
  sum(sub_clean) - sum(sub_unique) duplicate
from (
  select
    s.object_name sub_object_name,
    1 sub_unique,
    count(s.ssm_id) sub_clean
  from
    history h, ssm s, `fields` f
  where h.classification='$MOPS_EFF_CLEAN'
    and s.ssm_id=h.ssm_id
    and h.field_id=f.field_id
    and h.event_type=? and $field_str
  group by s.ssm_id
) as sub group by substr(sub_object_name, 1, 2)
SQL

    $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute(@sql_args) or croak $dbh->errstr;
    while ($href = $sth->fetchrow_hashref) {
        $clean_table{$href->{object_type}} = $href;
    }

    
    # Query #2.  Count of mixed, bad and nonsynthetic objects.
    $sql = <<"SQL";
select 
  substr(s.object_name, 1, 2) object_type,
  sum(if(h.classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(h.classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_FAIL',1,0)) unlinked,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code<>'$MOPS_EFF_ORBIT_FAIL',1,0)) failod,
  sum(if(h.classification='$MOPS_EFF_UNFOUND' and h.orbit_code='$MOPS_EFF_ORBIT_IODFAIL',1,0)) failiod,
  sum(if(h.classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(h.classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(if(h.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) nonsynthetic
from history h, ssm s, `fields` f
where s.ssm_id=h.ssm_id
  and h.field_id=f.field_id
  and h.event_type=? and $field_str
group by substr(s.object_name, 1, 2)
SQL

    $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute(@sql_args) or croak $dbh->errstr;
    while ($href = $sth->fetchrow_hashref) {
        $nonclean_table{$href->{object_type}} = $href;
    }


    # Now combine results.  Since we're not totally guaranteed to have the same
    # night numbers in the different results sets, create a list of all night numbers.
    my @res;
    my %clean_args;
    my %nonclean_args;
    my %clean_dummy = (
        clean => 0, duplicate => 0,
    );
    my %nonclean_dummy = (
        found => 0, unfound => 0, unlinked => 0, failod => 0, failiod => 0, mixed => 0, bad => 0, nonsynthetic => 0,
    );
    foreach my $object_type (sort(_remove_duplicates(keys %clean_table, keys %nonclean_table))) {

        if (exists($clean_table{$object_type})) {
            %clean_args = %{$clean_table{$object_type}};
        }
        else {
            %clean_args = (object_type => $object_type, %clean_dummy);     # no result for this night; use dummy
        }

        if (exists($nonclean_table{$object_type})) {
            %nonclean_args = %{$nonclean_table{$object_type}};
        }
        else {
            %nonclean_args = (object_type => $object_type, %nonclean_dummy);       # no result for this night; use dummy
        }

        # Assemble the 'avail' number: clean + unfound
        $clean_args{avail} = $clean_args{clean} + $nonclean_args{unfound};

        push @res, _new2_from_row($inst, category => $category, %clean_args, %nonclean_args);
    }

    return \@res;
}


sub _do_derived_by_type {
    # Same as do_history, except aggregated by SSM object type.
    my ($inst, $category, $epoch_mjd, $ocnum) = @_;
    my $dbh = $inst->dbh;

    my @args = ($category, $ocnum);

#    my $and;
#    if ($epoch_mjd) {
#        $and = 'and epoch_mjd >= ? and epoch_mjd < ?';
#        push @args, $stuff{EPOCH_MJD}, $stuff{EPOCH_MJD} + 1;
#    }

    # Count the number and classification of derived objects associated with each field.
    my $sql = <<"SQL";
select
  substr(sub_object_name, 1, 2) object_type,
  sum(if(sub_classification in ('$MOPS_EFF_UNFOUND', '$MOPS_EFF_CLEAN'),1,0)) avail,
  sum(if(sub_classification<>'$MOPS_EFF_UNFOUND',1,0)) found,
  sum(if(sub_classification='$MOPS_EFF_UNFOUND',1,0)) unfound,
  sum(if(sub_classification='$MOPS_EFF_UNFOUND' and sub_orbit_code='$MOPS_EFF_ORBIT_FAIL',1,0)) unlinked,
  sum(if(sub_classification='$MOPS_EFF_UNFOUND' and sub_orbit_code<>'$MOPS_EFF_ORBIT_FAIL',1,0)) failod,
  sum(if(sub_classification='$MOPS_EFF_UNFOUND' and sub_orbit_code='$MOPS_EFF_ORBIT_IODFAIL',1,0)) failiod,
  sum(if(sub_classification='$MOPS_EFF_CLEAN',1,0)) clean,
  sum(if(sub_classification='$MOPS_EFF_MIXED',1,0)) mixed,
  sum(if(sub_classification='$MOPS_EFF_BAD',1,0)) bad,
  sum(sub_nonsynthetic) nonsynthetic,
  sum(sub_duplicate) duplicate
from (
  select
    s.object_name sub_object_name,
    h.classification sub_classification,
    h.orbit_code sub_orbit_code,
    sum(if(h.ssm_id is NULL,0,1)) sub_duplicate,
    sum(if(h.classification='$MOPS_EFF_NONSYNTHETIC',1,0)) sub_nonsynthetic
  from
    history h, ssm s, `fields` f
  where s.ssm_id=h.ssm_id
    and h.field_id=f.field_id
    and h.event_type=? and f.ocnum=?
  group by h.ssm_id
) as sub group by substr(sub_object_name, 1, 2)
SQL
    
    my $sth = $dbh->prepare($sql) or croak $dbh->errstr;
    $sth->execute(@args) or croak $dbh->errstr;
    my @res;
    my $href;
    while ($href = $sth->fetchrow_hashref) {
        push @res, _new2_from_row(
            $inst,
            category => $category,
            %{$href},
        );
    }

    return \@res;
}



sub _do_dresids {
    # Return object describing orbit residuals for specified MJD.
    my ($inst, $category, $epoch_mjd) = @_;
    my $dbh = $inst->dbh;

    my @args = ($category);
    my $and = '';
    if ($epoch_mjd) {
        $and = 'and epoch_mjd >= ? and epoch_mjd < ?';
        push @args, $epoch_mjd, $epoch_mjd + 1;
    }

    # First get resids of clean objects.
    my $clean_aref = $dbh->selectcol_arrayref(<<"SQL", undef, @args);
select
  o.residual
from history h, `fields` f, orbits o
where h.orbit_id=o.orbit_id
and h.field_id=f.field_id
and h.event_type=?
and h.classification='$MOPS_EFF_CLEAN'
$and
SQL

    my $unfound_aref = $dbh->selectcol_arrayref(<<"SQL", undef, @args);
select
  o.residual
from history h, `fields` f, orbits o
where h.orbit_id=o.orbit_id
and h.field_id=f.field_id
and h.event_type=?
and h.classification='$MOPS_EFF_UNFOUND'
$and
SQL

    my $nonsynthetic_aref = $dbh->selectcol_arrayref(<<"SQL", undef, @args);
select
  o.residual
from history h, `fields` f, orbits o
where h.orbit_id=o.orbit_id
and h.field_id=f.field_id
and h.event_type=?
and h.classification='$MOPS_EFF_NONSYNTHETIC'
$and
SQL

    return {
        CLEAN => $clean_aref,
        UNFOUND => $unfound_aref,
        NONSYNTHETIC => $nonsynthetic_aref,
    };
}


sub _do_attr_ephemeris_distances {
    # Return object describing ephemeris distances for specified MJD/OCNUM.
    my ($inst, $epoch_mjd, $ocnum) = @_;
    my $dbh = $inst->dbh;

    my @args = ();
    my $and = '';

    if ($ocnum) {
        $and = 'and f.ocnum=?';
        push @args, $ocnum;
    }
    elsif ($epoch_mjd) {
        $and = 'and epoch_mjd >= ? and epoch_mjd < ?';
        push @args, $epoch_mjd, $epoch_mjd + 1;
    }

    # First get resids of clean objects.
    my $clean_aref = $dbh->selectcol_arrayref(<<"SQL", undef, @args);
select
  a.ephemeris_distance
from history h, `fields` f, history_attributions a
where h.event_id=a.event_id
and h.field_id=f.field_id
and h.event_type='$EVENT_TYPE_ATTRIBUTION'
and h.classification='$MOPS_EFF_CLEAN'
$and
SQL

    my $unfound_aref = $dbh->selectcol_arrayref(<<"SQL", undef, @args);
select
  a.ephemeris_distance
from history h, `fields` f, history_attributions a
where h.event_id=a.event_id
and h.field_id=f.field_id
and h.event_type='$EVENT_TYPE_ATTRIBUTION'
and h.classification='$MOPS_EFF_UNFOUND'
$and
SQL

    return {
        CLEAN => $clean_aref,
        UNFOUND => $unfound_aref,
    };
}


sub modce_retrieve {
    # Return requested efficiency values, aggregated by field, night, or derived object.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        category => 1,                  # category of efficiency tabulation 
        epoch_mjd => 0,                 # MJD for aggregation
        ocnum => 0,                     # OCNUM instead of MJD for aggregation
        byField => 0,                   # group summary results by field_id
        byObjectType => 0,              # group summary results by SSM object type
        dResids => 0,                   # derivation residuals
        attrEphemerisDistances => 0,    # attribution ehpemeris distances
    });

    my $result;
    my %datespec;

    if ($args{ocnum}) {
        %datespec = (OCNUM => $args{ocnum});
    }
    else {
        %datespec = (EPOCH_MJD => $args{epoch_mjd});
    }

    if ($args{dResids}) {
        # Tabulate derivation residuals.
        $result = _do_dresids($inst, $EVENT_TYPE_DERIVATION, $args{epoch_mjd});
    }
    elsif ($args{attrEphemerisDistances}) {
        $result = _do_attr_ephemeris_distances($inst, $args{epoch_mjd}, $args{ocnum});
    }
    else {
        if ($args{category} eq $EFF_DETECTIONS) {
            $result = _do_detections($inst, $args{epoch_mjd});
        }
        elsif ($args{category} eq $EFF_TRACKLETS) {
            $result = _do_tracklets($inst, $args{epoch_mjd}, $args{ocnum});
        }
        else {
            if ($args{category} eq $EFF_DERIVEDOBJECTS) {
                if ($args{byField}) {
                    $result = _do_derived_by_field($inst, $cat2eventType{$args{category}}, $args{epoch_mjd});
                }
                elsif ($args{byObjectType}) {
                    $result = _do_derived_by_object_type($inst, $cat2eventType{$args{category}}, $args{epoch_mjd}, $args{ocnum});
                }
                else {
                    $result = _do_derived_by_night($inst, $cat2eventType{$args{category}}, $args{ocnum});
                }
            }
            else {
                $result = _do_history($inst, $cat2eventType{$args{category}}, $args{epoch_mjd}, $args{ocnum});
            }
        }
    }

    return $result;
}


sub modce_retrieveProvisionalAttributions {
    # Return a list of attribution objects for tracklets in the specified field that are
    # unattributed and whose ssm_id match ssm_ids of existing
    # derived objects.

    # Return an ARRAYREF, but we really should return an iterator.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        fieldId => 1,                  # category of efficiency tabulation 
        precovery => 0,                 # true if request is for precoveries
    });

    # Perform SQL query to retrieve unattributed tracklets that have matching ssm_ids
    # from identified derived objects.
    # In case of precoveries, we need to make sure that we do not include 
    # tracklets belonging to a field observed after the derivedobject was last
    # modified.
    my $aref;
    if($args{precovery}) {
	$aref = $dbh->selectall_arrayref(<<"SQL", undef, $args{fieldId});
select t.tracklet_id t_id, do.derivedobject_id d_id, do.ssm_id s_id 
from tracklets t, derivedobjects do, eon_queue q, `fields` f 
where t.ssm_id=do.ssm_id 
and t.field_id=? 
and t.status='$TRACKLET_STATUS_UNATTRIBUTED' 
and do.status<>'$DERIVEDOBJECT_STATUS_MERGED' 
and q.status='$EONQUEUE_STATUS_IDENTIFIED'
and do.derivedobject_id=q.derivedobject_id 
and f.field_id=t.field_id 
SQL
    } 
    else {
	$aref = $dbh->selectall_arrayref(<<"SQL", undef, $args{fieldId}, $TRACKLET_STATUS_UNATTRIBUTED, $DERIVEDOBJECT_STATUS_MERGED);
select t.tracklet_id t_id, do.derivedobject_id d_id, do.ssm_id s_id
from tracklets t, derivedobjects do
where t.ssm_id=do.ssm_id
and t.field_id=?
and t.status=?
and do.status<>?
SQL
    }
    my @attrs;
    my $tid;
    foreach my $rowref (@{$aref}) {
        if ($args{precovery}) {
            push @attrs, PS::MOPS::DC::History::Precovery->new(
                $inst,
                derivedobjectId => $rowref->[1],    # d_id
                fieldId => $args{fieldId},
                orbitId => undef,
                orbitCode => $MOPS_EFF_ORBIT_FAIL,
                classification => $MOPS_EFF_UNFOUND,
                ssmId => $rowref->[2],              # s_id
                trackletId => $rowref->[0],         # t_id
            );
        }
        else {
            push @attrs, PS::MOPS::DC::History::Attribution->new(
                $inst,
                derivedobjectId => $rowref->[1],    # d_id
                fieldId => $args{fieldId},
                orbitId => undef,
                orbitCode => $MOPS_EFF_ORBIT_FAIL,
                classification => $MOPS_EFF_UNFOUND,
                ssmId => $rowref->[2],              # s_id
                trackletId => $rowref->[0],         # t_id
            );
        }
    }

    # Now calculate provisional ephemeris distances.  This is the sky-plane distance between
    # the last detection in the attributed tracklet and the predicted position .
    foreach my $attr (@attrs) {
        eval {
            my $dobj = modcdo_retrieve($inst, derivedobjectId => $attr->derivedobjectId);
            die "couldn't fetch derived object " . $attr->derivedobjectId unless $dobj;

            my $orbit = modco_retrieve($inst, orbitId => $dobj->orbitId);
            die "couldn't fetch orbit " . $dobj->orbitId unless $orbit;

            my $trk = modct_retrieve($inst, trackletId => $attr->trackletId);
            die "couldn't tracklet " . $attr->trackletId unless $trk;
            die "empty detection list for tracklet " . $attr->trackletId unless @{$trk->detections};

            my $last_detection = @{$trk->detections}[-1];        # position in last tracklet
            my $predicted_position = jpleph_calcEphemeris(
                orbit => $orbit,
                epoch_mjd => $last_detection->epoch,
                obscode => $last_detection->obscode,
            );

            if (!$predicted_position) {
                die sprintf "got undef position for %s %f %s\n", 
                    $last_detection->objectName, 
                    $last_detection->epoch, 
                    $last_detection->obscode;
            }

            my $dist_arcsec = slaDsep(
                $predicted_position->{ra} / $DEG_PER_RAD, 
                $predicted_position->{dec} / $DEG_PER_RAD, 
                $last_detection->ra / $DEG_PER_RAD, 
                $last_detection->dec / $DEG_PER_RAD
            ) * $DEG_PER_RAD * 3600;    # cvt result to arcsec
            $attr->ephemerisDistance($dist_arcsec);

        };
        if ($@) {
            carp "provisional attribution failed: $@";
        }
    }

    return \@attrs;
}


sub modce_retrieveIdentifiableDerivedObjects {
    # Return a list of (child, parent) derived object pairs for all objects in the
    # EON queue with status 'N'.
    my $inst = shift;
    my $dbh = $inst->dbh;

    my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
select do_child.derivedobject_id child_id, do_parent.derivedobject_id parent_id
from eon_queue eonq
join derivedobjects do_child using (derivedobject_id)
join derivedobjects do_parent using (ssm_id)
where eonq.status='$EONQUEUE_STATUS_NEW'
and do_child.derivedobject_id <> do_parent.derivedobject_id
SQL
    $sth->execute or die $sth->errstr;

    # First create list of IDs; then convert to derivedobjects.
    my @id_pairs;
    my $href;
    while ($href = $sth->fetchrow_hashref) {
        push @id_pairs, [
            $href->{child_id}, 
            $href->{parent_id},
        ];
    }
    $sth->finish;
    
    my @obj_pairs;
    foreach my $pair_ref (@id_pairs) {
        push @obj_pairs, [
            modcdo_retrieve($inst, derivedobjectId => $pair_ref->[0]),
            modcdo_retrieve($inst, derivedobjectId => $pair_ref->[1]),
        ];
    }

    return \@obj_pairs;
}


sub modce_retrieveAvailableLinkages {
    # Return a list of SSM object names that are available to be linked in the time
    # window specified.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        ut2local_hours => 1,            # UT to local offset for night counting
        min_nights => 1,                # numer nights required in interval
        target_field_id => 0,           # all linkable into specified field
        start_nn => 0,                  # starting night number
        end_nn => 0,                    # ending night number (inclusive)
        ocnum => 0,                     # specify entire OC num instead of start/end night
    });
    my $start_mjd = mopslib_nn2mjd($args{start_nn}, $args{ut2local_hours});
    my $end_mjd = mopslib_nn2mjd($args{end_nn}, $args{ut2local_hours}) + 1;     # +1 for inclusive of end_nn

    my $sql = <<"SQL";
select ssm.object_name
from (  
    select distinct ssm_id, floor(ext_epoch - 0.5 + $args{ut2local_hours} / 24.) nn
    from tracklets t
    where t.classification='C'
    and t.ext_epoch >= $start_mjd and t.ext_epoch <= $end_mjd
    order by ssm_id, ext_epoch  
) foo, ssm 
where ssm.ssm_id=foo.ssm_id 
group by ssm.ssm_id  
having count(ssm.ssm_id) >= $args{min_nights} /* and max(foo.nn)=$args{end_nn} */
SQL

    my $aref = $dbh->selectcol_arrayref($sql) or die $dbh->errstr;

    # First create list of IDs; then convert to derivedobjects.
    return $aref;
}


sub modce_retrieveAvailableLink2ages {
    # Return a list of SSM object names that are available to be linked via
    # Link2 linkages ending on the specified night.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        ocnum => 1,                         # specify ending OC num
    });

    my $sql = sprintf <<"SQL", $args{ocnum}, $args{ocnum} - 1;
select
s.ssm_id /*, s.object_name , count(distinct f.ocnum), count(distinct f.nn)*/
from tracklets t
join fields f using (field_id)
join ssm s using (ssm_id)
where t.status='U'
and f.ocnum in (%d, %d)
group by s.object_name
having count(distinct f.ocnum) > 1 and count(distinct f.nn) > 2
order by t.ssm_id
SQL

    my $aref = $dbh->selectcol_arrayref($sql) or die $dbh->errstr;

    # First create list of IDs; then convert to derivedobjects.
    return $aref;
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Efficiency - Module for manipulating MOPS efficiency tables

=head1 SYNOPSIS

  use PS::MOPS::DC::Efficiency;

=head1 DESCRIPTION

Retrieve tables of efficiency data.

=head2 EXPORT

modce_retrieve

=head1 SEE ALSO

PS::MOPS::DC::Orbit

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
