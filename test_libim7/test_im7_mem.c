#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "ReadIMX.h"
#include "ReadIM7.h"


void print_buff_scale_type(const char * identifier, BufferScaleType * scale)
{
  printf("\n");
  printf("BufferScaleType %s\n", identifier);
  printf("factor: %f\n", scale->factor);
  printf("offset: %f\n", scale->offset);
  printf("description: %s\n", scale->description);
  printf("unit: %s\n", scale->unit);
}

void print_im7_buffer(const char * identifier, BufferType * buffer)
{

  printf("Buffer %s\n", identifier);
  printf("isFloat: %d\n", buffer->isFloat);
  printf("nx: %d\tny: %d\tnz: %d\tnf: %d\n", buffer->nx, buffer->ny, buffer->nz, buffer->nf);
  printf("Total Lines: %d\n", buffer->totalLines);
  printf("Vector Grid: %d\n", buffer->vectorGrid);
  printf("Image Sub Type: %d\n", buffer->image_sub_type);
  print_buff_scale_type("scaleX", &(buffer->scaleX));
  print_buff_scale_type("scaleY", &(buffer->scaleY));
  print_buff_scale_type("scaleI", &(buffer->scaleI));

}

void print_attr_list(const char * identifier, AttributeList * attrs)
{
  printf("Attrlist for %s\n", identifier);
  while(attrs)
  {
    printf("Attr Name: %s\n", attrs->name);
    printf("Attr Value: %s\n", attrs->value);
    attrs = attrs->next;
  }
}
  


int main(int argc, char** argv)
{
  BufferType test_file, test_mem;
  AttributeList * attr_file = (AttributeList *) malloc(sizeof(AttributeList));
  AttributeList * attr_mem = (AttributeList *) malloc(sizeof(AttributeList));

  int fdin;
  struct stat statbuf;
  int size_of_file = -1;
  char * file_address = NULL;

  if ((fdin = open ("B00106.im7", O_RDONLY)) < 0)
  {
    fprintf(stderr, "Error: can't open im7 file for reading\n");
    exit(1);
  }

  if (fstat (fdin,&statbuf) < 0)
  {
    fprintf(stderr, "Error: can't read im7 file stat data\n");
    exit(1);
  }

  size_of_file = statbuf.st_size;


  if ((file_address = (char*)mmap (0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fdin, 0))
      == (caddr_t) -1)
    fprintf(stderr, "mmap error for file\n");

  
  
  int test = ReadIM7 ("B00106.im7", &test_file, &attr_file);
  int test2 = ReadIM7_MEM ( file_address, &test_mem, &attr_mem, size_of_file);
  print_im7_buffer("file_test", &test_file);
  printf("\n");
  print_im7_buffer("mem_test", &test_mem);

  print_attr_list("attr_file", attr_file);
  printf("\n");
  print_attr_list("attr_mem", attr_mem);

  return 1;
}
