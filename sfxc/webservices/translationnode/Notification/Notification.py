import sys
import httplib
from NotificationZSI.Notification_services import *
from wsutils import camelCase

def makeNotification(d):
    """makeTranslationNotification(d) => a ZSI "request", filled in with fields from dictionary d"""
    print >>sys.stderr, d
    req = updateRequest()
    n = req.new_notification()
    req.Notification = n
    if d.has_key("chunkInfo"):
        n.ChunkInfo = n.new_chunkInfo()  
        for k, v in d["chunkInfo"].items():
            setattr(n.ChunkInfo, camelCase(k), v)
    if d.has_key("state"):
        n.State = n.new_state() 
        for k, v in d["state"].items():
            setattr(n.State, camelCase(k), v)
    if d.has_key("message"):
        n.Message = d["message"]
    n.Header = n.new_header()        
    for k, v in d["header"].items():
        setattr(n.Header, camelCase(k), v)
    print >>sys.stderr, str(req)
    return req

