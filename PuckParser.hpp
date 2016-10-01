#ifndef PUCKPARSER_HPP_INCLUDED
#define PUCKPARSER_HPP_INCLUDED

#include "PuckScanner.hpp"
#include "Object.hpp"

class PuckParser
   {
   public:
      PuckParser( );

      PkObject*       parse( char* inputString );

   protected:
      PkObject*       _parseExpression( );
      PkObject*       _parseObject( );
      PkObject*       _parseMessage( PkObject* receiverObj );
      PkObject*       _parseList( );

   private:
      PuckScanner     _scanner;
      PkEnvironment*  _currentEnv;
   };

void parseAndPrintExpr( char* anExprObj );
void parseEvalAndPrintExpr( char* anExprObj );
void test( char* anExprStr );

#endif // PUCKPARSER_HPP_INCLUDED

