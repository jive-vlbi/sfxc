/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 *     flexible. Don't be afraid by the compile time as the code
 *     is highly templated to help the compile to detect the constant
 *     (number of channels, fan_out, etc..). The performance dependon 
 *     the bit pattern. With the worst bit pattern the performance on 
 *     an IntelCore2 is from [60MB/s (16 channel) to 130MB/s]. 
 *     On DAS3 cut the number by 2 to get a rule of thumb approximation. 
 */

// We use a lot of template to enforce all of this
// to be consider constant by the compiler
// We also use an hidden-class to hide these details to the 
// user of the channel_extractor.
        input_sample_size_ = input_sample_size;
      * 
      *  the input stream is cutted in sequence. A sequence is the number of 
      *  input byte needed to generate one output byte on each of the channel.  
      *  the precomputed table contains the following:
      *  newtable[input_byte_value][sequence_index][span_out_index] = value 
      *  
      *  The span_out_index is < span_out. Span_out is the number of different
      *  channels that are 'touched' by one input byte.
      *
      *  A second table named fasttmp stores the destination_channel for each
      *  sequence_index*span_out+span_out_index
      *
      *  while currbyteptr < totalbyteptr:
      *    for i in sequence_index: 
      *        for j in span_out_index:
      *          fasttmp |= newtable[*currbyteptr][i][j]
      *    copy temporary result to real output
      *    currbyteptr+=sequence_size
      */
      {
        while (currbuffer < endbuffer) {
                  }
                output_data[i][outindex] = tmpout[i];
      int input_sample_size_;
      

Channel_extractor_fast::Channel_extractor_fast() {
// as template arguments
// Create an extractor by converting the parameters 
// as template arguments
// as template arguments
// Create an extractor by converting the parameters 
// as template arguments
// as template arguments