package PS::MOPS::DC::TemplateInstance;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Class::Accessor);
our $VERSION = '0.01';

use DBI;
use Params::Validate;
use PS::MOPS::Config;
use PS::MOPS::Lib qw(:all);


__PACKAGE__->mk_accessors(qw(
    dbname
    backend
    gmt_offset_hours
));


my $DEFAULT_DBNAME = 'mops_template_db';


sub _load_config_module {
    my ($self, $which) = @_;
    my $global_config = $ENV{MOPS_HOME} . "/config/${which}.cf";

    if (-f $global_config) {
        return PS::MOPS::Config::LoadFile($global_config);
    }
    else {
        croak "can't find global config $global_config";
    }
}


sub new {
    my $pkg = shift;
    my %args = validate(@_, {
        GMT_OFFSET_HOURS => 1,
        DBNAME => 0,
        BACKEND => 0,
    });

    my %self = (
        dbname => $args{DBNAME} || $DEFAULT_DBNAME,
        backend => $args{BACKEND},
        gmt_offset_hours => $args{GMT_OFFSET_HOURS},
    );

    my $backend_cfg = _load_config_module(\%self, 'backend');

    # Database stuff.
    $self{_DBHOSTNAME} = $ENV{MOPS_DBHOSTNAME} || $backend_cfg->{hostname};       # host to connect to
    $self{_DBPORT} = $ENV{MOPS_DBPORT} || $backend_cfg->{port} || 3306;           # port to connect to
    $self{_DBUSERNAME} = $ENV{MOPS_DBUSERNAME} || $backend_cfg->{username};       # DB username
    $self{_DBPASSWORD} = $ENV{MOPS_DBPASSWORD} || $backend_cfg->{password};       # DB password
    $self{_DBH} = undef;

    my $dbname = $self{dbname};

    if (%{$backend_cfg}) {
        if ($backend_cfg->{backend} =~ /mysql/i) {
            # If hostname is 'local' or unspecified use local connection.
            if ($ENV{MOPS_DBCONNECT}) {
                $self{_DBCONNECT} = [
                    $ENV{MOPS_DBCONNECT},
                    $self{_DBUSERNAME},
                    $self{_DBPASSWORD},
                    {
                        RaiseError => 1,
                    },
                ];
            }
            elsif (!$self{_DBHOSTNAME} or ($self{_DBHOSTNAME} eq 'local')) {
                $self{_DBCONNECT} = [
                    "dbi:mysql:database=$dbname;port=$self{_DBPORT}",
                    $self{_DBUSERNAME},
                    $self{_DBPASSWORD},
                    {
                        RaiseError => 1,
                    },
                ];
            }
            else {
                $self{_DBCONNECT} = [
                    "dbi:mysql:host=$self{_DBHOSTNAME};database=$dbname;port=$self{_DBPORT}",
                    $self{_DBUSERNAME},
                    $self{_DBPASSWORD},
                    {
                        RaiseError => 1,
                    },
                ];
            }
        }
        else {
            die "unknown backend: $backend_cfg->{backend}";
        }
    }   # if (%{$backend_cfg})
    return bless \%self;
}


sub DESTROY {
    # Disconnect our database handle prior to destroy.
    my $self = shift;
    if ($self->{_DBH}) {
        $self->{_DBH}->disconnect;
        undef($self->{_DBH});
    }
}


sub dbh {
    my $self = shift;
    if (!$self->{_DBH}) {
        $self->{_DBH} = DBI->connect(@{$self->{_DBCONNECT}}) or croak "can't connect to database";
        $self->{_DBH}->{mysql_auto_reconnect} = 1;
    }
    return $self->{_DBH};
}


sub info {
    # Return a hashref describing the template_fields database:
    # each key should be the name of a different survey, and for
    # each key, items the min/max NN, min/max OCNUM, and date range.
    my $self = shift;
    my $dbh = $self->dbh();
    my $sql = <<"SQL";
select s.survey_id, s.survey_name, s.description, min(f.nn), max(f.nn), min(f.ocnum), max(f.ocnum) 
from template_surveys s join template_fields f using (survey_id)
group by f.survey_id
SQL
    my $aref = $dbh->selectall_arrayref($sql);
    my $row;
    my %info;
    foreach $row (@{$aref}) {
        $info{$row->[0]} = {
            SURVEY_NAME => $row->[1],
            SURVEY_DESC => $row->[2],
            MIN_NN => $row->[3],
            MAX_NN => $row->[4],
            MIN_OCNUM => $row->[5],
            MAX_OCNUM => $row->[6],
            MIN_DATE => mopslib_mjd2localtimestr(mopslib_nn2mjd($row->[3], $self->gmt_offset_hours), 
                $self->gmt_offset_hours,
                ' ',
            ),
            MAX_DATE => mopslib_mjd2localtimestr(mopslib_nn2mjd($row->[4], $self->gmt_offset_hours), 
                $self->gmt_offset_hours,
                ' ',
            ),
        };
    }

    return \%info;
}


sub lookupSurveyId {
    # Return an ID from a survey name, or undef if the survey was not found.
    my $self = shift;
    my $survey_name = shift;
    my $dbh = $self->dbh();
    my $sql = <<"SQL";
select survey_id from template_surveys where survey_name=?
SQL
    my $aref = $dbh->selectcol_arrayref($sql, undef, $survey_name);
    if (@{$aref}) {
        return $aref->[0];
    }
    else {
        return undef;
    }
}


sub createSurvey {
    my $self = shift;
    my %args = validate(@_, {
        SURVEY_NAME => 1,
        SURVEY_DESC => 1,
    });

    # First check if name already exists.  If so, return existing ID.
    my $survey_id = $self->lookupSurveyId($args{SURVEY_NAME});
    return $survey_id if $survey_id;

    my $dbh = $self->dbh();
    my $rv = $dbh->do(<<"SQL", undef, $args{SURVEY_NAME}, $args{SURVEY_DESC});
insert into template_surveys (survey_name, description)
values (?, ?)
SQL
    if ($rv) {
        return $dbh->{'mysql_insertid'};
    }
    else {
        croak("Couldn't create survey $args{SURVEY_NAME}: " . $dbh->errstr);
    }
}


1;
__END__

=head1 NAME

PS::MOPS::DC::TemplateInstance - MOPS template database instance management

=head1 SYNOPSIS

  use PS::MOPS::DC::TemplateInstance;

  my $tinst = PS::MOPS::DC::TemplateInstance->new(GMT_OFFSET_HOURS => -10);
  $tinst->createSurvey(
    SURVEY_NAME => 'foo',
    SURVEY_DESC => 'bar'
  );

  $surveyId = $tinst->lookupSurveyId('foo');
  $field = PS::MOPS::DC::TemplateField->new($tinst, surveyId => $surveyId, %PARAMS);
  $field->insert();

=head1 DESCRIPTION

PS::MOPS::DC::Instance provides database and filsystem management to
MOPS database instance.  The Instance object is the entry point to any
code that retreives, inserts or manipulates objects in a MOPS database.
Filesystem directories that are used by the instance are managed by a
Instance object.

=head1 METHODS

=item new

Return a new instance handle for the specified database instance.

=item dbh

Get the global database handle (DBH) for this instance.

=item info

Return a hashref containing information about the various template surveys
in the template DB.  The hash contains the following:

=item createSurvey

Given a survey name and description, create a record in the
template_surveys table for a new survey.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2008 by Larry Denneau, Jr., Institute for Astronomy,
University of Hawaii.

=cut
