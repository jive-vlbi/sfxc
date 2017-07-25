#! /usr/bin/env python
#
# Helper script to generate and compile a C++ Channel_extractor class specific
# for a given set of stream parameters and configuration. This permit to use
# the fastest algorithm with late optimization. A channel_extractor
# generated this way is in general 2x faster than all the others known
# approaches (channel_extractor_5, channel_extractor_fast)
#
import tempfile
import os,sys, types
import re

def which (filename):
  if not os.environ.has_key('PATH') or os.environ['PATH'] == '':
    p = os.defpath
  else:
    p = os.environ['PATH']

  pathlist = p.split (os.pathsep)

  for path in pathlist:
    f = os.path.join(path, filename)
    if os.access(f, os.X_OK):
      return f
  return None

#
# The C++ code that will be used to generate the Chanel_extractor final class
#
cfile = """
#include <cassert>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include "sfxc/channel_extractor_interface.h"
@dynamic_header@

class Channel_extractor_dynamic_impl : public Channel_extractor_interface
  {
    public:
      bool verbose;

      Channel_extractor_dynamic_impl() {}

      void initialise(const std::vector< std::vector<int> > &track_positions,
                      int IGNORED_size_of_one_input_word,
                      int input_sample_size, 
                      int bits_per_sample_) {
        input_sample_size_ = input_sample_size;
      }

      virtual void extract(unsigned char *in_data1,
                           unsigned char **output_data) {
        do_task_no_offset( input_sample_size_, in_data1, output_data );
      }

      void do_task_no_offset(int n_input_samples,
                             const unsigned char * in_data,
                             unsigned char ** output_dataIN) {

        unsigned int outindex=0;
        unsigned char* currbuffer = (unsigned char*)in_data;
        unsigned char* endbuffer = currbuffer+(n_input_samples)*Tsize_input_word;
	//std::cout << "Number input data :"<< (int)endbuffer - (int)currbuffer;
	@dynamic_mainloop@
	//std::cout << "Computation done :"<< (int)endbuffer - (int)currbuffer;
      }

    private:
      uint32_t input_sample_size_;
  };


extern "C" {
  Channel_extractor_dynamic_impl* channelizer_factory(){
    return new Channel_extractor_dynamic_impl();
  }
}
"""
do_compile = True

# Default input file or user given one ?
input_file = 'ch_ex_params.txt'
if len(sys.argv) > 1:
  input_file = sys.argv[1]
  pass
input_data = open(input_file).read()

exec input_data # isn't that tricky :)
tmpname = tempfile.NamedTemporaryFile(mode="r+t").name
tmpccfile = tmpname + ".cc"
tmpname = tempfile.NamedTemporaryFile(mode="r+t", dir=".").name
tmpsofile = tmpname + ".so"

size_input_word = size_of_one_input_word
samples_per_byte = 8/fan_out
sequence_size = size_of_one_input_word*8/fan_out
#print "Channel_extractor_compiler v0.1";
#print "N_SUBBANDS: "+`n_subbands`
#print "FANOUT: "+`fan_out`
#print "SEQUENCE_SIZE: "+`sequence_size`
#print "INPUT_SAMPLE_SIZE="+`input_sample_size`
#print "N(size_of_one_input_word: "+`size_input_word`

#idx = 0
#for ch in track_positions:
  #print 'Channel: '+`idx`+" [",
  #for j in ch:
  #  print `j`+", ",
  #print ']'
  #idx = idx+1

def get_channel_idx(idx):
  global track_positions
  cpt = 0
  for i in track_positions:
    for j in i:
     if j == idx:
       return [cpt,j]

    cpt=cpt+1

data_table = []
for i in range(0, sequence_size*8):
  ret = get_channel_idx(i%(size_input_word*8))
  data_table.append( ret )

if len(data_table) == 0:
		print "Invalid data table"
		sys.exit(-1)

#print "DATA_TABLE SIZE: "+`data_table`
cpt = 0
order = []
tag = []
line = []
linetag = []
status = []
oper = []
fastoper = []
for i in range(0, n_subbands):
  status.append('i')
  oper.append([])
  fastoper.append([])

for i in data_table:
	if not (i == None):
		cpt=cpt+1
		#print `i[0]`,

		if line.count(i[0]) == 0:
			line.append(i[0])
			if status[ i[0] ] == 'i':
				linetag.append( status[i[0]] )
				status[ i[0] ] = 'i'

		if cpt % 8 == 0:
			order.extend( line )
			tag.extend( linetag )
			line = []
			linetag = []
			#print ""

idx = 0
for i in reversed( range(0, len(order) ) ):
    dta = order[i]
    if status[ dta ] == 'i':
      tag[i] = 'f'
      status[ dta ] = 'f'

#print "ORDER:"
idx = 0
for i in order:
  ln = tag[idx]
  #print `i`+"."+ln+"",
  idx+=1



header="/// automatically genereated Channel_extractor_compiler v0.1\n"
header+="\n#define Tsize_input_word "+`size_input_word`
header+="\n#define Tseqsize "+`sequence_size`
header+="\n"

#fpt = open('autogen_mainloop.cc', 'wt')
mainloop = ""
init_dest="unsigned char* output_data["+`n_subbands`+"]={\n"
for i in range(0, n_subbands-1):
	init_dest+="\t\toutput_dataIN["+`i`+"],\n"
init_dest+="\t\toutput_dataIN["+`n_subbands-1`+"]\n\t};\n\n"
#print init_dest
#fpt.write(init_dest);
mainloop += init_dest

idx = 0
for ch in track_positions:
  print 'Channel: '+`idx`+" [",
  for j in ch:
    print `j`+", ",
  print ']'
  idx = idx+1

pcroll=[]
lines=[]

for i in range(0, n_subbands):
  pcroll.append(0)
  lines.append("\t output_data["+`i`+"][outindex] =")
seqd = 0
while pcroll[0] < 7:
  idx = 0
  for ch in track_positions:
    i = 0
    while i < len(ch):
      j = ch[i]
      if not (pcroll[idx] == 0):
        lines[idx] += " | "
        if(ch.index(j)%3==0):
          lines[idx] += "\n\t\t\t\t "

      if i < len(ch) - 1 and j == ch[i + 1] + 1 and not j % 8 == 0:
        mask = 3; j = ch[i + 1]; i += 2; pcroll[idx] += 1
      else:
        mask = 1; i += 1
      
      endpos=(pcroll[idx]/bits_per_sample+1)*bits_per_sample-pcroll[idx]%bits_per_sample-1
      curpos=j%8
      diff = curpos-endpos
      roll = " none "
      if diff <= 0:
        roll = "<<"+`-diff`
      if diff > 0:
        roll = ">>"+`diff`
      lines[idx]+= " (((currbuffer["+`seqd*size_input_word+j/8`+"]) & "+`(mask<<(j%8))`+") "+roll+") "
      pcroll[idx]+=1
    idx = idx+1

  seqd += 1
mainloop += "\n while (currbuffer < endbuffer) {\n"

for line in lines:
  mainloop += line+";\n"
mainloop +="\n\t currbuffer+=Tseqsize; \n\t ++outindex;\n}\n"

#fpt.write(main_loop2)
#fpt.close()
reg1 = re.compile("@dynamic_header@")
reg2 = re.compile("@dynamic_mainloop@")
tmp1 = reg1.sub(header, cfile)
totalfile = reg2.sub(mainloop, tmp1)

## We save the content into a temporary file file
fpt = open(tmpccfile, "w")
fpt.write(totalfile)
fpt.close()
if do_compile:
  # First find include directory
  position=sys.argv[0].rfind('/')
  if(position>=0):
    filename=sys.argv[0][position+1:]
  else:
    filename = sys.argv[0]
  path_filename = which(filename)
  position=path_filename[0:path_filename.rfind('/')].rfind('/')
  if(position>=0):
    include_dir = ' -I'+path_filename[0:position+1]+'include/'
  else:
    include_dir = ""
  # Perform actual compilation
  cmd = "g++ -fPIC -O3 -DNDEBUG " + tmpccfile + include_dir + " -shared -Wl,-soname," + outputname + " -o " + tmpsofile
  print "Performing precalculation....: "+cmd
  os.system(cmd);
  #os.unlink(tmpccfile)
  os.rename(tmpsofile, outputname)
  print "[OK]"

sys.exit(0);



