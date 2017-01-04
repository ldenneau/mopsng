
class Tracklet:
    def __init__(self, name):
        self.name = name
        self.stamps = [ ]

    def addStamp(self, stamp):
        self.stamps.append(stamp)

    @staticmethod
    def unjsonize(data):
        tracklet = Tracklet(data[0])
        for stamp in data[1]:
            tracklet.stamps.append(Stamp.unjsonize(stamp, trackletId = tracklet.name))
        return tracklet
