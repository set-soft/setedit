// -*- mode:C++; tab-width: 3 -*-
/**[txh]********************************************************************

  Copyright (c) 2004 by Grzegorz Adam Hankiewicz.
  This program is covered by the GPL license.

  Description:
  Parses a .py file looking for function and class definitions.
  It can be compiled as an standalone program by defining STANDALONE.

***************************************************************************/

#define Uses_stdio
#define Uses_ctype
#define Uses_string
//#define Uses_TVCodePage
#include <tv.h>

#include <bufun.h>
#include <assert.h>

#define STANDALONE

#define STATE_NOTHING               0
#define STATE_PARAGRAPH_QUOTE       1
#define STATE_CLASS                 2
#define STATE_FUNCTION              3

#define SYMBOL_EOL                  0
#define SYMBOL_COMMENT              1
#define SYMBOL_QUOTE_A              2
#define SYMBOL_QUOTE_B              3
#define SYMBOL_QUOTE_C              4
#define SYMBOL_WHITESPACE           5
#define SYMBOL_CLASS                6
#define SYMBOL_FUNCTION             7
#define SYMBOL_ESCAPE               8
#define SYMBOL_NULL                 9
#define SYMBOL_RUBBISH              10
#define SYMBOL_OPEN_BRACKET         11
#define SYMBOL_CLOSE_BRACKET        12
#define SYMBOL_COLON                13

class PARSE_STATE
{
public:
   int state;
   int line;
   char *text;
   char *text_end;

   PARSE_STATE(char *buffer, unsigned len)
   : state(STATE_NOTHING),
     line(1),
     text(buffer),
     text_end(buffer + len)
   { }

   PARSE_STATE(const PARSE_STATE &other)
   : state(other.state),
     line(other.line),
     text(other.text),
     text_end(other.text_end)
   { }
};

typedef struct {
   char text[MaxLenWith0];
   int len;
   int line_start;
   int line_end;
} PARSE_RESULT;

/**[txh]********************************************************************

  Description: Reads whatever logical symbol is found pointer by
  the text pointer. Pass the length of how much text can be read
  to discard long symbols and read past the end of the buffer.
  
  Return: One of the SYMBOL defines. As a special case, end of file
  (when length is less than 1) returns SYMBOL_EOL.
  
***************************************************************************/

static int read_symbol(const char *text, const int length)
{
   if (length < 1)
      return SYMBOL_EOL;
   
   switch (*text) {
      case 0: return SYMBOL_NULL;
      case ':': return SYMBOL_COLON;
      case '(': return SYMBOL_OPEN_BRACKET;
      case ')': return SYMBOL_CLOSE_BRACKET;
      case '\n': return SYMBOL_EOL;
      case '#': return SYMBOL_COMMENT;
      case '\'': return SYMBOL_QUOTE_A;
      case '"':
         if (length >= 3 && text[1] == '"' && text[2] == '"')
            return SYMBOL_QUOTE_C;
         else
            return SYMBOL_QUOTE_B;
      case ' ':
      case '\t':
      case '\r':
      case '\f':
      case '\v':
         return SYMBOL_WHITESPACE;
      case 'c':
         if (length >= 6 && text[1] == 'l' && text[2] == 'a' &&
               text[3] == 's' && text[4] == 's' &&
               read_symbol(text + 5, length - 5) == SYMBOL_WHITESPACE)
            return SYMBOL_CLASS;
         else
            return SYMBOL_RUBBISH;
      case 'd':
         if (length >= 4 && text[1] == 'e' && text[2] == 'f' &&
               read_symbol(text + 3, length - 3) == SYMBOL_WHITESPACE)
            return SYMBOL_FUNCTION;
         else
            return SYMBOL_RUBBISH;
      case '\\':
         if (length >= 2 && text[1] == '\\')
            return SYMBOL_RUBBISH;
         else
            return SYMBOL_ESCAPE;
      default:
         return SYMBOL_RUBBISH;
   }
}

/**[txh]********************************************************************

  Description: Advances the text pointer until the first instance
  of symbol, SYMBOL_EOL, or end of text buffer.
  
***************************************************************************/

void consume_line_until(PARSE_STATE &state, int symbol)
{
   while (state.text < state.text_end) {
      const int ret = read_symbol(state.text, state.text_end - state.text);
      if (ret == symbol || ret == SYMBOL_EOL)
         return;
      else
         state.text++;
   }
}

/**[txh]********************************************************************

  Description: Advances the text pointer until it doesn't point
  at whitespace.
  
***************************************************************************/

void consume_whitespace(PARSE_STATE &state)
{
   while (state.text < state.text_end) {
      if (*state.text == ' ' || *state.text == '\v' || *state.text == '\t' ||
            *state.text == '\f' || *state.text == '\r')
         state.text++;
      else
         return ;
   }
}

/**[txh]********************************************************************

  Description: Advances the text pointer while it points at some
  alphanumeric letter or underscore.
  
***************************************************************************/

void consume_alphanumeric(PARSE_STATE &state)
{
   while (state.text < state.text_end) {
      if (isalnum((unsigned)*state.text) || *state.text == '_')
         state.text++;
      else
         return ;
   }
}

/**[txh]********************************************************************

  Description: A mixed combination of other consumer helpers. This
  advances the text pointer while it looks at whitespace, opening
  or closing braces, commas, equal symbols and alphanumeric characters.
  
***************************************************************************/

void consume_until_colon(PARSE_STATE &state)
{
   while (state.text < state.text_end) {
      if (isalnum((unsigned)*state.text) || *state.text == '_' ||
            *state.text == ' ' || *state.text == '\v' ||
            *state.text == '\t' || *state.text == '\f' ||
            *state.text == '\r' || *state.text == '(' ||
            *state.text == ')' || *state.text == ',' ||
            *state.text == '=')
         state.text++;
      else
         return ;
   }
}

/**[txh]********************************************************************

  Description: Helper to extract_symbol(), this creates a duplicate parsing
  state to operate on the current line without disturbing the state of
  the parent function. The helper tries to verify that the current line is
  really pointing to a valid class line.
  
  Return: True if the line is valid and results were saved, false otherwise.
  
***************************************************************************/

static bool detect_class_name(PARSE_RESULT &result,
   const PARSE_STATE &parent_state)
{
   PARSE_STATE find_eol_state(parent_state);
   consume_line_until(find_eol_state, SYMBOL_EOL);
   const int length = find_eol_state.text - parent_state.text;
   if (length >= MaxLenWith0)
      return false;

   // Copy into result buffer.
   strncpy(result.text, parent_state.text, length);
   result.text[length] = 0;

   // Start parsing the result buffer.
   PARSE_STATE state(result.text, length);
   consume_alphanumeric(state);
   char *end_of_word = state.text;
   consume_until_colon(state);
   
   if (read_symbol(state.text, state.text_end - state.text) == SYMBOL_COLON) {
      *end_of_word = 0;
      strcat(result.text, " (class)");
      result.len = end_of_word - result.text + 8;
      result.line_start = parent_state.line;
      result.line_end = -1;
      return true;
   }

   return false;
}

/**[txh]********************************************************************

  Description: Helper to extract_symbol(), this creates a duplicate parsing
  state to operate on the current line without disturbing the state of
  the parent function. The helper tries to verify that the current line is
  really pointing to a valid function line.
  
  Return: True if the line is valid and results were saved, false otherwise.
  
***************************************************************************/

static bool detect_function_name(PARSE_RESULT &result,
   const PARSE_STATE &parent_state)
{
   PARSE_STATE find_eol_state(parent_state);
   consume_line_until(find_eol_state, SYMBOL_EOL);
   const int length = find_eol_state.text - parent_state.text;
   if (length >= MaxLenWith0)
      return false;

   // Copy into result buffer.
   strncpy(result.text, parent_state.text, length);
   result.text[length] = 0;

   // Start parsing the result buffer.
   PARSE_STATE state(result.text, length);
   consume_alphanumeric(state);
   consume_until_colon(state);
   
   if (read_symbol(state.text, state.text_end - state.text) == SYMBOL_COLON) {
      *state.text = 0;
      result.len = state.text - result.text;
      result.line_start = parent_state.line;
      result.line_end = -1;
      return true;
   }

   return false;
}

/**[txh]********************************************************************

  Description: Main function coordinating the parsing. This is
  basically the heart of the state machine. It works in an infinite
  loop until either a symbol is found or all the text has been
  processed.
  
  Return: False if the text has been processed, true if a symbol
  was found, in which case the results have been written in to the
  PARSE_RESULT structure.
  
***************************************************************************/

static bool extract_symbol(PARSE_STATE &state, PARSE_RESULT &result)
{
   bool ret;

   while (state.text < state.text_end) {
      if (read_symbol(state.text, 1) == SYMBOL_EOL) {
         state.text++;
         state.line++;
      }
      else switch (state.state) {
         case STATE_NOTHING:
            switch (read_symbol(state.text, state.text_end - state.text)) {
               case SYMBOL_RUBBISH:
               case SYMBOL_COMMENT:
               case SYMBOL_OPEN_BRACKET:
               case SYMBOL_CLOSE_BRACKET:
               case SYMBOL_COLON:
               case SYMBOL_NULL:
               case SYMBOL_QUOTE_A:
               case SYMBOL_QUOTE_B:
               case SYMBOL_ESCAPE:
                  consume_line_until(state, SYMBOL_EOL);
                  break;
               case SYMBOL_QUOTE_C:
                  state.text += 3;
                  state.state = STATE_PARAGRAPH_QUOTE;
                  break;
               case SYMBOL_WHITESPACE:
                  state.text++;
                  break;
               case SYMBOL_CLASS:
                  state.text += 5;
                  consume_whitespace(state);
                  if (read_symbol(state.text, 1) != SYMBOL_EOL)
                     state.state = STATE_CLASS;
                  break;
               case SYMBOL_FUNCTION:
                  state.text += 3;
                  consume_whitespace(state);
                  if (read_symbol(state.text, 1) != SYMBOL_EOL)
                     state.state = STATE_FUNCTION;
                  break;
               default:
                  assert(0 && "Shouldn't reach here");
                  break;
               }
            break;

         case STATE_PARAGRAPH_QUOTE:
            consume_line_until(state, SYMBOL_QUOTE_C);
            if (read_symbol(state.text, state.text_end - state.text) == SYMBOL_QUOTE_C) {
               state.text += 3;
               state.state = STATE_NOTHING;
            }
            break;

         case STATE_CLASS:
            ret = detect_class_name(result, state);
            consume_line_until(state, SYMBOL_EOL);
            state.state = STATE_NOTHING;
            if (ret)
               return ret;
            break;

         case STATE_FUNCTION:
            ret = detect_function_name(result, state);
            consume_line_until(state, SYMBOL_EOL);
            state.state = STATE_NOTHING;
            if (ret)
               return ret;
            break;

         default:
            assert(0 && "Shouldn't reach here");
            break;
      }
   }

   return false;
}

/**[txh]********************************************************************

  Description: Parent function of the parsing process. Given buffer
  of len length, it calls AddFunc for each found symbol.
  
  Return: Number of found symbols.
  
***************************************************************************/

int SearchPythonSymbols(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
   PARSE_STATE state(buffer, len);

   PARSE_RESULT result;

   int found_symbols = 0;

   while (extract_symbol(state, result)) {
      found_symbols++;
      AddFunc(result.text, result.len+1, result.line_start, result.line_end);
   }
   
   return found_symbols;
   mode = 0;
}

#ifdef STANDALONE_TEST
char bfBuffer[MaxLenWith0];
char bfNomFun[MaxLenWith0];
char bfTempNomFun[MaxLenWith0];

void PrintFunc(char *name, int len, int lineStart, int lineEnd)
{
   printf("`%s' [%d,%d:%d]\n", name, lineStart, lineEnd, len);
}

int main(int argc, char *argv[])
{
   printf("Python function/class parser, Copyright "
      "2004 by Grzegorz Adam Hankiewicz.\n");
   if (argc != 2) {
      printf("Use: ppython file\n");
      return 1;
   }
     
   FILE *f = fopen(argv[1], "rt");
   if (!f) {
      printf("Can't open %s\n", argv[1]);
      return 2;
   }
   
   fseek(f, 0, SEEK_END);
   long len = ftell(f);
   fseek(f, 0, SEEK_SET);
   char *buffer = new char[len + 1];
   long bread = fread(buffer, len, 1, f);
   
   if (bread != 1) {
      printf("Error reading file %s\n", argv[1]);
      perror("Error");
      fclose(f);
      return 3;
   }
   fclose(f);
   buffer[len]=0;
   SearchPythonSymbols(buffer, len, 0, PrintFunc);
  
   return 0;
}
#endif // STANDALONE_TEST
