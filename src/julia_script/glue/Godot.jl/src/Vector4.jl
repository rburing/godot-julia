include("generated/core_constants.jl")

"""
A 4-element structure that can be used to represent 4D coordinates or any other quadruplet of floating point values.
"""
mutable struct Vector4
    x::RealT
    y::RealT
    z::RealT
    w::RealT
end

# Display.

function Base.show(io::Core.IO, v::Vector4)
    print(io, "Vector4(", v.x, ", ", v.y, ", ", v.z, ", ", v.w, ")")
end

# Constructors.

Vector4() = Vector4(0, 0, 0, 0)

# Equality.

Base.:(==)(v1::Vector4, v2::Vector4) = v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w

# Indexing.

function Base.getindex(v::Vector4, i::Int)
    1 <= i <= 4 || Core.throw(Core.BoundsError(v, i))
    if i == 1
        return Core.getfield(v, :x)
    elseif i == 2
        return Core.getfield(v, :y)
    elseif i == 3
        return Core.getfield(v, :z)
    else
        return Core.getfield(v, :w)
    end
end

function Base.setindex!(v::Vector4, entry, i)
    1 <= i <= 4 || Core.throw(Core.BoundsError(v, i))
    if i == 1
        Core.setfield!(v, :x, entry)
    elseif i == 2
        Core.setfield!(v, :y, entry)
    elseif i == 3
        Core.setfield!(v, :z, entry)
    else
        Core.setfield!(v, :w, entry)
    end
end

Base.firstindex(v::Vector4) = 1

Base.lastindex(v::Vector4) = 4

# Arithmetic.

Base.:-(v::Vector4) = Vector4(-v.x, -v.y, -v.z, -v.w)

Base.:+(v::Vector4) = Vector4(v.x, v.y, v.z, v.w)

Base.:-(v1::Vector4, v2::Vector4) = Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w)

function Base.:+(summands::Vector4...)
    result = Vector4(0, 0, 0, 0)
    for v in summands
        result.x += v.x
        result.y += v.y
        result.z += v.z
        result.w += v.w
    end
    return result
end

Base.:*(v::Vector4, a::Number) = Vector4(a*v.x, a*v.y, a*v.z, a*v.w)

Base.:*(a::Number, v::Vector4) = Vector4(a*v.x, a*v.y, a*v.z, a*v.w)

Base.:/(v::Vector4, a::Number) = Vector4(v.x/a, v.y/a, v.z/a, v.w/a)

Base.:\(a::Number, v::Vector4) = Vector4(v.x/a, v.y/a, v.z/a, v.w/a)
