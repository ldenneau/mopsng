"""
MOPS.VOEvent.ParseUtils

VOEvent parsing utilities. Internal use.
"""
from MOPS.Detection import Detection
from MOPS.Orbit import Orbit



def parseObsData(xml):
    """
    xml is the WhereWhen portion of the XML packet.
    """
    detections = []
    orbit = None
    ephems = []
    
    for child in xml:
        tag = child.tag.split('}', 1)[-1]
        if(tag == 'ObsDataLocation'):
            for subChild in child:
                tag = subChild.tag.split('}', 1)[-1]
                if(tag == 'ObservationLocation'):
                    # Found it!
                    astroCoords = subChild[0]
                    typ = astroCoords.get('coord_system_id')
                    if(typ == 'UTC-ICRS-TOPO-VMAG'):
                        detections.append(parseDetBlock(astroCoords))
                    elif(typ == 'TDB-ECLIPTIC-MOPS'):
                        orbit = parseOrbitBlock(astroCoords)
                    elif(typ == 'UTC-ICRS-BARY-VMAG'):
                        ephems.append(parseEphemBlock(astroCoords))
                    # <-- end if
                # <-- end if
                continue
            # <-- end for
        # <-- end if
        continue
    # <-- end for
    return((detections, orbit, ephems))


def parseDetBlock(xml):
    """
    xml is an AstroCoords block.
    """
    detEl = [None, ] * 16
    
    for child in xml:
        tag = child.tag.split('}', 1)[-1]
        if(tag == 'ScalarCoordinate'):
            # Mag
            try:
                detEl[4] = float(child[0].text)
            except:
                pass
        elif(tag == 'Time'):
            # MJD
            try:
                detEl[3] = float(child[0][0].text)
            except:
                pass
        elif(tag == 'Position2D'):
            # RA/Dec
            try:
                radec = child[0]
                detEl[1] = float(radec[0].text)
                detEl[2] = float(radec[1].text)
            except:
                pass
    # <-- end for
    return(Detection(*detEl))


def parseOrbitBlock(xml):
    """
    xml is an AstroCoords block.
    """
    orbitEl = [None, ] * 15
    for child in xml:
        tag = child.tag.split('}', 1)[-1]
        if(tag == 'Time'):
            # epoch
            try:
                orbitEl[8] = float(child[0][0].text)
            except:
                pass
        elif(tag == 'Orbit'):
            # Orbit
            for orbEl in child:
                subTag = orbEl.tag.split('}', 1)[-1]
                if(subTag == 'q'):
                    try:
                        orbitEl[1] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'e'):
                    try:
                        orbitEl[2] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'i'):
                    try:
                        orbitEl[3] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'Node'):
                    try:
                        orbitEl[4] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'Aop'):
                    try:
                        orbitEl[5] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'T'):
                    try:
                        orbitEl[6] = float(orbEl[0].text)
                    except:
                        pass
                elif(subTag == 'SRC'):
                    try:
                        if(orbEl.text != 'None'):
                            orbitEl[9] = parseSRCBlock(orbEl)
                        # <-- end if
                    except:
                        pass
                elif(subTag == 'h_v'):
                    try:
                        orbitEl[7] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'residuals'):
                    try:
                        orbitEl[10] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'chiSq'):
                    try:
                        orbitEl[11] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'convCode'):
                    try:
                        orbitEl[13] = orbEl.text
                    except:
                        pass
                elif(subTag == 'moid1'):
                    try:
                        orbitEl[12] = float(orbEl.text)
                    except:
                        pass
                elif(subTag == 'moid2'):
                    try:
                        orbitEl[14] = float(orbEl.text)
                    except:
                        pass
    # <-- end for
    return(Orbit(*orbitEl))


def parseSRCBlock(xml):
    """
    xml is a SRC block.
    """
    src = []
    arrayXML = xml[0]
    for el in arrayXML:
        src.append(float(el.text))
    # <-- end for
    return(src)


def parseEphemBlock(xml):
    """
    xml is an AstroCoords block.
    """
    detEl = [None, ] * 16
    
    for child in xml:
        tag = child.tag.split('}', 1)[-1]
        if(tag == 'ScalarCoordinate'):
            # Mag
            detEl[4] = float(child[0].text)
        elif(tag == 'Time'):
            # MJD
            detEl[3] = float(child[0][0].text)
        elif(tag == 'Position2D'):
            # RA/Dec and their errors.
            for vecor in child:
                vectorTag = vecor.tag.split('}', 1)[-1]
                if(vectorTag == 'Value2'):
                    detEl[1] = float(vecor[0].text)
                    detEl[2] = float(vecor[1].text)
                elif(vectorTag == 'Error2'):
                    if(vecor[0].text == 'None'):
                        detEl[7] = vecor[0].text
                    else:
                        detEl[7] = float(vecor[0].text)
                    if(vecor[1].text == 'None'):
                        detEl[8] = vecor[1].text
                    else:
                        detEl[8] = float(vecor[1].text)
                # <-- end fi
            # <-- end for
    # <-- end for
    return(Detection(*detEl))
