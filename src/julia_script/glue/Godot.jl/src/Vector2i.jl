"""
A 2-element structure that can be used to represent 2D grid coordinates or any other pair of integers.
"""
struct Vector2i
    x::Int32
    y::Int32
end

# Display.

function Base.show(io::Core.IO, v::Vector2i)
    print(io, "Vector2i(", v.x, ", ", v.y, ")")
end

# Constructors.

Vector2i() = Vector2i(0, 0)

# Indexing.

function Base.getindex(v::Vector2i, i::Int)
    1 <= i <= 2 || Core.throw(Core.BoundsError(v, i))
    if i == 1
        return Core.getfield(v, :x)
    else
        return Core.getfield(v, :y)
    end
end

Base.firstindex(v::Vector2i) = 1

Base.lastindex(v::Vector2i) = 2

# Arithmetic.

Base.:-(v::Vector2i) = Vector2i(-v.x, -v.y)

Base.:+(v::Vector2i) = Vector2i(v.x, v.y)

Base.:-(v1::Vector2i, v2::Vector2i) = Vector2i(v1.x - v2.x, v1.y - v2.y)

function Base.:+(summands::Vector2i...)
    sum_x::Int32 = 0
    sum_y::Int32 = 0
    for v in summands
        sum_x += v.x
        sum_y += v.y
    end
    return Vector2(sum_x, sum_y)
end

Base.:*(v::Vector2i, a::Number) = Vector2i(a*v.x, a*v.y)

Base.:*(a::Number, v::Vector2i) = Vector2i(a*v.x, a*v.y)

Base.:/(v::Vector2i, a::Number) = Vector2i(v.x/a, v.y/a)

Base.:\(a::Number, v::Vector2i) = Vector2i(v.x/a, v.y/a)
