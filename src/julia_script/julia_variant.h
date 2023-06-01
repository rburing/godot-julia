#ifndef JULIA_VARIANT_H
#define JULIA_VARIANT_H

#include "core/variant/variant.h"

#include <julia.h>

jl_value_t *julia_value_from_variant(const Variant *p_variant) {
	jl_value_t *value = nullptr;
	switch (p_variant->get_type()) {
		case Variant::Type::NIL: {
			value = jl_nothing;
		} break;
		case Variant::Type::BOOL: {
			value = jl_box_bool(p_variant->operator bool());
		} break;
		case Variant::Type::INT: {
			value = jl_box_int64(p_variant->operator int64_t());
		} break;
		case Variant::Type::FLOAT: {
			value = jl_box_float64(p_variant->operator double());
		} break;
		// TODO: Handle other cases.
		default: {
			value = jl_nothing;
		}
	}
	return value;
}

Variant variant_from_julia_value(jl_value_t *p_value) {
	Variant variant;
	jl_datatype_t *type = (jl_datatype_t *)jl_typeof(p_value);
	if (type == jl_nothing_type) {
		// Nothing to do.
	} else if (type == jl_bool_type) {
		variant = Variant((bool)jl_unbox_bool(p_value));
	} else if (type == jl_int64_type) {
		variant = Variant(jl_unbox_int64(p_value));
	} else if (type == jl_float64_type) {
		variant = Variant(jl_unbox_float64(p_value));
	}
	// TODO: Handle other cases.
	return variant;
}

#endif // JULIA_VARIANT_H
