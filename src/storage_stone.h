#ifndef E_STORAGE_STONE
#define E_STORAGE_STONE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "evpath.h"
#include "ev_dfg.h"

#define NOT_FINISHED 0
#define FINISHED 1
#define PRINT 2
#define SEND_MISSING 3
#define IS_IT_DONE 4

#ifdef __cplusplus
extern "C" {
#endif

/* The following three structures are used to pass 
 * callback messages between the transaction, storage,
 * and end sink stone */

typedef struct relay {
	int which_message;
} relay, *relay_ptr;

static FMField relay_field_list[] =
{
	{"which_message", "integer", sizeof(int), FMOffset(relay_ptr, which_message)},
	{NULL, NULL, 0, 0}
};

static FMStructDescRec relay_format_list[] =
{
	{"relay", relay_field_list, sizeof(relay), NULL},
	{NULL, NULL}
};

/* These three data structures are intended for the internal message
 * from the storage stone to the transaction stone, whenever a new 
 * message is sent down a pathway.  The transaction stone has a sense
 * of who it should be expecting. */

typedef struct _heads_up_mesg {
	int incoming_mesg;
} heads_up_mesg, *heads_up_mesg_ptr;

static FMField heads_up_mesg_field_list[] =
{
	{"incoming_mesg", "integer", sizeof(int), FMOffset(heads_up_mesg_ptr, incoming_mesg)},
	{NULL, NULL, 0, 0}
};

static FMStructDescRec heads_up_mesg_format_list[] =
{
	{"heads_up", heads_up_mesg_field_list, sizeof(heads_up_mesg), NULL},
	{NULL, NULL}
};

/* These three data structures define a a message that tells the storage
 * stones exactly which items have not been received, but were expected 
 * to have been received. */

typedef struct _message_send_again {
	int size;
	int * store;
} message_sa, *message_sa_ptr;

static FMField message_field_list[] =
{
	{"size", "integer", sizeof(int), FMOffset(message_sa_ptr, size)},
	{"store", "integer[size]", sizeof(int), FMOffset(message_sa_ptr, store)},
	{NULL, NULL, 0, 0}
};

static FMStructDescRec message_format_list[] = 
{
	{"message_sa", message_field_list, sizeof(message_sa), NULL},
	{NULL, NULL}
};

// Data structures that are used by the storage and transaction stone for communication
static const FMStructDescList init_storage_queue_list[] = {relay_format_list, heads_up_mesg_format_list, message_format_list, NULL};

/* Creates a storage action spec.  I want to make this a real function in EV_dfg so that I can set 
 * the size here.  If I do that, then I don't have to worry about extra source stones */
extern char * create_e_storage_action_spec(FMStructDescList * list, char * storage_func_src);

/* This is a templated transaction spec that takes care of the messages on the recieving side.
 * We need to move some of the query logic out of the client side and into functions in the storage stone base
 * now. */
extern char * create_e_transaction_action_spec(FMStructDescList * list, int size);

extern char * create_e_rolling_bucket_action_spec(FMStructDescList * list, int size, char* bucket_func_src);


#ifdef __cplusplus
}
#endif

#endif
