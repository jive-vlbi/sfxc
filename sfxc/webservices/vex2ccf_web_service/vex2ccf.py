#!/usr/bin/python

# 2007, Developed by Nico Kruithof <Kruithof@jive.nl>

import sys, re
from vex_parser import Vex
import simplejson
import pprint

def add_time(time, delta):
  start_date = re.match("(\d*)y(\d*)d(\d*)h(\d*)m(\d*)s",time).groups()
  second = int(start_date[4]) + int(delta)
  minute = int(start_date[3]) + (second / 60)
  hour   = int(start_date[2]) + (minute / 60)
  day    = int(start_date[1]) + (hour   / 24)
  year   = int(start_date[0])
  days_per_year = 365
  if (year % 4 == 0) and ((year % 100 != 0) or (year % 400 == 0)):
    days_per_year = 366
  year = year + (day / days_per_year)
  return "%04dy%03dd%02dh%02dm%02ds" % \
         (year, (day%days_per_year), (hour%24), (minute%60), (second%60))

def get_start(vex):
  start = ""
  sched = vex["SCHED"]
  for scan in sched:
    if start == "":
      start = sched[scan]["start"]
    else:
      if start > sched[scan]["start"]:
        start = sched[scan]["start"]
  return start

def get_stop(vex):
  stop = ""
  sched = vex["SCHED"]
  for scan in sched:
    if stop == "":
      stop = add_time(sched[scan]["start"],
                      max([i[2].split()[0] \
                           for i in sched[scan].getall("station")]))
    else:
      new_stop = \
        add_time(sched[scan]["start"],
                 max([i[2].split()[0] \
                           for i in sched[scan].getall("station")]))
      if stop < new_stop:
        stop = new_stop
  return stop

def get_frequency(vex, IF, BBC, channel):
  if_def = ""
  for bbc in vex["BBC"][BBC].getall("BBC_assign"):
    if bbc[0] == channel[5]:
      if_def = bbc[2]
  
  result = dict()
  result["frequency"]    = float(channel[1].split()[0])*1000000
  result["bandwidth"]    = float(channel[3].split()[0])*1000000
  result["sideband"]     = channel[2]+"SB"
  for if_def_it in vex["IF"][IF].getall("if_def"):
    if if_def_it[0] == if_def:
      result["polarisation"] = if_def_it[2]
  return result

  
def get_frequencies(vex):
  # take the first frequency, even if there are multiple frequencies
  for i in vex["MODE"]: mode = i
  IF = vex["MODE"][mode]["IF"][0]
  ref_station = vex["MODE"][mode]["IF"][1]
  BBC = ""
  # Find the corresponding BBC:
  for bbc in vex["MODE"][mode].getall("BBC"):
    for station in bbc[1:]:
      if station == ref_station:
        BBC = bbc[0]

  result = []
  for freq in vex["FREQ"]:
    for channel in vex["FREQ"][freq].getall("chan_def"):
      result.append(get_frequency(vex, IF, BBC, channel))
    break
  return result


def vex2ccf(vex_content):
  vex = Vex(vex_content)

  json_output = dict()
  json_output["exper_name"] = vex["GLOBAL"]["EXPER"]
  json_output["start"] = get_start(vex)
  json_output["stop"] = get_stop(vex)
  json_output["stations"] = []
  for station in vex["STATION"]:
    json_output["stations"].append(station)
  json_output["reference_station"] = ""
  json_output["cross_polarize"]    = False
  json_output["number_channels"]   = 1024
  json_output["integr_time"]       = 1.
  json_output["message_level"]     = 0
  json_output["delay_directory"]   = ""
  json_output["output_file"]       = ""
  json_output["data_sources"]      = dict()
  for station in json_output["stations"]:
    json_output["data_sources"][station] = []
  json_output["subbands"] = get_frequencies(vex)
  json_output["site_position"] = dict()
  for station in vex["STATION"]:
    json_output["site_position"][station] = \
      [float(pos.split()[0])
       for pos in vex["SITE"][vex["STATION"][station]["SITE"]]["site_position"] ]

  return simplejson.dumps(json_output);

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print "%s takes a vex-file as argument" % sys.argv[0]
    sys.exit(1)

  print vex2ccf(sys.argv[1])
