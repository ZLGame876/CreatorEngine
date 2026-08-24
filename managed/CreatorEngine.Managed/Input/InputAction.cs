using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public sealed class InputAction : CreatorObject
    {
        public InputAction(string name, InputValueType valueType) : base(name)
        {
            ValueType = valueType;
        }

        public InputValueType ValueType { get; }
        public bool ConsumesInput { get; set; } = true;
    }

    public sealed class InputMapping
    {
        public InputMapping(string key, InputAction action)
        {
            Key = key ?? throw new ArgumentNullException(nameof(key));
            Action = action ?? throw new ArgumentNullException(nameof(action));
        }

        public string Key { get; }
        public InputAction Action { get; }
        public IList<InputModifier> Modifiers { get; } = new List<InputModifier>();
        public IList<InputTrigger> Triggers { get; } = new List<InputTrigger>();

        public InputMapping AddModifier(InputModifier modifier)
        {
            Modifiers.Add(modifier ?? throw new ArgumentNullException(nameof(modifier)));
            return this;
        }

        public InputMapping AddTrigger(InputTrigger trigger)
        {
            Triggers.Add(trigger ?? throw new ArgumentNullException(nameof(trigger)));
            return this;
        }
    }

    public sealed class InputMappingContext : CreatorObject
    {
        private readonly List<InputMapping> _mappings = new List<InputMapping>();

        public InputMappingContext(string name) : base(name) { }

        public IReadOnlyList<InputMapping> Mappings => _mappings;

        public InputMapping Map(string key, InputAction action)
        {
            var mapping = new InputMapping(key, action);
            _mappings.Add(mapping);
            return mapping;
        }
    }
}
