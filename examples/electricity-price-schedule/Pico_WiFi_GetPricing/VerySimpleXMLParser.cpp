#include <Arduino.h>
#include "VerySimpleXMLParser.h"

char *xml_ptr;
unsigned int xml_chars_left;

void xml_buffer(const char *buffer) {

  xml_ptr = (char *)buffer;
  xml_chars_left = (unsigned int)strlen(xml_ptr);
}

int xml_goto_char(char c) {
  
  do {

    xml_ptr++;
    xml_chars_left--;

    if(xml_chars_left == 0) {
        return(-1);
      }

  } while(c != *xml_ptr);

  // Serial.printf("[XML] found %c (%u)\r\n", *xml_ptr, xml_chars_left);

  xml_ptr++;
  xml_chars_left--;

  return(xml_chars_left);
}

int xml_goto_tag(const char *tag) {

  int i;

  do {
  
    if(xml_goto_char('<') < 0) {
      return(-1);
    }

    i = 0;
    
    while((tag[i] == *xml_ptr)) {
      
      i++;
      xml_ptr++;
      xml_chars_left--;

      if(xml_chars_left == 0) {
        return(-1);
      }
    }
  } while(tag[i] != 0);

  if(*xml_ptr != '>') {
    if(xml_goto_char('>') < 0) {
      return(-1);
    }
  }

  xml_ptr++;
  xml_chars_left--;

  return(xml_chars_left);
}

int xml_next_tag(char *tag, size_t size) {

  do {
    if(xml_goto_char('<') < 0) {
        *tag=0;
        return(-1);
    }
  } while(*xml_ptr == '/');

  // copy at most size-1 characters, never past the end of the received data
  size_t i = 0;

  while((i + 1 < size) && (i < xml_chars_left) && (xml_ptr[i] != '>') && (xml_ptr[i] != 0)) {
    tag[i] = xml_ptr[i];
    i++;
  }

  tag[i] = 0;

  return(xml_chars_left);
}

int xml_goto_one_of_tags(const char *tag[],int tags) {

    while(xml_chars_left) {
        
        do {
            if(xml_goto_char('<') < 0) {
                return(-1);
            }
        } while(*xml_ptr == '/');
        
        unsigned int i = 0;
        
        do {
            i++;
        } while ((i < xml_chars_left)&&(*(xml_ptr+i) != '>')&&(*(xml_ptr+i) != ' '));

        if(i >= xml_chars_left) {
            return(-1);
        }
        
        for(int j=0;j<tags;j++) {
            if(!strncmp(xml_ptr,tag[j],i)) {
                xml_goto_char('>');
                return(j);
            }
        }
    }
    
    return(-1);
}

void xml_tag_text(char *text, size_t size) {

  // copy at most size-1 characters, never past the end of the received data
  size_t i = 0;

  while((i + 1 < size) && (i < xml_chars_left) && (xml_ptr[i] != '<') && (xml_ptr[i] != 0)) {
    text[i] = xml_ptr[i];
    i++;
  }

  text[i] = 0;
}

int xml_tag_int(void) {

  char buffer[MAX_TAG_TEXT];
  xml_tag_text(buffer, sizeof(buffer));
  return(atoi(buffer));
}

double xml_tag_double(void) {

  char buffer[MAX_TAG_TEXT];
  xml_tag_text(buffer, sizeof(buffer));
  return(atof(buffer));
}