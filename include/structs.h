
typedef struct property {
    char        name[ MAX_PROP_STR ];
    boolean     value;
} Property;

/* a structure representation of an expression */
typedef struct expression {
    Flag                flags;              /* condensed information about this expression */
    Property            *propA, *propB;     /* location of two Properties which can be left & right operands */
    struct expression   *exprA, *exprB;     /* location of two Expressions which can be left & right operands */
} Expression;

typedef struct choice {
    char        name[ MAX_CHOICE_STR ];     /* the string associated with this Choice */
    Flag        flags;                      /* condensed information about this Choice */
    Expression* if_expr;                    /* location of the [if ...] expression */
    Property    *props[ MAX_CHOICE_PROPS ], /* location of Properties before any expression */
                **if_props,                 /* points to location of Properties after the [if ...] */
                **else_props;               /* points to location of Properties after the [else] */
    short       num_props,                  /* the number of above Properties */
                num_if_props,
                num_else_props;
    char        *single,
                *if_single,
                *else_single;
} Choice;

typedef struct category {
    char        name[ MAX_CAT_STR ];        /* the string associated with this Category */
    Choice*     choices[ MAX_CAT_CHOICES ]; /* location of Choices for this Category */
    short       num_choices;
} Category;
