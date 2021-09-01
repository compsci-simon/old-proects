/**
 * @file    valtypes.h
 * @brief   Value types for AMPL-2020 type checking.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2020-08-10
 */

#ifndef VALTYPES_H
#define VALTYPES_H

typedef enum valtype {
	TYPE_NONE     = 0,
	TYPE_ARRAY    = 1,
	TYPE_BOOLEAN  = 2,
	TYPE_INTEGER  = 4,
	TYPE_CALLABLE = 8
} ValType;

#define IS_ARRAY(type)          (IS_ARRAY_TYPE(type) && !IS_CALLABLE_TYPE(type))
#define IS_ARRAY_TYPE(type)     (BOOLEAN_VALUE(type & TYPE_ARRAY))
#define IS_BOOLEAN_TYPE(type)   (BOOLEAN_VALUE(type & TYPE_BOOLEAN))
#define IS_CALLABLE_TYPE(type)  (BOOLEAN_VALUE(type & TYPE_CALLABLE))
#define IS_FUNCTION(type)       (IS_CALLABLE_TYPE(type) && !IS_PROCEDURE(type))
#define IS_INTEGER_TYPE(type)   (BOOLEAN_VALUE(type & INTEGER_TYPE))
#define IS_PROCEDURE(type)      (BOOLEAN_VALUE(type & 15) //Check this again...
#define IS_VARIABLE(type)       /* TODO */

#define SET_AS_ARRAY(type)(     (type) |= TYPE_ARRAY)
#define SET_AS_CALLABLE(type)   /* TODO */
#define SET_BASE_TYPE(type)     ((type) &= ~TYPE_ARRAY)
#define SET_RETURN_TYPE(type)   /* TODO */

/**
 * Returns a string representation of the specified value type.
 *
 * @param[in]   type
 *     the type for which to return a string representation
 * @return      a string representation of the specified value type
 */
const char *get_valtype_string(ValType type);

#endif /* VALTYPES_H */
