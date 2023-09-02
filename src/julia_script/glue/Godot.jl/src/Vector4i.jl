"""
A 4-element structure that can be used to represent 4D grid coordinates or any other quadruplet of integers.
"""
struct Vector4i
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

Base.firstindex(v::Vector4i) = 1

Base.lastindex(v::Vector4i) = 4

# Arithmetic.

Base.:-(v::Vector4i) = Vector4i(-v.x, -v.y, -v.z, -v.w)

Base.:+(v::Vector4i) = Vector4i(v.x, v.y, v.z, v.w)

Base.:-(v1::Vector4i, v2::Vector4i) = Vector4i(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w)

function Base.:+(summands::Vector4i...)
    sum_x::Int32 = 0
    sum_y::Int32 = 0
    sum_z::Int32 = 0
    sum_w::Int32 = 0
    for v in summands
        sum_x += v.x
        sum_y += v.y
        sum_z += v.z
        sum_w += v.w
    end
    return Vector4i(sum_x, sum_y, sum_z, sum_w)
end

Base.:*(v::Vector4i, a::Number) = Vector4i(a*v.x, a*v.y, a*v.z, a*v.w)

Base.:*(a::Number, v::Vector4i) = Vector4i(a*v.x, a*v.y, a*v.z, a*v.w)

Base.:/(v::Vector4i, a::Number) = Vector4i(v.x/a, v.y/a, v.z/a, v.w/a)

Base.:\(a::Number, v::Vector4i) = Vector4i(v.x/a, v.y/a, v.z/a, v.w/a)
