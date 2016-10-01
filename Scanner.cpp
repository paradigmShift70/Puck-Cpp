#include "Scanner.hpp"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

/* *************************** */
/* *** FUNCTION DEFINTIONS *** */
/* *************************** */
bool charInCharSet( char ch, char const* aCharSet )
   {
   if (ch == '\0')
      return false;
   else
      return strchr( aCharSet, ch ) != NULL;
   }

/* ********************* */
/* *** ScannerBuffer *** */
/* ********************* */
char ScannerBuffer::dummy[1] = "";

ScannerBuffer::ScannerBuffer( )
      : _source( ScannerBuffer::dummy ),
        _point( ScannerBuffer::dummy ),
        _mark( ScannerBuffer::dummy ),
        _lineNum( 1 )
   {
   }

ScannerBuffer::~ScannerBuffer( )
   {
   }

void ScannerBuffer::reset( char* sourceString )
   {
   if (sourceString == 0)
      {
      this->_source = ScannerBuffer::dummy;
      }
   else
      this->_source = sourceString;

   this->_point   = this->_source;
   this->_mark    = this->_source;
   this->_lineNum = 1;
   }

char ScannerBuffer::peek( )
   {
   try
      {
      return *(this->_point);   // Returns '\0' at EOF
      }
   catch ( ... )
      {
      return '\0';
      }
   }

void ScannerBuffer::consume( )
   {
   if ( *(this->_point) == '\0' )
      return;

   if ( *(this->_point) == '\n')
      ++(this->_lineNum);

   ++(this->_point);
   }

void ScannerBuffer::consumeIf( char const* aCharSet )
   {
   try
      {
      if (charInCharSet( *(this->_point), aCharSet))
         this->consume( );
      }
   catch ( ... )
      {
      }
   }

void ScannerBuffer::consumeIfNot( char const* aCharSet )
   {
   try
      {
      if (!charInCharSet( *(this->_point), aCharSet))
         this->consume( );
      }
   catch ( ... )
      {
      }
   }

void ScannerBuffer::consumePast( char const* aCharSet )
   {
   try
      {
      while (charInCharSet( *(this->_point), aCharSet))
         this->consume( );
      }
   catch ( ... )
      {
      }
   }

void ScannerBuffer::consumeUpTo( char const* aCharSet )
   {
   try
      {
      while (!charInCharSet( *(this->_point), aCharSet))
         this->consume( );
      }
   catch ( ... )
      {
      }
   }

void ScannerBuffer::markStartOfLexeme( )
   {
   this->_mark = this->_point;
   }

int ScannerBuffer::lexemeSize( )
   {
   return this->_point - this->_mark;
   }

string ScannerBuffer::getLexeme( )
   {
   char buf[500];
   int len = this->_point - this->_mark;
   strncpy( buf, this->_mark, len );
   buf[ len ] = '\0';
   return string( buf );
   }

void ScannerBuffer::getLexeme( char* buf, int bufSize )
   {
   int len = this->_point - this->_mark;
   if ( (bufSize-1) < len )
      len = bufSize - 1;
   strncpy( buf, this->_mark, len );
   buf[ len ] = '\0';
   }

char const* ScannerBuffer::lexemePtr( ) const
   {
   return this->_mark;
   }

int ScannerBuffer::scanLineNum( )
   {
   return this->_lineNum;
   }

int ScannerBuffer::scanColNum( )
   {
   return this->_point - this->_linePos();
   }

int ScannerBuffer::scanLineTxtSize( )
   {
   char* linePos = this->_linePos( );
   char* toPtr   = strchr( linePos, '\n' );

   if ( toPtr == 0 )
      toPtr = this->_point + strlen(this->_point);

   return toPtr - linePos;
   }

void ScannerBuffer::scanLineTxt( char* buf, int bufSize )
   {
   strncpy( buf, (char const *)(this->_linePos()), bufSize - 1 );
   buf[ bufSize - 1 ] = '\0';
   }

string ScannerBuffer::scanLineTxt( )
   {
   char* linePos = this->_linePos( );
   char* toPtr   = strchr( linePos, '\n' );

   if ( toPtr == 0 )
      toPtr = this->_point + strlen(this->_point);

   int len = toPtr - linePos;

   return string( linePos, len );
   }

void ScannerBuffer::saveState( ScannerState* state )
   {
   state->buffer_source  = this->_source;
   state->buffer_point   = this->_point;
   state->buffer_mark    = this->_mark;
   state->buffer_lineNum = this->_lineNum;
   }

void ScannerBuffer::restoreState( ScannerState* state )
   {
   this->_source  = state->buffer_source;
   this->_point   = state->buffer_point;
   this->_mark    = state->buffer_mark;
   this->_lineNum = state->buffer_lineNum;
   }

char* ScannerBuffer::_linePos( )
   {
   char* linePos = this->_point;
   while ( (linePos >= this->_source) && (*linePos != '\n') )
      --linePos;

   while ( (*linePos < 32) && (*linePos != '\0') )
      ++linePos;

   return linePos;
   }

/* *************** */
/* *** Scanner *** */
/* *************** */
Scanner::Scanner( )
      : buffer( ), _tok( -1 )
   {
   }

void Scanner::reset( char* sourceString )
   {
   this->buffer.reset( sourceString );
   this->_tok = -1;
   this->consume( );
   }

int Scanner::peekToken( )
   {
   return this->_tok;
   }

void Scanner::consume( )
   {
   this->_tok = this->_scanNextToken( );
   }

int Scanner::lexemeSize( )
   {
   return this->buffer.lexemeSize( );
   }

string Scanner::getLexeme( )
   {
   return this->buffer.getLexeme( );
   }

void Scanner::getLexeme( char* buf, int bufSize )
   {
   this->buffer.getLexeme( buf, bufSize );
   }

char const* Scanner::lexemePtr( ) const
   {
   return this->buffer.lexemePtr( );
   }

void Scanner::saveState( ScannerState* state )
   {
   state->tok = this->_tok;
   this->buffer.saveState( state );
   }

void Scanner::restoreState( ScannerState* state )
   {
   this->_tok = state->tok;
   this->buffer.restoreState( state );
   }

std::vector< std::pair<int,std::string> > Scanner::tokenize( char* sourceStr, int EOFToken )
   {
   std::vector< std::pair<int,std::string> > tokenList;

   this->reset( sourceStr );
   int    tok = this->peekToken();
   string lex = this->getLexeme();
   while ( tok != EOFToken )
      {
      tokenList.push_back( std::pair<int,std::string>(tok,lex) );
      this->consume();
      lex = this->getLexeme();
      tok = this->peekToken();
      }

   return tokenList;
   }

/* ****************** */
/* *** ParseError *** */
/* ****************** */
ParseError::ParseError( Scanner& aScanner, string const& errMsg )
      : _filename(""), _errorMessage(errMsg)
   {
   this->_lineNum    = aScanner.buffer.scanLineNum( );
   this->_colNum     = aScanner.buffer.scanColNum( );
   this->_sourceLine = aScanner.buffer.scanLineTxt( );
   }

ParseError::ParseError( Scanner& aScanner, string const& errMsg, string const& fName )
      : _filename(fName), _errorMessage(errMsg)
   {
   this->_lineNum    = aScanner.buffer.scanLineNum( );
   this->_colNum     = aScanner.buffer.scanColNum( );
   this->_sourceLine = aScanner.buffer.scanLineTxt( );
   }

string& ParseError::generateVerboseErrorString( )
   {
   // Indent
   string indent( this->_colNum, ' ' );

   char buf[600];
   sprintf( buf,
           "Syntax Error: %s(%d,%d)\n%s\n%s^ %s\n",
            this->_filename.c_str(),
           this->_lineNum, this->_colNum, this->_sourceLine.c_str(), indent.c_str(), this->_errorMessage.c_str() );

   this->_errorReport = buf;
   return this->_errorReport;
   }

