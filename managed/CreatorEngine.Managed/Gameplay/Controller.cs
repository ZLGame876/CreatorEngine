using System;

namespace CreatorEngine
{
    public class Controller : Actor
    {
        public Controller(string? name = null) : base(name) { }

        public Pawn? ControlledPawn { get; private set; }

        public virtual void Possess(Pawn pawn)
        {
            if (pawn == null)
            {
                throw new ArgumentNullException(nameof(pawn));
            }
            if (ReferenceEquals(ControlledPawn, pawn))
            {
                return;
            }

            UnPossess();
            if (pawn.Controller != null)
            {
                pawn.Controller.UnPossess();
            }
            ControlledPawn = pawn;
            pawn.PossessedBy(this);
        }

        public virtual void UnPossess()
        {
            Pawn? pawn = ControlledPawn;
            ControlledPawn = null;
            pawn?.UnPossessed();
        }

        public override void EndPlay()
        {
            UnPossess();
            base.EndPlay();
        }
    }

    public class PlayerController : Controller
    {
        public PlayerController(string? name = null) : base(name) { }

        public EnhancedInputSubsystem InputSubsystem { get; } = new EnhancedInputSubsystem();

        public void AddMappingContext(InputMappingContext context, int priority = 0) =>
            InputSubsystem.AddMappingContext(context, priority);

        public void RemoveMappingContext(InputMappingContext context) =>
            InputSubsystem.RemoveMappingContext(context);

        public void ProcessInput(InputFrame frame, float deltaTime)
        {
            Pawn? pawn = ControlledPawn;
            if (pawn != null)
            {
                InputSubsystem.ProcessInput(frame, deltaTime, pawn.InputComponent);
            }
        }
    }
}
