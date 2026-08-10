using System;
using CreatorEngine;

namespace CreatorGame
{
    public sealed class VerticalBob : MonoBehaviour
    {
        private Vector3 _origin;
        private float _elapsed;

        public override void Awake()
        {
            _origin = Position;
        }

        public override void Update(float deltaTime)
        {
            _elapsed += deltaTime;
            Position = new Vector3(
                _origin.X,
                _origin.Y + (float)Math.Sin(_elapsed * 2.0f) * 40.0f,
                _origin.Z);
        }
    }
}
