using System.Runtime.InteropServices;

namespace CreatorEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public float X;
        public float Y;

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        public static Vector2 Zero => new Vector2(0.0f, 0.0f);
        public float Length => (float)System.Math.Sqrt(X * X + Y * Y);

        public static Vector2 operator +(Vector2 left, Vector2 right) =>
            new Vector2(left.X + right.X, left.Y + right.Y);

        public static Vector2 operator -(Vector2 left, Vector2 right) =>
            new Vector2(left.X - right.X, left.Y - right.Y);

        public static Vector2 operator *(Vector2 value, float scalar) =>
            new Vector2(value.X * scalar, value.Y * scalar);

        public override string ToString() => $"({X}, {Y})";
    }
}
