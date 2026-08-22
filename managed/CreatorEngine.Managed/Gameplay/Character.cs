namespace CreatorEngine
{
    public class CharacterMovementComponent : Component
    {
        private Vector3 _pendingMovement;

        public float MaxWalkSpeed { get; set; } = 5.0f;

        public void AddMovementInput(Vector3 worldDirection, float scale = 1.0f)
        {
            _pendingMovement += worldDirection * scale;
        }

        public void Tick(float deltaTime)
        {
            if (_pendingMovement.Length > 1.0f)
            {
                _pendingMovement = _pendingMovement.Normalized;
            }

            Transform.Position += _pendingMovement * MaxWalkSpeed * deltaTime;
            _pendingMovement = Vector3.Zero;
        }
    }

    public class Character : Pawn
    {
        public Character(string? name = null) : base(name)
        {
            Movement = AddComponent<CharacterMovementComponent>();
        }

        public CharacterMovementComponent Movement { get; }
        public bool IsJumping { get; private set; }

        public void AddMovementInput(Vector3 direction, float scale = 1.0f) =>
            Movement.AddMovementInput(direction, scale);

        public virtual void Jump() => IsJumping = true;
        public virtual void StopJumping() => IsJumping = false;

        public override void Tick(float deltaTime)
        {
            base.Tick(deltaTime);
            Movement.Tick(deltaTime);
        }
    }
}
