#!/usr/bin/env python
# -*- coding: utf-8 -*-

try:
  # For Python 3.0 and later
  from urllib.request import urlopen, Request
except ImportError:
  # Fall back to Python 2's urllib2
  from urllib2 import urlopen, Request

import json
from sys import argv, exit

if len(argv) != 4:
  print("Usage: " + argv[0] + " <tag name> <CXX> <GITHUB_TOKEN>")
  exit(1)


tag_name   = argv[1]

cxx = argv[2]
plus_idx = cxx.find('+')
if plus_idx >= 0:
    cxx = cxx[0:plus_idx]
deb_suffix = '-' + cxx + '.deb'

tok        = argv[3]

r = Request("https://api.github.com/repos/pfpsim/PFPSim/releases/tags/" + tag_name,
            None, # No data
            {"Authorization":"token " + tok,
             "Accept":"application/vnd.github.v3.raw"})

resp = urlopen(r).read()
try:
    resp = json.loads(resp)
except:
    resp = json.loads(resp.decode("utf-8"))

print("Got list of assets")

for asset in resp['assets']:
  if asset['name'].endswith(deb_suffix):
    print("found " + asset['name'])
    r = Request(asset['url'] + "?access_token=" + tok,
            None, # No data
            {"Accept":"application/octet-stream"})
    deb_resp = urlopen(r).read()
    print("Successfully downloaded")
    with open(asset['name'], 'wb') as debfile:
      debfile.write(deb_resp)
