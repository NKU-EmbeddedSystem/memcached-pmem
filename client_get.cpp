#include <libmemcached/memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;

int main(){
  //memcached_servers_parse (char *server_strings);
  memcached_server_st *servers = NULL;
  memcached_st *memc;
  memcached_return rc;
  char *key = "keystring";
  char *value = "keyvalue";

  char *retrieved_value;
  size_t value_length;
  uint32_t flags;

  memc = memcached_create(NULL);
  servers = memcached_server_list_append(servers, "localhost", 11211, &rc);
  rc = memcached_server_push(memc, servers);

  if (rc == MEMCACHED_SUCCESS)
    fprintf(stderr, "Added server successfully\n");
  else
    fprintf(stderr, "Couldn't add server: %s\n", memcached_strerror(memc, rc));


  int count = 10000; 
  for(int i = 0; i < count; i++){
      string key = "key";
      string part = to_string(i);
      key = key + part;
      retrieved_value = memcached_get(memc, key.c_str(), strlen(key.c_str()), &value_length, &flags, &rc);
      string get_value(retrieved_value);
      string value = "value";
      value = value + part;
      std::cout<<"returned value: "<<get_value<<" "<<"true value: "<<value<<std::endl;

      if (rc == MEMCACHED_SUCCESS)
        fprintf(stderr, "Key stored successfully\n");
      else
        fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc));
  }
  
  return 0;
}
