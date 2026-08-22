using System;

namespace CreatorEngine
{
    public class GameMode : Actor
    {
        public GameMode(string? name = null) : base(name) { }

        public Type DefaultPawnClass { get; set; } = typeof(DefaultPawn);
        public Type PlayerControllerClass { get; set; } = typeof(DefaultPlayerController);

        public virtual void StartDefaultPlayer()
        {
            World world = World ?? throw new InvalidOperationException("GameMode must belong to a World.");
            var controller = (PlayerController)world.SpawnActor(PlayerControllerClass, "DefaultPlayerController");
            var pawn = (Pawn)world.SpawnActor(DefaultPawnClass, "DefaultPawn");

            controller.AddMappingContext(DefaultWorldBootstrap.CreateDefaultMappingContext(), 0);
            controller.Possess(pawn);
            world.PrimaryPlayerController = controller;
            world.DefaultPawn = pawn;
        }
    }

    public sealed class DefaultGameMode : GameMode { }
    public sealed class DefaultPlayerController : PlayerController { }

    public sealed class DefaultPawn : Character
    {
        public DefaultPawn()
        {
            Camera = AddComponent<CameraComponent>();
            Camera.IsMainCamera = true;
            Camera.LocalOffset = new Vector3(0.0f, 1.6f, -4.0f);
        }

        public CameraComponent Camera { get; }

        public override void SetupPlayerInputComponent(EnhancedInputComponent input)
        {
            input.BindAction(DefaultWorldBootstrap.MoveAction, InputTriggerEvent.Triggered, OnMove);
            input.BindAction(DefaultWorldBootstrap.JumpAction, InputTriggerEvent.Triggered, _ => Jump());
            input.BindAction(DefaultWorldBootstrap.JumpAction, InputTriggerEvent.Completed, _ => StopJumping());
        }

        private void OnMove(InputActionContext context)
        {
            Vector2 movement = context.Value.AsVector2();
            AddMovementInput(new Vector3(movement.X, 0.0f, movement.Y));
        }
    }
}
