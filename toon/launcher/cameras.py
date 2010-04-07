"""
Cameras listing.
"""
import glob
from twisted.internet import defer
def list_cameras():
    """
    Returns a Deferred with a list of camera names.
    """
    #FIXME: this is very bad
    return defer.succeed(glob.glob("/dev/video*"))
