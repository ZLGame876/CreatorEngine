using System;

namespace CreatorEngine
{
    public abstract class InputModifier
    {
        public abstract InputValue Modify(InputValue value, float deltaTime);
    }

    public sealed class InputModifierDeadZone : InputModifier
    {
        public InputModifierDeadZone(float lowerThreshold = 0.1f, float upperThreshold = 1.0f)
        {
            LowerThreshold = lowerThreshold;
            UpperThreshold = upperThreshold;
        }

        public float LowerThreshold { get; set; }
        public float UpperThreshold { get; set; }

        public override InputValue Modify(InputValue value, float deltaTime)
        {
            float magnitude = value.Magnitude;
            if (magnitude <= LowerThreshold)
            {
                return InputValue.Zero(value.Type);
            }

            float range = Math.Max(UpperThreshold - LowerThreshold, 0.0001f);
            float normalized = Math.Min((magnitude - LowerThreshold) / range, 1.0f);
            return value.WithVector(value.AsVector3().Normalized * normalized);
        }
    }

    public sealed class InputModifierScalar : InputModifier
    {
        public InputModifierScalar(Vector3 scalar)
        {
            Scalar = scalar;
        }

        public Vector3 Scalar { get; set; }

        public override InputValue Modify(InputValue value, float deltaTime)
        {
            Vector3 current = value.AsVector3();
            return value.WithVector(new Vector3(
                current.X * Scalar.X,
                current.Y * Scalar.Y,
                current.Z * Scalar.Z));
        }
    }

    public sealed class InputModifierNegate : InputModifier
    {
        public InputModifierNegate(bool x = true, bool y = true, bool z = true)
        {
            NegateX = x;
            NegateY = y;
            NegateZ = z;
        }

        public bool NegateX { get; set; }
        public bool NegateY { get; set; }
        public bool NegateZ { get; set; }

        public override InputValue Modify(InputValue value, float deltaTime)
        {
            Vector3 current = value.AsVector3();
            return value.WithVector(new Vector3(
                NegateX ? -current.X : current.X,
                NegateY ? -current.Y : current.Y,
                NegateZ ? -current.Z : current.Z));
        }
    }

    public enum InputAxisOrder
    {
        XYZ,
        XZY,
        YXZ,
        YZX,
        ZXY,
        ZYX
    }

    public sealed class InputModifierSwizzle : InputModifier
    {
        public InputModifierSwizzle(InputAxisOrder order)
        {
            Order = order;
        }

        public InputAxisOrder Order { get; set; }

        public override InputValue Modify(InputValue value, float deltaTime)
        {
            Vector3 v = value.AsVector3();
            switch (Order)
            {
                case InputAxisOrder.XZY: return value.WithVector(new Vector3(v.X, v.Z, v.Y));
                case InputAxisOrder.YXZ: return value.WithVector(new Vector3(v.Y, v.X, v.Z));
                case InputAxisOrder.YZX: return value.WithVector(new Vector3(v.Y, v.Z, v.X));
                case InputAxisOrder.ZXY: return value.WithVector(new Vector3(v.Z, v.X, v.Y));
                case InputAxisOrder.ZYX: return value.WithVector(new Vector3(v.Z, v.Y, v.X));
                default: return value;
            }
        }
    }
}
