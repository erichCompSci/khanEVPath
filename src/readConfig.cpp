#include "readConfig.h"

int config_read_node(const ConfigParser_t & cfg, std::string stone_section, std::string & node_name)
{
    if(!cfg.getValue(stone_section, "node", &node_name))
    {
        fprintf(stderr, "Failure to find node value in %s\n", stone_section.c_str());
        return 0;
    }
    return 1;

}

int config_read_type(const ConfigParser_t & cfg, std::string stone_section, stone_type_t & what_type)
{
    std::string temp;
    if(!cfg.getValue(stone_section, "type", &temp))
    {
        log_err("Failure to find type value in %s", stone_section.c_str());
        return 0;
    }

    if(!temp.compare("source"))
    {
        what_type = SOURCE;
        return 1;
    }
    else if(!temp.compare("sink"))
    {
        what_type = SINK;
        return 1;
    }
    else if(!temp.compare("python"))
    {
        what_type = PYTHON;
        return 1;
    }
    else if(!temp.compare("bucketroll"))
    {
        what_type = BUCKETROLL;
        return 1;
    }
    {
        log_err("Unidentified stone_type of value %s", temp.c_str());
        return 0;
    }
    
    return 0;
}

int config_read_incoming(const ConfigParser_t & cfg, std::string stone_section, std::vector<std::string> & incoming_list)
{
  std::vector<std::string> temp_string_vec;
  if(!cfg.getValue(stone_section, "incoming", &temp_string_vec))
  {
      log_err("Failure to return correct incoming list from %s", stone_section.c_str());
      return 0;
  }

  /*This logic is a hack to give us "stone:stone:stone" config file option*/

  for(int i = 0; i < temp_string_vec.size(); ++i)
  {
    std::string check(temp_string_vec[i]);
    if(check.find(':') == std::string::npos)
    {
      incoming_list.push_back(check);
    }
    else
    {
      std::size_t found_at_first = 0;
      std::size_t found_at_second = 0;
      while(found_at_first != std::string::npos)
      {
        found_at_second = check.find(':', found_at_first);
        if(found_at_second != std::string::npos)
        {
          std::string temp_value = check.substr(found_at_first, found_at_second - found_at_first);
          incoming_list.push_back(temp_value);
          found_at_first = found_at_second + 1;
        }
        else
        {
          std::string temp_value = check.substr(found_at_first);
          incoming_list.push_back(temp_value);
          found_at_first = found_at_second;
        }
      }
    }
  }

  printf("The incoming stones for %s are: ", stone_section.c_str());
  for(int i = 0; i < incoming_list.size(); ++i)
  {
    printf("%s ", incoming_list[i].c_str());
  }
  printf("\n");

  return 1;
}


int config_read_script_name(const ConfigParser_t & cfg, std::string stone_section, std::string & script_name)
{
    if(!cfg.getValue(stone_section, "script", &script_name))
    {
        log_err("Failure to return correct script name from %s", stone_section.c_str());
        return 0;
    }
    return 1;
}

int config_read_method_name(const ConfigParser_t & cfg, std::string stone_section, std::string & method_name)
{
    if(!cfg.getValue(stone_section, "method", &method_name))
    {
        log_err("Failure to return correct method name from %s", stone_section.c_str());
        return 0;
    }
    return 1;
}
