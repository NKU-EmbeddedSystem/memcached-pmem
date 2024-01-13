#include <libmemcached/memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;

int main(){
  memcached_server_st *servers = NULL;
  memcached_st *memc;
  memcached_return rc;
  

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
      string value = "value";
      string part = to_string(i);
      key = key + part;
      value = value + part;
      char *key_c = (char*)(key.c_str());
      char *value_c = (char*)(value.c_str());
      printf("key: %s\n", key_c);
      printf("value: %s\n", value_c);
      rc = memcached_set(memc, key_c, strlen(key_c), value_c, strlen(value_c), (time_t)0, (uint32_t)0);
      if (rc == MEMCACHED_SUCCESS)
        fprintf(stderr, "Key stored successfully\n");
      else
        fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc));
      }
  

  

  return 0;
}
