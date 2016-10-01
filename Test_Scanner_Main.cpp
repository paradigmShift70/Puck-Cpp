#include "PuckScanner.hpp"

#include <string.h>
#include <stdio.h>

#include <vector>

char TEST_STRINGS[][100] =
   {
      "5 + 4",
      "6 - 3",
      "6.83 - 3",
      "2 * 8",
      "10 / 2",
      "10 / 2.659",
      "\"hello\" length",
      "5 = 5",
      "5 = x",
      "true = true",
      "false = true",
      "false = false",
      "true = false",
      "true sameObjectAs: true",
      "null sameObjectAs: 3",
      "[false member: #class] sameObjectAs: Object",
      "booleanNegation sameObjectAs: [true member: not]",
      "[true member: not] sameObjectAs: booleanNegation",
      "booleanNegation sameObjectAs: [Boolean member: not]",
      "[Boolean member: not] sameObjectAs: booleanNegation",
      "[ 1; 3; 3 ]",
      "[ 1; 3; 3 + 5 ]",
      "junk",
      "[ 1; junk <- 0; 3 + 5 ]",
      "junk",
      "lst <- #[ a; b; c ]",
      "lst at: 1",
      "junk <- 3",
      "junk",
      "[ 1; junk <- 0; 3 + 5 ]",
      "junk",
      "[ local <- 100; local ]",
      "local",
      "[ local <- 100; local + 1 ]",
      "#[ | val | val + 1 ] evalForArgs: #[ 4 ]",
      "inc <- #[ | val | val + -1 ]",
      "inc evalForArgs: #[ 6 ]",
      "#[ | val1 val2 | val1 + val2 ] evalForArgs: #[ 1; 2]",
      "junk",
      "3 doTimes: #[ | x | junk <- [ junk + 1 ] ]",
      "junk",
      "ct <- 0",
      "#[ct != 3] whileTrue: #[ ct <- [ ct + 1 ] ]",
      "[Boolean member: #not] evalForArgs: #[ false ]",
      "DONE!",
      ""
   };

int main( int, char** )
   {
   PuckScanner* scn = new PuckScanner( );

   for ( int index = 0; strlen(TEST_STRINGS[index]) != 0; index++ )
      {
      char* inputStr = TEST_STRINGS[index];
      printf( "\n>>> %s", inputStr );
      std::vector< std::pair<int,std::string> > tokLst = scn->tokenize( inputStr );
      for( unsigned idx = 0; idx < tokLst.size(); idx++)
         printf( "\n   -  %03d  /%s/", tokLst[idx].first, tokLst[idx].second.data() );
      }

   return 0;
   }
