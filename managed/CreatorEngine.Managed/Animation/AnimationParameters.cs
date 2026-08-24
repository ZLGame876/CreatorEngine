using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public enum AnimationParameterType
    {
        Float,
        Integer,
        Boolean,
        Trigger
    }

    public sealed class AnimationParameterSet
    {
        private sealed class Parameter
        {
            public AnimationParameterType Type;
            public float FloatValue;
            public int IntegerValue;
            public bool BooleanValue;
        }

        private readonly Dictionary<string, Parameter> _parameters =
            new Dictionary<string, Parameter>(StringComparer.Ordinal);

        public void SetFloat(string name, float value) =>
            Set(name, AnimationParameterType.Float).FloatValue = value;

        public float GetFloat(string name, float fallback = 0.0f) =>
            TryGet(name, AnimationParameterType.Float, out Parameter? value) ? value!.FloatValue : fallback;

        public void SetInteger(string name, int value) =>
            Set(name, AnimationParameterType.Integer).IntegerValue = value;

        public int GetInteger(string name, int fallback = 0) =>
            TryGet(name, AnimationParameterType.Integer, out Parameter? value) ? value!.IntegerValue : fallback;

        public void SetBoolean(string name, bool value) =>
            Set(name, AnimationParameterType.Boolean).BooleanValue = value;

        public bool GetBoolean(string name, bool fallback = false) =>
            TryGet(name, AnimationParameterType.Boolean, out Parameter? value) ? value!.BooleanValue : fallback;

        public void SetTrigger(string name) =>
            Set(name, AnimationParameterType.Trigger).BooleanValue = true;

        public bool PeekTrigger(string name) =>
            TryGet(name, AnimationParameterType.Trigger, out Parameter? value) && value!.BooleanValue;

        public bool ConsumeTrigger(string name)
        {
            if (!TryGet(name, AnimationParameterType.Trigger, out Parameter? value) || !value!.BooleanValue)
            {
                return false;
            }
            value!.BooleanValue = false;
            return true;
        }

        public void ResetTrigger(string name)
        {
            if (TryGet(name, AnimationParameterType.Trigger, out Parameter? value))
            {
                value!.BooleanValue = false;
            }
        }

        private Parameter Set(string name, AnimationParameterType type)
        {
            if (_parameters.TryGetValue(name, out Parameter? parameter))
            {
                if (parameter.Type != type)
                {
                    throw new InvalidOperationException(
                        $"Animation parameter '{name}' is {parameter.Type}, not {type}.");
                }
                return parameter;
            }

            parameter = new Parameter { Type = type };
            _parameters.Add(name, parameter);
            return parameter;
        }

        private bool TryGet(string name, AnimationParameterType type, out Parameter? parameter)
        {
            if (_parameters.TryGetValue(name, out parameter) && parameter.Type == type)
            {
                return true;
            }
            parameter = null;
            return false;
        }
    }
}
