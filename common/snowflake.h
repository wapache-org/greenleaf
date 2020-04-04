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

#ifndef __SNOWFLAKE__
#define __SNOWFLAKE__

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

// the timestamp in milliseconds of the start of the custom epoch
#define SNOWFLAKE_EPOCH 1577808000000 // 2020-01-01 00:00:00

#define SNOWFLAKE_TIME_BITS 41
#define SNOWFLAKE_REGIONID_BITS 4
#define SNOWFLAKE_WORKERID_BITS 10
#define SNOWFLAKE_SEQUENCE_BITS 8

struct _snowflake_state {
    int inited;

    // milliseconds since SNOWFLAKE_EPOCH 
    long int time;
    long int seq_max;
    long int worker_id;
    long int region_id;
    long int seq;

    long int epoch;
    int time_bits;
    int region_id_bits;
    int worker_id_bits;
    int sequence_bits;

    long int time_shift_bits;
    long int region_shift_bits;
    long int worker_shift_bits;

    time_t started_at;
    char *version;
    long int ids;
    long int waits;
    long int seq_cap;

} snowflake_global_state;

struct _snowflake_state snowflake_local_state;

/**
 * 线程不安全
 */
long int snowflake_id(struct _snowflake_state * state);
#define snowflake_id_default()  snowflake_id(&snowflake_global_state)
#define snowflake_id_local()  snowflake_id(&snowflake_local_state)

/**
 * return 0: suceess, other: failed
 */
int snowflake_init(struct _snowflake_state * state, long int epoch, int time_bits, int region_id_bits, int worker_id_bits, int sequence_bits, int region_id, int worker_id);

#define snowflake_init_default(region_id, worker_id) snowflake_init(&snowflake_global_state, \
SNOWFLAKE_EPOCH, SNOWFLAKE_TIME_BITS, SNOWFLAKE_REGIONID_BITS, SNOWFLAKE_WORKERID_BITS, SNOWFLAKE_SEQUENCE_BITS, region_id, worker_id)

#define snowflake_init_local() snowflake_init(&snowflake_local_state, \
SNOWFLAKE_EPOCH, SNOWFLAKE_TIME_BITS, SNOWFLAKE_REGIONID_BITS, 0, SNOWFLAKE_WORKERID_BITS+SNOWFLAKE_SEQUENCE_BITS, 0, 0)

#endif /* __SNOWFLAKE__ */