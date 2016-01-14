#include "Python.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "evpath.h"
#include "ev_dfg.h"
#include "dfg_functions.h"
#include "khan_ffs.h"
#include "khan.h"
#include "readConfig.h"
#include "fileprocessor.h"

#define MAX_SOURCES 20

typedef struct _simple_stone_holder
{
    std::string name;
    std::string py_file;
    std::string py_method;
    std::vector<std::string> incoming;

} simp_stone, * simp_stone_ptr;

//This structure is going to store the 
//relevant details for the python sink handlers
typedef struct _python_sink_handler
{
  std::string py_file;
  std::string py_method;
  EVsource source_handle;

} python_sink_handler, * python_sink_handler_ptr;


EVclient test_client;
python_sink_handler sink_details[MAX_SOURCES];
std::vector < std::string > servers;
std::vector < std::string > server_ids;
std::vector<std::pair<std::string, std::string> > python_master_list;

int for_comparison;
int old_comparison;
int current_value;


/* Used for creating the recursive subdirectories */ 
static void _mkdir(const char *dir) {
  char tmp[1000];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp),"%s",dir);
  len = strlen(tmp);
  if(tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for(p = tmp + 1; *p; p++)
    if(*p == '/') {
      *p = 0;
      mkdir(tmp, S_IRWXU);
      *p = '/';
    }
  mkdir(tmp, S_IRWXU);
}
/*The following code will eventually make it's way into a separate file hopefully...*/
char * call_pyfunc(void **


/* Set's up a shared memory location for the python files
   to pull their information from */
void file_receive(simple_rec_ptr event, python_sink_handler_ptr python_info){

  
  if(event) {
    log_info("file_path %s", event->file_path);
    log_info("file_buf_len %ld", event->file_buf_len);
  }
  
  // FIXME:Need to make a more dynamic way of doing this, rather than hard
  // coding this in the future. This has to be done for python code I think.
  std::string filepath (event->file_path);
  //49 is the length of the server name, the path name up to "data"
  //10 is the length of the im7 file name
  std::string dir_name = filepath.substr(49, strlen(event->file_path) - 59);

  log_info("Dir name %s", dir_name.c_str());

  char * data_location = event->file_buf;
  int file_length = event->file_buf_len;
  char * database_id = event->db_id;
  // Process attribute for python
  process_python_code(python_info->py_file, python_info->py_method, filepath, data_location, file_length, database_id);

}

static int 
python_general_handler(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
    log_info("Received a python event");
    simple_rec_ptr event = (_simple_rec*)vevent;
    python_sink_handler_ptr v_client_data = (python_sink_handler_ptr) client_data;


    //current_value = event->meta_compare_py;
    /* FIXME: Maybe this is fixed, but wait until it runs....*/ 
    if(EVclient_source_active(v_client_data->source_handle))
    {
        printf("Sending the message on...\n");
        //Potentially need to do some fancy footwork with for_comparison here to 
        //prevent a subtle data race?

        EVsubmit(v_client_data->source_handle, event, NULL);
    }
    else
        log_info("No active client source registered");

    file_receive(event, v_client_data);

    return 1;
}

static int
general_handler(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
    printf("Received an event\n");
    return 1;
}




char * python_func_call(int x)
{
  char * temp = (char *) malloc(sizeof(char) * 2);

  printf("The number recieved is %d\n", x);
  return itoa(x, temp, 10);

}




int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: dfg_general_client config_file_name node_name\n");
        return 1;
    }


    cod_extern_entry externs[] = { 
      {"python_storage_func", (void *) (long) python_func_call},
      {NULL, NULL}
    };

    char extern_string[] = "char * python_func_call(int x);\0\0";

    //for_comparison = 0;
    servers.push_back("nothing");
    server_ids.push_back("nothing");

    std::string config_file_name = argv[1];
    std::string client_node_name = argv[2];

    CManager cm;
    EVclient_sinks sink_capabilities = NULL;
    EVclient_sources source_capabilities = NULL;
    cm = CManager_create();
    EVadd_standard_routines(cm, extern_string, externs);
    CMlisten(cm);

    char master_address[200];
    dfg_get_master_contact_func(master_address,"master.info");

    /*Read the config file*/
    ConfigParser_t cfg_slave;
    if(cfg_slave.readFile(config_file_name))
    {
      printf("Error: Cannot open config file %s", config_file_name.c_str());
      return 1;
    }


    /* Search for source, sink, and python stones for this node and distinguish them*/
    std::vector<std::string> stone_names = cfg_slave.getSections();
    std::vector<std::string> handler_names;
    unsigned int num_of_sources = 0;
    unsigned int num_py_stones = 0;
     
    /*I'm going to hijack the first python instance and store the python 
      file name in a variable here.  This is not pretty and very hacky,
      but I'm on a deadline so I'll come back and change it later if I 
      have to...I also argue that this was always the functional case
      it just wasn't explicit*/
    std::string python_script_file_name;
    int first_python_stone_found = 0;

    for(std::vector<std::string>::iterator I = stone_names.begin(), E = stone_names.end(); I != E; ++I)
    {
        log_info("Stone: %s", I->c_str());
        std::string temp_node_name;
        stone_type_t temp_type;
        std::string temp_handler_name;

        /* Check for stones that are relevant to our node for registration purposes */
        if(!config_read_node(cfg_slave, *I, temp_node_name))
        {
            log_err("Could not read node name for: %s", (*I).c_str());
            exit(1);
        }

        if(client_node_name.compare(temp_node_name))
            continue;

        if(!config_read_type(cfg_slave, *I, temp_type))
        {
            log_err("Could not read node type for: %s", (*I).c_str());
            exit(1);
        }

        if(temp_type == SOURCE)
        {
          fprintf(stderr, "Error: this currently doesn't support source stones in the middle of the pipeline!\n");
          exit(1);
        }


        /* If type is a python type, create a unique sink and source handler name
           Otherwise, just do the normal naming convention */ 
        if(temp_type == PYTHON)
        {
            // Need the python stone to turn into two stone names *I + "py_src"
            // This check may be buggy...as it's max_sources not max_pys
            if(num_of_sources > MAX_SOURCES)
            {
                log_err("Maximum number of python stones per node is: %d", MAX_SOURCES);
                exit(1);
            }

            std::string temp_handler_name_sink = (*I) + "_py_sink_" + temp_node_name;
            std::string temp_handler_name_src = (*I) + "_py_src_" + temp_node_name;

            sink_details[num_py_stones].source_handle = EVcreate_submit_handle(cm, -1, simple_format_list);

            if(!config_read_script_name(cfg_slave, *I, sink_details[num_py_stones].py_file))
            {
                log_err("Could not read script file name for %s", (*I).c_str());
                exit(1);
            }
            
            /*This is where I'm doing the hacky bookkeeping*/
            if(!first_python_stone_found)
            {
              python_script_file_name = sink_details[num_py_stones].py_file;
              first_python_stone_found = 1;
            }
            else if(sink_details[num_py_stones].py_file.compare(python_script_file_name))
            {
              log_err("A process is only allowed to have a single Python Script per \"node\"\n"
                      "The two filenames are: %s and %s", sink_details[num_py_stones].py_file.c_str(), python_script_file_name.c_str());
              exit(1);
            }


            if(!config_read_method_name(cfg_slave, *I, sink_details[num_py_stones].py_method))
            {
                log_err("Could not read method name for %s", (*I).c_str());
                exit(1);
            }
            
            char * clean_up_src = strdup(temp_handler_name_src.c_str());
            char * clean_up_sink = strdup(temp_handler_name_sink.c_str());

            source_capabilities = EVclient_register_source(clean_up_src, sink_details[num_py_stones].source_handle);
	          sink_capabilities = EVclient_register_sink_handler(cm, clean_up_sink, simple_format_list, 
                                                                  (EVSimpleHandlerFunc) python_general_handler, 
                                                                  &(sink_details[num_py_stones])); 

            ++num_py_stones;
          }

          if(temp_type == SINK)
          {
            temp_handler_name = (*I) + "_" + temp_node_name;
            char * clean_up_name = strdup(temp_handler_name.c_str());
            sink_capabilities = EVclient_register_sink_handler(cm, clean_up_name, simple_format_list,
                                                                (EVSimpleHandlerFunc) general_handler , NULL);
          }

    }

            
    /*  Associate the client */
    char * temp_ptr = strdup(client_node_name.c_str());
    test_client = EVclient_assoc(cm, temp_ptr, master_address, source_capabilities, sink_capabilities);
    free(temp_ptr);
    temp_ptr = NULL;


    /* Set the python to the path for later processing */
    Py_SetProgramName(argv[2]);  /* optional but recommended */
    Py_Initialize();
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    char cwd[1024];
    std::string py = "/PyScripts";
    std::string pyscripts_path = strdup(getcwd(cwd, sizeof(cwd))) + py;
    PyList_Append(path, PyString_FromString(pyscripts_path.c_str()));
    PySys_SetObject("path", path);

    /*Set up connection to redis */
    std::string hostname = "localhost";
    int port = 6379;
    init_database_from_client(hostname, port);

    /*Initialize python interpreter always (no real need not to yet)*/
    init_python_processing(python_script_file_name);

	if (EVclient_ready_wait(test_client) != 1) {
	/* dfg initialization failed! */
	exit(1);
    }

    

    /*! [Shutdown code] */
	CMrun_network(cm);

	return 1;

}

