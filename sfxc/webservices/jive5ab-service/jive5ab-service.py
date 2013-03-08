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

def mark5_mask(vex, station):
    bitstreams = find_bitstreams(vex, station)
    stream_list = vex['BITSTREAMS'][bitstreams].getall('stream_def')
    mask = 0
    for stream in stream_list:
        mask |= (1 << int(stream[2]))
        continue
    return mask

def mark5_mode(vex, station):
    sample_rate = get_sample_rate(vex, station)
    decimation = int(32e6 / sample_rate)
    das = find_das(vex, station)
    record_transport_type = vex['DAS'][das]['record_transport_type']
    if record_transport_type == 'Mark5B':
        bitstreams = find_bitstreams(vex, station)
        if bitstreams:
            mask = mark5_mask(vex, station)
            return "mode=ext:0x%x:%d;" % (mask, decimation)
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
        else:
            tracks = find_tracks(vex, station)
            fanout_list = vex['TRACKS'][tracks].getall('fanout_def')
            num_streams = len(fanout_list)
            pass

        if num_streams == 32:
            split = "16bitx2+8Ch2bit_hv"
        elif num_streams == 16:
            split = "8Ch2bit_hv"
        else:
            raise AssertionError, "unsupported Mark5B format"

        split += "+swap_sign_mag"
    else:
        tracks = find_tracks(vex, station)
        if tracks:
            fanout_list = vex['TRACKS'][tracks].getall('fanout_def')
            num_tracks = 0
            fanout = 0
            for fanout_def in fanout_list:
                num_tracks += len(fanout_def) - 4
                if fanout == 0:
                    fanout = len(fanout_def) - 4
                    pass
                continue
            pass

        if num_tracks == 64 and fanout == 2:
            split = "16Ch2bit1to2_hv"
        elif num_tracks == 32 and fanout == 2:
            split = "8Ch2bit1to2_hv"
        else:
            raise AssertionError, "unsupported Mark4/VLBA format"
            pass

        pass
            
    return split

def split_channel(vex, station, channel):
    das = find_das(vex, station)
    record_transport_type = vex['DAS'][das]['record_transport_type']
    if record_transport_type == 'Mark5B':
        bitstreams = find_bitstreams(vex, station)
        if bitstreams:
            stream_list = vex['BITSTREAMS'][bitstreams].getall('stream_def')
            bits_per_sample = 1
            for stream_def in stream_list:
                if stream_def[1] == "mag":
                    bits_per_sample = 2
                    break
                continue
            for stream_def in stream_list:
                if stream_def[0] == channel:
                    return int(stream_def[3]) / bits_per_sample
                continue
            pass
        pass
    else:
        tracks = find_tracks(vex, station)
        if tracks:
            fanout_list = vex['TRACKS'][tracks].getall('fanout_def')
            fanout = 0
            for fanout_def in fanout_list:
                fanout = len(fanout_def) - 4
                break
            bits_per_sample = 1
            for fanout_def in fanout_list:
                if fanout_def[2] == "mag":
                    bits_per_sample = 2
                    break
                continue
            for fanout_def in fanout_list:
                if fanout_def[1] == channel:
                    headstack = int(fanout_def[3])
                    track = int(fanout_def[4])
                    bits_per_channel = bits_per_sample * fanout
                    index = ((track - 2) - (track % 1)) / bits_per_channel
                    index += (track %1)
                    if headstack == 2:
                        index += 32 / bits_per_channel
                    return index
                continue
            pass
        pass

    raise AssertionError, "cannot map channel"

def get_format(vex, station):
    das = find_das(vex, station)
    record_transport_type = vex['DAS'][das]['record_transport_type']
    return record_transport_type

class configure:
    def init(self):
        self.station = config.station
        das = find_das(self.vex, self.station)
        self.record_transport_type = \
            self.vex['DAS'][das]['record_transport_type']
        if self.record_transport_type == 'Mark5B':
            bitstreams = find_bitstreams(self.vex, self.station)
            if bitstreams:
                stream_list = \
                    self.vex['BITSTREAMS'][bitstreams].getall('stream_def')
                self.bits_per_sample = 1
                for stream_def in stream_list:
                    if stream_def[1] == "mag":
                        self.bits_per_sample = 2
                        break
                    continue
                self.bits_per_channel = self.bits_per_sample
                pass
            self.vdifsize = 5000
            pass
        else:
            tracks = find_tracks(self.vex, self.station)
            if tracks:
                fanout_list = self.vex['TRACKS'][tracks].getall('fanout_def')
                for fanout_def in fanout_list:
                    self.fanout = len(fanout_def) - 4
                    break
                self.bits_per_sample = 1
                for fanout_def in fanout_list:
                    if fanout_def[2] == "mag":
                        self.bits_per_sample = 2
                        break
                    continue
                self.bits_per_channel = self.bits_per_sample * self.fanout
                pass
            self.vdifsize = 8000
            pass
        pass

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
                try:
                    split = split_mode(vex, config.station)
                except:
                    result = {}
                    result['error'] = "unsupported data format"
                    return json.dumps(result)

                for destination in map:
                    channels = ""
                    for channel in map[destination]:
                        index = split_channel(vex, config.station, channel)
                        if not channels:
                            channels = str(index)
                        else:
                            channels += ',' + str(index)
                        continue
                    split += ':'
                    split += channels
                    split += '='
                    split += destination.replace(':', '@')
                    continue
                pass
            else:
                destination = map.keys()[0]
                hostname = destination.split(':')[0]
                port = int(destination.split(':')[1])
                pass
        except:
            result = {}
            result['error'] = "malformed destination: %s" % input['destination']
            return json.dumps(result)

        resp = send_command("version?;")
        if not check_reply(resp, 0):
            return error_reponse("version?", resp)

        if len(map) > 1:
            what = "spin2net"
            if config.simulate:
                what = "spill2net"
                pass
            pass
        else:
            what = "in2net"
            if config.simulate:
                what = "fill2net"
                pass
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

        if len(map) > 1:
            command = "%s=bitsperchannel:%d" % (what, self.bits_per_channel)
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)

            command = "%s=vdifsize:%d" % (what, self.vdifsize)
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)

            command = "%s=connect:%s" % (what, split)
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)
            pass
        else:
            command = "net_port=%d;" % port
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)

            command = "%s=connect:%s:0;" % (what, hostname)
            resp = send_command(command)
            if not check_reply(resp, 0):
                return error_response(command, resp)
            pass

        config.what = what
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
            self.vex = vex.parse(input['vex'])
        except:
            result = {}
            result['error'] = "couldn't parse VEX"
            return json.dumps(result)

        try:
            self.init()
        except:
            result = {}
            result['error'] = "couldn't determine data format"
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
            return self.sink(input, self.vex, mtu)
        else:
            return self.source(input, self.vex, mtu)

    pass

class start:
    def POST(self):
        web.header('Content-Type', 'application/json')

        if config.sfxc:
            result = {}
            return json.dumps(result)

        what = config.what

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

        what = config.what
        config.what = ""

        if what != "fill2net" and what != "spill2net":
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
