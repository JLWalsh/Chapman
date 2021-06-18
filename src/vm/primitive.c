#include "primitive.h"

bool ch_primitive_isfalsy(const ch_primitive value) {
	switch(value.type) {
		case PRIMITIVE_BOOLEAN:
			return !value.boolean_value;
		case PRIMITIVE_NUMBER:
			return value.number_value == 0;
		case PRIMITIVE_NULL:
			return true;
		case PRIMITIVE_OBJECT:
			return ch_object_isfalsy(AS_OBJECT(value));
		case PRIMITIVE_CHAR: 
			return value.char_value == '\0';
		default:
			return false;
	}
}
