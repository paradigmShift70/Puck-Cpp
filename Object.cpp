#include "Object.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <algorithm>
#include <iterator>
#include <exception>

using namespace std;

/* ********************* */
/* *** PkEnvironment *** */

PkEnvironment* GLOBAL = new PkEnvironment();

list<SymbolDefMap*> PkEnvironment::_SAVE = list<SymbolDefMap*>();

PkEnvironment::PkEnvironment( PkEnvironment* parentEnv )
      : _parentEnv( parentEnv ), _symTab( )
   {
   }

PkEnvironment* PkEnvironment::parentEnv( )
   {
   return this->_parentEnv;
   }

void PkEnvironment::resetLocal( )
   {
   this->_symTab.clear( );
   }

void PkEnvironment::declLocal( string const& aSymbol, PkObject* aValue )
   {
   this->_symTab[ aSymbol ] = aValue;
   }

void PkEnvironment::set( string const& aSymbol, PkObject* aValue )
   {
   this->_set( aSymbol, aValue, this );
   }

void PkEnvironment::_set( string const& aSymbol, PkObject* aValue, PkEnvironment* localScope )
   {
   SymbolDefMap::iterator iter = this->_symTab.find( aSymbol );
   if /*FOUND IN LOCAL SCOPE*/ ( iter != this->_symTab.end() )
      this->_symTab[ aSymbol ] = aValue;
   else if /*NO ENCLOSING SCOPE*/ ( this->_parentEnv == 0 )
      this->_symTab[ aSymbol ] = aValue;           // <<-- Force a previously unseen vardef to be global.
      //localScope->declLocal( aSymbol, aValue );  // <<-- Force a previously unused vardef to be local.
   else
      this->_parentEnv->_set( aSymbol, aValue, localScope );
   }

PkObject* PkEnvironment::get( string const& aSymbol )
   {
   SymbolDefMap::iterator iter = this->_symTab.find( aSymbol );
   if /*FOUND IN LOCAL SCOPE*/( iter != this->_symTab.end() )
      return iter->second;
   else if /*NO ENCLOSING SCOPE*/( this->_parentEnv == 0 )
      return 0;
   else
      return this->_parentEnv->get( aSymbol );
   }

void PkEnvironment::saveSymTabCopy( )
   {
   SymbolDefMap* symTabCopy = new SymbolDefMap();
   for ( SymbolDefMap::iterator iter = this->_symTab.begin(); iter != this->_symTab.end(); iter++ )
      {
      string    theKey = iter->first;
      PkObject* theVal = iter->second;
      (*symTabCopy)[ theKey ] = theVal;
      }

   PkEnvironment::_SAVE.push_back( symTabCopy );
   }

void PkEnvironment::restoreSymTabCopy( )
   {
   SymbolDefMap* symTabCopy = PkEnvironment::_SAVE.back( );
   for ( SymbolDefMap::iterator iter = symTabCopy->begin(); iter != symTabCopy->end(); iter++ )
      {
      string    theKey = iter->first;
      PkObject* theVal = iter->second;
      this->_symTab[ theKey ] = theVal;
      }

   PkEnvironment::_SAVE.pop_back();
   delete symTabCopy;
   }

/* ************************* */
/* *** PkEvaluationStack *** */
class PkEvaluationStack
   {
   public:
      PkEvaluationStack( );

      void                reset( );
      void                push( PkObject* obj );
      PkObject*           pop( );
      void                popMulti( int numToPop, vector<PkObject*>& target );
      PkObject*           top( int offset=0 );
      void                topSetTo( PkObject* newValue, int offset=0 );
      void                removeMulti( int numToRemove );
   private:
      vector<PkObject*>   _stk;
   };

PkEvaluationStack STACK;

PkEvaluationStack::PkEvaluationStack( )
      : _stk( )
   {
   }

void PkEvaluationStack::reset( )
   {
   this->_stk.clear( );
   }

void PkEvaluationStack::push( PkObject* obj )
   {
   this->_stk.push_back( obj );
   }

PkObject* PkEvaluationStack::pop( )
   {
   PkObject* theObj = this->_stk.back( );
   this->_stk.pop_back( );
   return theObj;
   }

void PkEvaluationStack::popMulti( int numToPop, vector<PkObject*>& target )
   {
   for ( int ct = 0; ct < numToPop; ct++ )
      {
      target.push_back( this->_stk.back() );
      this->_stk.pop_back( );
      }
   }

PkObject* PkEvaluationStack::top( int offset )
   {
   int pos = this->_stk.size() - offset - 1;
   return this->_stk[ pos ];
   }

void PkEvaluationStack::topSetTo( PkObject* newValue, int offset )
   {
   int pos = this->_stk.size() - offset - 1;
   this->_stk[ pos ] = newValue;
   }

void PkEvaluationStack::removeMulti( int numToRemove )
   {
   for ( int ct = 0; ct < numToRemove; ct++ )
      this->_stk.pop_back();
   }

/* **************** */
/* *** PkObject *** */

PkObject::PkObject( PkObject* aPkClass )
      : _members( )
   {
   this->_class       = aPkClass ? aPkClass : this;
   this->_environment = 0;
   this->_valType     = NULL_VAL;

   this->_members[ "class" ] = this->_class;

   if ( aPkClass == pkObjectClass )
      this->_valType = OBJECT_VAL;
   else if ( aPkClass == pkPrimitiveClass )
      this->_valType = PRIMITIVE_VAL;
   else if ( aPkClass == pkNullClass )
      this->_valType = NULL_VAL;
   else if ( aPkClass == pkBooleanClass )
      this->_valType = BOOL_VAL;
   else if ( aPkClass == pkMagnitudeClass )
      this->_valType = MAGNITUDE_VAL;
   else if ( aPkClass == pkStringClass )
      this->_valType = STRING_VAL;
   else if ( aPkClass == pkNumberClass )
      this->_valType = NUMBER_VAL;
   else if ( aPkClass == pkIntegerClass )
      this->_valType = INTEGER_VAL;
   else if ( aPkClass == pkFloatClass )
      this->_valType = FLOAT_VAL;
   else if ( aPkClass == pkQuoteClass )
      this->_valType = QUOTE_VAL;
   else if ( aPkClass == pkSymbolClass )
      this->_valType = SYMBOL_VAL;
   else if ( aPkClass == pkListClass )
      this->_valType = LIST_VAL;
   else if ( aPkClass == pkExprClass )
      this->_valType = EXPRESSION_VAL;
   else
      this->_valType = USER_VAL;
   }

PkObject::~PkObject( )
   {
   switch ( this->_valType )
      {
      case SYMBOL_VAL:
         delete [] this->_val.syVal;
         break;
      case STRING_VAL:
         delete [] this->_val.stVal;
         break;
      case LIST_VAL:
         delete this->_val.liVal;
         break;
      }
   }

PkObject* PkObject::findMember( char const* aMemberName )
   {
   PkObject* theObjToSearch = this;
   SymbolDefMap::iterator iter;

   // Find the handler for aSel (the selector)
   while ( true )
      {
      iter = theObjToSearch->_members.find( aMemberName );
      if ( iter != theObjToSearch->_members.end() )
         return iter->second;
      else if ( theObjToSearch->_class == theObjToSearch )
         {
         char buf[100];
         sprintf( buf, "Invalid member name %s.", aMemberName);
         string s = string(buf);
         throw PuckException( s );
         }
      else
         theObjToSearch = theObjToSearch->_class;
      }
   }

void PkObject::diag( int level )
   {
   char* indentStr = new char [ (level * 3) + 1 ];
   strcpy( indentStr, "" );
   for ( int ct=0; ct < level; ct++ )
      strcat( indentStr, "   " );

   // Open object and print its address
   printf( "%s( [0x%p]\n", indentStr, this );

   // print it's class
   if ( this == this->_class )
      {
      printf( "%s###BASE OBJECT###\n", indentStr );
      }
   else
      {
      PkObject* theClass = this->_class;
      SymbolDefMap& theMembers = theClass->_members;
      SymbolDefMap::iterator theIter = theMembers.find( "name" );

      SymbolDefMap::iterator iter = this->_class->_members.find( "name" );
      if ( iter != this->_class->_members.end( ) )
         printf( "%sclass=%s [0x%p]\n", indentStr, iter->second->_val.stVal, this->_class );
      else
         printf( "%sclass=[0x%p]\n", indentStr, this->_class );

      if ( this->_class != this )
         this->_class->diag( level + 1 );

      printf( "%svalType=%d\n", indentStr, this->_valType);
      }

   // Print Members
   for ( SymbolDefMap::iterator iter = this->_members.begin( ); iter != this->_members.end(); iter++ )
      {
      char const* name = iter->first.c_str();
      PkObject*   val  = iter->second;

      if ( strcmp( name, "name" ) == 0 )
         {
         string str = printable( val );
         char const* c_str = str.c_str();
         printf( "%smember: %s = [0x%p] String, val = %s\n", indentStr, name, val, c_str );
         }
      else
         printf( "%smember: %s = [0x%p]\n", indentStr, name, val );
      }

   printf( "%s)\n", indentStr );
   delete [] indentStr;
   }

string printableID( PkObject* anObj )
   {
   string name = "";
   try
      {
      PkObject* nameObj = anObj->_members[ "name" ];
      name = nameObj->_val.stVal;
      }
   catch ( ... )
      {
      }

   string primitiveAugmentStr = (anObj->_class == pkPrimitiveClass) ? " Primitive" : "";
   string nameAugmentStr      = (name != "") ? (" name=\"" + name + "\"") : "";

   char buf[ 100 ];
   sprintf( buf, "<object%s id=%p%s>", primitiveAugmentStr.c_str(), anObj, nameAugmentStr.c_str() );
   return buf;
   }

bool isInstanceOf( PkObject* anObj, string& classNameStr )
   {
   return true;
   }

string printable( PkObject* anObj )
   {
   char buf[ 300 ];
   strcpy( buf, "" );

   if ( anObj->_class == pkIntegerClass )
      sprintf( buf, "%ld", anObj->_val.niVal );

   else if ( anObj->_class == pkFloatClass )
      sprintf( buf, "%f", anObj->_val.nfVal );

   else if ( anObj->_class == pkSymbolClass )
      sprintf( buf, "%s", anObj->_val.syVal );

   else if ( anObj->_class == pkStringClass )
      sprintf( buf, "\"%s\"", anObj->_val.stVal );

   else if ( anObj->_class == pkNullClass )
      strcpy( buf, "null" );

   else if ( anObj->_class == pkBooleanClass )
      strcpy( buf, (anObj->_val.blVal) ? "true" : "false" );

   else if ( anObj->_class == pkQuoteClass )
      {
      strcpy( buf, "#" );
      strcat( buf, printable(anObj->_val.quVal).c_str() );
      }

   else if ( anObj->_class == pkListClass )
      {
      strcpy( buf, "[ " );
      // Parameters
      PkObject* params = anObj->_members[ "parameters" ];
      if ( params != pkNull )
         {
         strcat( buf, "| " );
         vector<PkObject*>* paramList = params->_val.liVal;
         for ( vector<PkObject*>::iterator iter = paramList->begin(); iter != paramList->end(); iter++ )
            {
            PkObject* paramSym = *iter;
            strcat( buf, paramSym->_val.syVal );
            strcat( buf, " " );
            }
         strcat( buf, "| " );
         }

      // Members
      vector<PkObject*>* elements = anObj->_val.liVal;
      for ( vector<PkObject*>::iterator iter = elements->begin(); iter != elements->end(); iter++ )
         {
         if ( iter != elements->begin() )
            strcat( buf, "; " );

         PkObject* expr = *iter;
         strcat( buf, printable(expr).c_str() );
         }

      strcat( buf, " ]" );
      }

   else if ( anObj->_class == pkExprClass )
      {
      char msgString[100];
      strcpy( msgString, "" );

      PkObject* keyList = anObj->_members[ "keys" ];
      vector<PkObject*>& keys = *(keyList->_val.liVal);

      PkObject* argList = anObj->_members[ "arguments" ];
      vector<PkObject*>& args = *(argList->_val.liVal);

      if ( args.size() == 1)
         {
         strcat( msgString, " " );
         strcat( msgString, keys[0]->_val.syVal );
         }
      else
         {
         for (int idx = 0; idx < (int)keys.size(); idx++ )
            {
            strcat( msgString, " " );
            strcat( msgString, keys[idx]->_val.syVal );
            strcat( msgString, " " );
            strcat( msgString, printable(args[idx+1]).c_str() );
            }
         }

      strcat( buf, printable(args[0]).c_str() );
      strcat( buf, msgString );
      }

   else
      strcpy( buf, printableID( anObj ).c_str());

   return string( buf );
   }

PkObject* EVAL_OBJ_R( PkObject* anObj, PkEnvironment* anEnv, bool isLValue )
   {
   PkList EMPTY_LIST;
   EVAL_EXPR( anObj, "eval", EMPTY_LIST, anEnv, false );
   return STACK.pop( );
   //STACK.push( anObj );
   //STACK_EVAL( "eval", 1, anEnv, isLValue );
   //return STACK.pop( );
   }

void EVAL_EXPR( PkObject* anObj, char const* aSel, vector<PkObject*>& anArgList, PkEnvironment* anEnv, bool isLValue )
   {
   // Push the args on the stack in reverse order
   for ( vector<PkObject*>::reverse_iterator iter = anArgList.rbegin(); iter != anArgList.rend(); iter++ )
      STACK.push( *iter );

   // Push the receiver onto the stack.
   STACK.push( anObj );

   // Call the evaluation function
   STACK_EVAL( aSel, anArgList.size() + 1, anEnv, isLValue=false );
   }

void STACK_EVAL( char const* aSel, int anArgCt, PkEnvironment* anEnv, bool isLValue )
   {
   PkObject* theHandler = STACK.top()->findMember( aSel );

   if ( theHandler->_class == pkListClass )
      {
      vector<PkObject*> theArgList;
      STACK.popMulti( anArgCt, theArgList );
      PkObject* theArgsPkList = OBJECT.makePkList( &theArgList, anEnv );
      STACK.push( theArgsPkList );
      STACK.push( theHandler );
      STACK_EVAL( "evalForArgs:", 2, anEnv, isLValue );
      }
   else if ( theHandler->_class == pkPrimitiveClass )
      evalPrimitive( theHandler->_val.prVal, anEnv, anArgCt, isLValue );
   }

void popAndTestArgs( int numArgsExpected, int numArgsPassed, ...)
   // Additional arguments come in pairs:  PkObject**, char const*
   // pops each PkObject* from the stack and checks that it's the correct type.
   // Upon return, PkObject* arguments will point to the popped stack objects
   {
   char errMsg[ 200 ];
   if ( numArgsExpected != numArgsPassed )
      {
      sprintf( errMsg, "Exactly %d arguments expected, got %d", numArgsExpected, numArgsPassed );
      throw PuckException( string(errMsg) );
      }

   va_list argPtr;
   va_start( argPtr, numArgsPassed );
   for ( int argNum = 0; argNum < numArgsPassed; argNum++ )
      {
      PkObject** argObj  = va_arg( argPtr, PkObject** );
      char*      argType = va_arg( argPtr, char*      );

      *argObj = STACK.pop();
      PkObject*  soughtPkObj = GLOBAL->get( argType );

      PkObject* traversePoint = (*argObj)->_class;
      while ( (traversePoint != soughtPkObj) && (traversePoint != pkObjectClass) )
         traversePoint = traversePoint->_class;

      if (traversePoint!= soughtPkObj)
         {
         sprintf( errMsg, "Argument %d expected to be a %s.", argNum, argType );
         throw PuckException( string(errMsg) );
         }
      }

   va_end( argPtr );
   }



PkList EMPTY_ARGS;

void evalPrimitive( unsigned char opCode, PkEnvironment* anEnv, int anArgCt, bool isLValue )
   {
   PkObject* receiver;
   PkObject* arg1;
   PkObject* arg2;
   PkObject* arg3;
   PkObject* arg4;

   switch ( opCode )
      {
      case No_Op:
         break;

      case No_Op1:
         STACK.topSetTo( pkNull );
         break;

      case primitive__eq_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if (arg1->_val.prVal == receiver->_val.prVal)
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case primitive__evalForArgs_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );
         {
         PkList* argLst = arg1->_val.liVal;
         PkList emptyArgs;
         for (PkList::reverse_iterator iter = argLst->rbegin(); iter != argLst->rend(); iter++ )
            EVAL_EXPR( *iter, "eval", emptyArgs, anEnv, isLValue );  // Evaluation leaves the result on the stack

         unsigned char opCode = receiver->_val.prVal;
         evalPrimitive( opCode, anEnv, argLst->size(), false );
         }
         break;

      case quote__eval:
         receiver = STACK.pop( );
         STACK.push( receiver->_val.quVal );
         break;

      case expr__eval:
         {
         receiver = STACK.pop();

         // In reverse order, push each arg and evaluate it
         PkObject* argListObjPtr = receiver->_members[ "arguments" ];
         vector<PkObject*>* argList = argListObjPtr->_val.liVal;
         char* selector = receiver->_members[ "selector" ]->_val.syVal;
         int numArgsInExpr = argList->size();
         PkObject* arg;
         for ( int argNum = numArgsInExpr - 1; argNum >= 0; argNum-- )
            {
            arg = argList->at( argNum );
            STACK.push( arg ); //argList[ argNum ] );
            STACK_EVAL( "eval", 1, anEnv, (argNum == 0) && (strcmp(selector,"<-")==0) );
            }

         // Now exec the code indicated by the selector
         STACK_EVAL( selector, numArgsInExpr, anEnv, isLValue );
         }
         break;

      case object__sameObjectAs_:
         receiver = STACK.pop();
         arg1     = STACK.pop();

         if ( receiver == arg1 )
            STACK.push( pkTrue  );
         else
            STACK.push( pkFalse );
         break;

      case object__isKindOf_:
         receiver = STACK.pop();
         arg1     = STACK.pop();

         while ((receiver->_class != receiver) && (receiver->_class != arg1))
            receiver = receiver->_class;

         if ( receiver->_class == arg1 )
            STACK.push( pkTrue );
         else
            STACK.push( pkFalse );
         break;

      case object__eq_:
         receiver = STACK.pop();
         arg1     = STACK.top();

         if ( receiver->_val.obVal == arg1->_val.obVal )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );

      case object__ne_:
         STACK_EVAL( "=", anArgCt, anEnv, isLValue );
         STACK_EVAL( "not", 1, anEnv, isLValue );
         break;

      case object__eval:
         // Nothing to do.  Returns the receiver, which is already at the top of the stack
         break;

      case object__evalMessage_:
         {
         receiver                         = STACK.pop();
         vector<PkObject*>& theListOfArgs = *(STACK.pop()->_val.liVal);
         // There should only be one arg, the list passed to evalMessage:
         vector<PkObject*>* theArgs = theListOfArgs[0]->_val.liVal;
         char const* theSelStr = (*theArgs)[0]->_val.syVal;

         vector<PkObject*> theArgsToTheSelStr;
         if ( theArgs->size() > 1 )
            {
            vector<PkObject*>::iterator beg = theArgs->begin();
            beg++;  // point the iterator at the second item (the first was the selector).

            theArgsToTheSelStr = vector<PkObject*>( beg, theArgs->end() );
            }

         EVAL_EXPR( receiver, theSelStr, theArgsToTheSelStr, anEnv, isLValue );
         }
         break;

      case object__member_:
         {
         PkObject* theObjToSearch  = STACK.pop( );
         char*     theSoughtSymStr = STACK.pop( )->_val.syVal;

         while ( true )
            {
            SymbolDefMap::iterator iter = theObjToSearch->_members.find( theSoughtSymStr );
            if ( iter != theObjToSearch->_members.end() )
               {
               STACK.push( iter->second );
               return;
               }
            else if ( theObjToSearch->_class == theObjToSearch )
               {
               STACK.push( pkNull );
               break;
               }
            else
               theObjToSearch = theObjToSearch->_class;
            }
         }
         break;

      case object__memberRemove_:
         throw VM_OpcodeNotImplemented();
         break;

      case object__member_setTo_:
         {
         PkObject* theObjToSearch  = STACK.pop( );
         char*     theSoughtSymStr = STACK.pop( )->_val.syVal;
         PkObject* theValueToSet   = STACK.pop( );

         PkObject* saveTheObjToSearch = theObjToSearch;
         while ( true )
            {
            SymbolDefMap::iterator iter = theObjToSearch->_members.find( theSoughtSymStr );
            if ( iter != theObjToSearch->_members.end() )
               {
               iter->second = theValueToSet;
               STACK.push( theValueToSet );
               return;
               }
            else if ( theObjToSearch->_class == theObjToSearch )
               {
               STACK.push( pkNull );
               break;
               }
            else
               theObjToSearch = theObjToSearch->_class;
            }

         // The symbol wasn't found as a member anywhere in the object/class hierarchy
         // so add it to the original object.
         saveTheObjToSearch->_members[ theSoughtSymStr ] = theValueToSet;
         STACK.push( theValueToSet );
         }
         break;

      case object__listMembers:
         {
         receiver = STACK.pop( );

         // Put all the member names into a vector of string
         vector<string> sortable;
         for (map<string,PkObject*>::iterator iter = receiver->_members.begin(); iter != receiver->_members.end(); iter++ )
            sortable.push_back( iter->first );

         // Sort the vector
         sort( sortable.begin(), sortable.end() );

         // Put all the elements into another vector turning them first into PkObject Symbols.
         vector<PkObject*> sortedSymbols;
         for ( vector<string>::iterator iter = sortable.begin(); iter != sortable.end(); iter++ )
            sortedSymbols.push_back( OBJECT.makePkSymbol( iter->data() ) );

         // Turn the symbol list into a PkObject List instance.
         PkObject* theSymList = OBJECT.makePkList( &sortedSymbols );

         // Push the PkList of PkSymbols
         STACK.push( theSymList );
         }
         break;

      case object__make:
         {
         receiver = STACK.pop( );
         PkObject* result = new PkObject( receiver );
         STACK.push( result );
         break;
         }

      case object__printValue:
         receiver = STACK.pop( );
         printf( "%s", printable(receiver).data() );
         break;

      case object__printMembers:
         receiver = STACK.pop( );
         for ( SymbolDefMap::iterator iter = receiver->_members.begin(); iter != receiver->_members.end(); iter++ )
            {
            string    sym = iter->first;
            PkObject* obj = iter->second;

            printf( "Member: %s", sym.data() );
            printf( "    %s", printable(obj).data() );
            }
         break;

      case object__asString:
         receiver = STACK.top( );
         STACK.topSetTo( OBJECT.makePkString( printable(receiver).data() ) );
         break;

      case object__sameTypeAs_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( arg1->_valType == receiver->_valType )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case object__delete:
         receiver = STACK.pop( );
         try
            {
            delete receiver;
            }
         catch ( ... )
            {
            STACK.push( pkFalse );
            return;
            }
         STACK.push( pkTrue );
         break;

      case boolean__eq_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( arg1->_val.blVal == receiver->_val.blVal )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case boolean__not:
         receiver = STACK.top( );
         if ( receiver->_val.blVal == true )
            STACK.topSetTo( pkFalse );
         else
            STACK.topSetTo( pkTrue );
         break;

      case boolean__and_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         bool obj = receiver->_val.blVal;
         bool arg = arg1->_val.blVal;
         if ( obj && arg )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         }
         break;

      case boolean__or_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         bool obj = receiver->_val.blVal;
         bool arg = arg1->_val.blVal;
         if ( obj || arg )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         }
         break;

      case boolean__ifTrue_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( receiver == pkTrue )
            EVAL_EXPR( arg1, "eval", EMPTY_ARGS, anEnv, isLValue );
         else
            STACK.push( GLOBAL->get("null") );
         break;

      case boolean__ifTrue_ifFalse_:
         receiver = STACK.pop( );
         arg1     = STACK.pop( );
         arg2     = STACK.top( );

         if ( receiver == pkTrue )
            EVAL_EXPR( arg1, "eval", EMPTY_ARGS, anEnv, isLValue );
         else
            EVAL_EXPR( arg2, "eval", EMPTY_ARGS, anEnv, isLValue );
         break;

      case magnitude__lt_:
         throw VM_OpcodeNotImplemented();
         break;

      case magnitude__le_:
         throw VM_OpcodeNotImplemented();
         break;

      case magnitude__gt_:
         throw VM_OpcodeNotImplemented();
         break;

      case magnitude__ge_:
         throw VM_OpcodeNotImplemented();
         break;

      case string__eq_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( strcmp(arg1->_val.stVal,receiver->_val.stVal) == 0 )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case string__le_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( strcmp(arg1->_val.stVal,receiver->_val.stVal) <= 0 )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case string__length:
         receiver = STACK.pop( );

         STACK.topSetTo( OBJECT.makePkInteger(strlen(receiver->_val.stVal)) );
         break;

      case string__at_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );
         {
         int arg = arg1->_val.niVal;
         if (arg < 0)
            arg = strlen(receiver->_val.stVal) - arg;
         unsigned char chVal = receiver->_val.stVal[arg];
         char strCharVal[8] = "\0\0\0\0\0\0\0";
         strCharVal[0] = chVal;
         PkObject* resultStr = OBJECT.makePkString( (char const*) &strCharVal );
         STACK.topSetTo( resultStr );
         }
         break;

      case string__from_to_:
         receiver = STACK.pop( );
         arg1     = STACK.pop( );
         arg2     = STACK.top( );
         {
         int idx1 = arg1->_val.niVal;
         int idx2 = arg2->_val.niVal;
         if (idx1 < 0)
            idx1 = strlen(receiver->_val.stVal) - idx1;
         if (idx2 < 0)
            idx2 = strlen(receiver->_val.stVal) - idx2;
         if (idx1 > idx2)
            swap( idx1, idx2 );
         int substrLen = idx2 - idx1 + 2;
         char* subStr  = new char[substrLen];
         strncpy( subStr, receiver->_val.stVal + idx1, substrLen );
         subStr[substrLen] = '\0';
         PkObject* resultStr = OBJECT.makePkString( subStr );
         STACK.topSetTo( resultStr );
         delete [] subStr;
         }
         break;

      case string__add_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );
         {
         char* obj = receiver->_val.stVal;
         char* arg = arg2->_val.stVal;
         int newStrLen = strlen(obj) + strlen(arg) + 2;
         char* newStr  = new char[newStrLen];
         strcpy( newStr, obj );
         strcat( newStr, arg );
         PkObject* resultStr = OBJECT.makePkString( newStr );
         STACK.topSetTo( resultStr );
         delete [] newStr;
         }
         break;

      case string__forEachCharacter_:
         throw VM_OpcodeNotImplemented();
         break;

      case string__parsePuckExpr:
         throw VM_OpcodeNotImplemented();
         break;

      case string__asInteger:
         throw VM_OpcodeNotImplemented();
         break;

      case string__asFloat:
         throw VM_OpcodeNotImplemented();
         break;

      case string__format_:
         throw VM_OpcodeNotImplemented();
         break;

      case number__eq_:
         throw VM_OpcodeNotImplemented();
         break;

      case number__le_:
         throw VM_OpcodeNotImplemented();
         break;

      case number__asInteger:
         throw VM_OpcodeNotImplemented();
         break;

      case number__asFloat:
         throw VM_OpcodeNotImplemented();
         break;

      case symbol__eq_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( strcmp(arg1->_val.syVal,receiver->_val.syVal) == 0 )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case symbol__eval:
         receiver = STACK.top( );

         if (!isLValue)
            {
            PkObject* symVal = anEnv->get( receiver->_val.syVal );
            if ( symVal != 0 )
               STACK.topSetTo( symVal );
            }
         // Otherwise, the symbol is at the top of the stack.
         // leave it as the ret val.
         break;

      case symbol__set_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );
         anEnv->set( receiver->_val.syVal, arg1 );
         break;

      case symbol__undefine_:
         throw VM_OpcodeNotImplemented();
         break;

      case integer__eq_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( arg1->_val.niVal == receiver->_val.niVal )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case integer__le_:
         throw VM_OpcodeNotImplemented();
         break;

      case integer__add_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.niVal;
         int arg;
         if (arg1->_valType == FLOAT_VAL)
            arg = int(arg1->_val.nfVal);
         else
            arg = arg1->_val.niVal;

         STACK.topSetTo( OBJECT.makePkInteger(obj + arg) );
         }
         break;
      case integer__sub_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.niVal;
         int arg;
         if (arg1->_valType == FLOAT_VAL)
            arg = int(arg1->_val.nfVal);
         else
            arg = arg1->_val.niVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj - arg) );
         }
         break;
      case integer__mul_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.niVal;
         int arg;
         if (arg1->_valType == FLOAT_VAL)
            arg = int(arg1->_val.nfVal);
         else
            arg = arg1->_val.niVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj * arg) );
         }
         break;
      case integer__div_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.niVal;
         int arg;
         if (arg1->_valType == FLOAT_VAL)
            arg = int(arg1->_val.nfVal);
         else
            arg = arg1->_val.niVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj / arg) );
         }
         break;
      case integer__mod_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.niVal;
         int arg;
         if (arg1->_valType == FLOAT_VAL)
            arg = int(arg1->_val.nfVal);
         else
            arg = arg1->_val.niVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj % arg) );
         }
         break;

      case integer__doTimes_:
         {
         receiver    = STACK.pop( );
         arg1        = STACK.top( );
         int ctLimit = receiver->_val.niVal;
         PkObject *mostRecentResult;
         for ( int ct = 0; ct < ctLimit; ct++ )
            {
            PkObject* counterParam = OBJECT.makePkInteger(ct);
            vector<PkObject*> *argLst = new vector<PkObject*>();
            argLst->push_back( counterParam );
            PkObject* theArgLst = OBJECT.makePkList( argLst, anEnv );
            vector<PkObject*> outerArgLst = vector<PkObject*>();
            outerArgLst.push_back( theArgLst );

            EVAL_EXPR( arg1, "evalForArgs:", outerArgLst, anEnv, isLValue );
            //EVAL_EXPR( arg1, "evalForArgs:", *outerArgLst, anEnv, isLValue );
            //EVAL_EXPR( arg1, "evalForArgs:", argLst, anEnv, isLValue );
            mostRecentResult = STACK.pop( );
            }
         STACK.topSetTo( mostRecentResult );
         }
         break;

      case integer__asInteger:
         // Nothing to do.  The item on the stack is already an integer.
         break;

      case integer__asFloat:
         receiver = STACK.top( );
         STACK.topSetTo( OBJECT.makePkFloat(float(receiver->_val.niVal)) );
         break;

      case float__eq_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( arg1->_val.nfVal == receiver->_val.nfVal )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case float__le_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         if ( arg1->_val.nfVal <= receiver->_val.nfVal )
            STACK.topSetTo( pkTrue );
         else
            STACK.topSetTo( pkFalse );
         break;

      case float__add_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.nfVal;
         int arg;
         if (arg1->_valType == INTEGER_VAL)
            arg = float(arg1->_val.niVal);
         else
            arg = arg1->_val.nfVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj + arg) );
         }
         break;

      case float__sub_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.nfVal;
         int arg;
         if (arg1->_valType == INTEGER_VAL)
            arg = float(arg1->_val.niVal);
         else
            arg = arg1->_val.nfVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj - arg) );
         }
         break;

      case float__mul_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.nfVal;
         int arg;
         if (arg1->_valType == INTEGER_VAL)
            arg = float(arg1->_val.niVal);
         else
            arg = arg1->_val.nfVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj * arg) );
         }
         break;

      case float__div_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         {
         int obj = receiver->_val.nfVal;
         int arg;
         if (arg1->_valType == INTEGER_VAL)
            arg = float(arg1->_val.niVal);
         else
            arg = arg1->_val.nfVal;
         STACK.topSetTo( OBJECT.makePkInteger(obj / arg) );
         }
         break;

      case float__asInteger:
         receiver = STACK.top( );
         STACK.topSetTo( OBJECT.makePkInteger(int(receiver->_val.nfVal)) );
         break;

      case float__asFloat:
         // Nothing to do.  Value on top of stack is already a float.
         break;

      case list__eval:
         {
         receiver = STACK.pop( );

         // Save the state of the prior call to this fn
         receiver->_environment->saveSymTabCopy();

         // Initialize the new symbol table
         std::vector<PkObject*>* locals = receiver->_members[ "allLocals" ]->_val.liVal;
         for ( std::vector<PkObject*>::iterator iter = locals->begin(); iter != locals->end(); iter++ )
            {
            PkObject* ptr = *iter;
            char*     sym = ptr->_val.syVal;
            receiver->_environment->declLocal( sym, pkNull ); //(*iter)->_val.syVal, GLOBAL->get("null") );
            }

         // Evaluate the items each in turn
         PkObject* mostRecent = 0;
         for ( std::vector<PkObject*>::iterator iter = receiver->_val.liVal->begin(); iter != receiver->_val.liVal->end(); iter++ )
            mostRecent = EVAL_OBJ_R( *iter, receiver->_environment, isLValue );

         // Restore the state of the prior call to this fn
         receiver->_environment->restoreSymTabCopy( );

         STACK.push( mostRecent );
         }
         break;

      case list__evalForArgs_:
         {
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         // Save the state of the prior call to this fn
         receiver->_environment->saveSymTabCopy( );

         // Initialize the new symbol table
         std::vector<PkObject*> *localSymLst = receiver->_members[ "allLocals" ]->_val.liVal;
         for ( std::vector<PkObject*>::iterator iter = localSymLst->begin(); iter != localSymLst->end(); iter++ )
            {
            string symVal = (*iter)->_val.syVal;
            receiver->_environment->declLocal( symVal, pkNull );
            }

         // In reverse order push each arg, evaluate it, and assign the result to its respective param.
         std::vector<PkObject*> *theArgList   = arg1->_val.liVal;
         std::vector<PkObject*> *theParamList = receiver->_members[ "parameters" ]->_val.liVal;
         unsigned int numArgs   = theArgList->size();
         unsigned int numParams = theParamList->size();
         if (numArgs != numParams)
            throw PuckException( "Invalid number of arguments passed to a list." );
         for (int argNum = numArgs - 1; argNum >= 0; argNum-- )
            {
            STACK.push( theArgList->at(argNum) );
            STACK_EVAL( "eval", 1, anEnv );
            receiver->_environment->declLocal( theParamList->at(argNum)->_val.syVal, STACK.top() );
            }

         // Now evaluate the elements in the list
         PkObject *mostRecentResult;
         std::vector<PkObject*> *bodyList = receiver->_val.liVal;
         for (std::vector<PkObject*>::iterator iter = bodyList->begin(); iter != bodyList->end(); iter++ )
            mostRecentResult = EVAL_OBJ_R( *iter, receiver->_environment, isLValue=false );

         // Remove the arguments, then push the return value
         STACK.removeMulti( numArgs );

         // Restore the state of the prior call to this fn
         receiver->_environment->restoreSymTabCopy( );

         STACK.topSetTo( mostRecentResult );
         }
         break;

      case list__evalInPlace:
         throw VM_OpcodeNotImplemented();
         break;

      case list__asExpr:
         throw VM_OpcodeNotImplemented();
         break;

      case list__eq_:
         throw VM_OpcodeNotImplemented();
         break;

      case list__length:
         throw VM_OpcodeNotImplemented();
         break;

      case list__at_:
         receiver = STACK.pop( );
         arg1     = STACK.top( );

         try
            {
            int loc = arg1->_val.niVal;
            STACK.topSetTo( receiver->_val.liVal->at(loc));
            }
         catch ( ... )
            {
            throw PuckException( "List index out of range." );
            }
         break;

      case list__from_to_:
         throw VM_OpcodeNotImplemented();
         break;

      case list__add_:
         throw VM_OpcodeNotImplemented();
         break;

      case list__append_:
         throw VM_OpcodeNotImplemented();
         break;

      case list__whileTrue_:
         {
         receiver = STACK.pop( );
         arg1     = STACK.top( );
         PkObject* mostRecentResult = 0;
         STACK.push( receiver );
         STACK_EVAL( "eval", 1, anEnv, false );

         while ( STACK.pop() == pkTrue )
            {
            STACK.push( arg1 );
            STACK_EVAL( "eval", 1, anEnv, false );
            mostRecentResult = STACK.pop( );

            STACK.push( receiver );
            STACK_EVAL( "eval", 1, anEnv, false );
            }

         STACK.push( mostRecentResult );
         }
         break;

      case list__forEachItem_:
         throw VM_OpcodeNotImplemented();
         break;
      }

   return;
   }


PkObject* pkObjectClass     = 0;
PkObject* pkPrimitiveClass  = 0;
PkObject* pkNullClass       = 0;
PkObject* pkBooleanClass    = 0;
PkObject* pkMagnitudeClass  = 0;
PkObject* pkStringClass     = 0;
PkObject* pkNumberClass     = 0;
PkObject* pkIntegerClass    = 0;
PkObject* pkFloatClass      = 0;
PkObject* pkQuoteClass      = 0;
PkObject* pkSymbolClass     = 0;
PkObject* pkListClass       = 0;
PkObject* pkExprClass       = 0;

PkObject* pkNull            = 0;
PkObject* pkTrue            = 0;
PkObject* pkFalse           = 0;


/* ********************* */
/* *** PkObjectStore *** */
PkObjectStore::PkObjectStore( int theSize )
   {
   this->_objectIdLimit = theSize;

   this->_objectArray = new PkObject* [ theSize ];
   for ( int idx = 0; idx < theSize; idx++ )
      this->_objectArray[idx] = 0;
   }

PkObjectStore::~PkObjectStore( )
   {
   }

int PkObjectStore::_newObjectId( )
   {
   // Find a free slot
   for ( int objId = 0; objId < this->_objectIdLimit; objId++ )
      if ( this->_objectArray[objId] == 0 )
         return objId;

   throw PuckException( "Out of free store." );
   }

PkObject* PkObjectStore::makePkInteger( long long   val )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   PkObject* theNewObj   = new PkObject( pkIntegerClass );
   theNewObj->_valType   = INTEGER_VAL;
   theNewObj->_val.niVal = val;

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkInteger( char const* val, int length )
   {
   return this->makePkInteger( atoi(val) );// CAUTION! atoi() not controlled by length arg
   }

PkObject* PkObjectStore::makePkFloat(   float       val )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   PkObject* theNewObj   = new PkObject( pkFloatClass );
   theNewObj->_valType   = FLOAT_VAL;
   theNewObj->_val.nfVal = val;

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkFloat(   char const* val, int length )
   {
   return this->makePkFloat( atof(val) );// CAUTION! atof() not controlled by length arg
   }

PkObject* PkObjectStore::makePkSymbol(  char const* val, int length )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   if ( length == -1 )
      length = strlen(val) + 1;

   PkObject* theNewObj   = new PkObject( pkSymbolClass );
   theNewObj->_valType   = SYMBOL_VAL;
   theNewObj->_val.syVal = new char [ strlen(val) + 1 ];
   strcpy( theNewObj->_val.syVal, val );
   theNewObj->_val.stVal[ length ] = '\0';

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkString(  char const* val, int length )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   if ( length == -1 )
      length = strlen(val);

   PkObject* theNewObj   = new PkObject( pkStringClass );
   theNewObj->_valType   = STRING_VAL;
   theNewObj->_val.syVal = new char [ length + 1 ];
   strcpy( theNewObj->_val.stVal, val );
   theNewObj->_val.stVal[ length ] = '\0';

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkPrimitive( unsigned char opCode, char const* opCodeStr )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   PkObject* theNewObj   = new PkObject( pkPrimitiveClass );
   theNewObj->_valType   = PRIMITIVE_VAL;
   theNewObj->_val.prVal = opCode;
   theNewObj->_members[ "name" ] = this->makePkString( opCodeStr );
   GLOBAL->set( opCodeStr, theNewObj );

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkList( std::vector<PkObject*>* items, PkEnvironment* anEnv, std::vector<PkObject*>* params, std::vector<PkObject*>* locals )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   PkObject* theNewObj        = new PkObject( pkListClass );
   theNewObj->_valType        = LIST_VAL;
   theNewObj->_val.liVal      = new vector<PkObject*>( *items );

   PkEnvironment* _env        = anEnv  != 0 ? anEnv : new PkEnvironment( GLOBAL );
   vector<PkObject*>* _params = params != 0 ? new vector<PkObject*>( *params ) : new vector<PkObject*>();
   vector<PkObject*>* _locals = locals != 0 ? new vector<PkObject*>( *locals ) : new vector<PkObject*>();

   if ( params != 0 )
      theNewObj->_members[ "parameters" ] = makePkList( _params );
   else
      theNewObj->_members[ "parameters" ] = pkNull;

   if ( locals != 0 )
      theNewObj->_members[ "allLocals"  ] = makePkList( _locals );
   else
      theNewObj->_members[ "allLocals"  ] = pkNull;

   theNewObj->_environment                = _env;

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkExpr( std::vector<std::string>* aKeyList, std::vector<PkObject*>* anArgList )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   // Create the new object
   // Cat the keys to form the selector
   string sel = "";
   vector<PkObject*> keys;
   for ( vector<string>::iterator iter = aKeyList->begin(); iter != aKeyList->end(); iter++ )
      {
      sel.append( *iter );
      keys.push_back( makePkSymbol(iter->c_str()) );
      }

   // Construct the instance
   PkObject* theNewObj = new PkObject( pkExprClass );
   theNewObj->_valType = EXPRESSION_VAL;

   theNewObj->_members[ "selector"     ] = makePkSymbol( sel.c_str() );
   theNewObj->_members[ "keys"         ] = makePkList( &keys  );
   theNewObj->_members[ "arguments"    ] = makePkList( anArgList );

   // Store the new object
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

PkObject* PkObjectStore::makePkQuote( PkObject* val )
   {
   // Determine the new object's ID.
   int newObjectId = this->_newObjectId( );

   PkObject   *theNewObj = new PkObject( pkQuoteClass );
   theNewObj->_valType   = QUOTE_VAL;
   theNewObj->_val.quVal = val;
   return this->_objectArray[ newObjectId ] = theNewObj;
   //return newObjectId;
   }

/*
PkObject *const PkObjectStore::operator[]( unsigned objId ) const
   {
   return 0;
   }

PkObject*& PkObjectStore::operator[]( unsigned objId )
   {
   return 0;
   }
*/

PkObjectStore OBJECT;

void initializeEnvironment( )
   {
   pkObjectClass     = new PkObject( 0                );
   pkPrimitiveClass  = new PkObject( pkObjectClass    );
   pkNullClass       = new PkObject( pkObjectClass    );
   pkBooleanClass    = new PkObject( pkObjectClass    );
   pkMagnitudeClass  = new PkObject( pkObjectClass    );
   pkStringClass     = new PkObject( pkMagnitudeClass );
   pkNumberClass     = new PkObject( pkMagnitudeClass );
   pkIntegerClass    = new PkObject( pkNumberClass    );
   pkFloatClass      = new PkObject( pkNumberClass    );
   pkQuoteClass      = new PkObject( pkObjectClass    );
   pkSymbolClass     = new PkObject( pkObjectClass    );
   pkListClass       = new PkObject( pkObjectClass    );
   pkExprClass       = new PkObject( pkObjectClass    );

   OBJECT.makePkPrimitive( No_Op,                         "No_Op"                    );
   OBJECT.makePkPrimitive( No_Op1,                        "No_Op1"                   );
   OBJECT.makePkPrimitive( primitive__eq_,                "primitive__eq_"           );
   OBJECT.makePkPrimitive( primitive__evalForArgs_,       "primitive__evalForArgs_"  );
   OBJECT.makePkPrimitive( quote__eval,                   "quote__eval"              );
   OBJECT.makePkPrimitive( expr__eval,                    "expr__eval"               );

   OBJECT.makePkPrimitive( object__sameObjectAs_,         "object__sameObjectAs_" );
   OBJECT.makePkPrimitive( object__isKindOf_,             "object__isKindOf_" );
   OBJECT.makePkPrimitive( object__eq_,                   "object__eq_" );
   OBJECT.makePkPrimitive( object__ne_,                   "object__ne_" );
   OBJECT.makePkPrimitive( object__eval,                  "object__eval" );
   OBJECT.makePkPrimitive( object__evalMessage_,          "object__evalMessage_" );
   OBJECT.makePkPrimitive( object__member_,               "object__member_" );
   OBJECT.makePkPrimitive( object__memberRemove_,         "object__memberRemove_" );
   OBJECT.makePkPrimitive( object__member_setTo_,         "object__member_setTo_" );
   OBJECT.makePkPrimitive( object__listMembers,           "object__listMembers" );
   OBJECT.makePkPrimitive( object__make,                  "object__make" );
   OBJECT.makePkPrimitive( object__printValue,            "object__printValue" );
   OBJECT.makePkPrimitive( object__printMembers,          "object__printMembers" );
   OBJECT.makePkPrimitive( object__asString,              "object__asString" );
   OBJECT.makePkPrimitive( object__sameTypeAs_,           "object__sameTypeAs_" );

   OBJECT.makePkPrimitive( null__eq_,                     "null__eq_" );

   OBJECT.makePkPrimitive( object__delete,                "object__delete" );

   OBJECT.makePkPrimitive( boolean__eq_,                  "boolean__eq_" );
   OBJECT.makePkPrimitive( boolean__not,                  "boolean__not" );
   OBJECT.makePkPrimitive( boolean__and_,                 "boolean__and_" );
   OBJECT.makePkPrimitive( boolean__or_,                  "boolean__or_" );
   OBJECT.makePkPrimitive( boolean__ifTrue_,              "boolean__ifTrue_" );
   OBJECT.makePkPrimitive( boolean__ifTrue_ifFalse_,      "boolean__ifTrue_ifFalse_" );

   OBJECT.makePkPrimitive( magnitude__lt_,                "magnitude__lt_" );
   OBJECT.makePkPrimitive( magnitude__le_,                "magnitude__le_" );
   OBJECT.makePkPrimitive( magnitude__gt_,                "magnitude__gt_" );
   OBJECT.makePkPrimitive( magnitude__ge_,                "magnitude__ge_" );

   OBJECT.makePkPrimitive( string__eq_,                   "string__eq_" );
   OBJECT.makePkPrimitive( string__le_,                   "string__le_" );
   OBJECT.makePkPrimitive( string__length,                "string__length" );
   OBJECT.makePkPrimitive( string__at_,                   "string__at_" );
   OBJECT.makePkPrimitive( string__from_to_,              "string__from_to_" );
   OBJECT.makePkPrimitive( string__add_,                  "string__add_" );
   OBJECT.makePkPrimitive( string__forEachCharacter_,     "string__forEachCharacter_" );
   OBJECT.makePkPrimitive( string__parsePuckExpr,         "string__parsePuckExpr" );
   OBJECT.makePkPrimitive( string__asInteger,             "string__asInteger" );
   OBJECT.makePkPrimitive( string__asFloat,               "string__asFloat" );
   OBJECT.makePkPrimitive( string__format_,               "string__format_" );

   OBJECT.makePkPrimitive( number__eq_,                   "number__eq_" );
   OBJECT.makePkPrimitive( number__le_,                   "number__le_" );
   OBJECT.makePkPrimitive( number__asInteger,             "number__asInteger" );
   OBJECT.makePkPrimitive( number__asFloat,               "number__asFloat" );

   OBJECT.makePkPrimitive( integer__eq_,                  "integer__eq_" );
   OBJECT.makePkPrimitive( integer__le_,                  "integer__le_" );
   OBJECT.makePkPrimitive( integer__add_,                 "integer__add_" );
   OBJECT.makePkPrimitive( integer__sub_,                 "integer__sub_" );
   OBJECT.makePkPrimitive( integer__mul_,                 "integer__mul_" );
   OBJECT.makePkPrimitive( integer__div_,                 "integer__div_" );
   OBJECT.makePkPrimitive( integer__mod_,                 "integer__mod_" );
   OBJECT.makePkPrimitive( integer__doTimes_,             "integer__doTimes_" );
   OBJECT.makePkPrimitive( integer__asInteger,            "integer__asInteger" );
   OBJECT.makePkPrimitive( integer__asFloat,              "integer__asFloat"   );

   OBJECT.makePkPrimitive( float__eq_,                    "float__eq_" );
   OBJECT.makePkPrimitive( float__le_,                    "float__le_" );
   OBJECT.makePkPrimitive( float__add_,                   "float__add_" );
   OBJECT.makePkPrimitive( float__sub_,                   "float__sub_" );
   OBJECT.makePkPrimitive( float__mul_,                   "float__mul_" );
   OBJECT.makePkPrimitive( float__div_,                   "float__div_" );
   OBJECT.makePkPrimitive( float__asInteger,              "float__asInteger" );
   OBJECT.makePkPrimitive( float__asFloat,                "float__asFloat"   );

   OBJECT.makePkPrimitive( symbol__eq_,                   "symbol__eq_" );
   OBJECT.makePkPrimitive( symbol__eval,                  "symbol__eval" );
   OBJECT.makePkPrimitive( symbol__set_,                  "symbol__set_" );
   OBJECT.makePkPrimitive( symbol__undefine_,             "symbol__undefine_" );

   OBJECT.makePkPrimitive( list__eval,                    "list__eval" );
   OBJECT.makePkPrimitive( list__evalForArgs_,            "list__evalForArgs_" );
   OBJECT.makePkPrimitive( list__evalInPlace,             "list__evalInPlace" );
   OBJECT.makePkPrimitive( list__asExpr,                  "list__asExpr" );
   OBJECT.makePkPrimitive( list__eq_,                     "list__eq_" );
   OBJECT.makePkPrimitive( list__length,                  "list__length" );
   OBJECT.makePkPrimitive( list__at_,                     "list__at_" );
   OBJECT.makePkPrimitive( list__from_to_,                "list__from_to_" );
   OBJECT.makePkPrimitive( list__add_,                    "list__add_" );
   OBJECT.makePkPrimitive( list__append_,                 "list__append_" );
   OBJECT.makePkPrimitive( list__whileTrue_,              "list__whileTrue_" );
   OBJECT.makePkPrimitive( list__forEachItem_,            "list__forEachItem_" );

//#define No_Op                          0x00
//#define No_Op1                         0x01

   PkObject *temp;
   GLOBAL->set( "Object",     pkObjectClass );
   pkObjectClass->_members[ "name"          ] = OBJECT.makePkString( "Object" );
   pkObjectClass->_members[ "sameObjectAs:" ] = GLOBAL->get( "object__sameObjectAs_" );
   pkObjectClass->_members[ "isKindOf:"     ] = GLOBAL->get( "object__isKindOf_"     );
   pkObjectClass->_members[ "eval"          ] = GLOBAL->get( "object__eval"          );
   pkObjectClass->_members[ "="             ] = GLOBAL->get( "object__eq_"           );
   pkObjectClass->_members[ "!="            ] = GLOBAL->get( "object__ne_"           );
   pkObjectClass->_members[ "evalMessage:"  ] = GLOBAL->get( "object__evalMessage_"  );
   pkObjectClass->_members[ "member:"       ] = GLOBAL->get( "object__member_"       );
   pkObjectClass->_members[ "memberRemove:" ] = GLOBAL->get( "object__memberRemove_" );
   pkObjectClass->_members[ "member:setTo:" ] = GLOBAL->get( "object__member_setTo_" );
   pkObjectClass->_members[ "listMembers"   ] = GLOBAL->get( "object__listMembers"   );
   pkObjectClass->_members[ "make"          ] = GLOBAL->get( "object__make"          );
   pkObjectClass->_members[ "printValue"    ] = GLOBAL->get( "object__printValue"    );
   pkObjectClass->_members[ "printMembers"  ] = GLOBAL->get( "object__printMembers"  );
   pkObjectClass->_members[ "asString"      ] = GLOBAL->get( "object__asString"      );
   pkObjectClass->_members[ "sameTypeAs_"   ] = GLOBAL->get( "object__sameTypeAs_"   );
   pkObjectClass->_members[ "delete"        ] = GLOBAL->get( "object__delete"        );

   GLOBAL->set( "Primitive",  pkPrimitiveClass );
   pkPrimitiveClass->_members[ "name"         ] = OBJECT.makePkString( "Primitive" );
   pkPrimitiveClass->_members[ "="            ] = GLOBAL->get( "primitive__eq_" );
   pkPrimitiveClass->_members[ "evalForArgs:" ] = GLOBAL->get( "primitive__evalForArgs_" );

   GLOBAL->set( "Null",       pkNullClass );
   pkPrimitiveClass->_members[ "name"         ] = OBJECT.makePkString( "Null"                   );
   pkPrimitiveClass->_members[ "="            ] = GLOBAL->get( "null__eq_"               );

   GLOBAL->set( "Boolean",    pkBooleanClass );
   pkBooleanClass->_members[ "name"            ] = OBJECT.makePkString( "Boolean" );
   pkBooleanClass->_members[ "="               ] = GLOBAL->get( "boolean__eq_"             );
   pkBooleanClass->_members[ "not"             ] = GLOBAL->get( "boolean__not"             );
   pkBooleanClass->_members[ "and:"            ] = GLOBAL->get( "boolean__and_"            );
   pkBooleanClass->_members[ "or:"             ] = GLOBAL->get( "boolean__or_"             );
   pkBooleanClass->_members[ "ifTrue:"         ] = GLOBAL->get( "boolean__ifTrue_"         );
   pkBooleanClass->_members[ "ifTrue:ifFalse:" ] = GLOBAL->get( "boolean__ifTrue_ifFalse_" );

   GLOBAL->set( "Magnitude",  pkMagnitudeClass );
   pkMagnitudeClass->_members[ "name"       ] = OBJECT.makePkString( "Magnitude" );
   pkMagnitudeClass->_members[ "<"          ] = GLOBAL->get( "magnitude__lt_"             );
   pkMagnitudeClass->_members[ "<="         ] = GLOBAL->get( "magnitude__le_"             );
   pkMagnitudeClass->_members[ ">"          ] = GLOBAL->get( "magnitude__gt_"             );
   pkMagnitudeClass->_members[ ">="         ] = GLOBAL->get( "magnitude__ge_"             );

   GLOBAL->set( "String",  pkStringClass );
   pkStringClass->_members[ "name"              ] = OBJECT.makePkString( "String" );
   pkStringClass->_members[ "="                 ] = GLOBAL->get( "string__eq_"                );
   pkStringClass->_members[ "<="                ] = GLOBAL->get( "string__le_"                );
   pkStringClass->_members[ "length"            ] = GLOBAL->get( "string__length"             );
   pkStringClass->_members[ "at"                ] = GLOBAL->get( "string__at_"                );
   pkStringClass->_members[ "from:to:"          ] = GLOBAL->get( "string__from_to_"           );
   pkStringClass->_members[ "add:"              ] = GLOBAL->get( "string__add_"               );
   pkStringClass->_members[ "forEachCharacter:" ] = GLOBAL->get( "string__forEachCharacter_"  );
   pkStringClass->_members[ "parsePuckExpr"     ] = GLOBAL->get( "string__parsePuckExpr"      );
   pkStringClass->_members[ "asInteger"         ] = GLOBAL->get( "string__asInteger"          );
   pkStringClass->_members[ "asFloat"           ] = GLOBAL->get( "string__asFloat"            );
   pkStringClass->_members[ "asFormat:"         ] = GLOBAL->get( "string__format_"            );

   GLOBAL->set( "Number",  pkNumberClass );
   pkNumberClass->_members[ "name"       ] = OBJECT.makePkString( "Number" );
   pkNumberClass->_members[ "="          ] = GLOBAL->get( "number__eq_" );
   pkNumberClass->_members[ "asInteger"  ] = GLOBAL->get( "number__asInteger" );
   pkNumberClass->_members[ "asFloat"    ] = GLOBAL->get( "number__asFloat" );

   GLOBAL->set( "Integer",  pkIntegerClass );
   pkIntegerClass->_members[ "name"       ] = OBJECT.makePkString( "Integer" );
   pkIntegerClass->_members[ "="          ] = GLOBAL->get( "integer__eq_" );
   pkIntegerClass->_members[ "<="         ] = GLOBAL->get( "integer__le_" );
   pkIntegerClass->_members[ "+"          ] = GLOBAL->get( "integer__add_" );
   pkIntegerClass->_members[ "-"          ] = GLOBAL->get( "integer__sub_" );
   pkIntegerClass->_members[ "*"          ] = GLOBAL->get( "integer__mul_" );
   pkIntegerClass->_members[ "/"          ] = GLOBAL->get( "integer__div_" );
   pkIntegerClass->_members[ "%"          ] = GLOBAL->get( "integer__mod_" );
   pkIntegerClass->_members[ "doTimes:"   ] = GLOBAL->get( "integer__doTimes_" );
   pkIntegerClass->_members[ "asInteger"  ] = GLOBAL->get( "integer__asInteger" );
   pkIntegerClass->_members[ "asFloat"    ] = GLOBAL->get( "integer__asFloat" );

   GLOBAL->set( "Float", pkFloatClass );
   pkFloatClass->_members[ "name"         ] = OBJECT.makePkString( "Float" );
   pkFloatClass->_members[ "="            ] = GLOBAL->get( "float__eq_" );
   pkFloatClass->_members[ "<="           ] = GLOBAL->get( "float__le_" );
   pkFloatClass->_members[ "+"            ] = GLOBAL->get( "float__add_" );
   pkFloatClass->_members[ "-"            ] = GLOBAL->get( "float__sub_" );
   pkFloatClass->_members[ "*"            ] = GLOBAL->get( "float__mul_" );
   pkFloatClass->_members[ "/"            ] = GLOBAL->get( "float__div_" );
   pkFloatClass->_members[ "asInteger"    ] = GLOBAL->get( "float__asInteger" );
   pkFloatClass->_members[ "asFloat"      ] = GLOBAL->get( "float__asFloat" );

   GLOBAL->set( "Quote",     pkQuoteClass );
   pkQuoteClass->_members[ "name"           ] = OBJECT.makePkString( "Qutoe" );
   pkQuoteClass->_members[ "eval"           ] = GLOBAL->get( "quote__eval" );

   GLOBAL->set( "Symbol",     pkSymbolClass );
   pkSymbolClass->_members[ "name"          ] = OBJECT.makePkString( "Symbol" );
   pkSymbolClass->_members[ "="             ] = GLOBAL->get( "symbol__eq_"         );
   pkSymbolClass->_members[ "eval"          ] = GLOBAL->get( "symbol__eval"        );
   pkSymbolClass->_members[ "<-"            ] = GLOBAL->get( "symbol__set_"        );
   pkSymbolClass->_members[ "undefine:"     ] = GLOBAL->get( "symbol__undefine_"   );

   GLOBAL->set( "List",     pkListClass );
   pkListClass->_members[ "name"            ] = OBJECT.makePkString( "List"   );
   pkListClass->_members[ "eval"            ] = GLOBAL->get( "list__eval"  );
   pkListClass->_members[ "evalForArgs:"    ] = GLOBAL->get( "list__evalForArgs_"  );
   pkListClass->_members[ "evalInPlace"     ] = GLOBAL->get( "list__evalInPlace"   );
   pkListClass->_members[ "asExpr"          ] = GLOBAL->get( "list__asExpr"        );
   pkListClass->_members[ "="               ] = GLOBAL->get( "list__eq_"           );
   pkListClass->_members[ "length"          ] = GLOBAL->get( "list__length"        );
   pkListClass->_members[ "at:"             ] = GLOBAL->get( "list__at_"           );
   pkListClass->_members[ "from:to:"        ] = GLOBAL->get( "list__from_to_"      );
   pkListClass->_members[ "+"               ] = GLOBAL->get( "list__add_"          );
   pkListClass->_members[ "append:"         ] = GLOBAL->get( "list__append_"       );
   pkListClass->_members[ "whileTrue:"      ] = GLOBAL->get( "list__whileTrue_"    );
   pkListClass->_members[ "forEachItem:"    ] = GLOBAL->get( "list__forEachItem_"  );

   GLOBAL->set( "Expression", pkExprClass   );
   pkExprClass->_members[   "name"          ] = OBJECT.makePkString( "Expression" );
   pkExprClass->_members[   "eval"          ] = GLOBAL->get( "expr__eval"    );

   pkNull = new PkObject( pkNullClass );
   pkNull->_valType = NULL_VAL;
   GLOBAL->set( "null",   pkNull );

   pkTrue     = new PkObject( pkBooleanClass );
   pkTrue->_valType = BOOL_VAL;
   pkTrue->_val.blVal = true;
   GLOBAL->set( "true",   pkTrue );

   pkFalse    = new PkObject( pkBooleanClass );
   pkFalse->_valType = BOOL_VAL;
   pkFalse->_val.blVal = false;
   GLOBAL->set( "false", pkFalse );
   }
