#! /usr/bin/python

import json
import os
import tempfile
import threading
import time

import web

# JIVE Python modules.
from vex_parser import Vex
import vex_update
from evlbi import DataFlow

from run_evlbi_jobs import run, wait

# Proper time.
os.environ['TZ'] = "UTC"

access_list = [ "10.87.6.229", "129.16.208.174" ]
pre_start = 10 * 60 # 10 minutes

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

def get_mode(vex):
    sched = vex['SCHED']
    for scan in sched:
        break
    return sched[scan]['mode']

def get_scan_stop(scan):
    time = vex2time(scan['start'])
    time += max(float(station[2].split()[0]) \
                    for station in scan.getall('station'))
    return time2vex(time)

def get_start(vex):
    sched = vex['SCHED']
    for scan in sched:
        return sched[scan]['start']
    return ""

def get_stop(vex):
    sched = vex['SCHED']
    for scan in sched:
        continue
    return get_scan_stop(sched[scan])

urls = (
    '/experiment.new', 'new_experiment'
)

app = web.application(urls, globals())

def error_response(msg):
    web.header('Content-Type', 'application/json')
    response = {}
    response['status'] = "error"
    response['error'] = msg
    return json.dumps(response) + "\n"

def create_job(vex, ctrl_file):
    # Find first scan.
    for scan in vex['SCHED']:
        break

    # Figure out the subbands to correlate based on the mode.
    mode = vex['SCHED'][scan]['mode']
    freq = vex['MODE'][mode]['FREQ'][0]
    channels = []
    for channel in vex['FREQ'][freq].getall('chan_def'):
        channels.append(channel[4])
        continue

    # Create a list of stations participating in this scan.
    stations = []
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        stations.append(station)
        continue
    stations.sort()

    # Loop over all stations, creating a list of input data files.
    data_sources = {}
    for station in stations:
        data_sources[station] = ["mk5://" + station + "-eVLBI:0"]
        continue

    start = time.time()
    stop = vex2time(get_stop(vex))

    # Clean the slate again.
    json_output = {}
    json_output["data_sources"] = data_sources
    json_output["start"] = time2vex(start)
    json_output["stop"] = time2vex(stop)
    json_output["stations"] = stations
    json_output["channels"] = channels

    # Semi-boring stuff.
    json_output["number_channels"] = 1024
    json_output["integr_time"] = 2
    json_output["exper_name"] = vex['GLOBAL']['EXPER']
    output_file = vex['GLOBAL']['EXPER'] + ".cor"
    output_uri = "file://" + os.getcwd() + "/" + output_file
    json_output["output_file"] =  output_uri
    delay_uri = "file://" + os.getcwd()
    json_output["delay_directory"] = delay_uri

    # Boring stuff.
    json_output["cross_polarize"] = True
    json_output["message_level"] = 1

    # Write out the correlator control file.
    fp = open(ctrl_file, 'w')
    json.dump(json_output, fp, indent=4)
    fp.close()

    return

running_job = None

def start_job(src, vex):
    global running_job
    exper = vex['GLOBAL']['EXPER']
    vex_file = "%s.vix" % exper
    ctrl_file = "%s.ctrl" % exper
    src.seek(0)
    dest = open(vex_file, "w")
    vex_update.update(src, dest)
    print "Running %s with %s" % (exper, vex_file)
    dest.close()

    if running_job:
        print "Killing job"
        try:
            running_job.terminate()
            while running_job.returncode == None:
                time.sleep(1)
                continue
        except:
            pass
        time.sleep(60)
        pass

    create_job(vex, ctrl_file)
    flow = DataFlow(vex)
    (running_job, stations) = run(vex_file, ctrl_file, flow)
    wait(running_job, stations, flow)
    return

class new_experiment:
    def POST(self):
        if not web.ctx['ip'] in access_list:
            return error_response("Access denied")
        try:
            fp = tempfile.NamedTemporaryFile()
            fp.write(web.data())
            fp.flush()
            vex = Vex(fp.name)
        except:
            return error_response("Can't parse VEX")

        start = get_start(vex)
        stop = get_stop(vex)
        start = vex2time(start)
        stop = vex2time(stop)
        if stop < time.time():
            return error_response("Experiment is in the past")

        # Schedule the correlation job for this experiment.  If the
        # experiment already started, the job will be started
        # immediately.
        interval = max(0, start - pre_start - time.time())
        t = threading.Timer(interval, start_job, args=[fp, vex])
        t.start()

        web.header('Content-Type', 'application/json')
        response = {}
        response['status'] = "scheduled"
        return json.dumps(response) + "\n"

    pass

if __name__ == "__main__":
    app.run()
