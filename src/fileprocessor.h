
#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H
#include <vector>

void extract_attr_init(std::string, int, std::string);
void process_transducers(std::string);
void process_statistics(int, std::string, std::string, std::string);
void process_analytics_pipeline(std::string);
void process_file(std::string, std::string, std::string);
int process_python_code(std::string py_script, std::string py_function, std::string filename, 
                        char * data_location, int file_length, char * db_id);


char * process_py_store(char * method_name, char** data, long * data_size, char * first_id, 
                              int size, char ** db_id);

int get_data_length();

int init_python_processing(std::string script_name);
void cleanup_python();
void initialize_attrs_for_data_types(const std::vector<std::string> & types);
#endif
