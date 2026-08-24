using System.Runtime.CompilerServices;

namespace CreatorEngine
{
    internal static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Transform_GetPosition(ulong nativeHandle, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Transform_SetPosition(ulong nativeHandle, ref Vector3 value);
    }
}
