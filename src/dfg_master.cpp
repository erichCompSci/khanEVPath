#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <glob.h>
#include <set>
#include <unistd.h>

#include "dfg_functions.h"
#include "synchron.h"
#include "fileprocessor.h"
#include "cfgparser.h"
#include "log.h"
#include "khan_ffs.h"
#include "khan.h"
#include "measurements.h"
#include "database.h"
#include "readConfig.h"

#define E_METRIC

extern struct dfg_unit test_dfg;

std::vector<stone_struct> stone_holder;

std::vector < std::string > servers;
std::vector < std::string > server_ids;

std::string this_server;
std::string this_server_id;

EVsource sync_source;

/* Init the database values.  Return 0 on failure, 1 on success */
int init_database_values(simple_rec & data)
{
    std::string original_fp;
    std::string shared_fn;
    char exp_id_str[10];

    if (sprintf(exp_id_str, "%d", data.exp_id) <= 0)
    {
        log_err("Copy of file id string failed.");
        return 0;
    }

    original_fp = data.file_path; 
    shared_fn = "/dev/shm/" + original_fp.substr(54, strlen(data.file_path) - 54);

    log_info("Extract attributed for %s", shared_fn.c_str());
    std::string ext = strrchr(original_fp.c_str(),'.')+1;
    std::string filename=strrchr(original_fp.c_str(),'/')+1;

    std::string fileid = database_setval("null","name",filename);
    if(!fileid.compare("fail"))
    {
        log_err("Failure to set filename value in database");
        return 0;
    }
    data.db_id = strdup(fileid.c_str());
    //test1 and test2 are non-descript here, we need to ask Akash about there purpose
    database_setval(fileid,"ext",ext);
    database_setval(fileid,"server","test1");
    database_setval(fileid,"location","test2");
    database_setval(fileid,"file_path", original_fp);
    database_setval(fileid,"experiment_id", exp_id_str);
    return 1;
}

int not_synced;

#ifdef E_METRIC
static int
synchro_handler(CManager cm, void * vevent, void * client_data, attr_list attrs)
{
  static chr_time offset_received;
  static chr_time empty;

  ping_pong_ptr event = (ping_pong_ptr) vevent;
  chr_get_time(&(event->t2_p));

  if(!memcmp(&offset_received, &empty, sizeof(chr_time)))
    chr_get_time(&offset_received);
  
  if(!memcmp(&(event->offset2), &(offset_received), sizeof(chr_time)))
  {
    not_synced = 0;
    return 1;
  }
  
  chr_time diff_time1, diff_time2;
  chr_timer_diff(&diff_time1, &(event->t1_p), &(event->t1));
  chr_timer_diff(&diff_time2, &diff_time1, &(event->t2_p));
  chr_timer_sum(&(event->offset2), &diff_time2, &(event->t2));


  double time;
  time = chr_time_to_nanosecs(&(event->offset2));
  time = time / 2.0;
  printf("The time difference in nanoseconds is: %f\n", time);

  time = chr_time_to_microsecs(&(event->offset2));
  time = time / 2.0;
  printf("The time difference in microseconds is: %f\n", time);

  time = chr_time_to_millisecs(&(event->offset2));
  time = time / 2.0;
  printf("The time difference in milleseconds is: %f\n", time);

  offset_received = event->offset2;
  //memcpy(&offset_received, &(event->offset2), sizeof(chr_time));

  EVsubmit(sync_source, event, NULL);



  return 1;
}

#endif

void usage()
{
  printf("Usage:\n ./dfg_master configfile \n");
}


static int setupBucketStone(const ConfigParser_t & cfg, std::string stone_name, stone_struct & bucket_struct, 
                                stone_struct & size_stone_struct, stone_type_t which_bucket)
{


    bucket_struct.stone_name = stone_name;
    bucket_struct.stone_type = which_bucket;
    std::string size_incoming = stone_name + "_size";

    size_stone_struct.node_name = "master_node";
    size_stone_struct.stone_name = size_incoming;
    size_stone_struct.src_sink_handler_name = size_stone_struct.stone_name + "_" + size_stone_struct.node_name;
    size_stone_struct.stone_type = SOURCE;

    if(!config_read_incoming(cfg, stone_name, bucket_struct.incoming_stones))
    {
      log_err("Error reading incoming stones");
      return 0; 
    }
    bucket_struct.incoming_stones.push_back(size_incoming);

    if(!config_read_stone_size(cfg, stone_name, &(bucket_struct.stone_size)))
    {
      log_err("Error reading bucket stone size");
      return 0;
    }

    return 1;
}


//  Does making this static do, what I think its doing or not because its C++?
static int setupPythonStones(const ConfigParser_t & cfg, std::string prev_name, 
                        stone_struct & py_source, stone_struct & py_sink)
{

    /* Set the node_name for the py_sink from the py_source structure */
    py_sink.node_name = py_source.node_name;

    /* Set the stone types of the respective PYTHON stones */
    py_source.stone_type = SOURCE;
    py_sink.stone_type = PYTHON;
     
    /* Set the stone names to be unique by adding _py_src and _py_sink to the stone names */
    py_source.stone_name = prev_name + "_py_src";
    py_sink.stone_name = prev_name + "_py_sink";

    /* Set up the unique handler name for each stone */
    py_source.src_sink_handler_name = py_source.stone_name + "_" + py_source.node_name;
    py_sink.src_sink_handler_name = py_sink.stone_name + "_" + py_source.node_name;

    /* Get the incoming stones into the py_sink stone */ 
    if(!config_read_incoming(cfg, prev_name, py_sink.incoming_stones))
    {
      log_err("Error reading incoming stones");
      return 0; 
    }

    return 1;

}



void JoinHandlerFunc(EVmaster master, char * identifier, void * cur_unused1, void * cur_unused2)
{
    /*This code does not handle code in the stones yet */
    //TODO: Make it so we can have stones actually do stuff
    static int num_of_nodes = 0;
    //We need this for the python lists
    //static std::vector<std::string> node_names;
    ++num_of_nodes;
    if(num_of_nodes < test_dfg.node_count)
    {
        printf("Received node %s\n", identifier);
        EVmaster_assign_canonical_name(master, identifier, identifier);
        return;
    }
    printf("Received node %s\n", identifier);
    EVmaster_assign_canonical_name(master, identifier, identifier);
    EVdfg_stone *stones = (EVdfg_stone*)malloc(sizeof(EVdfg_stone) * stone_holder.size());
    test_dfg.dfg = EVdfg_create(test_dfg.dfg_master);

    for(unsigned i = 0; i < stone_holder.size(); ++i)
    {
        /*Getting the stones correctly set up when they are not sources and sinks will go here*/

    }

    for(unsigned i = 0; i < stone_holder.size(); ++i)
    {
        stones[i] = create_stone(stone_holder[i]); 
        if(!stones[i])
        {
            fprintf(stderr, "Error: stone not created in handler function\n");
            exit(1);
        }
        char * p = strdup(stone_holder[i].node_name.c_str());
        EVdfg_assign_node(stones[i], p);
        free(p);

    }
    

    for(unsigned i = 0; i < stone_holder.size(); ++i)
    {
        for(unsigned j = 0; j < stone_holder[i].incoming_stones.size(); ++j)
        {
            std::string temp_string1 = stone_holder[i].incoming_stones[j];
            std::string temp_string2 = temp_string1 + "_py_src";
            for(unsigned k = 0; k < stone_holder.size(); ++k)
            {
                if((!temp_string1.compare(stone_holder[k].stone_name)) ||
                   (!temp_string2.compare(stone_holder[k].stone_name)))
                {
                  if(stone_holder[k].stone_type == SOURCE || stone_holder[k].stone_type == PYTHON)
                  {
                    EVdfg_link_dest(stones[k], stones[i]);
                    break;
                  }
                  else if(stone_holder[k].stone_type == BUCKETROLL)
                  {
                    EVdfg_link_port(stones[k], 0, stones[i]);
                    break;
                  }
                  else
                  {
                    fprintf(stderr, "The type of the stone is: %d\n", stone_holder[k].stone_type);
                    fprintf(stderr, "Error: This stone type is not currently supported!\n");
                    exit(1);
                  }
                }
            }
        }
    }
    /* Clock Synchronization stuff */

#ifdef E_METRIC

    EVdfg_stone sync_src, sync_sink, sync_src2, sync_sink2;

    sync_src = EVdfg_create_source_stone(test_dfg.dfg, "sync source");
    EVdfg_assign_node(sync_src, "master_node");

    sync_sink = EVdfg_create_sink_stone(test_dfg.dfg, "sync_handler2");
    EVdfg_assign_node(sync_sink, "new");

    sync_src2 = EVdfg_create_source_stone(test_dfg.dfg, "sync source2");
    EVdfg_assign_node(sync_src2, "new");

    sync_sink2 = EVdfg_create_sink_stone(test_dfg.dfg, "sync_handler");
    EVdfg_assign_node(sync_sink2, "master_node");

    EVdfg_link_dest(sync_src, sync_sink);
    EVdfg_link_dest(sync_src2, sync_sink2);

#endif
        
    // Realize the dfg hopefully
    EVdfg_realize(test_dfg.dfg);

}

int main(int argc, char *argv[])
{
  /*
   * 		We want the dfg creator file to be able to read in a list of nodes, a list of stones per node and links between stones.
   * 	*/
  if(argc != 2)
    usage();
  else
  {
    not_synced = 1;
    std::vector<std::string> type_names;
    std::string config_file_name = argv[1]; 
    /*Need this for some hacky bookkeeping of storage stone size stuff */
    int does_bucket_exist = 0;
    std::string stor_handler_name;

    ConfigParser_t cfg;
    if(cfg.readFile(config_file_name))
    {
      printf("Error: Cannot open config file %s", config_file_name.c_str());
      return 1;
    }
  
    /*Read the file information into the appropriate data structures for later use*/
    std::vector<std::string> stone_sections = cfg.getSections();
    std::vector<std::string> temp_node_name_list;

    for(std::vector<std::string>::iterator I = stone_sections.begin(), E = stone_sections.end(); I != E; ++I)
    {
      stone_struct new_stone_struct;
      stone_struct pot_python_struct;

      /*Get the node name for the stone and in doing so, 
        bookkeep which node names that we have already seen*/
      std::string which_node;
      if(!config_read_node(cfg, *I, which_node))
      {
          fprintf(stderr, "Failure to read node from config\n");
          exit(1);
      }

      unsigned int temp_i = 0;
      for(; temp_i < temp_node_name_list.size(); ++temp_i)
      {
        if(!which_node.compare(temp_node_name_list[temp_i]))
          break;
      }
      if(temp_i == temp_node_name_list.size())
      {
        temp_node_name_list.push_back(which_node);
        ++test_dfg.node_count;
      }
      new_stone_struct.node_name = which_node;

      /*Read the stone type into the enum value...*/
      if(!config_read_type(cfg, *I, new_stone_struct.stone_type))
      {
        fprintf(stderr, "Error: reading config type of stone failed\n");
        exit(1);
      }

      /*Store the section name as the stone name
        This must be a unique value for every stone*/
      if(new_stone_struct.stone_type == PYTHON)
      {
        if(!setupPythonStones(cfg, *I, new_stone_struct, pot_python_struct))
        {
            log_err("Error: failed to setup python stones");
            exit(1);
        }
        stone_holder.push_back(new_stone_struct);
        stone_holder.push_back(pot_python_struct);
      }
      else if(new_stone_struct.stone_type == BUCKETROLL)
      {
        
        stone_struct size_stone_struct;
        if(!setupBucketStone(cfg, *I, new_stone_struct, size_stone_struct, BUCKETROLL))
        {
            log_err("Error: failed to setup bucket stone");
            exit(1);
        }
        stor_handler_name = size_stone_struct.src_sink_handler_name;
        stone_holder.push_back(new_stone_struct);
        stone_holder.push_back(size_stone_struct);
        does_bucket_exist = 1;
        
      }
      else
      {
        new_stone_struct.stone_name = *I;

      /*Construct a unique handler for each source and sink node
        For now the nodes will have to read the config file*/
    

        std::string unique_handler_name = new_stone_struct.stone_name + "_" + new_stone_struct.node_name;
        new_stone_struct.src_sink_handler_name = unique_handler_name;
    
        /*Read the incoming stones to connect to from the config file*/
        if(!config_read_incoming(cfg, *I, new_stone_struct.incoming_stones))
        {
          log_err("Error reading incoming stones");
          exit(1);
        }

        /*TODO:Read the actual code, probably going to need to 
          have a seperate code file for the COD code*/
        

        //Push-back here
        stone_holder.push_back(new_stone_struct);

        /*  
        printf("Testing: node_name := %s\n", new_stone_struct.node_name.c_str());
        printf("Testing: stone_name := %s\n", new_stone_struct.stone_name.c_str());
        printf("Testing: handler_name := %s\n", new_stone_struct.src_sink_handler_name.c_str());
        printf("Testing: stone_type := %d\n", new_stone_struct.stone_type);
        printf("Testing: node_count := %d\n", test_dfg.node_count);
        for(unsigned int i = 0; i < new_stone_struct.incoming_stones.size(); ++i)
        {
            printf("Testing: incoming_stone for %s: %s\n", new_stone_struct.stone_name.c_str(),
                                                          new_stone_struct.incoming_stones[i].c_str());
        }
        printf("Testing: code_type := %d\n", new_stone_struct.code_type);
        */ 
    

      }
    }
    // Master node entry stone added to the end of the vector
    stone_struct entry_stone_struct;
    entry_stone_struct.node_name = "master_node";
    entry_stone_struct.stone_name = "entry";
    entry_stone_struct.src_sink_handler_name = entry_stone_struct.stone_name + "_" + entry_stone_struct.node_name;
    entry_stone_struct.stone_type = SOURCE;
    stone_holder.push_back(entry_stone_struct);

    ++test_dfg.node_count;


    if(dfg_init_func())
    {
      EVmaster_node_join_handler(test_dfg.dfg_master, JoinHandlerFunc);
      //printf("DFG handler read...\n");
    }

    /*EVsource * master_list_sources = (EVsource *) malloc(sizeof(EVsource) * temp_node_name_list.size());
    
    for(unsigned int i = 0; i < temp_node_name_list.size(); ++i)
    {
        //Place the list stones into the list
        stone_struct py_master_list_source;
        py_master_list_source.node_name = "master_node";
        py_master_list_source.stone_name = "to_" + temp_node_name_list[i];
        py_master_list_source.src_sink_handler_name = py_master_list_source.stone_name + "_" + py_master_list_source.node_name;
        py_master_list_source.stone_type = SOURCE;
        stone_holder.push_back(py_master_list_source);

        //Register the handler
        master_list_sources[i] = EVcreate_submit_handle(test_dfg.cm, -1, python_format_list);
        char * perm_ptr = strdup(py_master_list_source.src_sink_handler_name.c_str());
        source_capabilities = EVclient_register_source(perm_ptr, master_list_sources[i]);
        
        stone_struct py_master_list_sink;
        py_master_list_sink.node_name = temp_node_name_list[i];
        py_master_list_sink.stone_name = "python_list";
        py_master_list_sink.src_sink_handler_name = py_master_list_sink.stone_name + "_" + py_master_list_sink.node_name;
        py_master_list_sink.incoming_stones.push_back(py_master_list_source.stone_name);
        py_master_list_sink.stone_type = SINK;
        stone_holder.push_back(py_master_list_sink);
    }


    */
    
    EVclient_sources source_capabilities;
    EVclient_sinks sink_capabilities = NULL;
    EVsource source_handle, bucket_size_handle;
    EVclient master_client;

#ifdef E_METRIC
    sync_source = EVcreate_submit_handle(test_dfg.cm, -1, ping_pong_format_list);
    source_capabilities = EVclient_register_source("sync source", sync_source);

	  sink_capabilities = EVclient_register_sink_handler(test_dfg.cm, "sync_handler", 
                        ping_pong_format_list, (EVSimpleHandlerFunc) synchro_handler, NULL);
#endif

    if(does_bucket_exist)
    {
      /*Don't try to memclean this, it won't go well for you */
      bucket_size_handle = EVcreate_submit_handle(test_dfg.cm, -1, size_message_format_list);
      source_capabilities = EVclient_register_source(strdup(stor_handler_name.c_str()), bucket_size_handle);
    }


    source_handle = EVcreate_submit_handle(test_dfg.cm, -1, simple_format_list);
    char * temp_char_ptr = strdup(entry_stone_struct.src_sink_handler_name.c_str());
    source_capabilities = EVclient_register_source(temp_char_ptr, source_handle);
    temp_char_ptr = NULL;

    master_client = EVclient_assoc_local(test_dfg.cm, "master_node", test_dfg.dfg_master, source_capabilities, sink_capabilities);

    if (EVclient_ready_wait(master_client) != 1) {
      /* initialization failed! */
      log_err("EVclient_ready_wait: FAILED!");
      exit(1);
    }

#ifdef E_METRIC

    ping_pong_ptr new_ping = (ping_pong_ptr) calloc(1, sizeof(ping_pong));

    chr_get_time(&(new_ping->t1));
    EVsubmit(sync_source, new_ping, NULL);

    while(not_synced)
    {
      CMpoll_network(test_dfg.cm);
    }

#endif

    /*
    python_list test_py_list;
    test_py_list.dynamic_size = 4;
    test_py_list.ordered_method_list[0] =  0;
    test_py_list.ordered_method_list[1] =  1;
    test_py_list.ordered_method_list[2] = 2;
    test_py_list.ordered_method_list[3] = 3;
    for(int i = 0; i < temp_node_name_list.size(); ++i)
    {
        EVsubmit(master_list_sources[i], &test_py_list, NULL);
        printf("Submitted a python list...\n");

    }
    */

    /* Setting up the options and stuff, before sending anything */
    std::string store_filename="stores.txt"; /* Default */
    int port = 6379;

    int opt;
    std::string host = "localhost";
    while ((opt = getopt (argc, argv, "p:s:h:")) != -1)
    {
      switch (opt)
      {
        case 'p':
          port = atoi(optarg);
          break;

        case 's':
          store_filename = optarg;
          break;

        case 'h':
          host = optarg;
          break;
      }
    }

    FILE* stores = fopen(store_filename.c_str(), "r");
    char buffer[100];
    char buffer2[100];
    fscanf(stores, "%s\n", buffer);
    this_server_id = buffer;
    while(fscanf(stores, "%s %s\n", buffer, buffer2)!=EOF) {
      servers.push_back(buffer);
      server_ids.push_back(buffer2);
      if(this_server_id == buffer2) {
        this_server = buffer;
      } 
    }
    fclose(stores);

    /* Initialization of Khan */
    arg_struct khan_args;
    khan_args.mnt_dir = argv[1];
    khan_args.servers = servers;
    khan_args.server_ids = server_ids;
    khan_args.port = port;
    khan_args.host = host;

    initializing_khan((void*)&khan_args);
    log_info("Initialized Khan");

    // Hardcode the im7 type as the only type for now
    type_names.push_back("im7");
    initialize_attrs_for_data_types(type_names);
    
    /* In my experience non-initalized data can give EVPath and CoD issues
    *  sometimes, so I don't risk it anymore. */
    static simple_rec empty_data;
    /* Here's where we set up and send the data */
    simple_rec data = empty_data;

    std::string pattern = servers[0] + "/*";
    glob_t files;

    int fdin;
    struct stat statbuf;
    std::set<std::string> experiments;
    int change_size = 2;

    for(int count = 18; count > 0; count--) {

      glob((pattern +".im7").c_str(), 0, NULL, &files);        

      log_info("Globbing with pattern: %s.im7", pattern.c_str());
      for(unsigned j=0; j<files.gl_pathc; j++) {
        if(does_bucket_exist && (j % 2) == 0 && j != 0)
        {
          size_message msg;
          msg.size = change_size++;
          EVsubmit(bucket_size_handle, &msg, NULL);
        }

        std::string filepath = files.gl_pathv[j];
        log_info("File Path: %s", filepath.c_str());

        std::string exp_dir = filepath.substr(0, filepath.size() - 10);
        experiments.insert(exp_dir);
        data.exp_id = (int)experiments.size();

        data.file_path = strdup(filepath.c_str());

        if ((fdin = open (filepath.c_str(), O_RDONLY)) < 0)
          perror ("can't open %s for reading"); 

        if (fstat (fdin,&statbuf) < 0)
          perror ("fstat error");

        data.file_buf_len = statbuf.st_size;

        log_info("Size: %zu", data.file_buf_len);

        if ((data.file_buf = (char*)mmap (0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fdin, 0))
            == (caddr_t) -1)
          perror ("mmap error for input");

        /* Adding in code here to ensure that the event gets setup in Redis correctly before it gets sent out */
        //TODO:Delete the bottom two lines if I've compiled successfully without them
        //std::string original_fp (data.file_path);
        //std::string shared_fn = "/dev/shm/" + original_fp.substr(49, strlen(data.file_path) - 49);
        if(!init_database_values(data))
        {
            log_err("Error in initializing database values for %s", data.file_path);
            exit(1);
        }
#ifdef E_METRIC
        chr_get_time(&(data.start));
#endif


        EVsubmit(source_handle, &data, NULL);

        if (munmap(data.file_buf, statbuf.st_size) == -1) {
          perror("Error un-mmapping the file");
        }

        /*Free Memory*/
        close(fdin);
        free(data.file_path);
      }
      pattern += "/*";
    }
    log_info("Cleanup data structures");
    /* Cleanup */
    globfree(&files);

    //log_info("Shutdown evdfg");

    CMrun_network(test_dfg.cm);     
  }  
        
  return 0;
}
