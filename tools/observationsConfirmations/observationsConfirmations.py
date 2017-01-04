#!/usr/bin/env python

#
# 
#
import sys
import os
import shutil
import urllib
import re
from datetime import datetime, timedelta
import math
import itertools
from sklearn.cluster import KMeans
import numpy as np
import pytz

#
# Parameters (see usage() for details)
#
class Parameters:
    # The MPC code of the telescope that will observe
    OBSERVATORY = 'F52'
    # The default number of repeats in a set 
    REPEATS = 1
    # The maximum size of a chunk 
    MAXIMUM_OUTLIER_COUNT = 4 
    MINIMUM_CHUNK_SIZE = 6 
    MAXIMUM_CHUNK_SIZE = 12
    # The asteroids we want to observe
    ASTEROIDS_FILTER = 'PANSTARRS'
    # The different prefixes of asteroids we want to observe
    ASTEROID_PREFIX_FILTERS = { 'PANSTARRS': ['P1', 'P2'],
                                'ALL': [ '' ] }
    # The times for which we want the ephemerides
    # NOTE: MULTIPLE EPOCHS ARE NOT FULLY SUPPORTED YET
    TIMES = [ 10, # 10am UT = midnight HST
              ]
    # Defined for tonight
    WHEN_TODAY = datetime.utcnow().date().isoformat().replace("-", " ")
    # Next is defined just in case the script is run before 2pm HST
    WHEN_TOMORROW = (datetime.utcnow().date()+timedelta(days=1)).isoformat().replace("-", " ")
    # Maximum distance (in degrees) between two sites of one chunk
    MAXIMUM_DISTANCE = 1.0
    # 
    MAXIMUM_ITERATIONS = 50
    DEBUG = False
    JPEG = False
    MOPSTEST = False
    MAGNITUDE = 21.5
    AGE = timedelta(days = 3.0)
    pass

#
# How to use this script
#
def usage(arguments, exit_status = 1):
    if exit_status != 0 and len(arguments)>1:
        print 
        print "Can't interpret arguments [%s]" % " ".join(arguments[1:])
    print """
  %s [-v] [-mopstest] [-r <repeats>] [-c <maximum chunk size>] [-all] [-jpeg]
  %s [-h]

  Build chunks for confirmation of Pan-STARRS asteroids posted on 
  the MPC CP (the '-all' option allows selecting all asteroids posted on
  the MPC CP).

  The product name is the text file named "clusters.txt" which contains
  a list of (right ascension, declination) sky coordinates grouped in 
  chunks, that is, repeated positions in a cluster of size coordinates.
  The maximum size of a chunk can be user-defined using the '-c' option; 
  the number of repeats can be controlled using the '-r' options.

  The gnuplot directory contains files that can be used to plot data
  (just run "cd gnuplot; gnuplot plot_chunks.gnuplot" for that).

  -c <maximum chunk size>:
    Maximum chunk size (default: %d) e.g.:
     For 3 repeats, chunks built with a maximum chunk size of 20,
     will contain at most 6 different observations sites (i.e. 
     clusters size is at most 6).
     However at the end of the process, we try to aggregate isolated 
     points (outliers) to large clusters.

  -r <repeats>:
    Number of repeats in a cluster (default: %d)

  -all:
    Grab all observations from MPC CP (not only P1 and P2 ones)

  -mopstest:
    Run only the MOPS part of the test (and not the OTIS one).
    Default: False (that is: Do run the OTIS part)

  -magnitude <magnitude>:
    Limiting magnitude. Ignore objects with a V-magnitude greater
    than <magnitude>.
    Default: 21.5

  -age <age in days>
    Limiting age. Ignore objects older than <age in days> days.

  -jpeg:
    Output a jpg image of the clusters aspect

  -h: 
    Show this help

  -v:
    Verbose mode

""" % (arguments[0], arguments[0], 
       Parameters.REPEATS, Parameters.MAXIMUM_CHUNK_SIZE)
    sys.exit(exit_status)

class DerivedParameters:
    @staticmethod
    def setup():
        DerivedParameters.TIME_FILTERS = [ "%s %d" % (Parameters.WHEN_TODAY, 
                                                      time) for time in Parameters.TIMES]
        DerivedParameters.TIME_FILTERS.extend([ "%s %d" % (Parameters.WHEN_TOMORROW, 
                                                           time) for time in Parameters.TIMES])
        # DerivedParameters.MAXIMUM_DISTANCE = math.radians(Parameters.MAXIMUM_DISTANCE)
         # If DerivedParameters.MAXIMUM_CLUSTER_SIZE is too large (i.e. greater than 20, expect a long runtime)
        DerivedParameters.MAXIMUM_CLUSTER_SIZE = Parameters.MAXIMUM_CHUNK_SIZE #/ Parameters.REPEATS
        DerivedParameters.MINIMUM_CLUSTER_SIZE = Parameters.MINIMUM_CHUNK_SIZE
        if DerivedParameters.MAXIMUM_CLUSTER_SIZE < DerivedParameters.MINIMUM_CLUSTER_SIZE:
            sys.stderr.write('Not enough observations sites in chunk (%d) relatively to the number of repeats (%d)' 
                             % (Parameters.MAXIMUM_CHUNK_SIZE, Parameters.REPEATS) )
            #Parameters.MAXIMUM_CHUNK_SIZE = 3*Parameters.REPEATS
            #sys.stderr.write('Forcing the number of observations sites to 3 times the number of repeats, i.e. 3*%d = %d\n'
                             #% ( Parameters.REPEATS, Parameters.MAXIMUM_CHUNK_SIZE) )
            #DerivedParameters.MAXIMUM_CLUSTER_SIZE = 3
            exit()
        DerivedParameters.AVERAGE_CLUSTER_SIZE = (DerivedParameters.MAXIMUM_CLUSTER_SIZE+DerivedParameters.MINIMUM_CLUSTER_SIZE)/2
        DerivedParameters.MAXIMUM_DISTANCE = math.radians(Parameters.MAXIMUM_DISTANCE)
        DerivedParameters.NOW = datetime.utcnow().replace(tzinfo = pytz.utc)
        pass
    pass

# 
# Various constants
#
class Constants:
    # MPC NEO Confiramtion Page URL
    MPC_NEOCP_URL = 'http://www.minorplanetcenter.net/iau/NEO/ToConfirm.html'
    # MPC CGI script *string format* for querying ephemerides
    # The first argument (str) is "obj=<provisional designation of object 1>&obj=<provisional designation of object 2>...etc"
    # The second one (str) the observatory code
    # Use this way:
    #   MPC_EPHEMERIS_CGI % (list of objects with "&obj=", 'F52')
    # e.g.:
    #   MPC_EPHEMERIS_CGI % ('obj=P1abcdE&obj=P1abcdF&obj=P1abcdG', 'F52')
    MPC_EPHEMERIS_CGI = "http://cgi.minorplanetcenter.net/cgi-bin/confirmeph2.cgi?mb=-30&mf=%f&dl=-90&du=%%2B90&nl=0&nu=100&sort=d&W=j&%s&Parallax=1&obscode=%s&long=&lat=&alt=&int=0&start=0&raty=d&mot=m&dmot=p&out=f&sun=x&oalt=20"
    pass

# 
# An elementary ephemeris. 
#  - epoch is MJD UT
#  - ra, dec, and position_angle are stored in radians
#  - velocity in radians/sec
#
class Ephemeris:
    def __init__(self, epoch, ra, dec, velocity, position_angle, nickname="unknown"):
        self.epoch = epoch
        self.ra = ra
        self.dec = dec
        self.velocity = velocity
        self.position_angle = position_angle
        self.nickname = nickname
        self.cosLongitude = math.cos(self.ra)
        self.sinLongitude = math.sin(self.ra)
        self.cosLatitude = math.cos(self.dec)
        self.sinLatitude = math.sin(self.dec)
        self.coordinates = [0., 0., 0.]
        self.coordinates[0] = self.cosLatitude*self.sinLongitude
        self.coordinates[1] = self.sinLatitude*self.sinLongitude
        self.coordinates[2] = self.cosLongitude
        pass
    def __str__(self):
        return "[%s] (ra = %f (deg), dec = %f (deg)); %f (deg/day) ; %f (deg)" % (self.epoch,
                                                                                  math.degrees(self.ra),
                                                                                  math.degrees(self.dec),
                                                                                  math.degrees(self.velocity)*86400.,
                                                                                  math.degrees(self.position_angle) )
    def distance(self, ephemeris):
        x = self.coordinates[1]*ephemeris.coordinates[2]-self.coordinates[2]*ephemeris.coordinates[1]
        y = self.coordinates[2]*ephemeris.coordinates[0]-self.coordinates[0]*ephemeris.coordinates[2]
        z = self.coordinates[0]*ephemeris.coordinates[1]-self.coordinates[1]*ephemeris.coordinates[0]
        cross_product_norm = math.sqrt(x*x+y*y+z*z);
        dot_product = ( self.coordinates[0]*ephemeris.coordinates[0]
                        + self.coordinates[1]*ephemeris.coordinates[1]
                        + self.coordinates[2]*ephemeris.coordinates[2] )
        return math.atan2(cross_product_norm, dot_product);

def age_and_v_fine(line):
    interesting = line.split("[")[1].split("]")[0]
    # Check V-magnitude
    v = float(re.sub("^.*V = ", "", interesting))
    if v > Parameters.MAGNITUDE:
        return False
    # Check date now
    # MPC found yet another way to implement dates...
    date = re.sub(" UT.*", "", interesting)
    (year, month, day) = date.split(" ")
    month = month.replace(".", "")
    submitted = datetime.strptime("%s %s %d" % (year, month, int(float(day))), "%Y %b %d")
    return submitted > datetime.utcnow()-Parameters.AGE

#
# Get the ephemerides interesting us from the MPC NEO Confirmation Page
#
def grabEphemerides():
    print "Loading MPC NEO Confirmation Page"
    # Filter the asteroids we want to followup
    mpc_neocp_html = urllib.urlopen(Constants.MPC_NEOCP_URL)
    candidates_for_followup = [ ]
    for prefix_filter in Parameters.ASTEROID_PREFIX_FILTERS[Parameters.ASTEROIDS_FILTER]:
        pattern = re.compile('^<input type="checkbox" name="obj" VALUE="' + prefix_filter + '(.*)"> .*$')
        for line in mpc_neocp_html:
            matches = pattern.match(line)
            if matches is not None:
                if age_and_v_fine(line): # We don't want stuff too old or too faint
                    candidates_for_followup.append(prefix_filter + matches.group(1))
                pass
            pass
        pass
    mpc_neocp_html.close()
    if len(candidates_for_followup) == 0:
        print "No candidates for those paramters. Use -all? Increase -magnitude? Increase -age?"
        sys.exit(0)
        pass
    if Parameters.DEBUG:
        print "%d candidates for followup: %s" % (len(candidates_for_followup), candidates_for_followup)
        pass

    # Build up the url. 
    # First transform the candidates names since they can contain special characters
    urled_candidates = [ ]
    for candidate in candidates_for_followup:
        urled_candidates.append(urllib.quote(candidate))
        pass
    objects = "&obj=".join(urled_candidates)
    objects = "obj=%s" % objects
    url = Constants.MPC_EPHEMERIS_CGI % (Parameters.MAGNITUDE,
                                         objects,
                                         Parameters.OBSERVATORY)
    if Parameters.DEBUG:
        print "CGI url = [%s]" % url
        pass

    print "Requesting NEO ephemerides"
    contents = urllib.urlopen(url)
    current_object = None
    ephemerides = { }
    for line in contents:
        # First the name of the object is presented
        for candidate in candidates_for_followup:
            if candidate in line:
                current_object = candidate
                #print "Current Object = '%s'"%current_object
                pass
            pass
        # Then its ephemeris
        for time in DerivedParameters.TIME_FILTERS:
            if line.startswith(time):
                fields = line.split()
                ra = math.radians(float(fields[4])*15.)
                dec = math.radians(float(fields[5]))
                # velocity is in arcseconds/minute. We want it in radians/s
                velocity = math.radians(float(fields[8])/3600./60.)
                position_angle = math.radians(float(fields[9]))
                try:
                    ephemeris = Ephemeris(time, ra, dec, velocity, position_angle, current_object)
                    ephemerides_time = ephemerides[time]
                    ephemerides_time[current_object] = ephemeris
                except KeyError:
                    ephemerides[time] = { }
                    ephemerides_time = ephemerides[time]
                    ephemerides_time[current_object] = ephemeris
                pass
            pass
        pass
    return ephemerides

#
# Outliers are points that are too far from the others
#
def find_outliers(_ephemerides):
    if Parameters.DEBUG:
        #print "Isolating outliers NEO"
        pass
    ephemerides_index_set = [ ]
    outliers_index_set = [ ]
    for i0 in range(len(_ephemerides)):
        shortest_distance = 100.
        for i1 in range(len(_ephemerides)):
            if i0 != i1:
                distance = _ephemerides[i0].distance(_ephemerides[i1])
                if shortest_distance > distance:
                    shortest_distance = distance
            pass
        if shortest_distance < DerivedParameters.MAXIMUM_DISTANCE:
            ephemerides_index_set.append(i0)
        else:
            outliers_index_set.append(i0)
            pass
        pass
    
    return ( [_ephemerides[i] for i in ephemerides_index_set],
             [_ephemerides[i] for i in outliers_index_set] )

#
# Based on K-means clustering.
#   http://en.wikipedia.org/wiki/K-means_clustering
#
# If a cluster is too large, it is split using the same method (with a
# different number of clusters)
# 
def create_chunks(_ephemerides, number_of_chunks, depth = 0):
    if Parameters.DEBUG:
        print "Create Chunks", number_of_chunks
        pass

    # remove obvious outliers
    (ephemerides, outliers) = find_outliers(_ephemerides)

    if len(ephemerides) == 0:
        print "Only outliers in this set"
        return ({0: outliers}, 0, 0, len(outliers))
    if len(ephemerides) <  number_of_chunks:
        print "Not enough elements in the set of ephemerides compared to the number of chunks wanted (%d for %d)" % (len(ephemerides),
                                                                                                                     number_of_chunks)
        return ({0: ephemerides, -1: outliers,}, len(ephemerides), len(ephemerides), len(outliers))
    
    if depth == 0 and Parameters.DEBUG:
        #print "Creating NEO Clusters"
        pass
    if Parameters.DEBUG:
        print "%sL = %d, # = %d" % (" "*depth, len(ephemerides), number_of_chunks)
        pass
    kmeans = KMeans(init='k-means++', 
                    n_clusters = number_of_chunks,
                    n_init=10)
    ra_ = np.array([ephemeris.ra for ephemeris in ephemerides])
    dec_ = np.array([ephemeris.dec for ephemeris in ephemerides])
    X = np.array([ np.cos(ra_), 
                   np.sin(ra_), 
                   np.cos(dec_), 
                   np.sin(dec_) ]).transpose()
    kmeans.fit(X)
    labels = kmeans.labels_

    # Group the ephemerides by label
    chunks = { }
    index = 0
    for ephemeris in ephemerides:
        try:
            chunks[labels[index]].append(ephemeris)
        except KeyError:
            chunks[labels[index]] = [ ]
            chunks[labels[index]].append(ephemeris)
            pass
        index += 1
        pass

    # Split the chunks that are too large
    final_chunks = { }
    current_label = 0
    smallest_chunk_size = 100000
    largest_chunk_size = 0
    for chunk in chunks.itervalues():
        if len(chunk) <= DerivedParameters.MAXIMUM_CLUSTER_SIZE:
            if len(chunk) > 0:
                final_chunks[current_label] = chunk
                if largest_chunk_size < len(chunk):
                    largest_chunk_size = len(chunk)
                    pass
                if smallest_chunk_size > len(chunk):
                    smallest_chunk_size = len(chunk)
                    pass
                current_label += 1
                pass
            pass
        else:
            num_chunks_requested = len(chunk_ephemerides) / DerivedParameters.AVERAGE_CLUSTER_SIZE + 1

            (temporary_chunks, smallest_tmp, largest_tmp, num_outliers) = create_chunks(chunk, num_chunks_requested,depth + 1)
                                                             
            for temporary_chunk in temporary_chunks.itervalues():
                if len(temporary_chunk) >0:
                    final_chunks[current_label] = temporary_chunk
                    if largest_chunk_size < len(temporary_chunk):
                        largest_chunk_size = len(temporary_chunk)
                        pass
                    if smallest_chunk_size > len(temporary_chunk):
                        smallest_chunk_size = len(temporary_chunk)
                        pass
                    current_label += 1
                    pass
                pass
            pass
        pass
    final_chunks[current_label] = outliers
    current_label += 1
    return (final_chunks, smallest_chunk_size, largest_chunk_size, len(outliers))

#
# Traveling Salesman Problem.
# Brute Force implementation.
#
def tsp_brute(chunk):
    distances = { }
    for i0 in range(len(chunk)):
        distances[chunk[i0]] = { }
        for i1 in range(i0+1, len(chunk)):
            distances[chunk[i1]] = { }
            pass
        pass

    for i0 in range(len(chunk)):
        distances[chunk[i0]][chunk[i0]] = 0
        for i1 in range(i0+1, len(chunk)):
            if distances[chunk[i1]] is None:
                distances[chunk[i1]] = { }
            d = chunk[i0].distance(chunk[i1])
            distances[chunk[i0]][chunk[i1]] = d
            distances[chunk[i1]][chunk[i0]] = d
            pass
        pass
    minimum_length = 10000.
    minimum_cycle = None
    combination_length = len(chunk)
    for combination in itertools.combinations(chunk, combination_length):
        current_length = 0.
        for index in range(combination_length):
            current_length += distances[combination[index]][combination[(index+1) % combination_length]]
            pass
        if minimum_length > current_length:
            minimum_length = current_length
            minimum_cycle = combination
            pass
        pass
    return minimum_cycle

#
# Fix for small clusters: for each of them
# - either aggregate it to the nearest "large" chunk,
# - or considered it as an outlier
#
def fix_small_clusters(chunks):
    print "Fixing clusters that are too small"
    outliers_label = len(chunks)-1
    outliers = chunks[outliers_label]
    _clusters = []
    clusters_too_small = []
    for label, ephemerides in chunks.iteritems():
        if label != outliers_label:
            if len(ephemerides) >= DerivedParameters.MINIMUM_CLUSTER_SIZE:
                _clusters.append(ephemerides)
            else:
                clusters_too_small.append(ephemerides)
                pass
            pass
        pass # Don't do anything for outliers
    print "... %d clusters have more than %d elements" % (len(_clusters), DerivedParameters.MINIMUM_CLUSTER_SIZE)
    print "... %d clusters have less than %d elements" % (len(clusters_too_small), DerivedParameters.MINIMUM_CLUSTER_SIZE)
    print "... Outliers cluster contains %d elements" % (len(outliers))
    # Compute the distance of each "too small" cluster to the large ones
    # Aggregate it if it is "not too far", classify it as an outlier otherwise
    for small_cluster in clusters_too_small:
        smallest_distance = 100.
        best_large_cluster = None
        for large_cluster in _clusters:
            for ephemeris_in_large in large_cluster:
                for ephemeris_in_small in small_cluster:
                    current_distance = ephemeris_in_small.distance(ephemeris_in_large)
                    if current_distance <= DerivedParameters.MAXIMUM_DISTANCE:
                        if current_distance < smallest_distance:
                            smallest_distance = current_distance
                            best_large_cluster = large_cluster
                            pass
                        pass
                    pass
                pass
            pass
        if best_large_cluster is not None:
            best_large_cluster.extend(small_cluster)
        else:
            outliers.extend(small_cluster)
        pass
    # Build the clusters
    index = 0
    clusters = { }
    biggest_cluster = 0
    for cluster in _clusters:
        clusters[index] = cluster
        if(biggest_cluster < len(cluster)) :
           biggest_cluster = len(cluster)
        index += 1
        pass
    clusters[-1] = outliers
    #print "... Rearranged in %d clusters" % (len(clusters))
    #print "... Outliers cluster contains %d elements" % (len(clusters[-1]))
    return (biggest_cluster, clusters)

#
# Arguments processing
#
def process_arguments(arguments):
    index = 1
    while index<len(arguments):
        if arguments[index] == '-v':
            Parameters.DEBUG = True
            index += 1
        elif arguments[index] == '-h':
            usage(arguments, 0)
        elif arguments[index] == '-jpeg':
            Parameters.JPEG = True
            index += 1
        elif arguments[index] == '-all':
            Parameters.ASTEROIDS_FILTER = 'ALL'
            index += 1
        elif arguments[index] == '-c':
            Parameters.MAXIMUM_CHUNK_SIZE = int(arguments[index+1])
            index += 2
        elif arguments[index] == '-r':
            Parameters.REPEATS = int(arguments[index+1])
            index += 2
        elif arguments[index] == '-mopstest':
            Parameters.MOPSTEST = True
            index += 1
        elif arguments[index] == '-magnitude' or arguments[index] == "-m":
            Parameters.MAGNITUDE = float(arguments[index+1])
            index += 2
        elif arguments[index] == '-age' or arguments[index] == "-a":
            Parameters.AGE = timedelta(days = float(arguments[index+1]))
            index += 2
        else:
            usage(arguments)
            pass
        pass
    pass

#
# main()
#
if __name__ == '__main__':
    try:
        # Let's make sure we can write to the current directory
        os.mkdir('gnuplot')
    except OSError, e:
        if not 'File exists' in str(e):
            raise e # otherwise ignore
        pass
    
    # Process arguments
    process_arguments(sys.argv)

    chunks_filename = "chunks.txt"
    clusters_filename = "clusters.txt"
    image_filename = "clusters.jpg"
    positions_filename = "gnuplot/positions.txt"
    edges_filename = "gnuplot/edges.txt"
    gnuplot_filename = "gnuplot/plot_chunks.gnuplot"

    # Initializations
    DerivedParameters.setup()
    # Get data from MPC
    ephemerides = grabEphemerides()
    
    #######################
    # added by Todd 3/13
    chunks = None
    #######################

    # Iterate over different times
    for time in ephemerides.iterkeys():
        chunk_ephemerides = [ ]
        for item in ephemerides[time].iteritems():
            if Parameters.DEBUG:
                print "  %s: %s" % (item[0], item[1])
                pass
            chunk_ephemerides.append(item[1])
            pass
        print "... Got %d ephemerides at %s" % (len(ephemerides[time]), time)
        # Now iterate a few times to get some clustering
        chunks = []
        largest_chunk_size = 0
        smallest_chunk_size = 0
        num_outliers = 100
        while(num_outliers > Parameters.MAXIMUM_OUTLIER_COUNT and Parameters.MAXIMUM_DISTANCE < 30):
           for iteration in range(Parameters.MAXIMUM_ITERATIONS):
               if (iteration+1) % 10 == 0:
                   #print "... Iteration %d out of %d" % (iteration+1, Parameters.MAXIMUM_ITERATIONS)
                   pass

               num_chunks_requested = len(chunk_ephemerides) / DerivedParameters.AVERAGE_CLUSTER_SIZE + 1

               (current, smallest_chunk_in_current, largest_chunk_in_current, num_outliers) = create_chunks(chunk_ephemerides, num_chunks_requested)

               if Parameters.DEBUG:
                   for c in current: 
                       print "Chuck%d  %d"%(c, len(current[c]))
                       pass
                   print
                   pass

               if largest_chunk_size < largest_chunk_in_current:
                   largest_chunk_size = largest_chunk_in_current
               if smallest_chunk_in_current > 0 and smallest_chunk_size > smallest_chunk_in_current:
                   smallest_chunk_size = smallest_chunk_in_current
               if(len(chunks) > 5) :
                   largest_chunk_size = len(chunks)
                   #pass

               chunks = current
               if Parameters.DEBUG:
                   print "smallest chunk = %d/%d,  largest chunk = %d)"%(smallest_chunk_size,smallest_chunk_in_current, largest_chunk_size)
                   print "pass -> %d > %d @ %f degs"%(smallest_chunk_size, DerivedParameters.MINIMUM_CLUSTER_SIZE, math.degrees(DerivedParameters.MAXIMUM_DISTANCE))
                   print
                   pass
           if(num_outliers > Parameters.MAXIMUM_OUTLIER_COUNT):
              Parameters.MAXIMUM_DISTANCE += 0.5
              DerivedParameters.MAXIMUM_DISTANCE = math.radians(Parameters.MAXIMUM_DISTANCE)
        pass

        print "Completed pass ended (%d < x < %d) with MAX_DISTANCE = %f degrees"%(smallest_chunk_size, largest_chunk_size, math.degrees(DerivedParameters.MAXIMUM_DISTANCE))

    # Clusters with a size which is too small are:
    # either aggregated to the nearest "large" chunk,
    # or considered as outliers
    (biggest_cluster, clusters) = fix_small_clusters(chunks)

    positions_file = open(positions_filename, "w")
    edges_file = open(edges_filename, "w")
    chunks_file = open(chunks_filename, "w")
    clusters_file = open(clusters_filename, "w")
    outliers_label = -1
    print "Finding shortest cycles in %d clusters" % (len(clusters))
    positions_file.write("# Outliers label is %d\n" % outliers_label)
    for label, ephemerides in clusters.iteritems():
        if Parameters.DEBUG:
            print len(ephemerides), ephemerides
            pass
        if label == outliers_label:
            positions_file.write("# Outliers\n")
        else:
            positions_file.write("# label %d\n" % label)
            pass
        for ephemeris in ephemerides:
            positions_file.write("%f %f %d %s\n" % (math.degrees(ephemeris.ra), 
                                                 math.degrees(ephemeris.dec),
                                                 label, ephemeris.nickname) )
            pass
        #if  label == outliers_label:
        if  label == 99:
            print "... Excluding outliers cluster (label = %d) from analysis" % outliers_label
            chunks_file.write("# Outliers\n")
            clusters_file.write("# Outliers\n")
            for index in range(len(ephemerides)):
                chunks_file.write(" %f %f %s\n" % (math.degrees(ephemerides[index].ra), math.degrees(ephemerides[index].dec), ephemerides[index].nickname))
                clusters_file.write(" %f %f %s\n" % (math.degrees(ephemerides[index].ra), math.degrees(ephemerides[index].dec), ephemerides[index].nickname))
        else:
            #chunks_file.write("# Chunk %d\n" % label)
            chunks_file.write("\n")
            #clusters_file.write("# Cluster %d\n" % label)
            clusters_file.write("\n")
            #edges_file.write("# Cluster %d\n" % label)
            edges_file.write("\n")
            print "... For Cluster %d" % label
            edges = tsp_brute(ephemerides)
            for index in range(len(edges)):
                for repeat in range(Parameters.REPEATS):
                    chunks_file.write("%d, %f, %f, %s\n" % (label, math.degrees(edges[index].ra), math.degrees(edges[index].dec),edges[index].nickname))
                    pass
                clusters_file.write("%d, %f, %f, %s\n" % (label, math.degrees(edges[index].ra), math.degrees(edges[index].dec), edges[index].nickname))
                edges_file.write("%d, %f, %f, %s\n" % (label, math.degrees(edges[index].ra), math.degrees(edges[index].dec), edges[index].nickname))
                pass
            edges_file.write("%d, %f, %f, %s\n" % (label, math.degrees(edges[0].ra), math.degrees(edges[0].dec), edges[0].nickname))
            chunks_file.write("\n")
            clusters_file.write("\n")
            edges_file.write("\n")
            pass
        pass
    print "Done"

    positions_file.close()
    edges_file.close()
    chunks_file.close()
    clusters_file.close()

    gnuplot_drawing_commands = """
set xlabel "RA (degrees)"
set ylabel "DEC (degrees)"
set grid
plot "./positions.txt" u 1:2:3 w p palette pt 7 ps 4 t "", "./positions.txt" u 1:2:3 w labels t "", "./edges.txt" w l t ""
"""
    gnuplot_file = open(gnuplot_filename, "w")
    gnuplot_file.write("""
set terminal x11 size 1024,768
%s
pause mouse any
""" % gnuplot_drawing_commands)
    gnuplot_file.close()

    print
    print "For a visualization of the result, run:"
    print "cd gnuplot; gnuplot plot_chunks.gnuplot"
    print

    print "Chunks are in [%s] (Outliers are commented out at the end of that file)." % chunks_filename
    print

    if Parameters.JPEG:
        cmd_filename = "jpeg.gnuplot"
        gnuplot_file = open("gnuplot/%s" % cmd_filename, "w")
        gnuplot_file.write("""
set term jpeg size 1024,768
set output "%s"
%s
""" % (image_filename, gnuplot_drawing_commands))
        gnuplot_file.close()
        try:
            os.remove(image_filename)
        except OSError:
            pass
        import subprocess
        p = subprocess.Popen(["gnuplot", cmd_filename],
                             cwd = 'gnuplot')
        p.wait()
        os.remove("gnuplot/%s" % cmd_filename)
        os.rename("gnuplot/%s" % image_filename, image_filename)

    #move old chunks to history directory
    oldQues = os.listdir("./")
    for folder in oldQues :
      if("NeoChunks" in folder):
         #move it into historyA
         print "moving", folder, "to history/"+folder
         try :
            shutil.move(folder, "history/"+folder)
         except :
            shutil.rmtree('history/'+folder)
            shutil.move(folder, "history/"+folder)

    ### create queue format
    chunks_file = open(chunks_filename, "r")
    content = chunks_file.readlines()
    print time
    outputQ = []
    cur_ID = -1
    for line in content:
       if(line[0] is '#' or line[0] is ' ' or line[0] is "\n") :
          continue
          
       data = line.split(',')
       if(len(data) > 1) :
          thisID = int(data[0].strip())
          thisRA = float(data[1].strip())
          thisDEC = float(data[2].strip())
          thisNEO = data[3].strip()
          if(thisID != cur_ID):
             cur_ID = thisID
             outputQ.append({})
          outputQ[cur_ID][thisNEO] = (thisRA, thisDEC)
    chunks_file.close()

    if Parameters.MOPSTEST:
        print "-mopstest option set. Finishing here"
        sys.exit(0)

    readme_content = []
    readme_content.append(time)
    count = 0
    outputQ.sort()
    generated = False
    sortedQ = []
    for que in outputQ :
       sortedChunk = []
       for neo in que :
          sortedChunk.append((que[neo][1],que[neo][0],neo))
       sortedChunk.sort()
       sortedQ.append(sortedChunk)

    for que in sortedQ :
       time_str = time.split(' ')

       directory_name ="%s%s%s_%02dUTC_NeoChunks"%(time_str[0],time_str[1],time_str[2],int(time_str[3]))
       generated = True
       if not os.path.exists(directory_name):
          os.makedirs(directory_name)

       que_filename = "%s%s%s_%02dUTC_NeoChunk%d.ps2que"%(time_str[0],time_str[1],time_str[2],int(time_str[3]),count)
       queContent = []
       #print que_filename
       que_file = open(directory_name +'/'+ que_filename, "w")
       #print
       #print que
       #print
       v_count = 0
       max_ra = 0
       min_ra = 360
       max_dec = -90
       min_dec = 90
       for visit in range(4) :
          v_count += 1
          for neo in que :
             ra = neo[1]
             if ra > max_ra : max_ra = ra
             if ra < min_ra : min_ra = ra
   
             #ra = ra -120.0   #for day time testing
             dec = neo[0]
             if dec > max_dec : max_dec = dec
             if dec < min_dec : min_dec = dec
         
             if(v_count == 1) :
                #first visit stuff
                queContent.append('{"MOUNT_OFF":False, "TEL_RA":%f, "TEL_DEC":%f, "TEL_ROT":90, "TRACK": True, "PA_MARK":"%s", "EXP_COMMENT":"%s visit%d", "EXP_TYPE":"OBJECT", "FITS_OBJECT":"%s", "FITS_OBSMODE":"OSS", "FITS_QUEUEID":"%s%s%s_%sUTC_Chunk%d"}\r\n'%(ra, dec,neo[2],neo[2],v_count,neo[2],time_str[0],time_str[1],time_str[2],time_str[3],count))
             else :
                queContent.append('{"MOUNT_OFF":False, "TEL_RA":%f, "TEL_DEC":%f, "PA_RECALL":"%s", "EXP_COMMENT":"%s visit%d", "EXP_TYPE":"OBJECT", "FITS_OBJECT":"%s", "FITS_OBSMODE":"OSS", "FITS_QUEUEID":"%s%s%s_%sUTC_Chunk%d"}\r\n'%(ra, dec,neo[2],neo[2],v_count,neo[2],time_str[0],time_str[1],time_str[2],time_str[3],count))

       queContent.insert(0,  "# NeoChuck%d contains %2d neo candidates (%d visits) (%5.1f < RA < %5.1f)  (%5.1f < DEC < %5.1f)\r\n\r\n"%(count,len(que), 4, min_ra,max_ra,min_dec,max_dec))

       for line in queContent :
          que_file.write(line)

       readme_content.append("NeoChuck%d contains %2d neo candidates (%d visits) (%5.1f < RA < %5.1f)  (%5.1f < DEC < %5.1f)"%(count,len(que), v_count, min_ra,max_ra,min_dec,max_dec))
       readme_content.append("\r\n")
       print "NeoChuck%d contains %2d neo candidates (%d visits) (%5.1f < RA < %5.1f)  (%5.1f < DEC < %5.1f)"%(count,len(que), v_count, min_ra,max_ra,min_dec,max_dec)
       print
       for neo in que :
          readme_content.append("   %s  (%f, %f)"%(neo[2], neo[1], neo[0]))
          print "   %s  (%f, %f)"%(neo[2], neo[1], neo[0])
       que_file.close()
       count+=1
       print
       print
       readme_content.append("\r\n")
       readme_content.append("\r\n")

    if(generated) :
       readme_file = open(directory_name +'/readme', "w")
       for line in readme_content :
          readme_file.write(line +'\r\n')
       readme_file.close()

#
# CHANGES
#
# 2015-02-11:
#  * Repeat observations
#  * Chunks are repeated observations / Clusters are observations
#  * Observations are repeated in product chunks.txt, not in clusters.txt
#  * -all option to observe all observable on MPC CP
#  * Clusters of 2 are forbidden
#  * -jpeg option
#
# 2015-01-26:
#  First draft
#        
        
