namespace CreatorEngine
{
    public enum CameraProjection
    {
        Perspective,
        Orthographic
    }

    public sealed class CameraComponent : Component
    {
        public CameraProjection Projection { get; set; } = CameraProjection.Perspective;
        public float FieldOfView { get; set; } = 60.0f;
        public float OrthographicSize { get; set; } = 5.0f;
        public Vector3 LocalOffset { get; set; } = Vector3.Zero;
        public bool IsMainCamera { get; set; }
    }
}
