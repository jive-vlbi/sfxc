#! /usr/bin/python

import json
import optparse
import os
import re
import socket
import subprocess
import sys
import tempfile
import time
import urlparse

import web
import vex

import config
import daemon

urlparse.uses_relative.append('mk5')
urlparse.uses_netloc.append('mk5')
urlparse.uses_params.append('mk5')
urlparse.clear_cache()

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
        if track_frame_format == "Mark5B" or track_frame_format == "MARK5B":
            return "mode=ext:0x%x:%d;" % (((1 << num_tracks) - 1), decimation)
        pass
    raise AssertionError, "unsupported Mark5 mode"

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

def split_mode(vex, station):
    das = find_das(vex, station)
    record_transport_type = vex['DAS'][das]['record_transport_type']
    if record_transport_type == 'Mark5B':
        bitstreams = find_bitstreams(vex, station)
        if bitstreams:
            stream_list = vex['BITSTREAMS'][bitstreams].getall('stream_def')
            num_streams = len(stream_list)
            if num_streams == 32:
                split = "16bitx2+8Ch2bit_hv"
            elif num_streams == 16:
                split = "8Ch2bit_hv"
            else:
                raise AssertionError, "unsupported number of streams"

            split += "+swap_sign_mag"
            print split
            
        pass

class configure:
    def source(self, input, vex, mtu):
        if not 'destination' in input:
            result = {}
            result['error'] = "missing destination"
            return json.dumps(result)

        try:
            map = {}
            for key in input['destination']:
                destination = input['destination'][key]
                if not destination in map:
                    map[destination] = []
                    pass
                map[destination].append(key)
                continue

            if len(map) > 1:
                split_mode(vex, config.station)
                result = {}
                result['error'] = "splitting not implemented yet"
                return json.dumps(result)

            destination = map.keys()[0]
            hostname = destination.split(':')[0]
            port = int(destination.split(':')[1])
        except:
            result = {}
            result['error'] = "malformed destination: %s" % input['destination']
            return json.dumps(result)

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
            command = mark5_play_rate(vex, config.station)
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
            command = mark5_mode(vex, config.station)
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

        config.experiment = vex['GLOBAL']['EXPER']
        result = {'station': config.station,
                  'experiment': config.experiment}
        return json.dumps(result)

    def sink(self, input, vex, mtu):
        if not 'port' in input:
            result = {}
            result['error'] = "missing port"
            return json.dumps(result)

        try:
            port = int(input['port'])
        except:
            result = {}
            result['error'] = "malformed port: %s" % input['port']
            return json.dumps(result)

        if not 'destination' in input:
            result = {}
            result['error'] = "missing destination"
            return json.dumps(result)
        try:
            url = urlparse.urlparse(input['destination'])
            destination = url.path
        except:
            result = {}
            result['error'] = "malformed destination: %s" % input['destination']
            return json.dumps(result)

        resp = send_command("version?;")
        if not check_reply(resp, 0):
            return error_reponse("version?", resp)

        try:
            command = mark5_play_rate(vex, config.station)
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
            command = mark5_mode(vex, config.station)
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

        command = "net2sfxc=open:%s;" % destination
        resp = send_command(command)
        if not check_reply(resp, 0):
            return error_response(command, resp)

        config.experiment = vex['GLOBAL']['EXPER']
        result = {'station': config.station,
                  'experiment': config.experiment}
        return json.dumps(result)

    def POST(self):
        web.header('Content-Type', 'application/json')
        input = json.loads(web.data())
        try:
            v = vex.parse(input['vex'])
        except:
            result = {}
            result['error'] = "couldn't parse VEX"
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

        if config.sfxc:
            return self.sink(input, v, mtu)
        else:
            return self.source(input, v, mtu)

    pass

class start:
    def POST(self):
        web.header('Content-Type', 'application/json')

        if config.sfxc:
            result = {}
            return json.dumps(result)

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

        config.experiment = ""

        if config.sfxc:
            command = "net2sfxc=close;"
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)

            result = {}
            return json.dumps(result)

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
            result['station'] = config.station
            result['experiment'] = config.experiment
        except:
            result['error'] = 'not started'
            pass
        return json.dumps(result)

    pass

if __name__ == "__main__":
    usage = "usage: %prog [options] station [port]"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-p", "--port", dest="port",
                      type="int", default="2620",
                      help="jive5ab control port",
                      metavar="PORT")
    parser.add_option("-d", "--debug", dest="debug",
                      action="store_true", default=False,
                      help="turn on debug information")
    parser.add_option("-s", "--sfxc", dest="sfxc",
                      action="store_true", default=False,
                      help="receive and feed data to SFXC")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.error("incorrect number of arguments")
        pass

    config.port = options.port
    config.debug = options.debug
    config.sfxc = options.sfxc

    config.station = args[0]
    sys.argv = args

    args = ["jive5ab", "-m3", "-p", str(config.port)]
    log = open("jive5ab-" + config.station + ".log", "w")

    if not config.debug:
        daemon.daemonize()
        pass

    proc = subprocess.Popen(args, stdout=log, stderr=log)
    try:
        app.run()
    finally:
        proc.terminate()
        proc.wait()
        pass

    pass
