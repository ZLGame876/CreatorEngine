using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public readonly struct InputActionContext
    {
        public InputActionContext(
            InputAction action,
            InputValue value,
            InputTriggerEvent triggerEvent,
            float deltaTime)
        {
            Action = action;
            Value = value;
            TriggerEvent = triggerEvent;
            DeltaTime = deltaTime;
        }

        public InputAction Action { get; }
        public InputValue Value { get; }
        public InputTriggerEvent TriggerEvent { get; }
        public float DeltaTime { get; }
    }

    public sealed class EnhancedInputComponent : Component
    {
        private sealed class Binding
        {
            public InputTriggerEvent TriggerEvent = InputTriggerEvent.Triggered;
            public Action<InputActionContext> Handler = _ => { };
        }

        private readonly Dictionary<InputAction, List<Binding>> _bindings =
            new Dictionary<InputAction, List<Binding>>();

        public void BindAction(
            InputAction action,
            InputTriggerEvent triggerEvent,
            Action<InputActionContext> handler)
        {
            if (!_bindings.TryGetValue(action, out List<Binding>? bindings))
            {
                bindings = new List<Binding>();
                _bindings.Add(action, bindings);
            }
            bindings.Add(new Binding { TriggerEvent = triggerEvent, Handler = handler });
        }

        public void ClearBindings() => _bindings.Clear();

        internal void Dispatch(InputActionContext context)
        {
            if (!_bindings.TryGetValue(context.Action, out List<Binding>? bindings))
            {
                return;
            }

            foreach (Binding binding in new List<Binding>(bindings))
            {
                if (binding.TriggerEvent == context.TriggerEvent)
                {
                    binding.Handler(context);
                }
            }
        }
    }

    public sealed class EnhancedInputSubsystem
    {
        private sealed class ContextEntry
        {
            public InputMappingContext Context = null!;
            public int Priority;
            public long Order;
        }

        private sealed class MappingState
        {
            public readonly Dictionary<InputTrigger, InputTriggerRuntimeState> Triggers =
                new Dictionary<InputTrigger, InputTriggerRuntimeState>();
        }

        private sealed class ActionState
        {
            public bool WasActive;
        }

        private sealed class ActionBucket
        {
            public InputValue Value;
            public InputTriggerState TriggerState;

            public ActionBucket(InputValue value)
            {
                Value = value;
            }
        }

        private readonly List<ContextEntry> _contexts = new List<ContextEntry>();
        private readonly Dictionary<InputMapping, MappingState> _mappingStates =
            new Dictionary<InputMapping, MappingState>();
        private readonly Dictionary<InputAction, ActionState> _actionStates =
            new Dictionary<InputAction, ActionState>();
        private long _nextOrder;

        public void AddMappingContext(InputMappingContext context, int priority = 0)
        {
            RemoveMappingContext(context);
            _contexts.Add(new ContextEntry
            {
                Context = context,
                Priority = priority,
                Order = _nextOrder++
            });
            _contexts.Sort((left, right) =>
            {
                int priorityOrder = right.Priority.CompareTo(left.Priority);
                return priorityOrder != 0 ? priorityOrder : left.Order.CompareTo(right.Order);
            });
        }

        public void RemoveMappingContext(InputMappingContext context)
        {
            _contexts.RemoveAll(entry => ReferenceEquals(entry.Context, context));
            foreach (InputMapping mapping in context.Mappings)
            {
                _mappingStates.Remove(mapping);
            }
        }

        public void ClearMappingContexts()
        {
            _contexts.Clear();
            _mappingStates.Clear();
            _actionStates.Clear();
        }

        public void ProcessInput(
            InputFrame frame,
            float deltaTime,
            EnhancedInputComponent target)
        {
            var buckets = new Dictionary<InputAction, ActionBucket>();
            var consumedKeys = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (ContextEntry entry in _contexts)
            {
                foreach (InputMapping mapping in entry.Context.Mappings)
                {
                    if (consumedKeys.Contains(mapping.Key))
                    {
                        continue;
                    }

                    InputValue value = frame.GetValue(mapping.Key);
                    foreach (InputModifier modifier in mapping.Modifiers)
                    {
                        value = modifier.Modify(value, deltaTime);
                    }
                    value = value.Coerce(mapping.Action.ValueType);

                    InputTriggerState triggerState = EvaluateTriggers(mapping, value, deltaTime);
                    if (!buckets.TryGetValue(mapping.Action, out ActionBucket? bucket))
                    {
                        bucket = new ActionBucket(InputValue.Zero(mapping.Action.ValueType));
                        buckets.Add(mapping.Action, bucket);
                    }
                    bucket.Value = InputValue.Accumulate(bucket.Value, value, mapping.Action.ValueType);
                    if (triggerState > bucket.TriggerState)
                    {
                        bucket.TriggerState = triggerState;
                    }

                    if (mapping.Action.ConsumesInput && value.IsActuated())
                    {
                        consumedKeys.Add(mapping.Key);
                    }
                }
            }

            foreach (KeyValuePair<InputAction, ActionBucket> pair in buckets)
            {
                DispatchAction(pair.Key, pair.Value, deltaTime, target);
            }

            foreach (KeyValuePair<InputAction, ActionState> pair in
                     new List<KeyValuePair<InputAction, ActionState>>(_actionStates))
            {
                if (!buckets.ContainsKey(pair.Key) && pair.Value.WasActive)
                {
                    target.Dispatch(new InputActionContext(
                        pair.Key,
                        InputValue.Zero(pair.Key.ValueType),
                        InputTriggerEvent.Completed,
                        deltaTime));
                    pair.Value.WasActive = false;
                }
            }
        }

        private InputTriggerState EvaluateTriggers(
            InputMapping mapping,
            InputValue value,
            float deltaTime)
        {
            MappingState state = GetMappingState(mapping);
            if (mapping.Triggers.Count == 0)
            {
                return value.IsActuated() ? InputTriggerState.Triggered : InputTriggerState.None;
            }

            InputTriggerState result = InputTriggerState.Triggered;
            foreach (InputTrigger trigger in mapping.Triggers)
            {
                if (!state.Triggers.TryGetValue(trigger, out InputTriggerRuntimeState? triggerState))
                {
                    triggerState = new InputTriggerRuntimeState();
                    state.Triggers.Add(trigger, triggerState);
                }

                InputTriggerState current = trigger.Update(value, deltaTime, triggerState);
                triggerState.WasActuated = value.IsActuated(trigger.ActuationThreshold);
                if (current < result)
                {
                    result = current;
                }
            }
            return result;
        }

        private MappingState GetMappingState(InputMapping mapping)
        {
            if (!_mappingStates.TryGetValue(mapping, out MappingState? state))
            {
                state = new MappingState();
                _mappingStates.Add(mapping, state);
            }
            return state;
        }

        private void DispatchAction(
            InputAction action,
            ActionBucket bucket,
            float deltaTime,
            EnhancedInputComponent target)
        {
            if (!_actionStates.TryGetValue(action, out ActionState? state))
            {
                state = new ActionState();
                _actionStates.Add(action, state);
            }

            bool active = bucket.Value.IsActuated() || bucket.TriggerState != InputTriggerState.None;
            if (active && !state.WasActive)
            {
                target.Dispatch(new InputActionContext(
                    action, bucket.Value, InputTriggerEvent.Started, deltaTime));
            }

            if (bucket.TriggerState == InputTriggerState.Triggered)
            {
                target.Dispatch(new InputActionContext(
                    action, bucket.Value, InputTriggerEvent.Triggered, deltaTime));
            }
            else if (bucket.TriggerState == InputTriggerState.Ongoing)
            {
                target.Dispatch(new InputActionContext(
                    action, bucket.Value, InputTriggerEvent.Ongoing, deltaTime));
            }
            else if (!active && state.WasActive)
            {
                target.Dispatch(new InputActionContext(
                    action, bucket.Value, InputTriggerEvent.Completed, deltaTime));
            }
            state.WasActive = active;
        }
    }
}
