"""
$Id$

Module for select of conflicting objects populated by detections or
tracklets.

Analyze tracklet or detection memberships in a list of derived objects to
identify conflicting identifications, and select "winners" in the case of
conflicts if possible.

"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


class Obj(object):
    # Stub class to contain entries managed by IDManager
    def __init__(self, id, members, fom):
        self.id = id
        self.members = members
        self.fom = fom
        self.rejected = None

    def __repr__(self):
        return self.id + ' ' + str(self.members) + ' ' + str(self.fom) + ' ' + str(self.rejected)


class Manager(object):
    def __init__(self):
        self._usage = {}
        self._objs = []
        self._objs_by_name = {}

    def add(self, obj):
        self._objs.append(obj)
        self._objs_by_name[obj.id] = obj

    def analyze(self):
        # Sort our list of added objects.
        self._objs.sort(key=lambda x: x.fom) # lowest to highest FOM
        self._objs.reverse()                 # highest to lowest FOM

        # Build det/trk ID tables.
        for obj in self._objs:
            for member_id in obj.members:
                self._usage.setdefault(member_id, []).append(obj.id)
            # <-- for member_id
        # <-- for obj

        # Debugginsh.  Remove det/trk IDs with only one user-object, since we don't care
        # about them.
        delete_objs = []
        for id in self._usage.keys():
            if len(self._usage[id]) < 2:
                delete_objs.append(id)
            # <-- if
        # <-- for id

        for id in delete_objs:
            del(self._usage[id])
        # <-- for id

        # Interesting stuff here.
        for obj in self._objs:
            if obj.rejected:
                continue

            for member_id in obj.members:
                self.arbitrate(obj, member_id)
                if obj.rejected:
                    break
                # <-- if
            # <-- for member_id
        # <-- for obj

    def query_rejected(self, id):
        # Return the analysis result of an owner object.
        if id not in self._objs_by_name:
            raise RuntimeError("unfound ID " + id)
        # <-- if
        return self._objs_by_name[id].rejected

    def users(self, member_id):
        # Return a list of unrejected objects using the specified det/trk ID.
        return filter(lambda x: not self._objs_by_name[x].rejected, self._usage.get(member_id, []))

    def arbitrate(self, ref_obj, member_id):
        user_objs = [self._objs_by_name[x] for x in self.users(member_id)]
        num_objects = len(user_objs)
        if num_objects <= 1:
            return
        # <-- if num_objects

        if user_objs[0].fom > 1.5 * user_objs[1].fom:
            # Sanity check.
            if user_objs[0].id != ref_obj.id:
                raise RuntimeError("Assertion failed: best user object is not zeroth: " + user_objs[0].id)
            # <-- if user_objs

            # Reject other objects.
            for obj in user_objs[1:]:
                obj.rejected = ref_obj.id       # set to something, indicating rejected
            # <-- for obj
            return
        else:
            for obj in user_objs:
                obj.rejected = ref_obj.id       # set to something, indicating rejected
            # <-- for obj
            return
        # <-- if user_objs


if __name__ == "__main__":
    data = [
        'A=B=C=D 0.4',
        'E=F=G 0.1',
        'A=I=J 0.5',
        'K=I=L 0.2',
        'M=N=O 0.6',
        'C=F=O 0.3',
    ]

    mgr = Manager()
    for line in data:
        id, resid = line.split()
        fom = 1 / float(resid)
        members = id.split('=')
        obj = Obj(id, members, fom)
        mgr.add(obj)

    mgr.analyze()
    for obj in mgr._objs:
        print obj.id, obj.rejected

