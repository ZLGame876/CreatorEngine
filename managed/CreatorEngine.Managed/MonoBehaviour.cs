namespace CreatorEngine
{
    public abstract class MonoBehaviour : Component
    {
        public Vector3 Position
        {
            get
            {
                if (NativeHandle == 0)
                {
                    if (IsAttached)
                    {
                        return Transform.Position;
                    }
                    throw new System.InvalidOperationException(
                        $"{GetType().Name} has no managed owner or native handle.");
                }
                InternalCalls.Transform_GetPosition(NativeHandle, out Vector3 value);
                return value;
            }
            set
            {
                if (NativeHandle == 0)
                {
                    if (IsAttached)
                    {
                        Transform.Position = value;
                        return;
                    }
                    throw new System.InvalidOperationException(
                        $"{GetType().Name} has no managed owner or native handle.");
                }
                InternalCalls.Transform_SetPosition(NativeHandle, ref value);
            }
        }

        public virtual void Awake() { }
        public virtual void Start() { }
        public virtual void Update(float deltaTime) { }
        public virtual void OnDestroy() { }
    }
}
