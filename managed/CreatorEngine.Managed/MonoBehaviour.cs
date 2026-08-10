namespace CreatorEngine
{
    public abstract class MonoBehaviour
    {
        internal ulong NativeHandle = 0;

        public Vector3 Position
        {
            get
            {
                InternalCalls.Transform_GetPosition(NativeHandle, out Vector3 value);
                return value;
            }
            set
            {
                InternalCalls.Transform_SetPosition(NativeHandle, ref value);
            }
        }

        public virtual void Awake() { }
        public virtual void Start() { }
        public virtual void Update(float deltaTime) { }
        public virtual void OnDestroy() { }
    }
}
