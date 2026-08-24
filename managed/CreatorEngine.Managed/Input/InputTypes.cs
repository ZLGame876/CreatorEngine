using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public enum InputValueType
    {
        Boolean,
        Axis1D,
        Axis2D,
        Axis3D
    }

    public readonly struct InputValue
    {
        private readonly Vector3 _value;

        private InputValue(InputValueType type, Vector3 value)
        {
            Type = type;
            _value = value;
        }

        public InputValueType Type { get; }
        public float Magnitude => _value.Length;
        public bool IsActuated(float threshold = 0.0001f) => Magnitude > threshold;
        public bool AsBoolean() => _value.X > 0.5f;
        public float AsAxis1D() => _value.X;
        public Vector2 AsVector2() => new Vector2(_value.X, _value.Y);
        public Vector3 AsVector3() => _value;

        public static InputValue FromBoolean(bool value) =>
            new InputValue(InputValueType.Boolean, new Vector3(value ? 1.0f : 0.0f, 0.0f, 0.0f));

        public static InputValue FromAxis(float value) =>
            new InputValue(InputValueType.Axis1D, new Vector3(value, 0.0f, 0.0f));

        public static InputValue FromVector2(Vector2 value) =>
            new InputValue(InputValueType.Axis2D, new Vector3(value.X, value.Y, 0.0f));

        public static InputValue FromVector3(Vector3 value) =>
            new InputValue(InputValueType.Axis3D, value);

        public InputValue WithVector(Vector3 value) => new InputValue(Type, value);

        public InputValue Coerce(InputValueType type)
        {
            Vector3 value = _value;
            if (type == InputValueType.Boolean)
            {
                value = new Vector3(IsActuated() ? 1.0f : 0.0f, 0.0f, 0.0f);
            }
            else if (type == InputValueType.Axis1D)
            {
                value = new Vector3(value.X, 0.0f, 0.0f);
            }
            else if (type == InputValueType.Axis2D)
            {
                value = new Vector3(value.X, value.Y, 0.0f);
            }
            return new InputValue(type, value);
        }

        public static InputValue Accumulate(InputValue current, InputValue next, InputValueType type)
        {
            Vector3 sum = current._value + next._value;
            sum.X = Clamp(sum.X, -1.0f, 1.0f);
            sum.Y = Clamp(sum.Y, -1.0f, 1.0f);
            sum.Z = Clamp(sum.Z, -1.0f, 1.0f);
            return new InputValue(type, sum).Coerce(type);
        }

        public static InputValue Zero(InputValueType type) => new InputValue(type, Vector3.Zero);

        private static float Clamp(float value, float minimum, float maximum) =>
            value < minimum ? minimum : value > maximum ? maximum : value;
    }

    public sealed class InputFrame
    {
        private readonly Dictionary<string, InputValue> _values =
            new Dictionary<string, InputValue>(StringComparer.OrdinalIgnoreCase);

        public InputFrame Set(string key, InputValue value)
        {
            _values[key] = value;
            return this;
        }

        public InputFrame SetButton(string key, bool pressed) => Set(key, InputValue.FromBoolean(pressed));
        public InputFrame SetAxis(string key, float value) => Set(key, InputValue.FromAxis(value));

        public InputValue GetValue(string key) =>
            _values.TryGetValue(key, out InputValue value) ? value : InputValue.FromAxis(0.0f);
    }
}
