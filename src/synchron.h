#ifndef _SYNCHRON_EH
#define _SYNCHRON_EH

#include "cercs_env.h"


typedef struct _ping_pong {
  chr_time t1;
  chr_time t1_p;
  chr_time t2;
  chr_time t2_p;
  chr_time offset2;
} ping_pong, * ping_pong_ptr;

static FMField chr_time_field_list[] =
{
  {"d1", "float", sizeof(double), FMOffset(chr_time *, d1)},
  {"d2", "float", sizeof(double), FMOffset(chr_time *, d2)},
  {"d3", "float", sizeof(double), FMOffset(chr_time *, d3)},
  {NULL, NULL, 0, 0}
};

static FMField ping_pong_field_list[] =
{
  {"t1", "chr_time", sizeof(chr_time), FMOffset(ping_pong_ptr, t1)},
  {"t1_p", "chr_time", sizeof(chr_time), FMOffset(ping_pong_ptr, t1_p)},
  {"t2", "chr_time", sizeof(chr_time), FMOffset(ping_pong_ptr, t2)},
  {"t2_p", "chr_time", sizeof(chr_time), FMOffset(ping_pong_ptr, t2_p)},
  {"offset2", "chr_time", sizeof(chr_time), FMOffset(ping_pong_ptr, offset2)},
  {NULL, NULL, 0, 0}
};

static FMStructDescRec ping_pong_format_list[] =
{
  {"ping_pong", ping_pong_field_list, sizeof(ping_pong), NULL},
  {"chr_time", chr_time_field_list, sizeof(chr_time), NULL},
  {NULL, NULL}
};


#endif
