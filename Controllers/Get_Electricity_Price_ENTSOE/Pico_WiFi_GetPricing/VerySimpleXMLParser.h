#ifndef VERY_SIMPLE_XML_PARSER_H
#define VERY_SIMPLE_XML_PARSER_H

#define MAX_TAG_TEXT  20

void xml_buffer(const char *buffer);
int xml_goto_char(char c);
int xml_goto_tag(const char *tag);
int xml_next_tag(char *tag);
int xml_goto_one_of_tags(const char *tag[],int tags);
void xml_tag_text(char *text);
int xml_tag_int(void);
double xml_tag_double(void);

#endif