include("generated/core_constants.jl")

"""
A 3-element structure that can be used to represent 3D coordinates or any other triplet of floating point values.
"""
struct Vector3
    x::RealT
    y::RealT
    z::RealT
end

# Display.

function Base.show(io::Core.IO, v::Vector3)
    print(io, "Vector3(", v.x, ", ", v.y, ", ", v.z, ")")
end

# Constructors.

Vector3() = Vector3(0, 0, 0)

# Indexing.

function Base.getindex(v::Vector3, i::Int)
    1 <= i <= 3 || Core.throw(Core.BoundsError(v, i))
    if i == 1
        return Core.getfield(v, :x)
    elseif i == 2
        return Core.getfield(v, :y)
    else
        return Core.getfield(v, :z)
    end
end

Base.firstindex(v::Vector3) = 1

Base.lastindex(v::Vector3) = 3

# Arithmetic.

Base.:-(v::Vector3) = Vector3(-v.x, -v.y, -v.z)

Base.:+(v::Vector3) = Vector3(v.x, v.y, v.z)

Base.:-(v1::Vector3, v2::Vector3) = Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z)

function Base.:+(summands::Vector3...)
    sum_x::RealT = 0
    sum_y::RealT = 0
    sum_z::RealT = 0
    for v in summands
        sum_x += v.x
        sum_y += v.y
        sum_z += v.z
    end
    return Vector3(sum_x, sum_y, sum_z)
end

Base.:*(v::Vector3, a::Number) = Vector3(a*v.x, a*v.y, a*v.z)

Base.:*(a::Number, v::Vector3) = Vector3(a*v.x, a*v.y, a*v.z)

Base.:/(v::Vector3, a::Number) = Vector3(v.x/a, v.y/a, v.z/a)

Base.:\(a::Number, v::Vector3) = Vector3(v.x/a, v.y/a, v.z/a)
