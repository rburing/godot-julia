"""
A 3-element structure that can be used to represent 3D grid coordinates or any other triplet of integers.
"""
struct Vector3i
    x::Int32
    y::Int32
    z::Int32
end

# Display.

function Base.show(io::Core.IO, v::Vector3i)
    print(io, "Vector3i(", v.x, ", ", v.y, ", ", v.z, ")")
end

# Constructors.

Vector3i() = Vector3i(0, 0, 0)

# Indexing.

function Base.getindex(v::Vector3i, i::Int)
    1 <= i <= 3 || Core.throw(Core.BoundsError(v, i))
    if i == 1
        return Core.getfield(v, :x)
    elseif i == 2
        return Core.getfield(v, :y)
    else
        return Core.getfield(v, :z)
    end
end

Base.firstindex(v::Vector3i) = 1

Base.lastindex(v::Vector3i) = 3

# Arithmetic.

Base.:-(v::Vector3i) = Vector3i(-v.x, -v.y, -v.z)

Base.:+(v::Vector3i) = Vector3i(v.x, v.y, v.z)

Base.:-(v1::Vector3i, v2::Vector3i) = Vector3i(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z)

function Base.:+(summands::Vector3i...)
    sum_x::Int32 = 0
    sum_y::Int32 = 0
    sum_z::Int32 = 0
    for v in summands
        sum_x += v.x
        sum_y += v.y
        sum_z += v.z
    end
    return Vector3i(sum_x, sum_y, sum_z)
end

Base.:*(v::Vector3i, a::Number) = Vector3i(a*v.x, a*v.y, a*v.z)

Base.:*(a::Number, v::Vector3i) = Vector3i(a*v.x, a*v.y, a*v.z)

Base.:/(v::Vector3i, a::Number) = Vector3i(v.x/a, v.y/a, v.z/a)

Base.:\(a::Number, v::Vector3i) = Vector3i(v.x/a, v.y/a, v.z/a)
