namespace CreatorEngine
{
    public enum InputTriggerState
    {
        None,
        Ongoing,
        Triggered
    }

    public enum InputTriggerEvent
    {
        Started,
        Ongoing,
        Triggered,
        Completed,
        Canceled
    }

    public sealed class InputTriggerRuntimeState
    {
        public bool WasActuated { get; set; }
        public float ElapsedSeconds { get; set; }
    }

    public abstract class InputTrigger
    {
        public float ActuationThreshold { get; set; } = 0.0001f;

        public abstract InputTriggerState Update(
            InputValue value,
            float deltaTime,
            InputTriggerRuntimeState state);
    }

    public sealed class InputTriggerPressed : InputTrigger
    {
        public override InputTriggerState Update(
            InputValue value, float deltaTime, InputTriggerRuntimeState state)
        {
            bool actuated = value.IsActuated(ActuationThreshold);
            return actuated && !state.WasActuated ? InputTriggerState.Triggered : InputTriggerState.None;
        }
    }

    public sealed class InputTriggerReleased : InputTrigger
    {
        public override InputTriggerState Update(
            InputValue value, float deltaTime, InputTriggerRuntimeState state)
        {
            bool actuated = value.IsActuated(ActuationThreshold);
            return !actuated && state.WasActuated ? InputTriggerState.Triggered : InputTriggerState.None;
        }
    }

    public sealed class InputTriggerHold : InputTrigger
    {
        public InputTriggerHold(float holdTime = 0.5f)
        {
            HoldTime = holdTime;
        }

        public float HoldTime { get; set; }
        public bool TriggerOnce { get; set; } = true;

        public override InputTriggerState Update(
            InputValue value, float deltaTime, InputTriggerRuntimeState state)
        {
            if (!value.IsActuated(ActuationThreshold))
            {
                state.ElapsedSeconds = 0.0f;
                return InputTriggerState.None;
            }

            state.ElapsedSeconds += deltaTime;
            if (state.ElapsedSeconds < HoldTime)
            {
                return InputTriggerState.Ongoing;
            }
            if (TriggerOnce && state.ElapsedSeconds > HoldTime + deltaTime)
            {
                return InputTriggerState.Ongoing;
            }
            return InputTriggerState.Triggered;
        }
    }

    public sealed class InputTriggerTap : InputTrigger
    {
        public InputTriggerTap(float maximumTapTime = 0.25f)
        {
            MaximumTapTime = maximumTapTime;
        }

        public float MaximumTapTime { get; set; }

        public override InputTriggerState Update(
            InputValue value, float deltaTime, InputTriggerRuntimeState state)
        {
            bool actuated = value.IsActuated(ActuationThreshold);
            if (actuated)
            {
                state.ElapsedSeconds += deltaTime;
                return state.ElapsedSeconds <= MaximumTapTime
                    ? InputTriggerState.Ongoing
                    : InputTriggerState.None;
            }

            bool tapped = state.WasActuated && state.ElapsedSeconds <= MaximumTapTime;
            state.ElapsedSeconds = 0.0f;
            return tapped ? InputTriggerState.Triggered : InputTriggerState.None;
        }
    }
}
