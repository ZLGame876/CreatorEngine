using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public class GameObject : CreatorObject
    {
        private readonly List<Component> _components = new List<Component>();

        public GameObject(string? name = null) : base(name)
        {
            Transform = new Transform();
            AttachComponent(Transform);
        }

        public bool ActiveSelf { get; set; } = true;
        public Transform Transform { get; }
        public IReadOnlyList<Component> Components => _components;

        public T AddComponent<T>() where T : Component, new()
        {
            return (T)AttachComponent(new T());
        }

        public Component AddComponent(Type componentType)
        {
            if (componentType == null)
            {
                throw new ArgumentNullException(nameof(componentType));
            }
            if (!typeof(Component).IsAssignableFrom(componentType) || componentType.IsAbstract)
            {
                throw new ArgumentException("The requested type must be a concrete Component.", nameof(componentType));
            }

            var component = (Component?)Activator.CreateInstance(componentType);
            return AttachComponent(component ??
                throw new InvalidOperationException($"Could not construct {componentType.FullName}."));
        }

        public T? GetComponent<T>() where T : Component
        {
            foreach (Component component in _components)
            {
                if (component is T match)
                {
                    return match;
                }
            }
            return null;
        }

        public IEnumerable<T> GetComponents<T>() where T : Component
        {
            foreach (Component component in _components)
            {
                if (component is T match)
                {
                    yield return match;
                }
            }
        }

        public bool RemoveComponent(Component component)
        {
            if (component == null || ReferenceEquals(component, Transform) || !_components.Remove(component))
            {
                return false;
            }

            component.DetachFrom(this);
            component.Destroy();
            return true;
        }

        internal void OnComponentDestroying(Component component)
        {
            if (ReferenceEquals(component, Transform))
            {
                throw new InvalidOperationException("Transform can only be destroyed with its GameObject.");
            }

            if (_components.Remove(component))
            {
                component.DetachFrom(this);
            }
        }

        private Component AttachComponent(Component component)
        {
            ThrowIfDestroyed();
            if (component is Transform && _components.Count != 0)
            {
                throw new InvalidOperationException("A GameObject can only contain one Transform.");
            }

            component.AttachTo(this);
            _components.Add(component);
            return component;
        }

        protected override void OnDestroying()
        {
            var children = new List<Transform>(Transform.Children);
            foreach (Transform child in children)
            {
                child.GameObject.Destroy();
            }
            Transform.SetParent(null, false);

            for (int i = _components.Count - 1; i >= 0; --i)
            {
                Component component = _components[i];
                component.DetachFrom(this);
                component.Destroy();
            }
            _components.Clear();
        }
    }
}
