from __future__ import division
"""
$Id$

Python module for searching of low-significance 
detection (LSD) archive files.
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import sys
import os
import os.path
import re
import math
import time
import struct               # pack/unpack

import slalib
from MOPS.Constants import *
import MOPS.Detection
import auton


NSD_SRC_EXT = 'nsd.src.bin'
NSD_STRIP_EXT = 'nsd.strip.bin'
NSD_EXT = 'nsd.bin'
NSD_IDX_EXT = 'nsd.idx'

SYD_SRC_EXT = 'syd.src.bin'
SYD_STRIP_EXT = 'syd.strip.bin'
SYD_EXT = 'syd.bin'
SYD_IDX_EXT = 'syd.idx'

USED_EXT = 'used'

debug = 1


class LSD(MOPS.Detection.Detection):
    """
    Implements a container for low-significance detections (LSDs).
    """

    """
    LSDs
    use constant NSD_PACK_TEMPLATE => join('',
        'l',        # field_id (4)
        'l',        # det_num (4)
        'ddd',      # epoch/ra/dec (24)
        'fffff',    # mag/s2n/sigmas (20)
    );
    use constant NSD_TEMPLATE_LENGTH => 52;

    SYDs
    use constant SYD_PACK_TEMPLATE => join('',
        'l',        # field_id (4)
        'q',        # det_id (8)
        'ddd',      # epoch/ra/dec (24)
        'fffff',    # mag/s2n/sigmas (20)
        'A12',
    );
    use constant SYD_TEMPLATE_LENGTH => 56;

    Python 'l' is 8 bytes, but '!l' is 4 bytes.
    "=" means native byte ordering, "standard" alignment
    """

    NSD_PACK_TEMPLATE = '= ll ddd fffff'
    NSD_TEMPLATE_LENGTH = 52

    SYD_PACK_TEMPLATE = '= l q ddd fffff 12s'
    SYD_TEMPLATE_LENGTH = 68

    def __init__(self, obscode, nsd_data=None, syd_data=None):
        """
        Create the LSD from some data binary extracted from an archive
        file.  This data must be struct.unpacked() into separate elements.

        field_id (i)
        det_num (i)
        epoch_mjd/ra_deg/dec_deg (ddd)
        mag/s2n/ra_sigma/dec_sigma/mag_sigma (fffff)
        """

        if nsd_data is not None:
            foo = struct.unpack(self.NSD_PACK_TEMPLATE, nsd_data)
            field_id = foo[0]
            det_id = None
            det_num = foo[1]
            object_name = 'NS'
        else:
            foo = struct.unpack(self.SYD_PACK_TEMPLATE, syd_data)
            field_id = foo[0]
            det_id = foo[1]
            det_num = None
            object_name = foo[10].strip(' \x00')[0:9]  # strip leading/trailing whitespace, NULLs, first 9 chars

        MOPS.Detection.Detection.__init__(self, 
            det_id,         # _id (detId)
            foo[3],         # ra
            foo[4],         # dec
            foo[2],         # epoch_mjd
            foo[5],         # mag
            foo[5],         # refMag
            foo[6],         # s2n
            foo[7],         # raErr
            foo[8],         # decErr
            foo[9],         # magSigma
            0.0,            # orient
            0.0,            # length
            'L',            # LSD
            field_id,       # fieldId
            'r',            # filt, ugh, XXX need to fix this!
            obscode,        # MPC obscode
            object_name,    # objectName
            det_num         # detNum
        )


class Archive(object):
    """
    Implements an index to a nightly LSD archive file.  The index object
    can be queried for the starting and locations in a flat binary archive
    given a particular RA.

    The index is a text file containing field_id/slice_number entries pointing to
    a position in the binary detection file.
    """

    class ArchiveIndex(object):
        def __init__(self, idx_filename, debuglog=None):
            # Open the datafile so that we can work with it without re-opening it.
            self.idx_filename = idx_filename
            self.slice_data = {}
            self.slice_width_deg = 360.0 / 3600     # ugh XXX
            self.debuglog = debuglog

            if re.search(r'nsd', idx_filename, re.IGNORECASE):
                self.item_size = LSD.NSD_TEMPLATE_LENGTH
            elif re.search(r'syd', idx_filename, re.IGNORECASE):
                self.item_size = LSD.SYD_TEMPLATE_LENGTH
            else:
                raise RuntimeError("Can't determine type of LSD file: " + idx_filename)

            idx_lines = filter(lambda x: x[0] != '!' and x[0] != '#', file(self.idx_filename).readlines())

            for idx_line in idx_lines:
                field_id, ra, ra1, ra2, dec, dec1, dec2, start, length = idx_line.split()
                ra1 = float(ra1)
                start = int(start)
                length = int(length)
                field_id = int(field_id)
                slice_num = int(ra1 / self.slice_width_deg + 0.00001)   # handle round-off
                self.slice_data.setdefault(field_id, {}).setdefault(slice_num, {}).update(START=start, LENGTH=length)
            # <-- for idx_line

            if debug:
                sys.stderr.write("Found %d slices in index %s.\n" % (len(self.slice_data), self.idx_filename))

        def FieldRA2Ranges(self, field_id, min_ra_deg, max_ra_deg):
            """
            Given a field ID and RA in degrees, return a list of file
            ranges of slices containing the positions.  Note that we
            have to return a list, in case the span is discontinuous
            due to the 0-360 boundary.  
            """

            # Find slice numbers of endpoints.
            min_slice_num = int(min_ra_deg / self.slice_width_deg)
            max_slice_num = int(max_ra_deg / self.slice_width_deg)
            stuff = []

            # Scrub.
#            if min_slice_num >= self.num_slices:
#                min_slice_num = self.num_slices - 1
#            if min_slice_num < 0:
#                min_slice_num = 0
#
#            if max_slice_num >= self.num_slices:
#                max_slice_num = self.num_slices - 1
#            if max_slice_num < 0:
#                max_slice_num = 0


            if max_slice_num >= min_slice_num:
                # The normal case.  Return a list of positions and lengths for
                # each slice.
                for slice_num in range(min_slice_num, max_slice_num + 1):
                    thang = self.slice_data.get(field_id, {}).get(slice_num, None)
                    if thang:
                        stuff.append((thang['START'] * self.item_size, thang['LENGTH'] * self.item_size))
                # <-- for slice_num
            else:
                # The wraparound case.  Return at least two position/length
                # tuples, for min up to 360, then 0 to max.
                for slice_num in range(min_slice_num, self.num_slices):
                    thang = self.slice_data.get(field_id, {}).get(slice_num, None)
                    if thang:
                        stuff.append((thang['START'] * self.item_size, thang['LENGTH'] * self.item_size))
                # <-- for slice_num

                for slice_num in range(0, max_slice_num):
                    thang = self.slice_data.get(field_id, {}).get(slice_num, None)
                    if thang:
                        stuff.append((thang['START'] * self.item_size, thang['LENGTH'] * self.item_size))
                # <-- for slice_num

            return stuff

    class UsedIndex(object):
        def __init__(self, used_filename):
            """
            Read the used file and create a table of detection identifiers
            that can be easily looked up.  We'll concatenate field ID, det num
            and det ID.
            """
            self.table = {}
            all_lines = filter(lambda x: x[0] != '!' and x[0] != '#', file(used_filename).readlines())

            for line in all_lines:
                field_id, det_num, det_id = line.split()
                id = '%s:%s:%s' % (field_id, det_num, det_id)
                self.table[id] = 1

            if debug:
                sys.stderr.write("Found %d used detections in index %s.\n" % (len(self.table), used_filename))


        def is_used(self, det):
            """
            Return true if the detection is in the used detection table.
            """
            det_num = det.detNum
            if det_num is None:
                det_num = 0
            det_id = det._id
            if det_id is None:
                det_id = 0

            id = '%d:%d:%d' % (det.fieldId, det_num, det_id)
            return id in self.table


    def __init__(self, lsd_rootdir, nn):
        """
        Create our index object given a MOPS instance and night number (nn).
        """
        nn = int(nn)
        self.lsd_dir = os.path.join(lsd_rootdir, str(nn))
    
        self.nsd_filename = os.path.join(lsd_rootdir, str(nn), str(nn) + '.' + NSD_EXT)
        if os.path.exists(self.nsd_filename):
            self.nsd_file = file(self.nsd_filename)
            self.nsd_index = self.ArchiveIndex(os.path.join(lsd_rootdir, str(nn), str(nn) + '.' + NSD_IDX_EXT))
        else:
            self.nsd_file = None
            self.nsd_index = None
        # <- if 

        self.syd_filename = os.path.join(lsd_rootdir, str(nn), str(nn) + '.' + SYD_EXT)
        if os.path.exists(self.syd_filename):
            self.syd_file = file(self.syd_filename)
            self.syd_index = self.ArchiveIndex(os.path.join(lsd_rootdir, str(nn), str(nn) + '.' + SYD_IDX_EXT))
        else:
            self.syd_file = None
            self.syd_index = None
        # <- if 

        self.used_filename = os.path.join(lsd_rootdir, str(nn), str(nn) + '.' + USED_EXT)
        if os.path.exists(self.used_filename):
            self.used_index = self.UsedIndex(self.used_filename)
        else:
            self.used_index = None
        # <- if


    def FetchDetections(self, field, pos, maxSearchRadius_arcsec):
        """
        Return a list of all LSDs within pos+error as Detection objects.  Usually
        we expect that a prediction+error will lie completely within a single
        slice or at most straddle two slices.  The case we have to worry about is
        the 0-360 wraparound.

        usage_map is a UsageMap object that contains information about
        synthetic detections, SYDs, and previously upgraded LSDs.  This
        routine will use it to reject detections fetched from the LSD archive
        that have already been promoted into MOPS DB detections.
        """

        if not self.nsd_index:
            return []           # NSD does not exists; nothing to do

        detections = []         # list of detections from LSD archive
        fieldId = field._id

        maxSearchRadius_deg = maxSearchRadius_arcsec / ARCSECONDS_PER_DEG
        if pos.unc_smaa_arcsec < 0:
            err_deg = maxSearchRadius_deg
        else:
            err_deg = 3 * pos.unc_smaa_arcsec / ARCSECONDS_PER_DEG
        # <- if

        if err_deg > maxSearchRadius_deg:
            err_deg = maxSearchRadius_deg

        nsd_ranges = self.nsd_index.FieldRA2Ranges(fieldId, pos.ra_deg - err_deg, pos.ra_deg + err_deg)
        syd_ranges = self.syd_index.FieldRA2Ranges(fieldId, pos.ra_deg - err_deg, pos.ra_deg + err_deg)
        thresh_rad = err_deg * RAD_PER_DEG      # 3 => 3-sigma error ellipse

        try:    
            # LSDs.
            for (filepos, filelen) in nsd_ranges:
                # Ensure slice data has correct-looking size.
                if filelen % LSD.NSD_TEMPLATE_LENGTH != 0:
                    raise RuntimeError('Slice length is not a multiple of %s' % LSD.NSD_TEMPLATE_LENGTH)
                num_dets = int(filelen / LSD.NSD_TEMPLATE_LENGTH)

                # Load each range (slice) and scan for detections in the RA/DEC range.
                self.nsd_file.seek(filepos)
                buf = self.nsd_file.read(filelen)
                for n in range(0, num_dets):
                    det = LSD(field.obscode, nsd_data=buf[n * LSD.NSD_TEMPLATE_LENGTH:(n + 1) * LSD.NSD_TEMPLATE_LENGTH])
                    if det.fieldId == fieldId and slalib.sla_dsep(
                            pos.ra_deg * RAD_PER_DEG,
                            pos.dec_deg * RAD_PER_DEG,
                            det.ra * RAD_PER_DEG,
                            det.dec * RAD_PER_DEG
                        ) < thresh_rad and (not self.used_index.is_used(det)):
                        detections.append(det)
                    # <-- if
                # <-- for n
            # <-- for ranges

            # SYDs.
            for (filepos, filelen) in syd_ranges:
                # Ensure slice data has correct-looking size.
                if filelen % LSD.SYD_TEMPLATE_LENGTH != 0:
                    raise RuntimeError('Slice length is not a multiple of %s' % LSD.SYD_TEMPLATE_LENGTH)
                num_dets = int(filelen / LSD.SYD_TEMPLATE_LENGTH)

                # Load each range (slice) and scan for detections in the RA/DEC range.
                self.syd_file.seek(filepos)
                buf = self.syd_file.read(filelen)
                for n in range(0, num_dets):
                    det = LSD(field.obscode, syd_data=buf[n * LSD.SYD_TEMPLATE_LENGTH:(n + 1) * LSD.SYD_TEMPLATE_LENGTH])
                    if det.fieldId == fieldId and slalib.sla_dsep(
                            pos.ra_deg * RAD_PER_DEG,
                            pos.dec_deg * RAD_PER_DEG,
                            det.ra * RAD_PER_DEG,
                            det.dec * RAD_PER_DEG
                        ) < thresh_rad and not self.used_index.is_used(det):
                        detections.append(det)
                    # <-- if
                # <-- for n
            # <-- for ranges

        except IOError, e:
            sys.stderr.write('IO Error fetching detections: ' + str(e) + '\n')
            raise e
        except RuntimeError, e:
            sys.stderr.write('Runtime error: ' + str(e) + '\n')
            raise e
        # <-- try

        if len(detections) > 0:
            sys.stderr.write( 'Found %d detections in archive.\n' % len(detections))
        return detections


class det_mapping(object):
    def __init__(self, obscode, default_name):
        """
        Utility mapping object to convert det_id or det_num+field_id info to a single
        detection ID for FindTracklets.  We also conveniently package detections into
        tuples for FindTracklets, and create Tracklet objects out of FindTracklets
        result tuples.
        """
        self.obscode = obscode
        self.default_name = default_name
        self.detections = None
        self.mapping = {}

    def map_detections(self, detections):
        """
        Given a list of detections, create a mapping for them that converts
        an internal unique number back to each detection.
        """
        self.detections = detections
        for det in self.detections:
            self.mapping[id(det)] = det

    def dets2findtracklets(self):
        """
        Create a list of tuples from our detection set that will be used as input to FindTracklets.
        """
        ftdets = [(
            id(det),            # Python ID replaces DB det ID
            det.mjd,
            det.ra,
            det.dec,
            det.mag,
            self.obscode,       # not used by FindTracklets
            self.default_name,  # name
            0.0,        # length
            0.0         # angle
        ) for det in self.detections]
        return ftdets

    def ft2tracklet(self, ft_mapping):
        """
        Given a tuple that is a FindTracklets result, return a list of
        detections (eventually a real Tracklet object).
        """
        return [self.mapping[mapped_id] for mapped_id in ft_mapping]   # returns list of Detection objects


def all_hsd(trk, hsd_s2n_cutoff):
    """
    Return whether the tracklet's detections are all HSDs or not.  In general
    the LSD processing rejects all tracklets that are composed entirely of
    HSDs, since these should be processed and found by the normal PANDA code.
    """ 
    for det in trk.detections:
        if det.s2n < hsd_s2n_cutoff:
            return False
        # <- if
    # <- for

    # If we made it this far, they're all HSDs.
    return True


def FindTracklets(archive, fieldId2pos, tuples, unc_tester, hsd_s2n_cutoff, tti_size=None):
    """
    Given a directory to locate LSDs, a night number (nn), a collection of
    fields (tuples), and a lookup of positions (fieldId2pos), search the LSD
    archive for detections at the predicted positions in fieldId2pos and return
    tracklets.

    It is the responsibility of a wrapper program to handle the merge of
    database detections into the archive and untangle things afterwards.  The
    main issues are the

    * Upgrade of LSDs into database detections (DBDs)
    * Deletion of upgraded LSDs (into DBDs) before performing
      detection/tracklet search
    """

    all_tracklets = []          # tracklets from all tuples
    maxSearchRadius_arcsec = unc_tester.smaa_deg * 3600

    for field_list in tuples:
        if tti_size is not None and len(field_list) > tti_size:
            continue        # looks like deep stack, so skip this TTI tuple

        # Some setup.
        first_pos = fieldId2pos[field_list[0]._id]
        last_pos = fieldId2pos[field_list[-1]._id]
        delta_t = last_pos.epoch_mjd - first_pos.epoch_mjd
        if delta_t == 0:
            break           # shouldn't happen, but no-op if we encounter this

        # Estimate minimum and maximum tracklet velocities to search for based on
        # predicted positions and their errors.
        angular_distance_deg = slalib.sla_dsep(
            first_pos.ra_deg / DEG_PER_RAD,
            first_pos.dec_deg / DEG_PER_RAD,
            last_pos.ra_deg / DEG_PER_RAD,
            last_pos.dec_deg / DEG_PER_RAD
        ) * DEG_PER_RAD

        if first_pos.unc_smaa_arcsec < 0 or last_pos.unc_smaa_arcsec < 0:
            # The orbit is preliminary and does not have uncertainty info, so use our default maxSearchRadius.
            min_dist_deg = angular_distance_deg - 2 * maxSearchRadius_arcsec / ARCSECONDS_PER_DEG;
            if min_dist_deg < 0:
                min_dist_deg = 0
            max_dist_deg = angular_distance_deg + 2 * maxSearchRadius_arcsec / ARCSECONDS_PER_DEG;
        else:
            # Compute distance limits based on the 1-sigma ephemeris uncertainty.
            # 3 => 3-sigma error ellipse
            min_dist_deg = angular_distance_deg - 3 * first_pos.unc_smaa_arcsec / ARCSECONDS_PER_DEG - 3 * last_pos.unc_smaa_arcsec / ARCSECONDS_PER_DEG
            if min_dist_deg < 0:
                min_dist_deg = 0

            max_dist_deg = angular_distance_deg + 3 * first_pos.unc_smaa_arcsec / ARCSECONDS_PER_DEG + 3 * last_pos.unc_smaa_arcsec / ARCSECONDS_PER_DEG
            max_dist_deg = min(max_dist_deg, maxSearchRadius_arcsec / 3600)     # limit to configured max
        # <- if

        minv_degperday = min_dist_deg / delta_t
        maxv_degperday = max_dist_deg / delta_t

        # Accumulate detections from each field from LSD archive, then find tracklets
        # (id(det), epoch_mjd, ra_deg, dec_deg, mag, obscode[unused], name[unused], length[=0], angle[=0])
        detections = []
        for field in field_list:
            pos = fieldId2pos[field._id]
            detections += archive.FetchDetections(field, pos, maxSearchRadius_arcsec)
        # <-- for field


        # Create a temporary mapping of a det_id/det_num/field_id to a single identifier for FindTracklets.
        # The reason we need to do this is FindTracklets operates on single detection IDs, but in the LSD
        # regime we do not always have database detection IDs since most of the detections will be
        # originating from the LSD archive, where we have field_id+det_num identifiers.
        # FOO = dummy obscode, not used by findTracklets
        det_map = det_mapping(obscode='FOO', default_name='NS')
        det_map.map_detections(detections)
        ftdets = det_map.dets2findtracklets()

        if ftdets:
            mapping = auton.findtracklets(
                detections=ftdets,
                minv=minv_degperday,
                maxv=maxv_degperday,
                maxt=0.050,
                etime=30.0          # XXX need to handle this?
            )
            if mapping:
                for m in mapping:
                    trk = MOPS.Tracklet.Tracklet.new(trackletId=None, fieldId=field._id, detectionList=det_map.ft2tracklet(m))
                    if unc_tester.PointInEllipse(trk.extRa, trk.extDec, padding_deg=0):
                        all_tracklets.append(trk)
                # <-- for m
            # <- if mapping
#            mapping = None          # XXX not sure why this is necessary; seems there is a global re-use issue in auton module?
        # <-- if ftdets
    # <-- for field_list

    # Reject any tracklets that consist entirely of high-significance detections.  The returned
    # mapping contains detection IDs.
    all_tracklets = [trk for trk in all_tracklets if not all_hsd(trk, hsd_s2n_cutoff)]

    return all_tracklets


if __name__ == '__main__':
#    FindTracklets(*(sys.argv[1:])
    nn = int(sys.argv[1])
    lsd_dir = sys.argv[2]
    archive = Archive(lsd_dir, nn)

    ra_deg = 32.5
    print ra_deg, archive.LookupRA(ra_deg)

    ra_deg = 103.0
    print ra_deg, archive.LookupRA(ra_deg)

    ra_deg = 359.0
    print ra_deg, archive.LookupRA(ra_deg)

    ra_deg = 360.0
    print ra_deg, archive.LookupRA(ra_deg)
