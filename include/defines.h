
#define     MAX_TOT_CATS       100      /* max number of total categories */

#define     MAX_TOT_PROPS      100      /* max number of total properties */

#define     MAX_CAT_STR         81      /* max length of each category name, including null terminator*/

#define     MAX_CAT_CHOICES     50      /* max number of choices per category */

#define     MAX_CHOICE_STR      81      /* max length of each choice name, including null terminator */

#define     MAX_CHOICE_PROPS    10      /* max number of properties per choice */

#define     MAX_PROP_STR        33      /* max length of each property name, including null terminator */

#define     MAX_TMP_STR        256      /* should be greater than all the other max string lengths and
                                           probably greater than the longest line expected in the file */


/* Expression flag constants */
#define     NOT_A       1           /* operand A should be complemented */
#define     NOT_B       2           /* operand B should be complemented */
#define     OP_AND      4           /* the AND operator should be applied */
#define     EXPR_A      8           /* operand A is another expression (not a property) */
#define     EXPR_B     16           /* operand B is another expression (not a property) */

/* logical constants */
#define     FALSE       0
#define     TRUE        1

/* Choice flag constants */
#define     IF_EXPR         1       /* an [if ...] expression is present */
#define     ELSE_EXPR       2       /* an [else] expression is present */

/* program flag constants */
#define     COUNT_ONLY      1       /* only count the frames, don't write them */
#define     STD_OUTPUT      2       /* output goes to std_out */
#define     OUTPUT_FILE     4       /* output goes to a specified file */

/* **** debug **** */
#define     DEBUG       FALSE       /* enable (TRUE) or disable (FALSE) debugging */


typedef unsigned short Flag;
typedef short          boolean;
