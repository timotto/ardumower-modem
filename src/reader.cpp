#include "reader.h"
#include "log.h"
#include <string.h>

bool isPrintableCharacter(char c);

void Reader::reset()
{
  buffer = "";
}

String Reader::update(char c)
{
  if (c == 0x7f)
  {
    if (buffer.length() > 0) buffer = buffer.substring(0, buffer.length() - 1);
    return String("");
  }
  
  if (isPrintableCharacter(c))
    buffer += String(c);
  else Log(ERR, "Reader::update::bad-character-ignored(c=0x%02x)", c);

  if (!buffer.endsWith(eol))
    return String("");
  
  String result = buffer.substring(0, buffer.length() - eol.length());
  buffer = "";

  return result;
}

String Reader::peek()
{
  return buffer;
}

bool isPrintableCharacter(char c)
{
  return (c >= 0x20 && c < 0x7f) || c == '\r' || c == '\n';
}
