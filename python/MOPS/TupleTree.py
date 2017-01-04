"""
Implementation of the TupleTree class, a data structure that manages fields
acquired over a single night, arranging the fields into "stacks", or tuples.
The final field of each tuple is called the "parent field".
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


class TupleTree(object):
    def __init__(self, fieldList):
        """
        Given a list of Field objects, return a dict whose keys are parent
        Fields IDs (parentId == None), and whose values are lists of Field
        objects belonging to the parent, including the parent Field.  The
        parent field will always be the final element of each tuple list.

        Parent fields are the "fields of record" for a tuple; in other
        words, the final field in a tuple.  So we make two passes
        through the table: one to identify parent fields, and a
        second to identify children, which occur earlier in time.
        """
        self.tree = {}

        # First pass: find parent fields.
        for f in fieldList:
            if f.parentId is None:          # parent
                # Got a parent field, so setdefault() a key/val pair.
                self.tree[f._id] = [f, ]

        for f in fieldList:
            if f.parentId is not None:      # child (of somebody)
                # Got a parent field, so setdefault() a key/val pair.
                if f.parentId not in self.tree:
                    raise RuntimeError("Field ID " + str(f._id) + " is an orphan field")
                self.tree[f.parentId].append(f)

        # Sort each of the tuples by time.
        for tuple in self.tree.values():
            tuple.sort(cmp=lambda x, y: cmp(x.mjd, y.mjd))      # sort in time

        # Create a utility table of parent IDs.
        self.parent_tree = dict([(k, v[-1]) for (k, v) in self.tree.items()])


    def GetNumTuples(self):
        """
        Return the number of tuples in our tree.
        """
        return len(self.tree)


    def GetTuple(self, tuple_id):
        """
        Return the list of fields constituting the tuple.
        """
        return self.tree[tuple_id]


    def GetTupleParent(self, tuple_id):
        """
        Given the ID of a tuple (its parent's ID), return the parent field.
        """
        return self.tree[tuple_id][-1]

    def GetAllTuples(self):
        """
        Return an unsorted list of all tuples.
        """
        return self.tree.values()



    def FindRelevantTuples(self, fieldList):
        """
        Return a list of tuples for which the fields in fieldList are parents.
        """
        relevant_tuples = []
        for field in fieldList:
            if field._id in self.parent_tree:                       # field is a parent field in tuple tree
                relevant_tuples.append(self.tree[field._id])        # add entire tuple list of relevant fields
            # <-- if field._id
        # <-- for field
        return relevant_tuples
