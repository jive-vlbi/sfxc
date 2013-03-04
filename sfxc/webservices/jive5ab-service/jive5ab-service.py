#! /usr/bin/python

import json
import os
import re
import sys
import tempfile
import time
import socket
import subprocess

import web
import vex

import config

os.environ['TZ'] = 'UTC'
time.tzset()

urls = (
    '/configure', 'configure',
    '/start', 'start',
    '/status', 'status',
    '/stop', 'stop'
)

app = web.application(urls, globals())

def send_command(command):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        s.connect(("localhost", config.port))
        if config.debug:
            print command
            pass
        s.send(command)
        data = s.recv(1024)
        if config.debug:
            print repr(data)
            pass
        s.close()
        return data
    except:
        pass
    return

def check_reply(reply, expect):
    end_index = reply.find(';')
    if end_index != -1:
        reply = reply[:end_index]
        pass
    seperator_index = reply.find('=')
    if seperator_index == -1:
        seperator_index = reply.find('?')
        pass
    if seperator_index != -1:
        reply = reply[seperator_index + 1:]
        pass
    return reply.split(':')[0].strip() == str(expect)

def find_mode(vex, station):
    for scan in vex['SCHED']:
        return vex['SCHED'][scan]['mode']
    return

def find_tracks(vex, station):
    mode = find_mode(vex, station)
    for tracks in vex['MODE'][mode].getall('TRACKS'):
        if station in tracks[1:]:
            return tracks[0]
        continue
    return

def find_bitstreams(vex, station):
    mode = find_mode(vex, station)
    for bitstreams in vex['MODE'][mode].getall('BITSTREAMS'):
        if station in bitstreams[1:]:
            return bitstreams[0]
        continue
    return

def find_freq(vex, station):
    mode = find_mode(vex, station)
    for freq in vex['MODE'][mode].getall('FREQ'):
        if station in freq[1:]:
            return freq[0]
        continue
    return

def find_das(vex, station):
    return vex['STATION'][station]['DAS']

def get_sample_rate(vex, station):
    freq = find_freq(vex, station)
    sample_rate = vex['FREQ'][freq]['sample_rate']
    unit = sample_rate.split()[1]
    if unit == 'Ms/sec':
        unit = 1e6
    else:
        return
    return float(sample_rate.split()[0]) * unit

def mark5_mode(vex, station):
    sample_rate = get_sample_rate(vex, station)
    decimation = int(32e6 / sample_rate)
    das = find_das(vex, station)
    record_transport_type = vex['DAS'][das]['record_transport_type']
    if record_transport_type == 'Mark5B':
        bitstreams = find_bitstreams(vex, station)
        if bitstreams:
            stream_list = vex['BITSTREAMS'][bitstreams].getall('stream_def')
            num_streams = len(stream_list)
            return "mode=ext:0x%x:%d;" % (((1 << num_streams) - 1), decimation)
        pass
    tracks = find_tracks(vex, station)
    if tracks:
        track_frame_format = vex['TRACKS'][tracks]['track_frame_format']
        fanout_list = vex['TRACKS'][tracks].getall('fanout_def')
        num_tracks = 0
        for fanout_def in fanout_list:
            num_tracks += len(fanout_def) - 4
            continue
        if track_frame_format == "Mark4":
            return "mode=mark4:%d;" % num_tracks
        if track_frame_format == "Mark5B":
            return "mode=ext:0x%x:%d;" % (((1 << num_tracks) - 1), decimation)
        pass
    return

def error_response(command, resp):
    result = {}
    result['error'] = "unexpected response to %s command: %s" % (command, resp)
    return json.dumps(result)

def mark5_play_rate(vex, station):
    sample_rate = get_sample_rate(vex, station)
    das = find_das(vex, station)
    record_transport_type = vex['DAS'][das]['record_transport_type']
    if record_transport_type == 'Mark5B':
        return "play_rate=data:%f;" % (sample_rate * 1e-6)
    tracks = find_tracks(vex, station)
    if tracks:
        track_frame_format = vex['TRACKS'][tracks]['track_frame_format']
        fanout_list = vex['TRACKS'][tracks].getall('fanout_def')
        fanout = 0
        for fanout_def in fanout_list:
            fanout = len(fanout_def) - 4
            break
        return "play_rate=data:%f;" % ((sample_rate * 1e-6) / fanout)
    return

class configure:
    def POST(self):
        web.header('Content-Type', 'application/json')
        input = json.loads(web.data())
        try:
            v = vex.parse(input['vex'])
        except:
            result = {}
            result['error'] = "couldn't parse VEX"
            return json.dumps(result)

        if not 'destination' in input:
            result = {}
            result['error'] = "missing destination"
            return json.dumps(result)

        try:
            hostname = input['destination'].split(':')[0]
            port = int(input['destination'].split(':')[1])
        except:
            result = {}
            result['error'] = "malformed destination: %s" % input['destination']
            return json.dumps(result)

        mtu = 1500
        if 'mtu' in input:
            try:
                mtu = int(input['mtu'])
            except:
                result = {}
                result['error'] = "malformed mtu: %s" % input['mtu']
                return json.dumps(result)
            pass
            
        resp = send_command("version?;")
        if not check_reply(resp, 0):
            return error_reponse("version?", resp)

        what = "in2net"
        if config.simulate:
            what = "fill2net"
            pass

        command = "%s=disconnect" % what
        send_command(command)

        try:
            command = mark5_play_rate(v, config.station)
        except:
            result = {}
            result['error'] = "can't find play rate for station %s" % config.station
            return json.dumps(result)
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)
            result = {}
            result['error'] = "unexpected response to %s command: %s" % (command, resp)
            return json.dumps(result)

        try:
            command = mark5_mode(v, config.station)
        except:
            result = {}
            result['error'] = "can't find mode for station %s" % config.station
            return json.dumps(result)
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        command = "mtu=%d;" % mtu
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        command = "net_protocol=udp;"
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        command = "net_port=%d;" % port
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        command = "%s=connect:%s:0;" % (what, hostname)
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        result = {'station': config.station,
                  'experiment': v['GLOBAL']['EXPER']}
        return json.dumps(result)

    pass

class start:
    def POST(self):
        web.header('Content-Type', 'application/json')

        what = "in2net"
        if config.simulate:
            what = "fill2net"
            pass

        command = "%s=on" % what
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        result = {}
        return json.dumps(result)

    pass

class stop:
    def POST(self):
        web.header('Content-Type', 'application/json')

        what = "in2net"
        if config.simulate:
            what = "fill2net"
            pass

        if what != "fill2net":
            command = "%s=off" % what
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)
            pass

        command = "%s=disconnect" % what
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        result = {}
        return json.dumps(result)

    pass

class status:
    def POST(self):
        web.header('Content-Type', 'application/json')

        resp = send_command("tstat?;")
        if check_reply(resp, 1):
            time.sleep(1)
            resp = send_command("tstat?;")
        if not check_reply(resp, 0):
            return error_response("tstat?;", resp)        

        result = {}
        r = re.compile(r'(\d*)bps')
        try:
            m = r.search(resp.split(':')[4])
            result['datarate'] = int(m.group(1))
        except:
            result['error'] = 'not started'
            pass
        return json.dumps(result)

    pass

if __name__ == "__main__":
    config.station = sys.argv[1]
    config.port = int(sys.argv[2])
    del sys.argv[1]
    del sys.argv[1]

    args = ["jive5ab", "-m3", "-p", str(config.port)]
    log = open("jive5ab-" + config.station + ".log", "w")
    proc = subprocess.Popen(args, stdout=log, stderr=log)
    try:
        app.run()
    finally:
        proc.terminate()
        proc.wait()
        pass

    pass
