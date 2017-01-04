#!/usr/bin/env perl

=head1 NAME

PS::MOPS::Comments::CommentObj - Perl object used to represent individual comments added 
                       to a web page.

=head1 SYNOPSIS

  use PS::MOPS::Comments::CommentObj;

=head1 DESCRIPTION

Perl object used to represent an individual comment.
 
	
=head1 Methods

=cut

package PS::MOPS::Comments::CommentObj;

use PS::MOPS::Comments::Comment qw(save_comment);
use constant false => 0;
use constant true  => 1;
use DateTime;
use URI::Escape;
use strict;

#-------------------------------------------------------------------------------
# Constructor
#-------------------------------------------------------------------------------
sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = {};
    ($self->{NAME}, $self->{BODY}, $self->{PAGE_ID}, $self->{DATE}) = @_;
    bless($self, $class);
    return $self;
}

#-------------------------------------------------------------------------------
# Class methods.
#-------------------------------------------------------------------------------
sub markup()
{
	#
	# This method outputs the XHTML markup of the comment
	#
	
    my $self = shift;
	
	# Parse comment post time
	my ($yr, $mon, $day, $hr, $min, $sec) = $self->{DATE} =~ /^([0-9]{4})-([0-9]{2})-([0-9]{2}) ([0-9]{2}):([0-9]{2}):([0-9]{2})\z/;
	my $full_date = $self->{DATE};
	my $short_date = sprintf("%04d-%02d-%02d", $yr, $mon, $day);
	my $comment = uri_escape($self->{BODY});
	my $name = uri_escape($self->{NAME});
	return (<<HTML);
<div class="comment">
    <div class="name">$self->{NAME}</div>
    <div class="date" title="Added at $full_date">$full_date</div>
    <p>$self->{BODY}</p>
</div>
HTML
}

sub save {
    my $self = shift;
        
    PS::MOPS::Comments::Comment::save_comment($self->{PAGE_ID}, $self->{NAME}, $self->{BODY});
}

sub validate {
	#
	# This method is used to validate the data sent via AJAX.
	#
	# It return true/false depending on whether the data is valid, and populates
	# the array passed as a paremter with either the valid input data, or 
	# the error messages.
	#

    my $self = shift;
	my $results = shift;
	my $error_flag = undef;

	if(!(validate_text($self->{BODY})))
	{
		$results->{body} = 'Please enter a comment.';
		$error_flag = 1;
	}
	
    if(!(validate_text($self->{NAME})))	{
		$results->{name} = 'Please enter a name.';
		$error_flag = 1;
	}
	
	# There are errors, return the errors hash
	return false if($error_flag);
	
	$results->{body} = $self->{BODY};
	$results->{name} = $self->{NAME};

	return true;
} 

sub validate_text {
    
    my $text = shift;
    
    if(length($text)<1) {
	    return undef;
	} else {
	    return $text;
	}
}
   
#-------------------------------------------------------------------------------
# Accessor methods.
#-------------------------------------------------------------------------------
sub body {
    my $self = shift;
    # Set comment if method was called with a parameter
    $self->{BODY} = shift if (@_);
    return $self->{BODY};
}

sub name {
    my $self = shift;
    # Set name if method was called with a parameter
    $self->{NAME} = shift if (@_);
    return $self->{NAME};
}

sub date {
    my $self = shift;
    # Set name if method was called with a parameter
    $self->{DATE} = shift if (@_);
    return $self->{DATE};
}

sub page_id {
    my $self = shift;
    # Set name if method was called with a parameter
    $self->{PAGE_ID} = shift if (@_);
    return $self->{PAGE_ID};
}
1;