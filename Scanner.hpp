#ifndef SCANNER_HPP_INCLUDED
#define SCANNER_HPP_INCLUDED

#include <string>
#include <vector>

bool charInCharSet( char ch, char const* aCharSet );

struct ScannerState
   {
   int           tok;
   char*         buffer_source;
   char*         buffer_point;
   char*         buffer_mark;
   int           buffer_lineNum;
   };

class ScannerBuffer
   {
   public:
      ScannerBuffer( );
      ~ScannerBuffer( );

      void        reset( char* sourceString=0 );

      char        peek( );
      void        consume( );
      void        consumeIf( char const* aCharSet );
      void        consumeIfNot( char const* aCharSet );
      void        consumePast( char const* aCharSet );
      void        consumeUpTo( char const* aCharSet );

      void        saveState( ScannerState* state );
      void        restoreState( ScannerState* state );

      void        markStartOfLexeme( );
      int         lexemeSize( );
      std::string getLexeme( );
      void        getLexeme( char* buf, int bufSize );
      char const* lexemePtr( ) const;

      // Methods for error reporting
      int         scanLineNum( );
      int         scanColNum( );
      int         scanLineTxtSize( );
      void        scanLineTxt( char* buf, int bufSize );
      std::string scanLineTxt( );

   private:
      char*       _linePos( );

   private:
      char*       _source;    // The string being scanned
      char*       _point;     // The current scanner head position (index into _source)
      char*       _mark;      // the first character of the lexeme currently being scanned (index into _source)
      int         _lineNum;   // the current line number

      static char dummy[1];
   };

class Scanner
   {
   public:
      friend class ParseError;

   public:
      Scanner( );

      void            reset( char* sourceString=0 );
      int             peekToken( );
      void            consume( );

      char const*     lexemePtr( ) const;
      int             lexemeSize( );
      std::string     getLexeme( );
      void            getLexeme( char* buf, int bufSize );

      void            saveState( ScannerState* aSate );
      void            restoreState( ScannerState* aState );

      std::vector< std::pair<int,std::string> > tokenize( char* sourceStr, int EOFToken=0 );

   protected:
      virtual int     _scanNextToken( ) { return 0; }

   protected:
      ScannerBuffer   buffer;  // the underlying ScannerBuffer instance

   private:
      int             _tok;    // the next token
   };

class ParseError
   {
   public:
      ParseError( Scanner& aScanner, std::string const& errMsg );
      ParseError( Scanner& aScanner, std::string const& errMsg, std::string const& fName );

      std::string&   generateVerboseErrorString( );

   private:
      std::string    _filename;
      int            _lineNum;
      int            _colNum;
      std::string    _errorMessage;
      std::string    _sourceLine;

      std::string    _errorReport;
   };

class EOFException
   {
   };

#endif // SCANNER_HPP_INCLUDED
