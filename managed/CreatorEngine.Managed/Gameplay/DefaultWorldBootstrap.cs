namespace CreatorEngine
{
    public static class DefaultWorldBootstrap
    {
        public static InputAction MoveAction { get; } =
            new InputAction("Move", InputValueType.Axis2D);

        public static InputAction JumpAction { get; } =
            new InputAction("Jump", InputValueType.Boolean);

        public static World CreatePlayableWorld()
        {
            var world = new World("DefaultWorld");
            var gameMode = world.SpawnActor<DefaultGameMode>("GameMode");
            world.GameMode = gameMode;
            gameMode.StartDefaultPlayer();
            world.BeginPlay();
            return world;
        }

        public static InputMappingContext CreateDefaultMappingContext()
        {
            var context = new InputMappingContext("DefaultGameplay");

            context.Map("Keyboard.W", MoveAction)
                .AddModifier(new InputModifierSwizzle(InputAxisOrder.YXZ));
            context.Map("Keyboard.S", MoveAction)
                .AddModifier(new InputModifierNegate(true, false, false))
                .AddModifier(new InputModifierSwizzle(InputAxisOrder.YXZ));
            context.Map("Keyboard.D", MoveAction);
            context.Map("Keyboard.A", MoveAction)
                .AddModifier(new InputModifierNegate(true, false, false));
            context.Map("Keyboard.Space", JumpAction)
                .AddTrigger(new InputTriggerPressed());
            return context;
        }
    }
}
