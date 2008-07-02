#include <cassert>
#include <iostream>
#include <sstream>
#include <dlfcn.h>
#include "exception_common.h"
#include "channel_extractor_dynamic.h"
#include "channel_extractor_utils.h"
#include "channel_extractor_5.h"
#include "utils.h"

Channel_extractor_dynamic::Channel_extractor_dynamic
(const std::string& dirname, bool compile)
{
  dirname_ = dirname;
  compile_ = compile;
	name_ = "Channel_extractor_dynamic(fast algorithm)";
}

void Channel_extractor_dynamic::initialise(
  const std::vector< std::vector<int> > &track_positions_,
  int size_of_one_input_word_,
  int input_sample_size_)
{
	/// Table to convert the channels configuration in a unique filename using
	/// filesystem compatible char-codes.
  unsigned char table[64];
  int idx=0;

  /// letters and number are ok
  for (unsigned char i='a';i<'z';i++) table[idx++] = i;
  for (unsigned char i='A';i<'Z';i++) table[idx++] = i;
  for (unsigned char i='0';i<'9';i++) table[idx++] = i;
  /// Then complete with filesystem friendly symbols.
  table[idx++] = '-';
  table[idx++] = '_';
  table[idx++] = '@';
  table[idx++] = '=';
	table[idx++] = '+';
	//std::cout << "VALUE:" << idx;
	assert(idx == 64 );

	/// Generate the name
  std::stringstream str;
  str << "dynamic_channel_extractor";
  for (unsigned int i=0;i<track_positions_.size();i++){
      str << "[";
      for (unsigned int j=0;j<track_positions_[i].size();j++){
          str << table[ track_positions_[i][j] ];
        }
      str << "]";
    }
  str << ".so";

  //std::cout << "Searching for:" << str.str() << std::endl;

	/// Load the shared library with the corresponding name.
  hidden_implementation_ = dlload_and_construct( dirname_+str.str() );

  /// Does it work ?
  if ( hidden_implementation_ == NULL ){

			/// No ! so we need to compile ourself the extractor.
      DEBUG_MSG("Unable to load the dynamic extractor:" << dirname_ << str.str() );

      /// Are we allowed to compile ?
      if ( compile_ ){
					/// Yes... so let's do it
          hidden_implementation_ = compile_load_construct( dirname_+str.str(), track_positions_,
                                   size_of_one_input_word_, input_sample_size_ );
					if( hidden_implementation_ == NULL ){
							DEBUG_MSG("Compiling optimized extractor fails ! use slow one instead");
							name_ = "Channel_extractor_dynamic (slow mode!)";
							hidden_implementation_ = new Channel_extractor_5();

					}
        }
			else {
							DEBUG_MSG("Unable to compile extractor ! use slow one instead");
							name_ = "Channel_extractor_dynamic (slow mode!)";
							hidden_implementation_ = new Channel_extractor_5();
			}
    }
	/// Check it was succesfully loaded.
  assert( hidden_implementation_ != NULL );

  /// If so initialize it
	hidden_implementation_->initialise(track_positions_, size_of_one_input_word_, input_sample_size_);
}

Channel_extractor_interface* Channel_extractor_dynamic::compile_load_construct(
		const std::string& ch_filename,
    const std::vector< std::vector<int> > &track_positions_,
    int size_of_one_input_word_,
    int input_sample_size_)
{
	/// All the channel_extractor parameters are saved in a file,
	/// see utils/channel_extractor_compiler.py for the format to
	/// follow.
	track_positions = track_positions_;
  size_of_one_input_word = size_of_one_input_word_;
  input_sample_size = input_sample_size_+1;
  fan_out = track_positions[0].size();
  n_subbands = track_positions.size();

  int span_out = compute_span_out( track_positions_ );

  char* filename = (char*)"ch_ex_params.txt";
  FILE* fpt = fopen(filename,"wt");
  fprintf(fpt, "n_subbands = %d;\n", n_subbands);
  fprintf(fpt, "fan_out= %d;\n", fan_out);
  fprintf(fpt, "span_out= %d;\n", span_out);
	fprintf(fpt, "input_sample_size= %d;\n", input_sample_size);
	fprintf(fpt, "size_of_one_input_word= %d;\n", size_of_one_input_word);
	fprintf(fpt, "outputname= \"%s\";\n", ch_filename.c_str());
  fprintf(fpt, "track_positions = [\n");
  for (unsigned int i=0;i<track_positions.size();i++) {
    fprintf(fpt, "\t\t\t[", i);
    for (unsigned int j=0;j<track_positions[i].size();j++) {
      if ( j != track_positions[i].size()-1 )
        fprintf(fpt, "%d, ", track_positions[i][j]);
      else
        fprintf(fpt, "%d", track_positions[i][j]);
    }
    if ( i != track_positions.size()-1 )
      fprintf(fpt, "],\n");
    else
      fprintf(fpt, "]\n");
  }
  fprintf(fpt, "\t\t];\n");

  fclose(fpt);

	/// Let's now call the piece of python code that will generate
	/// a dedicated channel_extractor for this specific experiment.
  std::string cmd = "channel_extractor_compiler.py ";
  cmd=cmd+filename;
  int ret = system(cmd.c_str());
  if ( ret != 0 ) {
    std::cout << "Unable to compile the channel extractor using command: "<< cmd << std::endl;
    return  NULL;
  }

	/// If the compilation succeed we can load the generated
	/// channel extractor.
	return dlload_and_construct( ch_filename );
}

Channel_extractor_interface* Channel_extractor_dynamic::dlload_and_construct(const std::string& name)
{
	Channel_extractor_interface* (*channelizer_create)();

	/// Use dlopen to load the channel_extractor.
  void *chn = dlopen(name.c_str(), RTLD_NOW);
  if (chn == NULL) {
      //DEBUG_MSG( " Unable to open shared library:" << name );
      return  NULL;
    }

  /// load a function called "channelizer_factory" that we will use to
  /// create channelizer instances.
  channelizer_create=(Channel_extractor_interface* (*)())dlsym(chn,"channelizer_factory");

	/// Do this function existing in the loaded shared filed ?
  if (channelizer_create == NULL) {
			DEBUG_MSG("Missing function in shared library:" << dlerror() );
      return  NULL;
    }

	/// If the function exist, we can ask now to create the
	/// channelizer instance.
  return (*channelizer_create)();
}

void Channel_extractor_dynamic::extract(unsigned char *in_data1,
                                        unsigned char **output_data)
{
  assert( hidden_implementation_ != NULL );
  hidden_implementation_->extract( in_data1, output_data );
}
