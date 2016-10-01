#include "PuckScanner.hpp"

#include <stdio.h>

using namespace std;

// Character Classes
#define  WHITESPACE      " \t\n\r"
#define  SIGN            "+-"
#define  DIGIT           "0123456789"
#define  SIGN_OR_DIGIT   SIGN DIGIT
#define  ALPHA_LOWER     "abcdefghijklmnopqrstuvwxyz"
#define  ALPHA_UPPER     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define  ALPHA           ALPHA_LOWER ALPHA_UPPER
#define  SYMBOL_OTHER    "~!@$%^&*_=\\:/?<>"
#define  SYMBOL_FIRST    ALPHA SIGN SYMBOL_OTHER
#define  SYMBOL_REST     ALPHA SIGN SYMBOL_OTHER DIGIT

PuckScanner::PuckScanner( )
      : Scanner( )
   {
   }

int PuckScanner::_scanNextToken( )
   {
   try
      {
      this->_skipWhitespaceAndComments( );

      char nextChar = this->buffer.peek( );
      switch ( nextChar )
         {
         case '\0':
            return EOF_TOK;

         case '[':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return OPEN_BRACKET_TOK;

         case ']':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return CLOSE_BRACKET_TOK;

         case '(':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return OPEN_PAREN_TOK;

         case ')':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return CLOSE_PAREN_TOK;

         case ';':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return SEMI_COLON_TOK;

         case '#':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return POUND_SIGN_TOK;

         case '|':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return PIPE_TOK;

         case ':':
            this->buffer.markStartOfLexeme( );
            this->buffer.consume( );
            return COLON_TOK;

         case '\"':
            return this->_scanStringLiteral( );

         default:
            if (charInCharSet(nextChar,SIGN_OR_DIGIT))
               return this->_scanNumOrSymbol( );
            else if (charInCharSet(nextChar, SYMBOL_FIRST))
               return this->_scanSymbol( );
            else
               throw ParseError( *this, "Unknown Token" );
         }
      }
   catch ( ParseError& ex )
      {
      printf( ex.generateVerboseErrorString().c_str() );
      throw;
      }
   catch ( EOFException& ex )
      {
      return EOF_TOK;
      }
   catch ( ... )
      {
      printf( "##################Other Exception\n" );
      return EOF_TOK;
      }
   }

int PuckScanner::_scanStringLiteral( )
   {
   char nextChar = this->buffer.peek( );
   if (nextChar != '\"')
      throw ParseError( *this, "'\"' expected.");
   this->buffer.markStartOfLexeme( );
   this->buffer.consume( );
   this->buffer.consumeUpTo( "\"");
   this->buffer.consume( );

   return STRING_TOK;
   }

int PuckScanner::_scanNumOrSymbol( )
   {
   ScannerState SAVE;
   char nextChar = this->buffer.peek();

   this->buffer.markStartOfLexeme();
   this->saveState( &SAVE );

   this->buffer.consume( );

   if ( charInCharSet( nextChar, SIGN ) )
      {
      char secondChar = this->buffer.peek();
      if ((secondChar == '\0') || (!charInCharSet(secondChar, DIGIT)))
         {
         this->restoreState( &SAVE );
         return this->_scanSymbol();
         }
      }

   this->buffer.consumePast( DIGIT );

   if ( this->buffer.peek() == '.' )
      {
      this->saveState( &SAVE );
      this->buffer.consume( );
      if ( !charInCharSet( this->buffer.peek(), DIGIT) )
         this->restoreState( &SAVE );
      else
         this->buffer.consumePast( DIGIT );
      return FLOAT_TOK;
      }

   return INTEGER_TOK;
   }

int PuckScanner::_scanSymbol( )
   {
   this->buffer.markStartOfLexeme();
   char nextChar = this->buffer.peek();
   if (!charInCharSet(nextChar, SYMBOL_FIRST))
      throw ParseError( *this, "Invalid symbol character" );
   this->buffer.consume();

   this->buffer.consumePast( SYMBOL_REST );

   return SYMBOL_TOK;
   }

void PuckScanner::_skipWhitespaceAndComments( )
   {
   ScannerState STATE;

   while ( charInCharSet( this->buffer.peek(), "; \t\n\r" ) )
      {
      this->buffer.consumePast( " \t\n\r" );

      if (this->buffer.peek() == ';' )
         {
         this->saveState( &STATE );
         this->buffer.consume( );
         if ( this->buffer.peek() == ';' )
            this->buffer.consumeUpTo( "\n\r" );
         else
            {
            this->restoreState( &STATE );
            return;
            }
         }
      }
   }
