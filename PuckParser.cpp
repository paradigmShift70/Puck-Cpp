#include "PuckParser.hpp"
#include "Object.hpp"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <exception>

using namespace std;

PuckParser::PuckParser( )
      : _scanner( ), _currentEnv( GLOBAL )
   {
   }

PkObject* PuckParser::parse( char* inputString )
   {
   this->_scanner.reset( inputString );

   PkObject* pkExprObj = this->_parseExpression( );

   // EOF
   int tok = this->_scanner.peekToken();
   if ( (tok != EOF_TOK) && (tok != SEMI_COLON_TOK) )
      throw ParseError( this->_scanner, "EOF or ; Expected." );

   this->_scanner.consume();

   return pkExprObj;
   }

PkObject* PuckParser::_parseExpression( )
   {
   // Receiver
   PkObject* receiverObj = this->_parseObject();
   if ( receiverObj == 0 )
      return 0;

   // Message
   int tok = this->_scanner.peekToken();
   if ( (tok != SEMI_COLON_TOK) && (tok != CLOSE_BRACKET_TOK) && (tok != EOF_TOK))
      return this->_parseMessage( receiverObj );
   else
      return receiverObj;
   }

PkObject* PuckParser::_parseObject( )
   {
   PkObject* theObj = 0;

   int nextToken = this->_scanner.peekToken();
   switch ( nextToken )
      {
      case INTEGER_TOK:
         {
         char const* lexPtr = this->_scanner.lexemePtr();
         int         lexLen = this->_scanner.lexemeSize();
         theObj = OBJECT.makePkInteger( lexPtr, lexLen );
         this->_scanner.consume( );
         }
         break;
      case FLOAT_TOK:
         {
         char const* lexPtr = this->_scanner.lexemePtr();
         int         lexLen = this->_scanner.lexemeSize();
         theObj = OBJECT.makePkFloat( lexPtr, lexLen );
         this->_scanner.consume( );
         }
         break;
      case STRING_TOK:
         {
         char const* lexPtr = this->_scanner.lexemePtr();
         int         lexLen = this->_scanner.lexemeSize();
         theObj = OBJECT.makePkString( lexPtr + 1, lexLen - 2 );  // Drop Quotes
         this->_scanner.consume( );
         }
         break;
      case SYMBOL_TOK:
         {
         char const* lexPtr = this->_scanner.lexemePtr();
         int         lexLen = this->_scanner.lexemeSize();
         theObj = OBJECT.makePkSymbol( lexPtr, lexLen );
         this->_scanner.consume( );
         }
         break;
      case POUND_SIGN_TOK:
         this->_scanner.consume( );
         theObj = OBJECT.makePkQuote( this->_parseObject() );
         //theObj = new PkObject( pkQuoteClass );
         //theObj->_valType   = QUOTE_VAL;
         //theObj->_val.quVal = this->_parseObject();
         break;
      case OPEN_BRACKET_TOK:
         theObj = this->_parseList( );
         break;
      default:
         throw ParseError( this->_scanner, "Object expected." );
      }

   nextToken = this->_scanner.peekToken();
   while ( nextToken == POUND_SIGN_TOK )
      {
      // We may have use of shorthand-#
      this->_scanner.consume( );

      // Parse the Symbol
      if ( this->_scanner.peekToken() != SYMBOL_TOK )
         throw ParseError( this->_scanner, "Symbol expected after shorthand-#" );

      char const* lexPtr = this->_scanner.lexemePtr();
      int         lexLen = this->_scanner.lexemeSize();
      PkObject*   sym    = OBJECT.makePkSymbol( lexPtr, lexLen );
      this->_scanner.consume( );

      // Construct the [receiver member: #symbol] expresion.
      PkObject* theQuote   = new PkObject( pkQuoteClass );
      theQuote->_valType   = QUOTE_VAL;
      theQuote->_val.quVal = sym;

      vector<string> theKeyList;
      theKeyList.push_back( "member:" );

      vector<PkObject*> theArgList;
      theArgList.push_back( theObj );
      theArgList.push_back( theQuote );

      PkObject* theExpr = OBJECT.makePkExpr( &theKeyList, &theArgList );
      vector<PkObject*> lst;
      lst.push_back( theExpr );
      theObj = OBJECT.makePkList( &lst, new PkEnvironment(this->_currentEnv) );

      nextToken = this->_scanner.peekToken();
      }

   return theObj;
   }

PkObject* PuckParser::_parseMessage( PkObject* receiverObj )
   {
   vector<string>    aKeyList;
   vector<PkObject*> anArgList;
   anArgList.push_back( receiverObj );

   // Parse a key
   if (this->_scanner.peekToken() != SYMBOL_TOK)
      throw ParseError( this->_scanner, "Symbol expected." );

   aKeyList.push_back( this->_scanner.getLexeme() );
   this->_scanner.consume();

   // Parse argument & keys
   int tok = this->_scanner.peekToken();
   if ( (tok != SEMI_COLON_TOK) && (tok != CLOSE_BRACKET_TOK) && (tok != EOF_TOK) )
      {
      // Parse an argument
      anArgList.push_back( this->_parseObject() );

      while ( this->_scanner.peekToken() == SYMBOL_TOK )
         {
         // Parse Keys
         aKeyList.push_back( this->_scanner.getLexeme() );
         this->_scanner.consume( );

         // Parse arguments
         anArgList.push_back( this->_parseObject() );
         }
      }

   return OBJECT.makePkExpr( &aKeyList, &anArgList );
   }

PkObject* PuckParser::_parseList( )
   {
   // Open Bracket
   if ( this->_scanner.peekToken() != OPEN_BRACKET_TOK )
      throw ParseError( this->_scanner, "'[' expected." );

   // Begin a lexical scope
   this->_currentEnv = new PkEnvironment( this->_currentEnv );
   this->_scanner.consume();

   // Parameters and Locals
   vector<PkObject*> params;
   vector<PkObject*> allLocals;
   if ( this->_scanner.peekToken() == PIPE_TOK )
      {
      this->_scanner.consume( );

      int nextToken = this->_scanner.peekToken( );
      while ( (nextToken == COLON_TOK) || (nextToken == SYMBOL_TOK ) )
         {
         bool isParam = false;

         if (nextToken == COLON_TOK)
            {
            isParam = true;
            this->_scanner.consume( );
            }

         if (this->_scanner.peekToken( ) != SYMBOL_TOK)
            throw ParseError( this->_scanner, "Symbol expected." );
         char const* lexPtr = this->_scanner.lexemePtr();
         int         lexLen = this->_scanner.lexemeSize();
         PkObject* sym = OBJECT.makePkSymbol( lexPtr, lexLen );

         if (isParam)
            params.push_back( sym );
         allLocals.push_back( sym );
         this->_scanner.consume( );

         nextToken = this->_scanner.peekToken( );
         }

      if ( this->_scanner.peekToken() != PIPE_TOK )
         throw ParseError( this->_scanner, "'|' expected." );

      this->_scanner.consume();
      }

   // Expressions
   vector<PkObject*> expressions;
   if ( this->_scanner.peekToken() != CLOSE_BRACKET_TOK )
      {
      PkObject* expr = this->_parseExpression();
      if (expr != 0)
         expressions.push_back( expr );

      while ( this->_scanner.peekToken() != CLOSE_BRACKET_TOK )
         {
         // Semi-colon
         if ( this->_scanner.peekToken() != SEMI_COLON_TOK )
            throw ParseError( this->_scanner, "';' expected." );

         this->_scanner.consume();

         expr = this->_parseExpression();
         if (expr != 0)
            expressions.push_back( expr );
         }
      }

   // Close Bracket
   if ( this->_scanner.peekToken() != CLOSE_BRACKET_TOK )
      throw ParseError( this->_scanner, "']' expected." );

   this->_scanner.consume();

   PkObject* result = OBJECT.makePkList( &expressions, this->_currentEnv, &params, &allLocals );

   // Close a lexical scope
   this->_currentEnv = this->_currentEnv->parentEnv();

   return result;
   }

void parseAndPrintExpr( char* anExprStr )
   {
   PuckParser puck;

   try
      {
      PkObject* result = puck.parse( anExprStr );
      string resultStr = printable( result );
      printf( "%s\n", resultStr.c_str() );
      }
   catch ( ParseError& ex )
      {
      printf( "%s\n", ex.generateVerboseErrorString().c_str() );
      }
   catch ( exception& ex )
      {
      printf( "%s\n", ex.what() );
      }
   }

void parseEvalAndPrintExpr( char* anExprStr )
   {
   PuckParser puck;
   PkObject* expr = puck.parse( anExprStr );
   PkObject* result = EVAL_OBJ_R( expr, GLOBAL );
   string resultStr = printable( result );
   printf( "%s\n", resultStr.c_str() );
   }

void test( char* anExprStr )
   {
   PuckParser puck;

   try
      {
      PkObject* expr = puck.parse( anExprStr );
      PkObject* result = EVAL_OBJ_R( expr, GLOBAL );
      string resultStr = printable( result );
      printf( "%s\n", resultStr.c_str() );
      }
   catch ( ParseError& ex )
      {
      printf( "%s\n", ex.generateVerboseErrorString().c_str() );
      }
   catch ( PuckException& ex )
      {
      printf( "%s\n", ex.what() );
      }
   }
