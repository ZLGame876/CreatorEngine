using System;

namespace CreatorEngine
{
    public abstract class Component : CreatorObject
    {
        private GameObject? _gameObject;

        // The native host injects this field by walking the managed inheritance tree.
        internal ulong NativeHandle = 0;

        public bool Enabled { get; set; } = true;
        public bool IsAttached => _gameObject != null;

        public GameObject GameObject => _gameObject ??
            throw new InvalidOperationException($"{GetType().Name} is not attached to a GameObject.");

        public Transform Transform => GameObject.Transform;

        internal void AttachTo(GameObject gameObject)
        {
            if (_gameObject != null)
            {
                throw new InvalidOperationException($"{GetType().Name} is already attached.");
            }

            _gameObject = gameObject ?? throw new ArgumentNullException(nameof(gameObject));
            OnAttached();
        }

        internal void DetachFrom(GameObject gameObject)
        {
            if (!ReferenceEquals(_gameObject, gameObject))
            {
                return;
            }

            OnDetached();
            _gameObject = null;
        }

        protected override void OnDestroying()
        {
            _gameObject?.OnComponentDestroying(this);
        }

        protected virtual void OnAttached() { }
        protected virtual void OnDetached() { }
    }
}
