class Stamp:
    def __init__(self, detId, exposure, ra, dec, stageId = -1, trackletId = None):
        self.detId = int(detId)
        self.exposure = exposure
        self.ra = float(ra)
        self.dec = float(dec)
        self.stageId = stageId
        self.trackletId = trackletId

    @staticmethod
    def unjsonize(data, trackletId = None):
        subdata = data[1].split(":")
        if len(subdata) == 1:
            return Stamp(data[0], subdata[0], data[2], data[3], trackletId = trackletId)
        else:
            return Stamp(data[0], subdata[0], data[2], data[3], int(subdata[1]), trackletId = trackletId)

    def __str__(self):
        return "%s %s %g %g %s %s" % (self.detId, self.exposure, self.ra, self.dec, self.stageId, self.trackletId)
