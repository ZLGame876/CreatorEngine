using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public sealed class AnimationClip : CreatorObject
    {
        public AnimationClip(string name, float durationSeconds, bool loop = true) : base(name)
        {
            DurationSeconds = Math.Max(durationSeconds, 0.0001f);
            Loop = loop;
        }

        public float DurationSeconds { get; }
        public bool Loop { get; set; }
    }

    public readonly struct WeightedAnimationClip
    {
        public WeightedAnimationClip(AnimationClip clip, float weight, float normalizedTime)
        {
            Clip = clip;
            Weight = weight;
            NormalizedTime = normalizedTime;
        }

        public AnimationClip Clip { get; }
        public float Weight { get; }
        public float NormalizedTime { get; }
    }

    public sealed class AnimationPose
    {
        private readonly List<WeightedAnimationClip> _clips = new List<WeightedAnimationClip>();

        public IReadOnlyList<WeightedAnimationClip> Clips => _clips;

        public void Add(AnimationClip clip, float weight, float normalizedTime)
        {
            if (weight > 0.0001f)
            {
                _clips.Add(new WeightedAnimationClip(clip, weight, normalizedTime));
            }
        }

        public static AnimationPose Blend(AnimationPose from, AnimationPose to, float alpha)
        {
            alpha = Clamp01(alpha);
            var result = new AnimationPose();
            foreach (WeightedAnimationClip clip in from.Clips)
            {
                result.Add(clip.Clip, clip.Weight * (1.0f - alpha), clip.NormalizedTime);
            }
            foreach (WeightedAnimationClip clip in to.Clips)
            {
                result.Add(clip.Clip, clip.Weight * alpha, clip.NormalizedTime);
            }
            return result;
        }

        private static float Clamp01(float value) => value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
    }

    public sealed class AnimationEvaluationContext
    {
        private readonly AnimationGraphInstance _instance;

        internal AnimationEvaluationContext(AnimationGraphInstance instance, float deltaTime)
        {
            _instance = instance;
            DeltaTime = deltaTime;
        }

        public AnimationParameterSet Parameters => _instance.Parameters;
        public float DeltaTime { get; }

        internal T GetRuntime<T>(AnimationNode node, Func<T> factory) where T : class =>
            _instance.GetRuntime(node, factory);
    }

    public abstract class AnimationNode : CreatorObject
    {
        protected AnimationNode(string? name = null) : base(name) { }
        public abstract AnimationPose Evaluate(AnimationEvaluationContext context);
    }

    public sealed class AnimationClipNode : AnimationNode
    {
        private sealed class Runtime
        {
            public float Time;
        }

        public AnimationClipNode(AnimationClip clip) : base(clip.Name)
        {
            Clip = clip;
        }

        public AnimationClip Clip { get; }
        public float PlayRate { get; set; } = 1.0f;

        public override AnimationPose Evaluate(AnimationEvaluationContext context)
        {
            Runtime runtime = context.GetRuntime(this, () => new Runtime());
            runtime.Time += context.DeltaTime * PlayRate;
            float normalized = runtime.Time / Clip.DurationSeconds;
            normalized = Clip.Loop ? normalized - (float)Math.Floor(normalized) : Math.Min(normalized, 1.0f);

            var pose = new AnimationPose();
            pose.Add(Clip, 1.0f, normalized);
            return pose;
        }
    }

    public sealed class BlendSpace1DSample
    {
        public BlendSpace1DSample(float position, AnimationNode motion)
        {
            Position = position;
            Motion = motion;
        }

        public float Position { get; }
        public AnimationNode Motion { get; }
    }

    public sealed class BlendSpace1DNode : AnimationNode
    {
        private readonly List<BlendSpace1DSample> _samples = new List<BlendSpace1DSample>();

        public BlendSpace1DNode(string parameterName) : base("Blend Space 1D")
        {
            ParameterName = parameterName;
        }

        public string ParameterName { get; }
        public IReadOnlyList<BlendSpace1DSample> Samples => _samples;

        public void AddSample(float position, AnimationNode motion)
        {
            _samples.Add(new BlendSpace1DSample(position, motion));
            _samples.Sort((left, right) => left.Position.CompareTo(right.Position));
        }

        public override AnimationPose Evaluate(AnimationEvaluationContext context)
        {
            if (_samples.Count == 0)
            {
                return new AnimationPose();
            }
            if (_samples.Count == 1)
            {
                return _samples[0].Motion.Evaluate(context);
            }

            float value = context.Parameters.GetFloat(ParameterName);
            BlendSpace1DSample lower = _samples[0];
            BlendSpace1DSample upper = _samples[_samples.Count - 1];
            for (int i = 1; i < _samples.Count; ++i)
            {
                if (value <= _samples[i].Position)
                {
                    lower = _samples[i - 1];
                    upper = _samples[i];
                    break;
                }
            }

            float range = Math.Max(upper.Position - lower.Position, 0.0001f);
            float alpha = (value - lower.Position) / range;
            return AnimationPose.Blend(
                lower.Motion.Evaluate(context),
                upper.Motion.Evaluate(context),
                alpha);
        }
    }

    public sealed class BlendSpace2DSample
    {
        public BlendSpace2DSample(Vector2 position, AnimationNode motion)
        {
            Position = position;
            Motion = motion;
        }

        public Vector2 Position { get; }
        public AnimationNode Motion { get; }
    }

    public sealed class BlendSpace2DNode : AnimationNode
    {
        private readonly List<BlendSpace2DSample> _samples = new List<BlendSpace2DSample>();

        public BlendSpace2DNode(string xParameter, string yParameter) : base("Blend Space 2D")
        {
            XParameter = xParameter;
            YParameter = yParameter;
        }

        public string XParameter { get; }
        public string YParameter { get; }
        public IReadOnlyList<BlendSpace2DSample> Samples => _samples;

        public void AddSample(Vector2 position, AnimationNode motion) =>
            _samples.Add(new BlendSpace2DSample(position, motion));

        public override AnimationPose Evaluate(AnimationEvaluationContext context)
        {
            if (_samples.Count == 0)
            {
                return new AnimationPose();
            }

            var point = new Vector2(
                context.Parameters.GetFloat(XParameter),
                context.Parameters.GetFloat(YParameter));
            var distances = new List<KeyValuePair<BlendSpace2DSample, float>>();
            foreach (BlendSpace2DSample sample in _samples)
            {
                float distance = (sample.Position - point).Length;
                if (distance < 0.0001f)
                {
                    return sample.Motion.Evaluate(context);
                }
                distances.Add(new KeyValuePair<BlendSpace2DSample, float>(sample, distance));
            }
            distances.Sort((left, right) => left.Value.CompareTo(right.Value));

            int count = Math.Min(3, distances.Count);
            float total = 0.0f;
            for (int i = 0; i < count; ++i)
            {
                total += 1.0f / distances[i].Value;
            }

            var result = new AnimationPose();
            for (int i = 0; i < count; ++i)
            {
                float weight = (1.0f / distances[i].Value) / total;
                AnimationPose samplePose = distances[i].Key.Motion.Evaluate(context);
                foreach (WeightedAnimationClip clip in samplePose.Clips)
                {
                    result.Add(clip.Clip, clip.Weight * weight, clip.NormalizedTime);
                }
            }
            return result;
        }
    }

    public sealed class AnimationGraph : CreatorObject
    {
        public AnimationGraph(string name, AnimationNode outputNode) : base(name)
        {
            OutputNode = outputNode;
        }

        public AnimationNode OutputNode { get; set; }
    }

    public sealed class AnimationGraphInstance
    {
        private readonly Dictionary<AnimationNode, object> _runtime =
            new Dictionary<AnimationNode, object>();

        public AnimationGraphInstance(AnimationGraph graph)
        {
            Graph = graph;
        }

        public AnimationGraph Graph { get; }
        public AnimationParameterSet Parameters { get; } = new AnimationParameterSet();

        public AnimationPose Evaluate(float deltaTime) =>
            Graph.OutputNode.Evaluate(new AnimationEvaluationContext(this, deltaTime));

        public void SetSpeed(float speed) => Parameters.SetFloat("Speed", speed);
        public void SetFloat(string name, float value) => Parameters.SetFloat(name, value);
        public void SetBoolean(string name, bool value) => Parameters.SetBoolean(name, value);
        public void SetTrigger(string name) => Parameters.SetTrigger(name);

        internal T GetRuntime<T>(AnimationNode node, Func<T> factory) where T : class
        {
            if (_runtime.TryGetValue(node, out object? value))
            {
                return (T)value;
            }

            T runtime = factory();
            _runtime.Add(node, runtime);
            return runtime;
        }
    }

    public sealed class AnimatorComponent : Component
    {
        public AnimationGraph? Graph { get; private set; }
        public AnimationGraphInstance? Instance { get; private set; }
        public AnimationPose? CurrentPose { get; private set; }

        public void SetGraph(AnimationGraph graph)
        {
            Graph = graph ?? throw new ArgumentNullException(nameof(graph));
            Instance = new AnimationGraphInstance(graph);
        }

        public void Tick(float deltaTime)
        {
            if (Enabled && Instance != null)
            {
                CurrentPose = Instance.Evaluate(deltaTime);
            }
        }

        public void SetSpeed(float speed) => Instance?.SetSpeed(speed);
        public void SetFloat(string name, float value) => Instance?.SetFloat(name, value);
        public void SetBoolean(string name, bool value) => Instance?.SetBoolean(name, value);
        public void SetTrigger(string name) => Instance?.SetTrigger(name);
    }
}
