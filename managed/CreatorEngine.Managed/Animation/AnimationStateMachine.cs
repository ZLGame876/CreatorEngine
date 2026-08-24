using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public enum AnimationComparison
    {
        Equal,
        NotEqual,
        Greater,
        GreaterOrEqual,
        Less,
        LessOrEqual
    }

    public abstract class AnimationCondition
    {
        public abstract bool Evaluate(AnimationParameterSet parameters);
    }

    public sealed class FloatAnimationCondition : AnimationCondition
    {
        public FloatAnimationCondition(string parameter, AnimationComparison comparison, float threshold)
        {
            Parameter = parameter;
            Comparison = comparison;
            Threshold = threshold;
        }

        public string Parameter { get; }
        public AnimationComparison Comparison { get; }
        public float Threshold { get; }

        public override bool Evaluate(AnimationParameterSet parameters)
        {
            float value = parameters.GetFloat(Parameter);
            switch (Comparison)
            {
                case AnimationComparison.Equal: return Math.Abs(value - Threshold) < 0.0001f;
                case AnimationComparison.NotEqual: return Math.Abs(value - Threshold) >= 0.0001f;
                case AnimationComparison.Greater: return value > Threshold;
                case AnimationComparison.GreaterOrEqual: return value >= Threshold;
                case AnimationComparison.Less: return value < Threshold;
                case AnimationComparison.LessOrEqual: return value <= Threshold;
                default: return false;
            }
        }
    }

    public sealed class BooleanAnimationCondition : AnimationCondition
    {
        public BooleanAnimationCondition(string parameter, bool expected)
        {
            Parameter = parameter;
            Expected = expected;
        }

        public string Parameter { get; }
        public bool Expected { get; }
        public override bool Evaluate(AnimationParameterSet parameters) =>
            parameters.GetBoolean(Parameter) == Expected;
    }

    public sealed class TriggerAnimationCondition : AnimationCondition
    {
        public TriggerAnimationCondition(string parameter)
        {
            Parameter = parameter;
        }

        public string Parameter { get; }
        public override bool Evaluate(AnimationParameterSet parameters) => parameters.PeekTrigger(Parameter);
    }

    public sealed class AnimationState : CreatorObject
    {
        public AnimationState(string name, AnimationNode motion) : base(name)
        {
            Motion = motion;
        }

        public AnimationNode Motion { get; set; }
        public float NominalDurationSeconds { get; set; } = 1.0f;
    }

    public sealed class AnimationTransition
    {
        public AnimationTransition(string fromState, string toState)
        {
            FromState = fromState;
            ToState = toState;
        }

        public string FromState { get; }
        public string ToState { get; }
        public float DurationSeconds { get; set; } = 0.15f;
        public bool HasExitTime { get; set; }
        public float ExitTime { get; set; } = 0.9f;
        public IList<AnimationCondition> Conditions { get; } = new List<AnimationCondition>();

        public bool CanEnter(AnimationParameterSet parameters, float normalizedStateTime)
        {
            if (HasExitTime && normalizedStateTime < ExitTime)
            {
                return false;
            }
            foreach (AnimationCondition condition in Conditions)
            {
                if (!condition.Evaluate(parameters))
                {
                    return false;
                }
            }
            return true;
        }
    }

    public sealed class AnimationStateMachineNode : AnimationNode
    {
        private sealed class Runtime
        {
            public string CurrentState = string.Empty;
            public string PreviousState = string.Empty;
            public float StateTime;
            public float TransitionTime;
            public float TransitionDuration;
        }

        private readonly Dictionary<string, AnimationState> _states =
            new Dictionary<string, AnimationState>(StringComparer.Ordinal);
        private readonly List<AnimationTransition> _transitions = new List<AnimationTransition>();

        public AnimationStateMachineNode(string entryState) : base("Animation State Machine")
        {
            EntryState = entryState;
        }

        public string EntryState { get; set; }
        public IReadOnlyDictionary<string, AnimationState> States => _states;
        public IReadOnlyList<AnimationTransition> Transitions => _transitions;

        public void AddState(AnimationState state) => _states.Add(state.Name, state);
        public void AddTransition(AnimationTransition transition) => _transitions.Add(transition);

        public override AnimationPose Evaluate(AnimationEvaluationContext context)
        {
            Runtime runtime = context.GetRuntime(this, () => new Runtime { CurrentState = EntryState });
            if (!_states.TryGetValue(runtime.CurrentState, out AnimationState? current))
            {
                throw new InvalidOperationException($"Animation state '{runtime.CurrentState}' does not exist.");
            }

            runtime.StateTime += context.DeltaTime;
            float normalizedStateTime = runtime.StateTime / Math.Max(current.NominalDurationSeconds, 0.0001f);
            foreach (AnimationTransition transition in _transitions)
            {
                if (!string.Equals(transition.FromState, current.Name, StringComparison.Ordinal) ||
                    !transition.CanEnter(context.Parameters, normalizedStateTime))
                {
                    continue;
                }

                runtime.PreviousState = runtime.CurrentState;
                runtime.CurrentState = transition.ToState;
                runtime.StateTime = 0.0f;
                runtime.TransitionTime = 0.0f;
                runtime.TransitionDuration = Math.Max(transition.DurationSeconds, 0.0001f);
                ConsumeTransitionTriggers(transition, context.Parameters);
                current = _states[runtime.CurrentState];
                break;
            }

            AnimationPose currentPose = current.Motion.Evaluate(context);
            if (string.IsNullOrEmpty(runtime.PreviousState))
            {
                return currentPose;
            }

            runtime.TransitionTime += context.DeltaTime;
            float alpha = runtime.TransitionTime / runtime.TransitionDuration;
            if (alpha >= 1.0f || !_states.TryGetValue(runtime.PreviousState, out AnimationState? previous))
            {
                runtime.PreviousState = string.Empty;
                return currentPose;
            }
            return AnimationPose.Blend(previous.Motion.Evaluate(context), currentPose, alpha);
        }

        private static void ConsumeTransitionTriggers(
            AnimationTransition transition,
            AnimationParameterSet parameters)
        {
            foreach (AnimationCondition condition in transition.Conditions)
            {
                if (condition is TriggerAnimationCondition trigger)
                {
                    parameters.ConsumeTrigger(trigger.Parameter);
                }
            }
        }
    }
}
