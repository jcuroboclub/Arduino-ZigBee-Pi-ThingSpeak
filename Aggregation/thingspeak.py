# modified from https://github.com/bergey/thingspeak/blob/master/thingspeak.py
import http.client as httplib
import urllib.parse as urllib

__version__ = '0.1.1'

field_keys = ['field' + str(n) for n in range(1,9)]
headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}

def non_null_values(**kwargs):
    return [(k,v) for (k,v) in kwargs.items() if v != None]

def non_null_in_dict(d):
    return [(k,v) for (k,v) in d if v != None]

class TooManyFields(ValueError):
    pass

class channel(object):
    def __init__(self, write_key, cid):
        """write_key is the Write API Key.
        cid is the read_key"""
        self.write_key = write_key

    def update(self, field_vals, lat=None, long=None, elevation=None, status=None):
        if len(field_vals) > 8:
            raise TooManyFields('update can only handle 8 field values per channel')
        # this verbosity, rather than just using kwargs,
        # so that callers get a prompt error if they supply an arg `update` cannot handle
        named_args = non_null_values(lat=lat, long=long, elevation=elevation, status=status)
        params = urllib.urlencode(non_null_in_dict(zip(field_keys, field_vals)) + [('key', self.write_key)] + named_args)
        conn = httplib.HTTPConnection("api.thingspeak.com:80")
        conn.request("POST", "/update", params, headers)
        response = conn.getresponse()
        conn.close()
        return response

    def fetch(self, format):
        conn = httplib.HTTPConnection("anpi.thingspeak.com:80")
        path = "/channels/{0}/feed.{1}".format(self.cid, format)
        params = urllib.urlencode([('key',self.key)])
        conn.request("GET", path, params, headers)
        response = conn.getresponse()
        return response
