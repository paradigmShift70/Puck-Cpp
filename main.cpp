#include "PuckScanner.hpp"
#include "PuckParser.hpp"
#include "Object.hpp"

#include <string.h>
#include <stdio.h>

using namespace std;

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
      "[false member: #class] sameObjectAs: Boolean",
      "boolean__not sameObjectAs: [true member: not]",
      "[true member: not] sameObjectAs: boolean__not",
      "boolean__not sameObjectAs: [Boolean member: not]",
      "[Boolean member: not] sameObjectAs: boolean__not",
      "[ 1; 3; 3 ]",
      "[ 1; 3; 3 + 5 ]",
      "junk",
      "[ 1; junk <- 9; 3 + 5 ]",
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
      "#[ | :val | val + 1 ] evalForArgs: #[ 4 ]",
      "inc <- #[ | :val | val + -1 ]",
      "inc evalForArgs: #[ 6 ]",
      "#[ | :val1 :val2 | val1 + val2 ] evalForArgs: #[ 1; 2]",
      "junk",
      "3 doTimes: #[ | :x | junk <- [ junk + 1 ] ]",
      "junk",
      "ct <- 0",
      "#[ct != 3] whileTrue: #[ ct <- [ ct + 1 ] ]",
      "[Boolean member: #not] evalForArgs: #[ false ]",
      "DONE!",
      ""
   };

void testScannerBuffer( )
   {
   printf( "\n" );
   printf( "*********************\n" );
   printf( "Testing ScannerBuffer\n" );
   char src[ ] = "\"hello\" length";
   ScannerBuffer buf;
   buf.reset( src );

   while( buf.peek() != '\0' )
      {
      printf( "%c", buf.peek() );
      buf.consume();
      }
   }

void testPuckScanner( )
   {
   printf( "\n" );
   printf( "******************\n" );
   printf( "Testing PuckScanner\n" );

   PuckScanner* scn = new PuckScanner( );

   for ( int index = 0; strlen(TEST_STRINGS[index]) != 0; index++ )
      {
      char* testStr = TEST_STRINGS[index];
      printf( "\n>>> %s", testStr );
      std::vector< std::pair<int,std::string> > tokLst = scn->tokenize( testStr );
      for( unsigned idx = 0; idx < tokLst.size(); idx++)
         printf( "\n   -  %03d  /%s/", tokLst[idx].first, tokLst[idx].second.data() );
      }
   }

void testPuckObject( )
   {
   printf( "\n" );
   printf( "******************\n" );
   printf( "Testing PuckObject\n" );

   string    str;

   // Symbol
   printf( "\nMaking symbol: 'One'\n" );
   PkObject* symInst = OBJECT.makePkSymbol( "One" );
   str = printable( symInst );
   printf( "%s\n", str.c_str() );

   // Null
   printf( "\nMaking NULL.\n" );
   PkObject* nullInst = new PkObject( pkNullClass );
   nullInst->_members[ "name" ] = OBJECT.makePkSymbol( "Null" );
   str = printable( nullInst );
   printf( "%s\n", str.c_str() );

   // Boolean: true
   printf( "\nMaking boolean value: true\n" );
   PkObject* trueInst = new PkObject( pkBooleanClass );
   trueInst->_val.blVal = true;
   str = printable( trueInst );
   printf( "%s\n", str.c_str() );

   // Boolean: false
   printf( "\nMaking boolean value: false\n" );
   PkObject* falseInst = new PkObject( pkBooleanClass );
   falseInst->_val.blVal = false;
   str = printable( falseInst );
   printf( "%s\n", str.c_str() );

   // String
   printf( "\nMaking string instance\n" );
   PkObject* strInst = OBJECT.makePkString( "This is a test string." );
   str = printable( strInst );
   printf( "%s\n", str.c_str() );

   // Primitive
   printf( "\nMaking primitive instance\n" );
   PkObject* primInst = OBJECT.makePkPrimitive( 101, "booleanNegation" );
   str = printable( primInst );
   printf( "%s\n", str.c_str() );

   // Integer
   printf( "\nMaking integer instance\n" );
   PkObject* intInst = new PkObject( pkIntegerClass );
   intInst->_val.niVal = 100;
   str = printable( intInst );
   printf( "%s\n", str.c_str() );

   // Float
   printf( "\nMaking float instance\n" );
   PkObject* fltInst = new PkObject( pkFloatClass );
   fltInst->_val.nfVal = 3.14159;
   str = printable( fltInst );
   printf( "%s\n", str.c_str() );

   // Quoted symbol
   printf( "\nMaking quoted symbol: 'two'\n" );
   PkObject* quotedObj = OBJECT.makePkSymbol( "two" );
   PkObject* quoteInst = new PkObject( pkQuoteClass );
   quoteInst->_val.quVal = quotedObj;
   str = printable( quoteInst );
   printf( "%s\n", str.c_str() );

   // Simple data list
   printf( "\nMaking simple data list\n" );
   vector<PkObject*> *items = new vector<PkObject*>();
   items->push_back( symInst   );
   items->push_back( nullInst  );
   items->push_back( trueInst  );
   items->push_back( falseInst );
   items->push_back( strInst   );
   items->push_back( primInst  );
   items->push_back( intInst   );
   items->push_back( fltInst   );
   items->push_back( quoteInst );
   vector<PkObject*> *params = new vector<PkObject*>();
   params->push_back( OBJECT.makePkSymbol( "param1" ) );
   params->push_back( OBJECT.makePkSymbol( "param2" ) );
   params->push_back( OBJECT.makePkSymbol( "param3" ) );
   PkObject* listInst = OBJECT.makePkList( items, 0 /*NULL*/, params );
   str = printable( listInst );
   printf( "%s\n", str.c_str() );
   }

void testPuckParser( )
   {
   printf( "\n" );
   printf( "******************\n" );
   printf( "Testing PuckParser\n" );

   for ( int index = 0; strlen(TEST_STRINGS[index]) != 0; index++ )
      {
      char* testStr = TEST_STRINGS[index];
      printf( "\n>>> %s\n", testStr );
      parseAndPrintExpr( testStr );
      printf( "\n" );
      }
   }

void testExpressionEvaluation( )
   {
   printf( "\n" );
   printf( "******************************\n" );
   printf( "Testing Evaluation Environment\n" );

   for ( int index = 0; strlen(TEST_STRINGS[index]) != 0; index++ )
      {
      char* testStr = TEST_STRINGS[index];
      printf( "\n>>> %s\n", testStr );
      test( testStr );
      printf( "\n" );
      }
   //char exprStr[] = "Object";
   //char exprStr[] = "true not";
   //char exprStr[] = "Object sameObjectAs: Object";
   /*
   printf( ">>> %s\n", exprStr );
   PuckParser parser;
   PkObject* obj = parser.parse( exprStr );
   printf( "\n\n###PRINTABLE###\n%s", printable(obj).c_str() );
   printf( "\n\n###DIAGNOSTIC###\n" );
   obj->diag();
   printf( "\n\n###EVALUATING###\n");
   PkObject* result = EVAL_OBJ_R( obj, GLOBAL );
   printf( printable(result).c_str() );
   */
   //test( exprStr );
   }

int main( )
   {
   try
      {
      printf( "\n zero " );
      initializeEnvironment();

      printf( "\n one" );
      //testScannerBuffer();
      //testPuckScanner();
      //testPuckObject();
      //testPuckParser();
      testExpressionEvaluation();
      }
   catch ( PuckException& ex )
      {
      printf( "Puck Exception Caught.\n" );
      printf( ex.what() );
      }
   catch ( VM_OpcodeNotImplemented& ex )
      {
      printf( "VM Exception - OpCode not implemented.\n" );
      printf( ex.what() );
      }
   catch ( ... )
      {
      printf( "VM Terminating - Unknown Exception\n");
      }

   return 0;
   }
