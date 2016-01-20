#include "dfg_functions.h"
#include "khan_ffs.h"

struct dfg_unit test_dfg;

extern FMField simple_field_list[];
extern FMStructDescRec simple_format_list[];

static char *router_function = "\
{\n\
  static int count = 0;\n\
    return (count++) % 2;\n\
}\0\0";

char *router_action;

int dfg_init_func(void)
{
  test_dfg.cm=CManager_create();
  CMlisten(test_dfg.cm);
  test_dfg.dfg_master = EVmaster_create(test_dfg.cm);
  test_dfg.str_contact = EVmaster_get_contact_list(test_dfg.dfg_master);
  FILE *op;
  op = fopen("master.info","w");
  fprintf(op,"%s",test_dfg.str_contact);
  fflush(op);
  fclose(op);
  router_action = create_router_action_spec(simple_format_list, router_function);
  //Router not necessary at this point....
  return 1;
}

/*
int dfg_create_func(char *mode, int ncount, char **nodelist, EVmasterJoinHandlerFunc func)
{

  int ret=0,i;
  if(test_dfg.dfg_master)
  {
    if(strcmp(mode,"static")==0)
    {
      if(ncount>0)
      {
        test_dfg.node_count = ncount;
        for (i=0; i <= test_dfg.node_count; i++) 
          test_dfg.nodes[i] = (char*)malloc(15);
        //NOTE--This makes the max length of a name 15 with no overspill check, also in dfg_master.cpp
        test_dfg.nodes[0]="masternode";
        for(i=1;i<=ncount;i++)
          test_dfg.nodes[i]=nodelist[i-1];
        //Note--You fall of the end of this array at 24 nodes...don't forget
        test_dfg.nodes[test_dfg.node_count+1]=NULL;
        //Note--He doesn't increment the node_count meaning that there are actually node_count + 1 nodes.
        test_dfg.srcstone = (source_stone_unit*)malloc(sizeof(ss_unit) * MAXSTONES); // I changed this sketchy code 
        test_dfg.numsourcestones = 0;
        EVmaster_register_node_list(test_dfg.dfg_master,&test_dfg.nodes[0]);
        test_dfg.dfg = EVdfg_create(test_dfg.dfg_master);
        ret = 1;
      }
      else
        fprintf(stderr,"Nodelist is empty; failed to create DFG in static mode\n");

    }
    else if(strcmp(mode,"dynamic")==0)
    {
      if(func)
      {
        EVmaster_node_join_handler(test_dfg.dfg_master,func);
        if(ncount > 0)
        {
            test_dfg.node_count = ncount;
            for (i = 0; i <= test_dfg.node_count; ++i)
                test_dfg.nodes[i] = (char*)malloc(15);
            
            test_dfg.nodes[0] = "masternode";
            for(i = 1; i <= ncount; ++i)
            {
                if(strlen(nodelist[i-1]) <= 15)
                {
                    test_dfg.nodes[i] = nodelist[i-1];
                }
                else
                {
                    fprintf(stderr, "Error: length of the node name %s is greater than 15 characters\n", 
                                    nodelist[i-1]);
                    exit(1);
                }
            }
            test_dfg.node_count++;
            test_dfg.nodes[test_dfg.node_count] = NULL;
        }
        test_dfg.dfg = EVdfg_create(test_dfg.dfg_master);
        ret = 2;
      }
      else
        fprintf(stderr,"Node Join Handler Function not valid \n");
    }
  }
  else
    fprintf(stderr,"DFG not initialized correctly, call dfg_init_func first\n");
  return ret;
}
*/

int dfg_create_assign_source_stones_func(char *nodename, char *sourcestone)
{
  int ret = 0, i;

  assert(test_dfg.dfg);

  if(sourcestone!=NULL)
  {
    for(i = 0; strcmp(test_dfg.nodes[i],nodename)!=0 && i<=test_dfg.node_count; ++i);
    if(i<=test_dfg.node_count) {
      test_dfg.srcstone[test_dfg.numsourcestones].src = EVdfg_create_source_stone(test_dfg.dfg, sourcestone);
      EVdfg_assign_node(test_dfg.srcstone[test_dfg.numsourcestones].src, nodename);

      // test_dfg.srcstone[test_dfg.numsourcestones].router = EVdfg_create_stone(test_dfg.dfg, router_action);
      //EVdfg_assign_node(test_dfg.srcstone[test_dfg.numsourcestones].router, nodename);

      test_dfg.srcstone[test_dfg.numsourcestones].sourcename=sourcestone;
      test_dfg.numsourcestones++;

      //test_dfg.srcstone[test_dfg.numsourcestones].port = 0;

      ret = 1;
    }
    else
      fprintf(stderr,"Node isn't on the registered list\n");
  }
  return ret;
}

int dfg_create_assign_link_sink_stones_func(char *nodename, char *handler, int numsources, char **sourcename)
{
  int ret = 0, i,j;
  assert(test_dfg.dfg);
  if(handler!=NULL)
  {
    EVdfg_stone sink;
    for(i=1; strcmp(test_dfg.nodes[i],nodename)!=0 && i<=test_dfg.node_count ; ++i);
    if(i<=test_dfg.node_count) {
      sink = EVdfg_create_sink_stone(test_dfg.dfg, handler);
      EVdfg_assign_node(sink, nodename);
      for(i=0;i<numsources;++i)
      {
        for(j=0; j<test_dfg.numsourcestones; ++j)
        {
            if(strcmp(sourcename[i],test_dfg.srcstone[j].sourcename) == 0)
                 break;
        }
        if(j<test_dfg.node_count) {
/*          if(once == 0)
          {
              EVdfg_link_dest(test_dfg.srcstone[j].src, test_dfg.srcstone[j].router);
              once = 1;
          }
          EVdfg_link_port(test_dfg.srcstone[j].router, test_dfg.srcstone[j].port++, sink);
  */      
          EVdfg_link_dest(test_dfg.srcstone[j].src,sink);
          ret = 1;
        }
        else
          fprintf(stderr,"Source not registered. Failed to establish link. Call dfg_create_assign_source_stone_func first\n");
      }
    }
    else
      fprintf(stderr,"Node isn't on the registered list\n");
  }
  return ret;

}

int dfg_finalize_func_static(void)
{
  int ret = 0;
  if(test_dfg.dfg)
  {
    if(/*dfg_create_assign_source_stones_func(test_dfg.nodes[0],"master_source")*/1)
    {
      EVdfg_realize(test_dfg.dfg);
      test_dfg.test_client = EVclient_assoc_local(test_dfg.cm,test_dfg.nodes[0],test_dfg.dfg_master,NULL,NULL);
      EVclient_ready_wait(test_dfg.test_client);
      if(EVclient_active_sink_count(test_dfg.test_client)==0)
        EVclient_ready_for_shutdown(test_dfg.test_client);

      EVclient_wait_for_shutdown(test_dfg.test_client);

      ret = 1;
    }
    else
      fprintf(stderr,"Couldn't create a stone for master. Check if dfg_create_func has been called first\n");
  }
  else
    fprintf(stderr,"DFG not created correctly. Call dfg_init_func first\n");

  return ret;
}

void dfg_get_master_contact_func(char *retvalue, char* contact_file)
{
  FILE *op;
  op = fopen(contact_file,"r");
  if(op != NULL)
  {
    fscanf(op,"%s",retvalue);
    fclose(op);
  }
  else
  {
    fprintf(stderr, "Could not open master.info file\n");
  }
}

std::string storage_template = "int i;\n\
        static chr_time zero;\n\
        char ** data_ptr = (char *) malloc(sizeof(char*) * (the_size + 1));\n\
        long * file_sizes = (long *) malloc(sizeof(long) * the_size);\n\
        char * first_db_id;\n\
        simple * tmp_ptr;\n\
        tmp_ptr = (simple *) EVdata_simple(0);\n\
        first_db_id = tmp_ptr->db_id;\n\
        simple new_data;\n\
        for(i = 0; i < the_size; ++i)\n\
        {\n\
          simple * tmp_ptr;\n\
          tmp_ptr = (simple *) EVdata_simple(i);\n\
          file_sizes[i] = tmp_ptr->file_buf_len;\n\
          data_ptr[i] = &(tmp_ptr->file_buf[0]);\n\
        }\n\
        data_ptr[the_size] = NULL;\n\
        char * res_data;\n\
        res_data = process_py_store(\"heatDensityMap\", data_ptr, file_sizes, first_db_id, the_size, &(new_data.db_id));\n\
        int data_size = get_data_length();\n\
        new_data.file_path = \"null\";\n\
        new_data.file_buf_len = data_size;\n\
        for(i = 0; i < data_size; ++i)\n\
        {\n\
          new_data.file_buf[i] = res_data[i];\n\
        }\n\
        new_data.exp_id = -1;\n\
        new_data.start.d1 = 0.0;\n\
        new_data.start.d2 = 0.0;\n\
        new_data.start.d3 = 0.0;\n\
        new_data.end.d1 = 0.0;\n\
        new_data.end.d2 = 0.0;\n\
        new_data.end.d3 = 0.0;\n\
        printf(\"This is the db_id: \%s\\n\", new_data.db_id);\n\
        EVsubmit(0, new_data);\n\
        for(i = 0; i < the_size; ++i)\n\
        {\n\
          EVsubmit_simple(0, i);\n\
        }\n\0";

 /* 
        }\n\
        
        
        res_data[0] = \'n\';\n\


          char * temp_ptr = python_func_call(i);\n\
          printf(\"Received back a %s\\n\", temp_ptr);\n\
          EVdiscard_and_submit_full(0, 0);"
*/

EVdfg_stone create_stone(const stone_struct &stone_info)
{
    EVdfg_stone the_stone;
    char * p;
    switch (stone_info.stone_type)
    {
        case SOURCE:
            p = strdup(stone_info.src_sink_handler_name.c_str());
            the_stone = EVdfg_create_source_stone(test_dfg.dfg, p); 
            free(p);
            p = NULL;
            break;
        case SINK:
        case PYTHON:
            p = strdup(stone_info.src_sink_handler_name.c_str());
            the_stone = EVdfg_create_sink_stone(test_dfg.dfg, p);
            free(p);
            p = NULL;
            break;
        case BUCKETROLL:
            p = create_e_rolling_bucket_action_spec(storage_list, strdup(storage_template.c_str()));
            the_stone = EVdfg_create_stone(test_dfg.dfg, p);
            init_stone_size(the_stone, NULL, stone_info.stone_size);
            free(p);
            p = NULL;
            break;
        default:
            the_stone = NULL;
    }

    return the_stone;
}
