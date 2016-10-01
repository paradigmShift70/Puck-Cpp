#ifndef OBJECT_HPP_INCLUDED
#define OBJECT_HPP_INCLUDED

#include <map>
#include <list>
#include <vector>
#include <string>
#include <exception>

// Puck Exceptions
class PuckException
   {
   public:
      PuckException( ): theMsg( ) { }
      PuckException( std::string const& msg ): theMsg( msg ) { }
      PuckException( PuckException const& exc ): theMsg( exc.theMsg ) { }

      const char* what( ) const { return theMsg.c_str(); }

   private:
      std::string theMsg;
   };

class VM_OpcodeNotImplemented
   {
   public:
      VM_OpcodeNotImplemented( ): theMsg( ) { }
      VM_OpcodeNotImplemented( std::string const& msg ): theMsg( msg ) { }
      VM_OpcodeNotImplemented( VM_OpcodeNotImplemented const& exc ): theMsg( exc.theMsg ) { }

      const char* what( ) const { return theMsg.c_str(); }

   private:
      std::string theMsg;
   };

// Forward Declarations
class PkEnvironment;
class PkObject;
class PkObjectStore;
class PkEvaluationStack;

// Convenient types
typedef std::map<std::string,PkObject*>  SymbolDefMap;
typedef std::pair<std::string,PkObject*> SymbolDefPair;
typedef std::vector<PkObject*>           PkList;

// values for PkObject::_valType
#define  NULL_VAL         '0'
#define  OBJECT_VAL       'o'           // 111
#define  BOOL_VAL         'b'
#define  SYMBOL_VAL       'y'           // 121
#define  MAGNITUDE_VAL    '>'
#define  NUMBER_VAL       '#'
#define  INTEGER_VAL      '9'
#define  FLOAT_VAL        '.'
#define  STRING_VAL       '"'
#define  QUOTE_VAL        '\''
#define  LIST_VAL         '&'           //  38
#define  PRIMITIVE_VAL    'a'           //  97
#define  EXPRESSION_VAL   'x'           // 120
#define  USER_VAL         'u'           // 117

class PkObject
   {
   public:
      PkObject( PkObject* aPkClass );
      ~PkObject( );

      PkObject* findMember( char const* aMemberName );

   public:
      unsigned char              _valType;
      union
         {
         bool                    blVal;    // Boolean
         char*                   syVal;    // Symbol
         unsigned char           prVal;    // Primitive
         long                    niVal;    // Number Integer
         float                   nfVal;    // Number float
         char*                   stVal;    // String
         PkObject*               quVal;    // Quote
         std::vector<PkObject*>* liVal;    // list
         void*                   obVal;    // Object
         } _val;

      PkObject*                  _class;
      PkEnvironment*             _environment;  // for list instances only
      SymbolDefMap               _members;

   public:
      void                       diag( int level=0 );
   };

std::string printableID( PkObject* anObj );
std::string printable( PkObject* anObj );

class PkEnvironment
   {
   public:
      PkEnvironment( PkEnvironment* parentEnv=0 );

      PkEnvironment*         parentEnv( );
      void                   resetLocal( );
      void                   declLocal( std::string const& aSymbol, PkObject* aValue=0 );
      void                   set( std::string const& aSymbol, PkObject* aValue=0 );
      PkObject*              get( std::string const& aSymbol );
      void                   saveSymTabCopy( );
      void                   restoreSymTabCopy( );

   private:
      void                   _set( std::string const& aSymbol, PkObject* aValue, PkEnvironment* localScope );

   private:
      PkEnvironment*         _parentEnv;
      SymbolDefMap           _symTab;

      static std::list<SymbolDefMap*>    _SAVE;
   };

class PkObjectStore
   {
   public:
      PkObjectStore( int size=1000 );
      ~PkObjectStore( );

      PkObject* makePkInteger( long long   val );
      PkObject* makePkInteger( char const* val, int length=-1 );
      PkObject* makePkFloat(   float       val );
      PkObject* makePkFloat(   char const* val, int length=-1 );
      PkObject* makePkSymbol(  char const* val, int length=-1 );
      PkObject* makePkString(  char const* val, int length=-1 );
      PkObject* makePkPrimitive( unsigned char opCode, char const* opCodeStr=0 );
      PkObject* makePkList( std::vector<PkObject*>* items, PkEnvironment* anEnv=0, std::vector<PkObject*>* params=0, std::vector<PkObject*>* locals=0 );
      PkObject* makePkExpr( std::vector<std::string>* aKeyList, std::vector<PkObject*>* anArgList );
      PkObject* makePkQuote( PkObject* val );

      PkObject *const operator[]( unsigned objId ) const;
      PkObject*& operator[]( unsigned objId );
/*
      int         valueTypeAt( unsigned objId ) const;

      long long&  integerAt( unsigned objId );
      long long   integerAt( unsigned objId ) const;

      float&      floatAt( unsigned objId );
      float       floatAt( unsigned objId ) const;

      char const*&      stringAt( unsigned objId );
      char const *const stringAt( unsigned objId ) const;
*/
   protected:
      int        _newObjectId( );

   protected:
      PkObject** _objectArray;
      int        _objectIdLimit;
   };

// ***************
// Virtual Machine
void initializeEnvironment( );

// Evaluation Functions
PkObject* EVAL_OBJ_R( PkObject* anObj, PkEnvironment* anEnv, bool isLValue=false );
void EVAL_EXPR( PkObject* anObj, char const* aSel, std::vector<PkObject*>& anArgList, PkEnvironment* anEnv, bool isLValue=false );
void STACK_EVAL( char const* aSel, int anArgCt, PkEnvironment* anEnv, bool isLValue=false );
void evalPrimitive( unsigned char opCode, PkEnvironment* anEnv, int numArgs, bool isLValue=false );

/* ### VM OP CODES ## */
#define No_Op                          0x00
#define No_Op1                         0x01

#define primitive__eq_                 0x03
#define primitive__evalForArgs_        0x04

#define quote__eval                    0x08

#define expr__eval                     0x0E

#define object__sameObjectAs_          0x20
#define object__isKindOf_              0x21
#define object__eq_                    0x22
#define object__ne_                    0x23
#define object__eval                   0x24
#define object__evalMessage_           0x25
#define object__member_                0x26
#define object__memberRemove_          0x27
#define object__member_setTo_          0x28
#define object__listMembers            0x29
#define object__make                   0x2A
#define object__printValue             0x2C
#define object__printMembers           0x2D
#define object__asString               0x2E
#define object__sameTypeAs_            0x2F
#define null__eq_                      object__sameTypeAs_

#define object__delete                 0x3F

#define boolean__eq_                   0x50
#define boolean__not                   0x51
#define boolean__and_                  0x52
#define boolean__or_                   0x53
#define boolean__ifTrue_               0x54
#define boolean__ifTrue_ifFalse_       0x55

#define magnitude__lt_                 0x60
#define magnitude__le_                 0x61
#define magnitude__gt_                 0x62
#define magnitude__ge_                 0x63

#define string__eq_                    0x70
#define string__le_                    0x71
#define string__length                 0x72
#define string__at_                    0x73
#define string__from_to_               0x74
#define string__add_                   0x75
#define string__forEachCharacter_      0x76
#define string__parsePuckExpr          0x77
#define string__asInteger              0x78
#define string__asFloat                0x79
#define string__format_                0x7A    // "My name is {0}." format: #[ "Ron" ]

#define number__eq_                    0x80
#define number__le_                    0x81
#define number__asInteger              0x86
#define number__asFloat                0x87

#define integer__eq_                   0x90
#define integer__le_                   0x91
#define integer__add_                  0x92
#define integer__sub_                  0x93
#define integer__mul_                  0x94
#define integer__div_                  0x95
#define integer__mod_                  0x96
#define integer__doTimes_              0x97
#define integer__asInteger             0x98
#define integer__asFloat               0x99

#define float__eq_                     0xA0
#define float__le_                     0xA1
#define float__add_                    0xA2
#define float__sub_                    0xA3
#define float__mul_                    0xA4
#define float__div_                    0xA5
#define float__asInteger               0xA6
#define float__asFloat                 0xA7

#define symbol__eq_                    0xB0
#define symbol__eval                   0xB1
#define symbol__set_                   0xB2
#define symbol__undefine_              0xB3

#define list__eval                     0xC0
#define list__evalForArgs_             0xC1
#define list__evalInPlace              0xC2
#define list__asExpr                   0xC3
#define list__eq_                      0xC4
#define list__length                   0xC5
#define list__at_                      0xC6
#define list__from_to_                 0xC7
#define list__add_                     0xC8
#define list__append_                  0xC9
#define list__whileTrue_               0xCA
#define list__forEachItem_             0xCB

// VM Components
extern PkEnvironment* GLOBAL;
extern PkObjectStore OBJECT;

// PKObjectClass bases used to instantiate PkObject
extern PkObject* pkObjectClass;
extern PkObject* pkPrimitiveClass;
extern PkObject* pkNullClass;
extern PkObject* pkBooleanClass;
extern PkObject* pkMagnitudeClass;
extern PkObject* pkStringClass;
extern PkObject* pkNumberClass;
extern PkObject* pkIntegerClass;
extern PkObject* pkFloatClass;
extern PkObject* pkQuoteClass;
extern PkObject* pkSymbolClass;
extern PkObject* pkListClass;
extern PkObject* pkExprClass;

extern PkObject* pkNull;
extern PkObject* pkTrue;
extern PkObject* pkFalse;


#endif // OBJECT_HPP_INCLUDED
