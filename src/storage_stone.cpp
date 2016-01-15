/* The major difference between this file and the one above is that the storage logic is 
automatically taken care of by this version of the storage stone.  The rolling window and the 
tumbling window will both work automatically.  This is currently done by artificially creating 
some hard coded CoD code and pasting it around the user supplied function. */

#include "storage_stone.h"


static EVsource source_handle_storage;

char * create_e_storage_action_spec(FMStructDescList * list, char * storage_func_src)
{
	int i;
	int new_size;
	int old_size;
	char * temp;

	if(!list[0])
	{
		fprintf(stderr, "FMStructDescList passed to storage action spec has size of 0\n");
		return NULL;
	}


	// Use old_size to count the elements in the incoming list
	for (old_size = 0; list[old_size]; ++old_size);
	new_size = old_size + 5;
	FMStructDescList * new_list = (FMStructDescList *)malloc(new_size * sizeof(FMStructDescList));


	for (i = 0; i < new_size; ++i)
	{
		if (i < (new_size - 5))
		{
			new_list[i] = list[i];
		}
		else
		{
			new_list[i] = init_storage_queue_list[(i - old_size)];
		}
	}

	temp = create_multityped_action_spec(new_list, storage_func_src);	
	

    //I can't remember why I'm doing this, and I seriously question my judgement in doing so
    //...but it works, so....
    //I'll keep it around just in case, but kill it in 10/2015
	/*for (i = 0; temp[i]; ++i);
	action_spec = (char *)malloc((i+2) * sizeof(char));
	memcpy(action_spec, temp, (i+2));*/


	return temp;

}



static char * multi_func_transac_temp = "{ \n\
	static int * table;\n\
	static int num_missing = 0;\n\
    static int the_size = 0;\n\
	if(the_size == 0){\n\
		the_size = %d;\n\
		table = (int *) malloc(the_size * sizeof(int));\n\
		int i;\n\
		for(i = 0; i < the_size; ++i){\n\
			int * temp_ptr;\n\
			temp_ptr = table + i;\n\
			(*temp_ptr) = 0;\n\
		}\n\
	}\n\
	int total_events = EVcount_full();\n\
	int total_heads = EVcount_heads_up();\n\
	int total_simp = EVcount_simple();\n\
	void * last = EVdata_full(total_events - 1);\n\
	if(EVdata_heads_up(total_heads - 1) == last){\n\
		heads_up * temporary = (heads_up *) last;\n\
		int	inc = temporary->incoming_mesg;\n\
		if(inc < 0 || inc > the_size){\n\
			printf(\"Error message out of bounds!!!\\n\");}\n\
		else{\n\
			if((*(table + inc)) != -1){\n\
				(*(table + inc)) = 1;\n\
				EVdiscard_heads_up(total_heads - 1);\n\
		}}}\n\
	else{\n\
		if(EVdata_simple(total_simp - 1) == last){\n\
			attr_list event_temp;\n\
			event_temp = EVget_attrs_simple(total_simp - 1);\n\
			int ID = attr_ivalue(event_temp, \"ID_atom\");\n\
			if(ID < 0 || ID > the_size){\n\
				printf(\"Error message out of bounds!!!\\n\");}\n\
			else{\n\
				if((*(table + ID)) != -1){\n\
					(*(table + ID)) = -1;\n\
					EVdiscard_and_submit_simple(0, total_simp - 1);\n\
				}\n\
				else{\n\
					printf(\"Error: %s, simple_message received twice!!\\n\", ID);\n\
					EVdiscard_and_submit_simple(0, total_simp - 1);\n\
				}}}\n\
		else{\n\
			relay * last_mesg;\n\
			last_mesg = (relay *) EVdata_relay(0);\n\
			if(last_mesg != last){\n\
				printf(\"Error: unexpected data_type.  Expected relay message\\n\");\n\
			}\n\
			if(last_mesg->which_message == 4 || last_mesg->which_message == 2){\n\
				int i;\n\
				int is_done = 1;\n\
				simple temp_simp;\n\
				for(i = 0; i < the_size; ++i){\n\
					if((*(table + i)) == 1){\n\
						is_done = 0;\n\
						++num_missing;\n\
					}\n\
					else{\n\
						if((*(table + i)) == 0){\n\
							printf(\"No forewarned message nor actual message received\\n\");\n\
                        }\n\
					}\n\
					if(last_mesg->which_message == 2)\n\
						printf(\"Value of array index %s: %s\\n\", i, (*(table + i)));\n\
				}\n\
				if(last_mesg->which_message == 4){\n\
					last_mesg->which_message = is_done;\n\
					EVdiscard_and_submit_relay(1, 0);\n\
				}\n\
			}\n\
			else{\n\
				if(last_mesg->which_message == 3){\n\
					message_sa new_message;\n\
					int count;\n\
					new_message.store = (int *) malloc(sizeof(int) * num_missing);\n\
					new_message.size = num_missing;\n\
					int i = 0;\n\
					for(count = 0; count < the_size; ++count){\n\
						if((*(table + count)) == 1){\n\
							new_message.store[i] = count;\n\
							++i;\n\
						}\n\
					}\n\
					if(i != num_missing){\n\
						printf(\"Error in the logic above!\\n\");\n\
					}\n\
					EVdiscard_relay(0);\n\
					EVsubmit(2, new_message);\n\
					num_missing = 0;\n\
				}\n\
				else{\n\
					printf(\"Unknown data structure received.\\n\");\n\ }}}}}\0\0";


char * create_e_transaction_action_spec(FMStructDescList * list, int size)
{
    int length_of_add = 1;
    char * transact_func_final = (char *) malloc((strlen(multi_func_transac_temp) + length_of_add) * sizeof(char));
    sprintf(transact_func_final, multi_func_transac_temp, size, "\%d", "\%d", "\%d" );

    /*Sanity check for this madness */
    if((strlen(multi_func_transac_temp) + length_of_add) != strlen(transact_func_final) + 1)
    {
        fprintf(stderr, "Warning: the length of the hardcoded string doesn't add up...may cause strange problems\n");
        fprintf(stderr, "The value of the malloc is: %d\nThe value of the string is: %d\n", strlen(multi_func_transac_temp) + length_of_add, strlen(transact_func_final));
    }

    //I'm worried about a possible error here, since you can't really see NULL char's...
    transact_func_final[strlen(transact_func_final) + 1] = '\0';

	return create_e_storage_action_spec(list, transact_func_final); 

}

/*DANGER!!! EVcount_simple assumes the data type will be called simple...I will 
  fix that by moving everything to EVPath after this paper*/
static char * bucket_top_src = "{\n\
    static int the_size = 0;\n\
    if(the_size == 0)\n\
    {\n\
        the_size = %d;\n\
    }\n\
    if(EVcount_simple() == the_size)\n\
    {\n\0";

static char * bucket_roll_bottom = "for(i = 0; i < EVcount_simple(); ++i)\n\
        {\n\
            EVdiscard_simple(0);\n\
        }\n\
    }}\0\0";


char * create_e_rolling_bucket_action_spec(FMStructDescList * list, int size, char * bucket_func_src)
{
    if(size <= 0)
        return NULL;

    /* I do a quick algorithm to determine how many extra characters I will need 
     * when I replace the size in the string.  It's fairly easy to understand, and 
     * it seems to work for all my current cases, so I'll leave it at that... */
    int length_of_add = 0;
    int temp_size = size;
    while(temp_size != 0)
    {
        temp_size = temp_size / 10;
        if(temp_size != 0)
            ++length_of_add;
    }

    char * action_spec;
    char * bucket_top_final = (char *) malloc((strlen(bucket_top_src) + length_of_add) * sizeof(char));
    sprintf(bucket_top_final, bucket_top_src, size);

    //Done it to here
    size_t string_size = strlen(bucket_top_final) + strlen(bucket_func_src) + strlen(bucket_roll_bottom);
    char * complete_bucket_func = (char *)malloc((string_size + 2) * sizeof(char));

    strcpy(complete_bucket_func, bucket_top_final);
    strcat(complete_bucket_func, bucket_func_src);
    strcat(complete_bucket_func, bucket_roll_bottom);

    if((strlen(complete_bucket_func)) != string_size)
    {
        fprintf(stderr, "Error: the strlength is not as expected (should be equal)\nString length is: %d\t\tString size is: %d\n", strlen(complete_bucket_func), string_size);
        exit(1);
    }
    complete_bucket_func[strlen(complete_bucket_func) + 1] = '\0';
    
    printf("%s", complete_bucket_func);

	  action_spec = create_multityped_action_spec(list, complete_bucket_func);	

    return action_spec;
}
