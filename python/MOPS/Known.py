"""
MOPS Python object-relational mapper: Known class.
"""

class Known:
    def __init__(self, known_id, known_name, q, e, i, node, arg_peri, time_peri, 
                 epoch, h_v):
        self._known_id = known_id
        self._known_name = known_name
        self._q = q
        self._e = e
        self._i = i
        self._node = node
        self._arg_peri = arg_peri
        self._time_peri = time_peri
        self._epoch = epoch
        self._h_v = h_v
        return
    # <--end def
    
    #--------------------------------------------------------------------------   
    # Getter methods.
    #--------------------------------------------------------------------------   

    @property
    def known_id(self):
        """Unique identifier"""
        return long(self._known_id)
    # <-- end def 

    @property
    def known_name(self):
        """Name object is known as"""
        return self._known_name
    # <-- end def
    
    @property
    def q(self):
        """Perihelion distance of object from sun in AU"""
        return float(self._q)
    # <-- end def 

    @property
    def e(self):
        """Eccentricity of object"""
        return float(self._e)
    # <-- end def

    @property
    def i(self):
        """
        Orbital inclination/vertical tilt of the ellipse with respect to the 
        reference plane, measured at the ascending node (where the orbit passes 
        upward through the reference plane)
        """
        return float(self._i)
    # <-- end def 

    @property
    def node(self):
        """
        Right ascension of ascending node horizontally orients the ascending 
        node of the ellipse (where the orbit passes upward through the reference 
        plane) with respect to the reference frame's vernal point
        """
        return float(self._node)
    # <-- end def

    @property
    def arg_peri(self):
        """
        Argument of periapsis defines the orientation of the ellipse (in which 
        direction it is flattened compared to a circle) in the orbital plane, 
        as an angle measured from the ascending node to the semimajor axis
        """
        return float(self._arg_peri)
    # <-- end def 

    @property
    def time_peri(self):
        """Time of perihelion"""
        return float(self._time_peri)
    # <-- end def

    @property
    def epoch(self):
        """Time at which the orbital elements were obtained."""
        return float(self._epoch)
    # <-- end def 

    @property
    def h_v(self):
        """Absolute magnitude"""
        return float(self._h_v)
    # <-- end def
# <-- end class