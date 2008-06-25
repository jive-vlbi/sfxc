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

#
# The C++ code that will be used to generate the Chanel_extractor final class
#
cfile = """
#include <cassert>
#include <iostream>
#include "../include/channel_extractor_interface.h"
#include "../include/channel_extractor_utils.h"
@dynamic_header@

class Channel_extractor_dynamic_impl : public Channel_extractor_interface
  {
    public:
      bool verbose;

      Channel_extractor_dynamic_impl() {}

      void initialise(const std::vector< std::vector<int> > &track_positions,
                      int IGNORED_size_of_one_input_word,
                      int input_sample_size) {
        initialise( track_positions );
        input_sample_size_ = input_sample_size;
      }


      void initialise(const std::vector< std::vector<int> > &track_positions) {
        assert( Tn_subbands == track_positions.size() );
        assert( Tfan_out == track_positions[0].size() );
        assert(Tfan_out <= 8);
        assert(8%Tfan_out == 0);
        assert(Tfan_out*Tsamples_per_byte == 8);
        verbose = false;
        if (verbose){

            std::cout << "REAL Table builder: " << std::endl;
            std::cout << "     Subbands: " << Tn_subbands << std::endl;
            std::cout << "      fan out: " << Tfan_out  << std::endl;
            std::cout << " samples/byte: " << Tsamples_per_byte << std::endl;
            std::cout << "sequence size: " << Tseqsize  << std::endl;
            std::cout << "     span_out: " << Tspan_out << std::endl;
            std::cout << "size of input word:" << Tsize_input_word << std::endl;
            std::cout << "Table builder: " << std::endl;

            for (unsigned int i=0;i<track_positions.size();i++) {
                std::cout << "Channel "<<i<<": ";
                for (unsigned int j=0;j<track_positions[i].size();j++) {
                    std::cout << track_positions[i][j] << ", ";
                  }
                std::cout << std::endl;
              }
          }
        table = new std::vector<Action>**[Tseqsize];
        //ftable = new uint8_t**[seqsize];
        for (unsigned int i=0;i<Tseqsize;i++) {
            table[i] = new std::vector<Action>*[256];
            for (unsigned int j=0;j<256;j++) {
                table[i][j] = new std::vector<Action>(0,0);
              }
          }

        int seqpref = 0;
        int cshift[256][Tn_subbands];
        for (unsigned int i=0;i<256;i++)
          for (int j=0;j<Tn_subbands;j++)
            cshift[i][j] = 7;

        for (unsigned int r=0;r<Tseqsize;r+=Tsize_input_word) {
            for (unsigned int i=0;i<Tn_subbands;i++) {
                for (unsigned int j=0;j<track_positions[i].size();j++) {
                    uint8_t idx = track_positions[i][j];
                    uint8_t seqidx = r + (idx/8);
                    uint8_t cidx = (idx) % 8;
                    for (unsigned int k=0;k<256;k++) {
                        find_add( table[seqidx][k], i, ((k>>cidx)&1)<<cshift[k][i], (cshift[k][i]) );
                        //find_add( table[seqidx][k], i, ((k>>cidx)&1)<<cshift[k][i], (cshift[k][i]) );
                        (cshift[k][i])--;
                      }
                  }
              }
          }
        int tidx=0;
        for (unsigned int i=0;i<256;i++) {
            for (unsigned int j=0;j<Tseqsize;j++) {
                std::vector<Action>::iterator bg = (*table[j][i]).begin();
                std::vector<Action>::iterator en = (*table[j][i]).end();
                //std::cout << "VALUE-SEQUENCE: " << i << "," << j << "  :";
                int idefix=0;
                while ( bg != en ) {
                    assert( idefix < Tspan_out);
                    newtab[i][j][idefix] = ((unsigned char)(*bg).value);
                    //std::cout << (*bg).channel << ":("<< (*bg).value << ") ";
                    //std::cout << "toto:"<< j<< " :" << (int)newtab[i][j][idefix] << std::endl;
                    idefix+=1;
                    if (i == 255 ) {
                        assert( tidx < Tseqsize*Tspan_out);
                        order[tidx] = (*bg).channel;
                        //std::cout << "Adding: " << (*table[j][i]).size() << " " << tidx << " " << (*bg).channel <<" " << std::endl;
                        tidx++;
                      }
                    bg++;
                  }
                //std::cout << std::endl;
              }
          }
        //std::cout << "Info: " << std::endl;
        for (unsigned int i=0; i<Tseqsize*Tspan_out;i++) {
            //std::cout << (int) order[i] << ", ";
            fasttmp[i] = (unsigned char*)&(tmpout[ order[i] ] );
          }
        //std::cout << "Endl" << std::endl;
        memset(tmpout, 0, Tn_subbands+1);
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

    private:
      std::vector<Action> ***table;

      uint8_t  order[Tseqsize*Tspan_out];
      unsigned char* fasttmp[Tseqsize*Tspan_out];
      uint8_t  newtab[256][Tseqsize][Tspan_out];

      char tmpout[Tn_subbands+1];
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
input_data = open(input_file).read()

exec input_data # isn't that tricky :)
tmpfile =tempfile.NamedTemporaryFile(mode="r+t").name+".cc"


size_input_word = size_of_one_input_word
samples_per_byte = 8/fan_out
sequence_size = size_of_one_input_word*8/fan_out
#print "Channel_extractor_compiler v0.1";
#print "N_SUBBANDS: "+`n_subbands`
#print "FANOUT: "+`fan_out`
#print "SPANOUT: "+`span_out`
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

table=[]
for i in range(0, 256):
  table.append([])
  for j in range(0, sequence_size):
    table[i].append([])
    for k in range(0, span_out):
      table[i][j].append( "V("+`i`+","+`j`+","+`k`+")" )


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
header+="\n#define Tn_subbands "+`n_subbands`
header+="\n#define Tfan_out "+`fan_out`
header+="\n#define Tsamples_per_byte "+`samples_per_byte`
header+="\n#define Tseqsize "+`sequence_size`
header+="\n#define Tspan_out "+`span_out`
header+="\n"

#fpt = open('autogen_mainloop.cc', 'wt')
mainloop = ""
init_dest="unsigned char* output_data["+`n_subbands`+"]={\n"
for i in range(0, n_subbands-1):
	init_dest+="\toutput_dataIN["+`i`+"],\n"
init_dest+="\toutput_dataIN["+`n_subbands-1`+"]\n};\n\n"
#print init_dest
#fpt.write(init_dest);
mainloop += init_dest

if False:
  mainloop += "\nwhile (currbuffer < endbuffer) {\n"
  for i in range(0, sequence_size):
    for j in range(0, span_out):
      dst = order[span_out*i+j]
      if tag[span_out*i+j] == 'i':
        oper[dst].append( " newtab[ currbuffer["+`i`+"] ] ["+`i`+"]["+`j`+"] " );
      elif tag[span_out*i+j] == 'f':
        #print "output_data["+`dst`+"][outindex] = ",
        #for str in oper[dst]:
        #  print ""+str+" | ",
        #print "newtab[ currbuffer["+`i`+"] ] ["+`i`+"]["+`j`+"];";
        #main_loop+=" std::cout << \"Value:\" << outindex << std::endl;";
        mainloop+="\toutput_data["+`dst`+"][outindex] = "
        for str in oper[dst]:
          mainloop+=""+str+" | "
        mainloop+="newtab[ currbuffer["+`i`+"] ] ["+`i`+"]["+`j`+"];\n";
  mainloop+="\n\t currbuffer+=Tseqsize; \n\t ++outindex;\n}\n"
  #fpt.write(main_loop);

else:
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
    lines.append("output_data["+`i`+"][outindex] =")
  seqd = 0
  while pcroll[0] < 7:
    idx = 0
    for ch in track_positions:
       for j in ch:
        if not (pcroll[idx] == 0):
          lines[idx] += " | "
        endpos=7-pcroll[idx]
        curpos=j%8
        diff = curpos-endpos
        roll = " none "
        #if diff == 0:
        #  roll = ""
        if diff <= 0:
          roll = "<<"+`-diff`
        if diff > 0:
          roll = ">>"+`diff`
        #if diff == -7 or diff == 7:
        #  lines[idx]+= " ((currbuffer["+`seqd*size_input_word+j/8`+"]) "+roll+") "
        #else:
        lines[idx]+= " (((currbuffer["+`seqd*size_input_word+j/8`+"]) & "+`(1<<(j%8))`+") "+roll+") "
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
fpt = open(tmpfile, "w")
fpt.write(totalfile)
fpt.close()

if do_compile:
	cmd = "g++ -fPIC -O3 -DNDEBUG "+tmpfile+" -I./ -I../../include/ -shared -Wl,-soname,"+outputname+" -o "+outputname
	print "Performing precalculation....: "+cmd
	os.system(cmd);
	#os.unlink(tmpfile)
	print "[OK]"

sys.exit(0);



