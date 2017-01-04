#!/usr/bin/env perl

package PS::MOPS::Comments::Comment;

use Exporter;
use strict;
use warnings;
use Carp;
use DBI;
use constant false => 0;
use constant true  => 1;
use PS::MOPS::Comments::CommentObj;

our $VERSION = '1.00';

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ('all' => [qw(
    get_comments
    save_comment 
)]);

our @EXPORT_OK = (@{$EXPORT_TAGS{'all'}});
our @EXPORT = qw(
    get_comments
    save_comment 
);

# Connection parameters to the comments schema. The user account used must have
# update privileges to the schema.
our @conspec = (
    'dbi:mysql:database=comments;port=3306;host=mops01.ifa.hawaii.edu',
    'mops',
    'mops',
    {RaiseError => 1, AutoCommit=>1}
);

sub open_connection {
    # Creates a connection to the comments schema.
    my $dbh = DBI->connect(@conspec) or die "can't connect to comments database";
    return $dbh;
}

sub get_comments {
    # Retrieves all of the comments associated with the specified page.
    
    my $page_id = shift or die "Please specify the page id of the page when retrieving comments.";
    my $aref = [];
   
    # Connect to comments schema and retrieve all comments for the page. 
    my $dbh = open_connection();
    my $sth = $dbh->prepare(<<"SQL" ) or die $dbh->errstr;
SELECT name, body, date FROM comments WHERE page_id = ? ORDER BY id DESC
SQL
    $sth->execute($page_id) or die $sth->errstr;
    
    # Create a comment object for each comment and add it to an array
    my $href;
    while ($href = $sth->fetchrow_hashref) {
        my $comment = PS::MOPS::Comments::CommentObj->new($href->{name}, $href->{body}, $page_id, $href->{date});
        push(@$aref, $comment);
    }
    $sth->finish;
    $dbh->disconnect;
    return $aref;
}

sub save_comment {
    # Saves contents of comment object to the database.
    
    my ($page_id, $name, $body) = @_;
    my $dbh = open_connection();
    my $sth = $dbh->prepare(<<"SQL" ) or die $dbh->errstr;
INSERT INTO comments(name, body, page_id) VALUES (?, ?, ?) 
SQL
    $sth->execute($name, $body, $page_id) or die $sth->errstr;
    $sth->finish;
    $dbh->disconnect;
}  
1;