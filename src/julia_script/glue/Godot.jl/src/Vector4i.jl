"""
A 4-element structure that can be used to represent 4D grid coordinates or any other quadruplet of integers.
"""
mutable struct Vector4i
    x::Int32
    y::Int32
    z::Int32
    w::Int32
end

# Display.

function Base.show(io::Core.IO, v::Vector4i)
    print(io, "Vector4i(", v.x, ", ", v.y, ", ", v.z, ", ", v.w, ")")
end

# Constructors.

Vector4i() = Vector4i(0, 0, 0, 0)

# Equality.

Base.:(==)(v1::Vector4i, v2::Vector4i) = v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w

# Indexing.

function Base.getindex(v::Vector4i, i::Int)
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

function Base.setindex!(v::Vector4i, entry, i)
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

Base.firstindex(v::Vector4i) = 1

Base.lastindex(v::Vector4i) = 4

# Arithmetic.

Base.:-(v::Vector4i) = Vector4i(-v.x, -v.y, -v.z, -v.w)

Base.:+(v::Vector4i) = Vector4i(v.x, v.y, v.z, v.w)

Base.:-(v1::Vector4i, v2::Vector4i) = Vector4i(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w)

function Base.:+(summands::Vector4i...)
    result = Vector4i(0, 0, 0, 0)
    for v in summands
        result.x += v.x
        result.y += v.y
        result.z += v.z
        result.w += v.w
    end
    return result
end

Base.:*(v::Vector4i, a::Number) = Vector4i(a*v.x, a*v.y, a*v.z, a*v.w)

Base.:*(a::Number, v::Vector4i) = Vector4i(a*v.x, a*v.y, a*v.z, a*v.w)

Base.:/(v::Vector4i, a::Number) = Vector4i(v.x/a, v.y/a, v.z/a, v.w/a)

Base.:\(a::Number, v::Vector4i) = Vector4i(v.x/a, v.y/a, v.z/a, v.w/a)
