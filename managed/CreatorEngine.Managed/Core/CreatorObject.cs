using System;
using System.Threading;

namespace CreatorEngine
{
    public abstract class CreatorObject
    {
        private static long s_NextObjectId;

        protected CreatorObject(string? name = null)
        {
            ObjectId = unchecked((ulong)Interlocked.Increment(ref s_NextObjectId));
            Name = name ?? GetType().Name;
        }

        public ulong ObjectId { get; }
        public string Name { get; set; }
        public bool IsDestroyed { get; private set; }

        public void Destroy()
        {
            if (IsDestroyed)
            {
                return;
            }

            OnDestroying();
            IsDestroyed = true;
            OnDestroyed();
        }

        protected virtual void OnDestroying() { }
        protected virtual void OnDestroyed() { }

        protected void ThrowIfDestroyed()
        {
            if (IsDestroyed)
            {
                throw new ObjectDisposedException(Name);
            }
        }
    }
}
