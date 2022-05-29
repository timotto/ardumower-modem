#include <AUnit.h>

#include "../src/reader.h"

test(reader_reads_consecutive_crlf_lines) {
  const char lineContent[] = "line content";
  String expectedResult = String(lineContent);
  const char lineContent2[] = "more stuff";
  String expectedResult2 = String(lineContent2);
  String emptyLine = String("");

  Reader uut(String("\r\n"));

  // Line 1

  for(auto i=0; i<strlen(lineContent);i++) {
    assertEqual(uut.update(lineContent[i]), emptyLine);
  }

  assertEqual(uut.update('\r'), emptyLine);
  assertEqual(uut.update('\n'), expectedResult);

  // Line 2

  for(auto i=0; i<strlen(lineContent2);i++) {
    assertEqual(uut.update(lineContent2[i]), emptyLine);
  }

  assertEqual(uut.update('\r'), emptyLine);
  assertEqual(uut.update('\n'), expectedResult2);
}

test(reader_understands_delete)
{
  const char givenInput[] = "input123";
  String expectedResult = "input";

  Reader uut(String("\r\n"));

  for(auto i=0; i<strlen(givenInput);i++)
    uut.update(givenInput[i]);
  
  for(auto i=0; i<3;i++)
    uut.update(0x7f); // ASCII delete
  
  uut.update('\r');
  String actualResult = uut.update('\n');
  assertEqual(actualResult, expectedResult);
}

test(reader_peek_returns_buffer)
{
  const char givenInput[] = "content";
  String expectedResult = String(givenInput);

  Reader uut(String("\n"));

  for(auto i=0; i<strlen(givenInput);i++)
  {
    uut.update(givenInput[i]);

    String expectedContent = expectedResult.substring(0, i+1);
    String actualResult = uut.peek();
    assertEqual(actualResult, expectedContent);
  }

  String actualLine = uut.update('\n');
  assertEqual(actualLine, expectedResult);

  String afterLineResult = uut.peek();
  String expectedEmptyResult = "";
  assertEqual(afterLineResult, expectedEmptyResult);
}