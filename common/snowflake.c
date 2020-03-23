/*
Copyright (c) 2014 Dwayn Matthies <dwayn dot matthies at gmail dot com>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "snowflake.h"

long int snowflake_id(struct _snowflake_state * state) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int millisecs = tp.tv_sec * 1000 + tp.tv_usec / 1000 - state->epoch;
    long int id = 0L;

    // Catch NTP clock adjustment that rolls time backwards and sequence number overflow
    if ((state->seq > state->seq_max ) || state->time > millisecs) {
        ++state->waits;
        while (state->time >= millisecs) {
            gettimeofday(&tp, NULL);
            millisecs = tp.tv_sec * 1000 + tp.tv_usec / 1000 - state->epoch;
        }
    }

    if (state->time < millisecs) {
        state->time = millisecs;
        state->seq = 0L;
    }
    
    
    id = (millisecs << state->time_shift_bits) 
            | (state->region_id << state->region_shift_bits) 
            | (state->worker_id << state->worker_shift_bits) 
            | (state->seq++); 

    if (state->seq_max < state->seq)
        state->seq_max = state->seq;
    
    ++state->ids;
    return id;
}

int snowflake_init(struct _snowflake_state * state, 
long int epoch, int time_bits, int region_id_bits, int worker_id_bits, int sequence_bits, 
int region_id, int worker_id) 
{
    if(state->inited){
        printf("snowflake state allready inited.\n");
        return -1;
    }
    if(time_bits+region_id_bits+worker_id_bits+sequence_bits != 63){
        printf("time_bits+region_id_bits+worker_id_bits+sequence_bits!=63\n");
        return -1;
    }
    int max_region_id = (1 << region_id_bits) - 1;
    if(region_id < 0 || region_id > max_region_id){
        printf("Region ID must be in the range : 0-%d\n", max_region_id);
        return -1;
    }
    int max_worker_id = (1 << worker_id_bits) - 1;
    if(worker_id < 0 || worker_id > max_worker_id){
        printf("Worker ID must be in the range: 0-%d\n", max_worker_id);
        return -1;
    }
    
    state->epoch = epoch;
    state->time_bits = time_bits;
    state->region_id_bits = region_id_bits;
    state->worker_id_bits = worker_id_bits;
    state->sequence_bits = sequence_bits;

    state->time_shift_bits   = region_id_bits + worker_id_bits + sequence_bits;
    state->region_shift_bits = worker_id_bits + sequence_bits;
    state->worker_shift_bits = sequence_bits;
    
    state->worker_id    = worker_id;
    state->region_id    = region_id;
    state->seq_max      = (1L << sequence_bits) - 1;
    state->seq          = 0L;
    state->time         = 0L;

    state->seq_cap      = state->seq_max;
    state->waits        = 0L;
    state->seq_max      = 0L;
    state->ids          = 0L;

    return 0;
}
