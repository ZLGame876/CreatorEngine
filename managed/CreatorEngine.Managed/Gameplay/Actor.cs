using System;

namespace CreatorEngine
{
    public class Actor : GameObject
    {
        private bool _hasBegunPlay;

        public Actor(string? name = null) : base(name) { }

        public World? World { get; internal set; }
        public bool HasBegunPlay => _hasBegunPlay;

        public virtual void BeginPlay() { }

        public virtual void Tick(float deltaTime)
        {
            foreach (MonoBehaviour script in GetComponents<MonoBehaviour>())
            {
                if (script.Enabled)
                {
                    script.Update(deltaTime);
                }
            }
            GetComponent<AnimatorComponent>()?.Tick(deltaTime);
        }

        public virtual void EndPlay()
        {
            foreach (MonoBehaviour script in GetComponents<MonoBehaviour>())
            {
                script.OnDestroy();
            }
        }

        internal void BeginPlayInternal()
        {
            if (_hasBegunPlay || IsDestroyed)
            {
                return;
            }

            _hasBegunPlay = true;
            foreach (MonoBehaviour script in GetComponents<MonoBehaviour>())
            {
                if (!script.Enabled)
                {
                    continue;
                }
                script.Awake();
                script.Start();
            }
            BeginPlay();
        }

        internal void EndPlayInternal()
        {
            if (!_hasBegunPlay)
            {
                return;
            }
            EndPlay();
            _hasBegunPlay = false;
        }

        protected override void OnDestroying()
        {
            EndPlayInternal();
            base.OnDestroying();
        }
    }
}
