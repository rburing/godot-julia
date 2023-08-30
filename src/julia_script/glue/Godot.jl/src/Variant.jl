include("generated/core_constants.jl")

abstract type GodotVariant end

mutable struct Variant <: GodotVariant
	data::NTuple{VARIANT_SIZE, UInt8}
	Variant() = new(tuple(zeros(UInt8, VARIANT_SIZE)...))
end

variant_type(v::Variant) = VariantType(v.data[1])
