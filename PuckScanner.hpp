#ifndef PUCKSCANNER_HPP_INCLUDED
#define PUCKSCANNER_HPP_INCLUDED

#include "Scanner.hpp"

#include <string>

// Tokens
#define  EOF_TOK               0

#define  SYMBOL_TOK          101
#define  STRING_TOK          102
#define  INTEGER_TOK         111
#define  FLOAT_TOK           112

#define  OPEN_BRACKET_TOK    201
#define  CLOSE_BRACKET_TOK   202
#define  OPEN_PAREN_TOK      211
#define  CLOSE_PAREN_TOK     212

#define  SEMI_COLON_TOK      500
#define  POUND_SIGN_TOK      501
#define  PIPE_TOK            502
#define  COLON_TOK           503

class PuckScanner: public Scanner
   {
   public:
      PuckScanner( );

   private:
      int      _scanNextToken( );
      int      _scanStringLiteral( );
      int      _scanNumOrSymbol( );
      int      _scanSymbol( );
      void     _skipWhitespaceAndComments( );
   };

#endif // PUCKSCANNER_HPP_INCLUDED
