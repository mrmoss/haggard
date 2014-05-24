/**
  Reads a file formatted like:
    keyword = value
  or
    keyword: value
    keyword2: value2
    with multiline.

  And passes the keywords and values to this consumer.
  Ignores whitespace, %, !, and # comments.

  Orion Sky Lawlor, olawlor@acm.org, 2004/11/6
*/
#ifndef __OSL_KEYWORDFILE_H
#define __OSL_KEYWORDFILE_H
#include <stdio.h>
#include <string>

typedef const std::string &KeywordFileString;

class KeywordFileConsumer {
public:
	virtual ~KeywordFileConsumer();
	
	virtual void consume(KeywordFileString keyword,KeywordFileString value) =0;
};

class VerboseKeywordFileConsumer : public KeywordFileConsumer {
public:
	KeywordFileConsumer *next;
	VerboseKeywordFileConsumer(KeywordFileConsumer *next_=0) :next(next_) {}
	
	virtual void consume(KeywordFileString keyword,KeywordFileString value);
};

/// Read this file.
void read(KeywordFileString fName,KeywordFileConsumer &dest);

/// Read this file.  Closes file afterwards.
void read(FILE *f,KeywordFileConsumer &dest);


#endif
