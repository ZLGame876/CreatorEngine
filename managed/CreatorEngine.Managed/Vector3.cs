using System.Runtime.InteropServices;

namespace CreatorEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float X;
        public float Y;
        public float Z;

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static Vector3 Zero => new Vector3(0.0f, 0.0f, 0.0f);
        public static Vector3 One => new Vector3(1.0f, 1.0f, 1.0f);

        public float Length => (float)System.Math.Sqrt(X * X + Y * Y + Z * Z);

        public Vector3 Normalized
        {
            get
            {
                float length = Length;
                return length > 0.000001f ? this / length : Zero;
            }
        }

        public static Vector3 operator +(Vector3 left, Vector3 right) =>
            new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);

        public static Vector3 operator -(Vector3 left, Vector3 right) =>
            new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);

        public static Vector3 operator *(Vector3 value, float scalar) =>
            new Vector3(value.X * scalar, value.Y * scalar, value.Z * scalar);

        public static Vector3 operator /(Vector3 value, float scalar) =>
            new Vector3(value.X / scalar, value.Y / scalar, value.Z / scalar);

        public override string ToString() => $"({X}, {Y}, {Z})";
    }
}
