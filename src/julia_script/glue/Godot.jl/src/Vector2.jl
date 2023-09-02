include("generated/core_constants.jl")

"""
A 2-element structure that can be used to represent 2D coordinates or any other pair of floating point values.
"""
struct Vector2
    x::RealT
    y::RealT
end

# Display.

function Base.show(io::Core.IO, v::Vector2)
    print(io, "Vector2(", v.x, ", ", v.y, ")")
end

# Constructors.

Vector2() = Vector2(0, 0)

# Indexing.

function Base.getindex(v::Vector2, i::Int)
    1 <= i <= 2 || Core.throw(Core.BoundsError(v, i))
    if i == 1
        return Core.getfield(v, :x)
    else
        return Core.getfield(v, :y)
    end
end

Base.firstindex(v::Vector2) = 1

Base.lastindex(v::Vector2) = 2

# Arithmetic.

Base.:-(v::Vector2) = Vector2(-v.x, -v.y)

Base.:+(v::Vector2) = Vector2(v.x, v.y)

Base.:-(v1::Vector2, v2::Vector2) = Vector2(v1.x - v2.x, v1.y - v2.y)

function Base.:+(summands::Vector2...)
    sum_x::RealT = 0
    sum_y::RealT = 0
    for v in summands
        sum_x += v.x
        sum_y += v.y
    end
    return Vector2(sum_x, sum_y)
end

Base.:*(v::Vector2, a::Number) = Vector2(a*v.x, a*v.y)

Base.:*(a::Number, v::Vector2) = Vector2(a*v.x, a*v.y)

Base.:/(v::Vector2, a::Number) = Vector2(v.x/a, v.y/a)

Base.:\(a::Number, v::Vector2) = Vector2(v.x/a, v.y/a)
