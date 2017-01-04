"""
VOEvent

Module that provides support for the creation and dissemination of VOEvents.
"""
import os
import sys


from VOEvent import VOEvent
#from MOPS.Alerts.Publisher import publishAlerts


#__all__ = ['VOEvent', 'send']
__all__ = ['VOEvent']
'''
def send(instance, channel_events, bandwidth=None, verbose=None):
    """
    Send the VOEvent instance voevent to the server specified by the instance
    for further distribution. The channel_events list specifies the channel each
    MOPS.VOEvent instance should e published to.
    
    @param instance: MOPS.Instance instance
    @param channel_events: list of the form 
           [(channelName, MOPS.VOEvent instance), ]
    @param bandwidth: if not None or 0 specify max B/s bandwidth for alert
           submission.
    """
    # Send the event to the server.
    config = instance.config['alert']

    user = config['username']
    passwd = config['password']
    server = config['server']

    if verbose:
        if len(channel_events) == 1:
            channel_msg = " to channel %s" % channel_events[0][0]
        else:
            channel_msg = ''
        msg = "Sending message as %s@%s%s\n" % (user, server, channel_msg)
        sys.stderr.write(msg)
        
    return(publishAlerts(user, passwd, server, channel_events, bandwidth))
'''