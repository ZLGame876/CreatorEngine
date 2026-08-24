using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public sealed class Transform : Component
    {
        private readonly List<Transform> _children = new List<Transform>();
        private Transform? _parent;

        public Vector3 LocalPosition { get; set; } = Vector3.Zero;
        public Vector3 LocalEulerAngles { get; set; } = Vector3.Zero;
        public Vector3 LocalScale { get; set; } = Vector3.One;
        public Transform? Parent => _parent;
        public IReadOnlyList<Transform> Children => _children;

        public Vector3 Position
        {
            get => _parent == null ? LocalPosition : _parent.Position + LocalPosition;
            set => LocalPosition = _parent == null ? value : value - _parent.Position;
        }

        public void SetParent(Transform? newParent, bool worldPositionStays = true)
        {
            if (ReferenceEquals(_parent, newParent))
            {
                return;
            }
            if (newParent != null && (ReferenceEquals(newParent, this) || newParent.IsDescendantOf(this)))
            {
                throw new InvalidOperationException("A Transform cannot be parented to itself or its descendant.");
            }

            Vector3 worldPosition = Position;
            _parent?._children.Remove(this);
            _parent = newParent;
            _parent?._children.Add(this);
            if (worldPositionStays)
            {
                Position = worldPosition;
            }
        }

        public bool IsDescendantOf(Transform potentialAncestor)
        {
            for (Transform? current = _parent; current != null; current = current._parent)
            {
                if (ReferenceEquals(current, potentialAncestor))
                {
                    return true;
                }
            }
            return false;
        }
    }
}
